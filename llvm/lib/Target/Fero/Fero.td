//=== Fero.td - Describe the Fero Target Machine ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Top tablegen file that pulls in code from the LLVM framework along with
// code specific to this target.
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Calling Conv, Instruction Descriptions, Register information, etc
//===----------------------------------------------------------------------===//

include "FeroSchedule.td"
include "FeroRegisterInfo.td"
include "FeroCallingConv.td"
include "FeroInstrInfo.td"

def FeroInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Fero processors supported
//===----------------------------------------------------------------------===//

def : ProcessorModel<"generic",  NoSchedModel, []>;

//===----------------------------------------------------------------------===//
// Fero Target declaration
//===----------------------------------------------------------------------===//

def FeroAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  int PassSubtarget = 1;
}

def FeroAsmParser : AsmParser {
}

def FeroAsmParserVariant : AsmParserVariant {
  string Name = "Fero";
}

def Fero : Target {
  let InstructionSet = FeroInstrInfo;
  let AssemblyWriters = [FeroAsmWriter];
  let AssemblyParsers = [FeroAsmParser];
  let AssemblyParserVariants = [FeroAsmParserVariant];
}
