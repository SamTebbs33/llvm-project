#include "ARM.h"
#include "ARMBaseInstrInfo.h"
#include "ARMBasicBlockInfo.h"
#include "ARMSubtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define DEBUG_TYPE "fero-branch-lowering"
#define DEBUG_PREFIX "Fero Branch Lowering"

namespace llvm {
class FeroBranchLowering : public MachineFunctionPass {
public:
  static char ID;
  FeroBranchLowering() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF);

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
}
}
