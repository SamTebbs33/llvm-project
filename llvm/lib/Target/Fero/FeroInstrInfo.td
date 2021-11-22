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

// Procedure return
def fero_ret : SDNode<"FeroISD::Ret", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// Procedure calling
def SDT_CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>,
                                       SDTCisVT<1, i32>]>;
def SDT_CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>,
                                    SDTCisVT<1, i32>]>;

// These are target-independent nodes, but have target-specific formats.
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_CallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_CallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

include "FeroInstrFormats.td"

let Defs = [R2], Uses = [R2] in {
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP   : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              [(callseq_end timm:$amt1, timm:$amt2)]>;
} // end Defs = [R2], Uses = [R2]

/* Arithmetic and logic */
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class ALU_rr<bits<7> funct7, bits<3> funct3, string opcodestr>
    : RWInstR<funct7, funct3, OPC_OP, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
              opcodestr, "$rd, $rs1, $rs2">;

def ADD  : ALU_rr<0b0000000, 0b000, "add">,
           Sched<[WriteIALU, ReadIALU, ReadIALU]>;

class PatGprGpr<SDPatternOperator OpNode, RWInst Inst>
    : Pat<(OpNode GPR:$rs1, GPR:$rs2), (Inst GPR:$rs1, GPR:$rs2)>;

def : PatGprGpr<add, ADD>;

/* Procedure calling instructions */
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
let isCall = 1 in
def JALR : RWInstI<0b000, OPC_JALR, (outs GPR:$rd),
                   (ins GPR:$rs1, i32imm:$imm12),
                   "jalr", "$rd, ${imm12}(${rs1})">,
           Sched<[WriteJalr, ReadJalr]>;
}

let isReturn = 1, isBarrier = 1, isTerminator = 1 in
def PseudoRET : Pseudo<(outs), (ins), [(fero_ret)]>,
                PseudoInstExpansion<(JALR R0, R1, 0)>;

def : InstAlias<"ret", (JALR      R0,      R1, 0), 4>;
