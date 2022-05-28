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
def jmptarget    : Operand<OtherVT> {
  let EncoderMethod = "getJumpTargetOpValue";
  let OperandType = "OPERAND_PCREL";
}
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

/* Type 1 */
let isCall = 1 in
def JMR : Type1Inst<(outs), (ins GPR:$Ra), 0b0001110, 0, "jmr", "$Ra", "">;
def INC : Type1Inst<(outs GPR:$Ra), (ins GPR:$Rai), 0b0001000, 0, "inc", "$Ra", "$Ra = $Rai">;
def DEC : Type1Inst<(outs GPR:$Ra), (ins GPR:$Rai), 0b0001001, 0, "dec", "$Ra", "$Ra = $Rai">;

// Type 1 patterns
def : Pat<(add GPR:$Ra, (i16 1)), (INC GPR:$Ra)>;
def : Pat<(sub GPR:$Ra, (i16 1)), (DEC GPR:$Ra)>;

// Type 1 pseudos
def fero_ret : SDNode<"FeroISD::Ret", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
let isReturn = 1, isBarrier = 1, isTerminator = 1 in
def PseudoRET : Pseudo<(outs), (ins), [(fero_ret)]>, PseudoInstExpansion<(JMR R15)>;
def : InstAlias<"ret", (JMR R15), 4>;

/* Type 2 */
def ADDI : Type2Inst<(outs GPR:$Ra), (ins GPR:$Rai, i16imm:$imm), 0b0010110, 0, "add", "$Ra, $imm", "$Ra = $Rai">;
def LD : Type2Inst<(outs GPR:$Ra), (ins i16imm:$imm), 0b0010001, 0, "ld", "$Ra, $imm", "">;
def JMP : Type2Inst<(outs), (ins jmptarget:$addr),  0b0010010, 0, "jmp", "$addr", ""> {
  let Inst{14-11} = 0b0000;
}

/* Type 3 */
def MOV : Type3Inst<(outs GPR:$Ra), (ins GPR:$Rb), 0b0011101, 0, "mov", "$Ra, $Rb", "">;
def MOV_indirect : Type3Inst<(outs GPR:$Ra), (ins GPR:$Rb), 0b0011101, 1, "mov", "$Ra, $Rb", "">;
def ADD : Type3Inst<(outs GPR:$Ra), (ins GPR:$Rai, GPR:$Rb), 0b0011010, 0, "add", "$Ra, $Rb", "$Ra = $Rai">;
def SUB : Type3Inst<(outs GPR:$Ra), (ins GPR:$Rai, GPR:$Rb), 0b0011101, 0, "sub", "$Ra, $Rb", "$Ra = $Rai">;

def STR : Type3Inst<(outs), (ins MEMr:$Ra, GPR:$Rb), 0b0100001, 0, "str", "$Ra, $Rb", "">;

// Type 3 patterns
def : Pat<(load GPR:$Rb), (MOV_indirect MEMr:$Rb)>;
def : Pat<(add GPR:$Ra, GPR:$Rb), (ADD GPR:$Ra, GPR:$Rb)>;
def : Pat<(sub GPR:$Ra, GPR:$Rb), (SUB GPR:$Ra, GPR:$Rb)>;
def : Pat<(store GPR:$Rb, GPR:$Ra), (STR GPR:$Ra, GPR:$Rb)>;
