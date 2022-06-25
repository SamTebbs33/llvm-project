
//===-- FeroMICombine.cpp - Fero Psueod Instruction Expansion -------------===//
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
#include "llvm/CodeGen/ReachingDefAnalysis.h"

using namespace llvm;

#define DEBUG_TYPE "fero-mi-combine"
#define DEBUG_PREFIX "Fero MI Combine: "

namespace llvm {
class FeroMICombine : public MachineFunctionPass {
private:
  const FeroInstrInfo *TII;

public:
  static char ID;
  ReachingDefAnalysis       *RDA = nullptr;
  FeroMICombine() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;
  bool CombineLD(MachineInstr* MI);

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<ReachingDefAnalysis>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

} // namespace llvm

FunctionPass *llvm::createFeroMICombinePass() {
  return new FeroMICombine();
}

char FeroMICombine::ID = 0;

INITIALIZE_PASS(FeroMICombine, DEBUG_TYPE, "Fero MI Combine", false,
                false)

bool FeroMICombine::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  RDA = &getAnalysis<ReachingDefAnalysis>();
  const FeroSubtarget &ST = MF.getSubtarget<FeroSubtarget>();
  TII = static_cast<const FeroInstrInfo *>(ST.getInstrInfo());
  bool Changed = false;

  SmallVector<MachineInstr*, 2> ToLower;

  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      switch (MI.getOpcode()) {
          case Fero::LD:
            ToLower.push_back(&MI);
            break;
      }
    }
  }

  for (auto *MI : ToLower) {
    switch (MI->getOpcode()) {
      case Fero::LD:
        Changed |= CombineLD(MI);
        break;
    }
  }

  return Changed;
}

bool FeroMICombine::CombineLD(MachineInstr* MI) {
  MCPhysReg Def = MI->getOperand(0).getReg();
  unsigned int Imm = MI->getOperand(1).getImm();
  SmallPtrSet<MachineInstr*, 1> Uses;
  RDA->getGlobalUses(MI, Def, Uses);
  auto Supported = [Def](MachineInstr* I) {
    return I->getOpcode() == Fero::CMP && I->getOperand(2).getReg() == Def;
  };

  bool Changed = false;
  bool KeepLD = false;

  for (auto *Use : Uses) {
    if (!Supported(Use)) {
        KeepLD = true;
        continue;
    }

    switch (Use->getOpcode()) {
      case Fero::CMP:
        // Replace CMP(x, (LD i)) with CMPI(x, i)
        MCPhysReg CmpDef = Use->getOperand(0).getReg();
        MCPhysReg CmpReg = Use->getOperand(1).getReg();
        BuildMI(*Use->getParent(), Use, Use->getDebugLoc(), TII->get(Fero::CMPI)).addDef(CmpDef).addReg(CmpReg).addImm(Imm);
        Use->eraseFromParent();
        Changed = true;
        break;
    }
  }

  if (!Changed) return false;
  if (!KeepLD) MI->eraseFromParent();
  return Changed;
}
