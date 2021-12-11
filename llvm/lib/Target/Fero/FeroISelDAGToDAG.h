//===---- FeroISelDAGToDAG.h - A Dag to Dag Inst Selector for Fero ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the Fero target.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Fero_FeroISELDAGTODAG_H
#define LLVM_LIB_TARGET_Fero_FeroISELDAGTODAG_H

#include "FeroSubtarget.h"
#include "FeroTargetMachine.h"
#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {
class FeroDAGToDAGISel : public SelectionDAGISel {
public:
  explicit FeroDAGToDAGISel(FeroTargetMachine &TM, CodeGenOpt::Level OL)
      : SelectionDAGISel(TM, OL), Subtarget(nullptr) {}

  // Pass Name
  StringRef getPassName() const override {
    return "Fero DAG->DAG Pattern Instruction Selection";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void Select(SDNode *Node) override;
  void selectFrameIndex(SDNode *Node);
  void selectConstantAssignment(SDNode *Node);
  SDNode* selectBranch(SDNode *Node, unsigned int TargetOp, FeroISD::CondCode Condition);
  void selectBranchCC(SDNode *Node, unsigned int TargetOp);

#include "FeroGenDAGISel.inc"

private:
  const FeroSubtarget *Subtarget;
};
}

#endif // end LLVM_LIB_TARGET_Fero_FeroISELDAGTODAG_H
