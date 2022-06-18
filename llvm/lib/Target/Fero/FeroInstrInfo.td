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

class SDTCisChain<int OpNum> : SDTCisVT<OpNum, OtherVT>;
class SDTCisI16  <int OpNum> : SDTCisVT<OpNum, i16>;
class SDTCisFlag <int OpNum> : SDTCisI16<OpNum>;
def SDT_JmpCC       : SDTypeProfile<0, 4, [SDTCisChain<0>,
                                               SDTCisI16<1>,
                                               SDTCisI16<2>,
                                               SDTCisFlag<3>]>;

/* Custom operands */
def jmptarget : Operand<OtherVT>;
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

def MemRegOffsetAsmOperand : AsmOperandClass {
  let Name = "MemRegOffset";
  let ParserMethod  = "parseMemoryOperand";
}
def MEMroffset : Operand<i16> {
  let DecoderMethod = "decodeROffsetMemoryValue";
  let EncoderMethod = "getROffsetMemoryOpValue";
  let MIOperandInfo = (ops GPR:$base, i16imm:$offset);
  let ParserMatchClass = MemRegOffsetAsmOperand;
  let PrintMethod   = "printMemROffsetOperand";
}

/* Type 1 */
let isCall = 1 in
def JMR : Type1Inst<(outs), (ins GPR:$Ra), 0b0001110, 0, "jmr", "$Ra", "">;
def INC : Type1Inst<(outs GPR:$Ra), (ins GPR:$Rai), 0b0001000, 0, "inc", "$Ra", "$Ra = $Rai">;
def DEC : Type1Inst<(outs GPR:$Ra), (ins GPR:$Rai), 0b0001001, 0, "dec", "$Ra", "$Ra = $Rai">;

def fero_ret : SDNode<"FeroISD::Ret", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
let isReturn = 1, isBarrier = 1, isTerminator = 1 in
def PseudoRET : Pseudo<(outs), (ins), [(fero_ret)]>, PseudoInstExpansion<(JMR R15)>;
def : InstAlias<"ret", (JMR R15), 4>;

/* Type 2 */
def ADDI : Type2Inst<(outs GPR:$Ra), (ins GPR:$Rai, i16imm:$imm), 0b0010110, 0, "addi", "$Ra, $imm", "$Ra = $Rai">;
def LD : Type2Inst<(outs GPR:$Ra), (ins i16imm:$imm), 0b0010001, 0, "ld", "$Ra, $imm", "">;
def CMPI : Type2Inst<(outs FR:$f), (ins GPR:$Ra, i16imm:$imm), 0b0010011, 0, "cmpi", "$Ra, $imm", "">;
def SET : Type2Inst<(outs GPR:$Ra), (ins FR:$f, i16imm:$imm), 0b0011000, 0, "set", "$Ra, $imm", "">;

class JMP_c<bits<4> condition, string suffix, dag iops> : Type2Inst<(outs), !con((ins jmptarget:$addr), iops),  0b0010010, 0, "jmp" # suffix, "$addr", ""> {
  let Inst{14-11} = 0b0000;
  let Inst{10-7} = condition;
}
multiclass JMP_m {
  def "" : JMP_c<0, "", (ins)>;
  def _Z : JMP_c<1, ".Z", (ins FR:$f)>;
  def _notZ : JMP_c<2, ".notZ", (ins FR:$f)>;
  def _N : JMP_c<3, ".N", (ins FR:$f)>;
  def _notN : JMP_c<4, ".notN", (ins FR:$f)>;
  def _C : JMP_c<5, ".C", (ins FR:$f)>;
  def _notC : JMP_c<6, ".notC", (ins FR:$f)>;
  def _NorZ : JMP_c<7, ".NZ", (ins FR:$f)>;
  def _notNnotZ : JMP_c<8, ".notNnotZ", (ins FR:$f)>;
}
defm JMP : JMP_m;
def FeroJmpCC : SDNode<"FeroISD::JmpCC", SDT_JmpCC, [SDNPHasChain]>;

