//== FeroInstrInfo.td - Target Description for Fero Target -*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Fero implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

include "FeroInstrFormats.td"

// Procedure calling
def SDT_CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i16>,
                                       SDTCisVT<1, i16>]>;
def SDT_CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i16>,
                                    SDTCisVT<1, i16>]>;

// These are target-independent nodes, but have target-specific formats.
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_CallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_CallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

let Defs = [R14], Uses = [R14] in {
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i16imm:$amt1, i16imm:$amt2),
                              [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP   : Pseudo<(outs), (ins i16imm:$amt1, i16imm:$amt2),
                              [(callseq_end timm:$amt1, timm:$amt2)]>;
} // end Defs = [R2], Uses = [R2]

/* Custom operands */
def MemRegAsmOperand : AsmOperandClass {
  let Name = "MemReg";
  let ParserMethod  = "parseMemoryOperand";
}
def MEMr : Operand<i16> {
  let DecoderMethod = "decodeRMemoryValue";
  let EncoderMethod = "getRMemoryOpValue";
  let MIOperandInfo = (ops GPR:$Op1);
  let ParserMatchClass = MemRegAsmOperand;
  let PrintMethod   = "printMemROperand";
}

def MemImmAsmOperand : AsmOperandClass {
  let Name = "MemImm";
  let ParserMethod  = "parseMemoryOperand";
}
def MEMi16imm : Operand<i16> {
  let DecoderMethod = "decodeIMemoryValue";
  let EncoderMethod = "getIMemoryOpValue";
  let MIOperandInfo = (ops i16imm:$Op1);
  let ParserMatchClass = MemImmAsmOperand;
  let PrintMethod   = "printMemIOperand";
}

def JmpTarget    : Operand<OtherVT> {
  let EncoderMethod = "getJumpTargetOpValue";
  let OperandType = "OPERAND_PCREL";
}

def SDTPtrFlags    : SDTypeProfile<0, 2, [SDTCisPtrTy<0>, SDTCisVT<1, i16>]>;
def SDTRegReg : SDTypeProfile<1, 2, [SDTCisVT<0, i16>, SDTCisSameAs<0, 1>, SDTCisSameAs<0, 2>]>;

/* Type 1 */
class Type1Jmp<bits<7> opcode, string asm, bits<4> condition, bit indirect> : FeroInst<(outs), (ins GPR:$Ra), opcode, condition, indirect, asm, "$Ra", "", 1> {
  bits<4> Ra;
  bits<16> imm;

  let Inst{14-11} = Ra;
  let Inst{31-16} = imm;
  let Size = 4;
}
let isCall = 1 in
def JMR : Type1Jmp<0b0001110, "jmr", 0, 0>;

// Type 1 pseudos
def fero_ret : SDNode<"FeroISD::Ret", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
let isReturn = 1, isBarrier = 1, isTerminator = 1 in
def PseudoRET : Pseudo<(outs), (ins), [(fero_ret)]>, PseudoInstExpansion<(JMR R15)>;
def : InstAlias<"ret", (JMR R15), 4>;

/* Type 2 */
class Type2Arithmetic<bits<7> opcode, string asm, bits<4> condition, bit indirect> : FeroInst<(outs GPR:$Ra), (ins GPR:$Rai, i16imm:$imm), opcode, condition, indirect, asm, "$Ra, $imm", "$Ra = $Rai", 2> {
  bits<4> Ra;
  bits<16> imm;

  let Inst{14-11} = Ra;
  let Inst{31-16} = imm;
  let Size = 4;
}

class Type2Load<bits<7> opcode, string asm, bits<4> condition, bit indirect> : FeroInst<(outs GPR:$Ra), (ins i16imm:$imm), opcode, condition, indirect, asm, "$Ra, $imm", "", 2> {
  bits<4> Ra;
  bits<16> imm;

  let Inst{14-11} = Ra;
  let Inst{31-16} = imm;
  let Size = 4;
}

class Type2Jmp<bits<7> opcode, string asm, bits<4> condition, bit indirect> : FeroInst<(outs), (ins JmpTarget:$imm, GPRFlags:$flags), opcode, condition, indirect, asm, "$imm", "", 2> {
  bits<16> imm;

