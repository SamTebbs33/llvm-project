//===-- FeroPseudoExpansion.cpp - Fero Psueod Instruction Expansion -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass re-arranges machine basic blocks to suit target requirements.
// Currently it only moves blocks to fix backwards WLS branches.
//
//===----------------------------------------------------------------------===//

#include "Fero.h"
#include "FeroInstrInfo.h"
#include "FeroSubtarget.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineLoopInfo.h"

using namespace llvm;

#define DEBUG_TYPE "fero-pseudo-expansion"
#define DEBUG_PREFIX "Fero Pseudo Expansion: "

namespace llvm {
class FeroPseudoExpansion : public MachineFunctionPass {
private:
  const FeroInstrInfo *TII;

public:
  static char ID;
  FeroPseudoExpansion() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;
  bool LowerSelectCC(MachineInstr* MI);

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

} // namespace llvm

FunctionPass *llvm::createFeroPseudoExpansionPass() {
  return new FeroPseudoExpansion();
}

char FeroPseudoExpansion::ID = 0;

INITIALIZE_PASS(FeroPseudoExpansion, DEBUG_TYPE, "Fero Pseudo Expansion", false,
                false)

bool FeroPseudoExpansion::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;
  const FeroSubtarget &ST = MF.getSubtarget<FeroSubtarget>();
  TII = static_cast<const FeroInstrInfo *>(ST.getInstrInfo());
  bool Changed = false;

  SmallVector<MachineInstr*, 2> ToLower;

  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      switch (MI.getOpcode()) {
          case Fero::PseudoSelectCC:
            ToLower.push_back(&MI);
            break;
      }
    }
  }

  for (auto *MI : ToLower) {
    switch (MI->getOpcode()) {
      case Fero::PseudoSelectCC:
        Changed |= LowerSelectCC(MI);
        break;
    }
  }

  return Changed;
}

bool FeroPseudoExpansion::LowerSelectCC(MachineInstr* MI) {
  MachineOperand Dest = MI->getOperand(0);
  MachineOperand CmpX = MI->getOperand(2);
  MachineOperand CmpY = MI->getOperand(3);
  MachineOperand SrcA = MI->getOperand(4);
  MachineOperand SrcB = MI->getOperand(5);
  MachineOperand Flag = MI->getOperand(6);
  assert (Dest.isReg() && CmpX.isReg() && CmpY.isReg() && SrcA.isReg() && SrcB.isReg() && Flag.isImm() && "Unexpected PseudoSelectCC operand type");

  MCPhysReg DestReg = Dest.getReg();
  MCPhysReg CmpXReg = CmpX.getReg();
  MCPhysReg CmpYReg = CmpY.getReg();
  MCPhysReg SrcAReg = SrcA.getReg();
  MCPhysReg SrcBReg = SrcB.getReg();
  unsigned FlagImm = Flag.getImm();

  auto DL = MI->getDebugLoc();
  unsigned OpcTrue = 0;
  unsigned OpcFalse = 0;
  switch (FlagImm) {
    case 1:
      OpcTrue = Fero::MOV_Z;
      OpcFalse = Fero::MOV_notZ;
      break;
    default:
      llvm_unreachable("unsupported flag value in LowerSelectCC");
  }
  BuildMI(*MI->getParent(), MI, DL, TII->get(Fero::CMP)).addDef(Fero::FR).addReg(CmpXReg).addReg(CmpYReg);
  if (SrcAReg != DestReg) BuildMI(*MI->getParent(), MI, DL, TII->get(OpcTrue)).addDef(DestReg).addReg(SrcAReg).addReg(Fero::FR);
  if (SrcBReg != DestReg) BuildMI(*MI->getParent(), MI, DL, TII->get(OpcFalse)).addDef(DestReg).addReg(SrcBReg).addReg(Fero::FR);
  MI->eraseFromParent();
  return true;
}
