//===-- FeroInstPrinter.cpp - Convert Fero MCInst to assembly syntax ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an Fero MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "FeroInstPrinter.h"

#include "FeroInstrInfo.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define PRINT_ALIAS_INSTR
#include "FeroGenAsmWriter.inc"

FeroInstPrinter::FeroInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                                   const MCRegisterInfo &MRI)
    : MCInstPrinter(MAI, MII, MRI) {}

void FeroInstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const {
  OS << StringRef(getRegisterName(RegNo)).lower();
}

void FeroInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                 StringRef Annot, const MCSubtargetInfo &STI,
                                 raw_ostream &O) {
  // Try to print any aliases first.
  if (!printAliasInstr(MI, Address, STI, O)) {
    printInstruction(MI, Address, STI, O);
  }
  printAnnotation(O, Annot);
}

void FeroInstPrinter::printOperand(const MCInst *MI, unsigned OpNo, const MCSubtargetInfo &STI,
                                    raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    printRegName(O, Op.getReg());
    return;
  }

  if (Op.isImm()) {
    O << Op.getImm();
    return;
  }

  assert(Op.isExpr() && "unknown operand kind in printOperand");
  Op.getExpr()->print(O, &MAI, true);
}

void FeroInstPrinter::printOperand(const MCInst *MI, unsigned Address, unsigned OpNo, const MCSubtargetInfo &STI, raw_ostream &O) {
  O << Address;
}

void FeroInstPrinter::printMemIOperand(const MCInst *MI, unsigned OpNo, const MCSubtargetInfo &STI, raw_ostream &O) {
    const MCOperand &Op = MI->getOperand(OpNo);
    assert(Op.isImm() && "unknown operand kind in printMemIOperand");
    O << "[";
    O << Op.getImm();
    O << "]";
}

void FeroInstPrinter::printMemROperand(const MCInst *MI, unsigned OpNo, const MCSubtargetInfo &STI, raw_ostream &O) {
    const MCOperand &Op = MI->getOperand(OpNo);
    assert(Op.isReg() && "unknown operand kind in printMemROperand");
    O << "[";
    printRegName(O, Op.getReg());
    O << "]";
}