  let Inst{14-11} = 0b0000;
  let Inst{31-16} = imm;
  let Size = 4;
}

def ADDI : Type2Arithmetic<0b0010110, "add", 0, 0>;
def LD : Type2Load<0b0010001, "ld", 0, 0>;
def JMP : Type2Jmp<0b0010010, "jmp", 0, 0>;
def JMP_notZ : Type2Jmp<0b0010010, "jmp", 4, 0>;

// Type 2 pseudos
def Fero_Jmp_notZ : SDNode<"FeroISD::Jmp_notZ", SDTPtrFlags, [SDNPHasChain]>;
def PseudoJMP_notZ : Pseudo<(outs), (ins JmpTarget:$target, GPRFlags:$flags), [(Fero_Jmp_notZ bb:$target, GPRFlags:$flags)]>;

/* Type 3 */
class Type3Load<bits<7> opcode, string asm, bits<4> condition, bit indirect> : FeroInst<(outs GPR:$Ra), (ins GPR:$Rb), opcode, condition, indirect, asm, "$Ra, $Rb", "", 3> {
  bits<4> Ra;
  bits<4> Rb;

  let Inst{19-16} = Ra;
  let Inst{23-20} = Rb;
  let Size = 3;
}

class Type3Arithmetic<bits<7> opcode, string asm, bits<4> condition, bit indirect> : FeroInst<(outs GPR:$Ra), (ins GPR:$Rai, GPR:$Rb), opcode, condition, indirect, asm, "$Ra, $Rb", "$Ra = $Rai", 3> {
  bits<4> Ra;
  bits<4> Rb;

  let Inst{19-16} = Ra;
  let Inst{23-20} = Rb;
  let Size = 3;
}

class Type3Cmp<bits<7> opcode, string asm, bits<4> condition, bit indirect> : FeroInst<(outs GPRFlags:$flags), (ins GPR:$Ra, GPR:$Rb), opcode, condition, indirect, asm, "$Ra, $Rb", "", 3> {
  bits<4> Ra;
  bits<4> Rb;

  let Inst{19-16} = Ra;
  let Inst{23-20} = Rb;
  let Size = 3;
}

class Type3Store<bits<7> opcode, string asm, bits<4> condition> : FeroInst<(outs), (ins GPR:$Ra, GPR:$Rb), opcode, condition, 0, asm, "$Ra, $Rb", "", 3> {
  bits<4> Ra;
  bits<4> Rb;

  let Inst{19-16} = Ra;
  let Inst{23-20} = Rb;
  let Size = 3;
}
def MOV : Type3Load<0b0011101, "mov", 0, 0>;
def MOV_indirect : Type3Load<0b0011101, "mov", 0, 1>;
def ADD : Type3Arithmetic<0b0011010, "add", 0, 0>;
def SUB : Type3Arithmetic<0b0011101, "sub", 0, 0>;
def STR : Type3Store<0b0100001, "str", 0>;
def CMP : Type3Cmp<0b0011110, "cmp", 0, 0>;

// Type 3 pseudos
def Fero_Cmp : SDNode<"FeroISD::Cmp", SDTRegReg, []>;

// Type 2 patterns
def : Pat<(br bb:$target), (JMP bb:$target, NOREG)>;
def : Pat<(Fero_Jmp_notZ bb:$target, GPR:$flags), (JMP_notZ bb:$target, GPR:$flags)>;

// Type 3 patterns
def : Pat<(load GPR:$Rb), (MOV_indirect MEMr:$Rb)>;
def : Pat<(add GPR:$Ra, GPR:$Rb), (ADD GPR:$Ra, GPR:$Rb)>;
def : Pat<(sub GPR:$Ra, GPR:$Rb), (SUB GPR:$Ra, GPR:$Rb)>;
def : Pat<(store GPR:$Rb, GPR:$Ra), (STR GPR:$Ra, GPR:$Rb)>;
def : Pat<(Fero_Jmp_notZ bb:$target, (Fero_Cmp GPR:$Ra, GPR:$Rb)), (JMP_notZ JmpTarget:$target, (CMP GPR:$Ra, GPR:$Rb))>;
