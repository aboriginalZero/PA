#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  if (id_dest->width == 1) {
		id_dest->val = (int32_t)(int8_t)id_dest->val;
	}
	rtl_push(&id_dest->val);
  print_asm_template1(push);//打印显示在屏幕上的汇编代码
}

make_EHelper(pop) {
  rtl_pop(&t0);//t0即pop出来的那个值
  operand_write(id_dest,&t0);//将该值写入id_dest中
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  TODO();

  print_asm("pusha");
}

make_EHelper(popa) {
  TODO();

  print_asm("popa");
}

make_EHelper(leave) {
  rtl_mv(&cpu.esp,&cpu.ebp);
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtl_lr(&t0,R_AX,2);
    rtl_slti(&t0,&t2,0);
    if(t0){
      rtl_li(&t1,0xffff);
      rtl_sr(R_DX,2,&t1);
    }else{
      rtl_sr(R_DX,2,&tzero);
    }
  }
  else {
    rtl_lr(&t0,R_EAX,4);
    rtl_slti(&t0,&t2,0);
    if(t0){
      rtl_li(&t1,0xffffffff);
      rtl_sr(R_EDX,4,&t1);
    }else{
      rtl_sr(R_EDX,4,&tzero);
    }
  }
  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if(decoding.is_operand_size_16) {
    rtl_lr(&t1,R_AL,1);
    rtl_sext(&t2,&t1,1);
    rtl_sr(R_AX,2,&t2);
  }else{
    rtl_lr(&t1,R_AX,2);
    rtl_sext(&t2,&t1,2);
    rtl_sr(R_EAX,4,&t2);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
