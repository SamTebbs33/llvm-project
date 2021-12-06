//=== FeroInstrFormats.td - Fero Instruction Formats -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//
//  These instruction format definitions are structured to match the
//  description in the RISC-V User-Level ISA specification as closely as
//  possible. For instance, the specification describes instructions with the
//  MSB (31st bit) on the left and the LSB (0th bit) on the right. This is
//  reflected in the order of parameters to each instruction class.
//
//  One area of divergence is in the description of immediates. The
//  specification describes immediate encoding in terms of bit-slicing
//  operations on the logical value represented. The immediate argument to
//  these instruction formats instead represents the bit sequence that will be
//  inserted into the instruction. e.g. although JAL's immediate is logically
//  a 21-bit value (where the LSB is always zero), we describe it as an imm20
//  to match how it is encoded.
//
//===----------------------------------------------------------------------===//

// Format specifies the encoding used by the instruction. This is used by
// FeroMCCodeEmitter to determine which form of fixup to use. These
// definitions must be kept in-sync with FeroBaseInfo.h.
class InstFormat<bits<5> val> {
  bits<5> Value = val;
}
def InstFormatPseudo : InstFormat<0>;
def InstFormat0     : InstFormat<1>;
def InstFormat1     : InstFormat<2>;
def InstFormat2     : InstFormat<3>;
def InstFormat3     : InstFormat<4>;

class FeroOpcode<bits<7> val> {
  bits<7> Value = val;
}
def OPC_MOV      : FeroOpcode<0b0011101>;

class FeroInst<dag outs, dag ins, bits<7> opcode, string opcodestr, string argstr, string cstr,
            list<dag> pattern=[], InstFormat format> : Instruction {
  field bits<32> Inst;
  let Inst{6-0} = opcode;

  // SoftFail is a field the disassembler can use to provide a way for
  // instructions to not match without killing the whole decode process. It is
  // mainly used for ARM, but Tablegen expects this field to exist or it fails
  // to build the decode table.
  field bits<32> SoftFail = 0;

  let Namespace = "Fero";
  let Constraints = cstr;
  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = opcodestr # "\t" # argstr;
  let Pattern = pattern;
  let TSFlags{4-0} = format.Value;
}

class Type0Inst<dag outs, dag ins, bits<7> opcode, string opcodestr, string argstr, string cstr,
             list<dag> pattern=[]>
    : FeroInst<outs, ins, opcode, opcodestr, argstr, cstr, pattern, InstFormat0> {

  let Inst{15-7} = 0;
  let Size = 2;
}

class Type1Inst<dag outs, dag ins, bits<7> opcode, bits<4> condition, bit indirect, string opcodestr, string argstr, string cstr,
             list<dag> pattern=[]>
    : FeroInst<outs, ins, opcode, opcodestr, argstr, cstr, pattern, InstFormat1> {

  bits<4> Ra;

  let Inst{10-7} = condition;
  let Inst{14-11} = Ra;
  let Inst{15} = indirect;

  let Size = 2;
}

class Type2Inst<dag outs, dag ins, bits<7> opcode, bits<4> condition, bit indirect, string opcodestr, string argstr, string cstr,
             list<dag> pattern=[]>
    : FeroInst<outs, ins, opcode, opcodestr, argstr, cstr, pattern, InstFormat2> {

  bits<4> Ra;
  bits<16> Immediate;

  let Inst{10-7} = condition;
  let Inst{14-11} = Ra;
  let Inst{15} = indirect;
  let Inst{31-16} = Immediate;

  let Size = 4;
}

class Type3Inst<dag outs, dag ins, bits<7> opcode, bits<4> condition, bit indirect, string opcodestr, string argstr, string cstr,
             list<dag> pattern=[]>
    : FeroInst<outs, ins, opcode, opcodestr, argstr, cstr, pattern, InstFormat3> {

  bits<4> Offset;
  bits<4> Ra;
  bits<4> Rb;

  let Inst{10-7} = condition;
  let Inst{14-11} = Offset;
  let Inst{15} = indirect;
  let Inst{19-16} = Ra;
  let Inst{23-20} = Rb;

  let Size = 3;
}

// Pseudo instructions
class Pseudo<dag outs, dag ins, list<dag> pattern=[], string opcodestr = "",
             string argstr = "", string cstr = "">
    : FeroInst<outs, ins, 0, opcodestr, argstr, cstr, pattern, InstFormatPseudo>,
      Sched<[]> {
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}
