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

// Type 1 instructions that can be conditionally executed
// TODO: Add other condition codes
multiclass Type1InstConditional<dag outs, dag ins, bits<7> opcode, bit indirect, string opcodestr, string argstr, string cstr> {
  def "" : Type1Inst<outs, ins, opcode, 0, indirect, opcodestr, argstr, cstr>;
}

// Type 1 instructions that can be indirect
// TODO: Add indirect multiclass option
multiclass Type1InstIndirectable<dag outs, dag ins, bits<7> opcode, string opcodestr, string argstr, string cstr> {
  defm "" : Type1InstConditional<outs, ins, opcode, 0, opcodestr, argstr, cstr>;
}

// Type 2 instructions that can be conditionally executed
// TODO: Add other condition codes
multiclass Type2InstConditional<dag outs, dag ins, bits<7> opcode, bit indirect, string opcodestr, string argstr, string cstr> {
  def "" : Type2Inst<outs, ins, opcode, 0, indirect, opcodestr, argstr, cstr>;
}

// Type 2 instructions that can be indirect
// TODO: Add indirect multiclass option
multiclass Type2InstIndirectable<dag outs, dag ins, bits<7> opcode, string opcodestr, string argstr, string cstr> {
  defm "" : Type2InstConditional<outs, ins, opcode, 0, opcodestr, argstr, cstr>;
}

// Type 3 instructions that can be conditionally executed
// TODO: Add other condition codes
multiclass Type3InstConditional<dag outs, dag ins, bits<7> opcode, bit indirect, string opcodestr, string argstr, string cstr> {
  def "" : Type3Inst<outs, ins, opcode, 0, indirect, opcodestr, argstr, cstr>;
}

/* Type 1 */
let isCall = 1 in
defm JMR : Type1InstIndirectable<(outs), (ins GPR:$Ra), 0b0001110, "jmr", "$Ra", "">;

// Type 1 pseudos
def fero_ret : SDNode<"FeroISD::Ret", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
let isReturn = 1, isBarrier = 1, isTerminator = 1 in
def PseudoRET : Pseudo<(outs), (ins), [(fero_ret)]>, PseudoInstExpansion<(JMR R15)>;
def : InstAlias<"ret", (JMR R15), 4>;

/* Type 2 */
defm ADDI : Type2InstIndirectable<(outs GPR:$Ra), (ins GPR:$Rai, i16imm:$imm), 0b0010110, "add", "$Ra, $imm", "$Ra = $Rai">;
defm LD : Type2InstIndirectable<(outs GPR:$Ra), (ins i16imm:$imm), 0b0010001, "ld", "$Ra, $imm", "">;

/* Type 3 */
def MOV : Type3Inst<(outs GPR:$Ra), (ins GPR:$Rb), 0b0011101, 0, 0, "mov", "$Ra, $Rb", "">;
def MOV_i : Type3Inst<(outs GPR:$Ra), (ins MEMr:$Rb), 0b0011101, 0, 1, "mov", "$Ra, $Rb", "">;

def ADD : Type3Inst<(outs GPR:$Ra), (ins GPR:$Rai, GPR:$Rb), 0b0011010, 0, 0, "add", "$Ra, $Rb", "$Ra = $Rai">;
def ADD_i : Type3Inst<(outs GPR:$Ra), (ins GPR:$Rai, MEMr:$Rb), 0b0011010, 0, 1, "add", "$Ra, $Rb", "$Ra = $Rai">;

def SUB : Type3Inst<(outs GPR:$Ra), (ins GPR:$Rai, GPR:$Rb), 0b0011101, 0, 0, "sub", "$Ra, $Rb", "$Ra = $Rai">;
def SUB_i : Type3Inst<(outs GPR:$Ra), (ins GPR:$Rai, MEMr:$Rb), 0b0011010, 0, 1, "sub", "$Ra, $Rb", "$Ra = $Rai">;

def STR : Type3Inst<(outs), (ins MEMr:$Ra, GPR:$Rb), 0b0100001, 0, 0, "str", "$Ra, $Rb", "">;

// Type 3 patterns
def : Pat<(load GPR:$Rb), (MOV_i MEMr:$Rb)>;
def : Pat<(add GPR:$Ra, GPR:$Rb), (ADD GPR:$Ra, GPR:$Rb)>;
def : Pat<(sub GPR:$Ra, GPR:$Rb), (SUB GPR:$Ra, GPR:$Rb)>;
def : Pat<(store GPR:$Rb, GPR:$Ra), (STR GPR:$Ra, GPR:$Rb)>;
