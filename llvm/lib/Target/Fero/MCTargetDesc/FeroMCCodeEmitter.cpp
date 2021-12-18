//===-- FeroMCCodeEmitter.cpp - Convert Fero Code to Machine Code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the FeroMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//
#define DEBUG_TYPE "mccodeemitter"
#include "FeroFixupKinds.h"
#include "FeroMCCodeEmitter.h"
#include "FeroMCExpr.h"
#include "FeroMCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

inline static unsigned getFeroRegisterNumbering(unsigned RegEnum)
{
  switch (RegEnum) {
    case Fero::R0:
      return 0;
    case Fero::R1:
      return 1;
    case Fero::R2:
      return 2;
    case Fero::R3:
      return 3;
    case Fero::R4:
      return 4;
    case Fero::R5:
      return 5;
    case Fero::R6:
      return 6;
    case Fero::R7:
      return 7;
    case Fero::R8:
      return 8;
    case Fero::R9:
      return 9;
    case Fero::R10:
      return 10;
    case Fero::R11:
      return 11;
    case Fero::R12:
      return 12;
    case Fero::R13:
      return 13;
    case Fero::R14:
      return 14;
    case Fero::R15:
      return 15;
  default: llvm_unreachable("Unknown register number!");
  }
}

/// EncodeInstruction - Emit the instruction.
/// Size the instruction (currently only 4 bytes
void FeroMCCodeEmitter::
  encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const {
  uint32_t Binary = getBinaryCodeForInstr(MI, Fixups, STI);
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  int Size = Desc.Size;

  EmitInstruction(Binary, Size, OS);
}

void FeroMCCodeEmitter::EmitByte(unsigned char C, raw_ostream &OS) const {
  OS << (char)C;
}

void FeroMCCodeEmitter::EmitInstruction(uint64_t Val, unsigned int Size, raw_ostream &OS) const {
    // Output the instruction encoding in little endian byte order.
  for (unsigned i = 0; i < Size; ++i) {
    unsigned Shift = IsLittleEndian ? i * 8 : (Size - 1 - i) * 8;
    EmitByte((Val >> Shift) & 0xff, OS);
  }
}

/// getJumpTargetOpValue - Return binary encoding of the jump
/// target operand. Such as SWI and JSUB.
/// If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned FeroMCCodeEmitter::
   getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const {

  const MCOperand &MO = MI.getOperand(OpNo);
  // If the destination is an immediate, we have nothing to do.
  if (MO.isImm()) return MO.getImm();
  assert(MO.isExpr() && "getJumpTargetOpValue expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(MCFixup::create(0, Expr,
                                   MCFixupKind(Fero::fixup_Fero_JMP)));

  return 0;
}

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned FeroMCCodeEmitter::
  getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const {
  if (MO.isReg()) {
    unsigned Reg = MO.getReg();
    unsigned RegNo = getFeroRegisterNumbering(Reg);
    return RegNo;
  } else if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  } else llvm_unreachable("uncrecognised machine operand type");

  // MO must be an Expr.
  assert(MO.isExpr());

  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();

  if (Kind == MCExpr::Binary) {
    Expr = static_cast<const MCBinaryExpr*>(Expr)->getLHS();
    Kind = Expr->getKind();
  }

  assert (Kind == MCExpr::SymbolRef);
  Fero::Fixups FixupKind = Fero::fixup_Fero_JMP;
  Fixups.push_back(MCFixup::create(0, MO.getExpr(), MCFixupKind(FixupKind)));

  // All of the information is in the fixup.
  return 0;
}

unsigned FeroMCCodeEmitter::getRMemoryOpValue(const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const {
  assert(MI.getOperand(OpNo).isReg());
  return MI.getOperand(OpNo).getReg();
}

#include "FeroGenMCCodeEmitter.inc"

