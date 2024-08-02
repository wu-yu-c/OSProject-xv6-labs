// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmems[NCPU];

void
kinit()
{
  for(int i=0;i<NCPU;i++)
    initlock(&kmems[i].lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;
  
  //获取当前运行cpu的编号
  push_off();
  int current_cpu=cpuid();
  pop_off();
  //回收物理页到空闲列表
  acquire(&kmems[current_cpu].lock);
  r->next = kmems[current_cpu].freelist;
  kmems[current_cpu].freelist = r;
  release(&kmems[current_cpu].lock);
}

struct run *steal(int now_cpu)
{
  int temp = now_cpu;
  struct run *fast,*slow,*head;
  if(now_cpu!=cpuid())
    panic("运行的cpu已被切换");
  for(int i=1;i<NCPU;i++){
    temp = (temp+1)%NCPU;
    acquire(&kmems[temp].lock);
    if(kmems[temp].freelist){
      slow=head=kmems[temp].freelist;
      fast=slow->next;
      //使用快慢指针将链表分为两部分
      while(fast){
        fast=fast->next;
        if(fast){
          slow=slow->next;
          fast=fast->next;
        }
      }
      kmems[temp].freelist=slow->next;
      release(&kmems[temp].lock);
      slow->next=0;
      return head;
    }
    release(&kmems[temp].lock);
  }
  //其他空闲列表页都为空时，返回空指针
  return 0;
}
// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  //获取cpu编号
  push_off();
  int current_cpu=cpuid();
  pop_off();
  
  acquire(&kmems[current_cpu].lock);
  r = kmems[current_cpu].freelist;
  if(r)
    kmems[current_cpu].freelist = r->next;
  release(&kmems[current_cpu].lock);
  //若当前cpu有空闲物理页，则分配
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  //否则，从其他cpu偷取物理页并修改该cpu空闲列表，再分配
  else{
    r=steal(current_cpu);
    if(r){
      acquire(&kmems[current_cpu].lock);
      kmems[current_cpu].freelist = r->next;
      release(&kmems[current_cpu].lock);
      memset((char*)r, 5, PGSIZE);
    }
  }
  return (void*)r;
}
