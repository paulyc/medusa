#ifndef ARCH_AVR8_INSTRUCTION_HPP
#define ARCH_AVR8_INSTRUCTION_HPP

enum EAvr8InstructionType
{
  AVR8_Invalid_Insn,
  AVR8_Adc,
  AVR8_Add,
  AVR8_Adiw,
  AVR8_And,
  AVR8_Andi,
  AVR8_Asr,
  AVR8_Bclr,
  AVR8_Bld,
  AVR8_Brbc,
  AVR8_Brbs,
  AVR8_Brcc,
  AVR8_Brcs,
  AVR8_Break,
  AVR8_Breq,
  AVR8_Brge,
  AVR8_Brhc,
  AVR8_Brhs,
  AVR8_Brid,
  AVR8_Brie,
  AVR8_Brlo,
  AVR8_Brlt,
  AVR8_Brmi,
  AVR8_Brne,
  AVR8_Brpl,
  AVR8_Brsh,
  AVR8_Brtc,
  AVR8_Brts,
  AVR8_Brvc,
  AVR8_Brvs,
  AVR8_Bset,
  AVR8_Bst,
  AVR8_Call,
  AVR8_Cbi,
  AVR8_Cbr,
  AVR8_Clc,
  AVR8_Clh,
  AVR8_Cli,
  AVR8_Cln,
  AVR8_Clr,
  AVR8_Cls,
  AVR8_Clt,
  AVR8_Clv,
  AVR8_Clz,
  AVR8_Com,
  AVR8_Cp,
  AVR8_Cpc,
  AVR8_Cpi,
  AVR8_Cpse,
  AVR8_Dec,
  AVR8_Des,
  AVR8_Eicall,
  AVR8_Eijmp,
  AVR8_Elpm,
  AVR8_Eor,
  AVR8_Fmul,
  AVR8_Fmuls,
  AVR8_Fmulsu,
  AVR8_Icall,
  AVR8_Ijmp,
  AVR8_In,
  AVR8_Inc,
  AVR8_Jmp,
  AVR8_Ldd,
  AVR8_Ldi,
  AVR8_Lds,
  AVR8_Ld,
  AVR8_Lpm,
  AVR8_Lsl,
  AVR8_Lsr,
  AVR8_Mov,
  AVR8_Movw,
  AVR8_Muls,
  AVR8_Mulsu,
  AVR8_Mulu,
  AVR8_Neg,
  AVR8_Nop,
  AVR8_Or,
  AVR8_Ori,
  AVR8_Out,
  AVR8_Pop,
  AVR8_Push,
  AVR8_Rcall,
  AVR8_Ret,
  AVR8_Reti,
  AVR8_Rjmp,
  AVR8_Rol,
  AVR8_Ror,
  AVR8_Sbc,
  AVR8_Sbci,
  AVR8_Sbi,
  AVR8_Sbic,
  AVR8_Sbis,
  AVR8_Sbiw,
  AVR8_Sbr,
  AVR8_Sbrc,
  AVR8_Sbrs,
  AVR8_Sec,
  AVR8_Seh,
  AVR8_Sei,
  AVR8_Sen,
  AVR8_Ser,
  AVR8_Ses,
  AVR8_Set,
  AVR8_Sev,
  AVR8_Sez,
  AVR8_Sleep,
  AVR8_Spm,
  AVR8_Std,
  AVR8_Sts,
  AVR8_St,
  AVR8_Sub,
  AVR8_Subi,
  AVR8_Swap,
  AVR8_Tst,
  AVR8_Wdr
};

enum EAvr8ConditionType
{
  AVR8_Cond_None,
  AVR8_Cond_Lo,
  AVR8_Cond_Eq,
  AVR8_Cond_Mi,
  AVR8_Cond_Vs,
  AVR8_Cond_Lt,
  AVR8_Cond_Hs,
  AVR8_Cond_Ts,
  AVR8_Cond_Ie,
  AVR8_Cond_Sh,
  AVR8_Cond_Ne,
  AVR8_Cond_Pl,
  AVR8_Cond_Vc,
  AVR8_Cond_Ge,
  AVR8_Cond_Hc,
  AVR8_Cond_Tc,
  AVR8_Cond_Id
};

#endif // !ARCH_AVR8_INSTRUCTION_HPP