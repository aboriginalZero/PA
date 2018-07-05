#include <x86.h>
#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;
  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }
  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }
  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
  PDE *pde, *pgdir = p->ptr;//页目录项基地址
  PTE *pgtab;               //页表项基地址
  pde = &pgdir[PDX(va)];//由10位页目录项索引找到对应的页目录项
  if (*pde & PTE_P) { //该页目录项不为空且该页在内存中
    pgtab = (PTE *)PTE_ADDR(*pde);//根据页目录项中20位基地址指出的页表首地址找到对应页表的基地址
  } else {
    pgtab = (PTE *)palloc_f();//申请一张页表
    for (int i = 0; i < NR_PTE; i ++) {//初始化每个页表项
      pgtab[i] = 0;
    }
    *pde = PTE_ADDR(pgtab) | PTE_P;//设置好新的页目录项（修改present位就行）
  }
  //将传入的物理地址组合present位，形成一个页表项，存入选中的页表项，pa的地址的前20位加上0x001
  pgtab[PTX(va)] = PTE_ADDR(pa) | PTE_P;  
}


void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  _RegSet **tf=ustack.start;
  uint32_t *tempStack=(uint32_t *)(ustack.end - 4);
  for(int i=0;i<3;i++){
    (*tempStack)=0;
    (*tempStack)--;
  }
  (*tf)=(void *)(tempStack-sizeof(_RegSet));
  (*tf)->eflags=0x2|(1<<9);
  (*tf)->cs=8;
  (*tf)->eip=(uintptr_t)entry;
  return *tf;
}