/* Type 3 */
class MOV_c<dag iops, bits<4> cond, string suffix> : Type3Inst<(outs GPR:$Ra), !con((ins GPR:$Rb), iops), 0b0011101, 0, "mov" # suffix, "$Ra, $Rb", ""> {
  let Inst{10-7} = cond;
}
multiclass MOV_m {
  def "" : MOV_c<(ins), 0, "">;
  def _Z : MOV_c<(ins FR:$f), 3, ".Z">;
  def _notZ : MOV_c<(ins FR:$f), 4, ".notZ">;
}
defm MOV : MOV_m;
def MOV_indirect : Type3Inst<(outs GPR:$Ra), (ins MEMroffset:$Rb), 0b0011101, 1, "mov", "$Ra, $Rb", "">;
def ADD : Type3Inst<(outs GPR:$Ra), (ins GPR:$Rai, GPR:$Rb), 0b0011010, 0, "add", "$Ra, $Rb", "$Ra = $Rai">;
def SUB : Type3Inst<(outs GPR:$Ra), (ins GPR:$Rai, GPR:$Rb), 0b0011101, 0, "sub", "$Ra, $Rb", "$Ra = $Rai">;
def CMP : Type2Inst<(outs FR:$f), (ins GPR:$Ra, GPR:$Rb), 0b0000010, 0, "cmp", "$Ra, $Rb", "">;
def OR : Type3Inst<(outs GPR:$Ra), (ins GPR:$Rai, GPR:$Rb), 0b0100000, 0, "or", "$Ra, $Rb", "$Ra = $Rai">;
def STR : Type3Inst<(outs), (ins MEMroffset:$Ra, GPR:$Rb), 0b0100001, 0, "str", "$Ra, $Rb", "">;

def PseudoSelectCC : Pseudo<(outs GPR:$out, FR:$f), (ins GPR:$x, GPR:$y, GPR:$Ra, GPR:$Rb, i16imm:$flag)>;

// Type 1 patterns
def : Pat<(add GPR:$Ra, (i16 1)), (INC GPR:$Ra)>;
def : Pat<(sub GPR:$Ra, (i16 1)), (DEC GPR:$Ra)>;

// Type 2 patterns
def : Pat<(br bb:$addr), (JMP jmptarget:$addr)>;
def : Pat<(brcond (i16 (setne GPR:$Ra, GPR:$Rb)), bb:$addr),
          (JMP_notZ jmptarget:$addr, (CMP GPR:$Ra, GPR:$Rb))>;
def : Pat<(brcond (i16 (setne GPR:$Ra, GPR:$Rb)), bb:$addr),
          (JMP_notZ jmptarget:$addr, (CMP GPR:$Ra, GPR:$Rb))>;
def : Pat<(setcc GPR:$Ra, i16:$imm, SETEQ), (SET (CMP GPR:$Ra, (LD i16:$imm)), (i16 1))>;
def : Pat<(setcc GPR:$Ra, i16:$imm, SETNE), (SET (CMP GPR:$Ra, (LD i16:$imm)), (i16 2))>;
def : Pat<(setcc GPR:$Ra, GPR:$Rb, SETEQ), (SET (CMP GPR:$Ra, GPR:$Rb), (i16 1))>;
def : Pat<(setcc GPR:$Ra, GPR:$Rb, SETNE), (SET (CMP GPR:$Ra, GPR:$Rb), (i16 2))>;
def : Pat<(selectcc GPR:$x, GPR:$y, GPR:$Ra, GPR:$Rb, SETEQ), (PseudoSelectCC GPR:$x, GPR:$y, GPR:$Ra, GPR:$Rb, (i16 1))>;
def : Pat<(select (setcc GPR:$x, GPR:$y, SETEQ), GPR:$Ra, GPR:$Rb), (PseudoSelectCC GPR:$x, GPR:$y, GPR:$Ra, GPR:$Rb, (i16 1))>;

// Type 3 patterns
def : Pat<(load GPR:$Rb), (MOV_indirect GPR:$Rb, (i16 0))>;
def : Pat<(add GPR:$Ra, GPR:$Rb), (ADD GPR:$Ra, GPR:$Rb)>;
def : Pat<(sub GPR:$Ra, GPR:$Rb), (SUB GPR:$Ra, GPR:$Rb)>;
def : Pat<(store GPR:$Rb, GPR:$Ra), (STR GPR:$Ra, (i16 0), GPR:$Rb)>;
def : Pat<(or GPR:$Ra, GPR:$Rb), (OR GPR:$Ra, GPR:$Rb)>;
