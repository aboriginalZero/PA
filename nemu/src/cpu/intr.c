#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  rtl_push(&cpu.value);
  rtl_push(&cpu.CS);
  rtl_push(&ret_addr);

  uint32_t base=cpu.IDTR.Base;
  uint16_t da[3];

 Log("base 0x%x limit 0x%x",cpu.IDTR.Base,cpu.IDTR.Limit);
 for(int i=0;i<3;i++)
 {
         int j=i;
         if(i>1)
                 j++;
         da[i]=vaddr_read(base+NO*8+j*2,2);
 }
 uint32_t addr;
 addr=da[2];
 Log("data0 0x%x data2 0x%x",da[0],da[2]);
 rtl_shli(&addr,&addr,16);
 rtl_addi(&addr,&addr,da[0]);
 decoding.is_jmp=1;
 rtl_mv(&decoding.jmp_eip,&addr);
 cpu.CS=da[1];
  // uint32_t temp1,temp2,jumpTarget;
  // temp1 = vaddr_read(cpu.IDTR.Base + 8 * NO, 4);
  // temp2 = vaddr_read(cpu.IDTR.Base + 8 * NO + 4, 4);
  // jumpTarget = ((temp1 & 0x0000FFFF) | (temp2 & 0xFFFF0000));
  
  // decoding.is_jmp = 1;
  // decoding.jmp_eip = jumpTarget;
}

void dev_raise_intr() {
}
