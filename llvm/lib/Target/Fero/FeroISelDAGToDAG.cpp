//===-- FeroISelDAGToDAG.cpp - A Dag to Dag Inst Selector for Fero ------===//
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

#include "FeroISelDAGToDAG.h"
#include "FeroSubtarget.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/SelectionDAGISel.h"

using namespace llvm;

#define DEBUG_TYPE "fero-isel"

bool FeroDAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
  Subtarget = &static_cast<const FeroSubtarget &>(MF.getSubtarget());
  return SelectionDAGISel::runOnMachineFunction(MF);
}

void FeroDAGToDAGISel::Select(SDNode *Node) {
  unsigned Opcode = Node->getOpcode();

  // If we have a custom node, we already have selected!
  if (Node->isMachineOpcode()) {
    LLVM_DEBUG(errs() << "== "; Node->dump(CurDAG); errs() << "\n");
    Node->setNodeId(-1);
    return;
  }

  // Instruction Selection not handled by the auto-generated tablegen selection
  // should be handled here.
  switch(Opcode) {
    case ISD::FrameIndex:
      selectFrameIndex(Node);
      return;
    case ISD::Constant:
      selectConstantAssignment(Node);
      return;
    case ISD::BR:
      selectBranch(Node, 0, FeroISD::Al);
      return;
    case ISD::BR_CC:
      selectBranchCC(Node, 4);
      return;
    default: break;
  }

  // Select the default instruction
  SelectCode(Node);
}

void FeroDAGToDAGISel::selectBranchCC(SDNode *Node, unsigned int TargetOp) {
  auto Code = (cast<CondCodeSDNode>(Node->getOperand(1)))->get();
  Node->getOperand(1).dump(); // cond
  Node->getOperand(2).dump(); // a
  Node->getOperand(3).dump(); // b
  // 4: target
  SDValue TargetVal;
  unsigned int i = 0;
  for (auto It : Node->op_values()) {
    if (i++ == TargetOp) {
      TargetVal = It;
      break;
    }
  }
  SDValue Values[] = {TargetVal, CurDAG->getTargetConstant(FeroISD::getCondCode(Code), SDLoc(Node), MVT::i16), Node->getOperand(2), Node->getOperand(3)};
  CurDAG->SelectNodeTo(Node, FeroISD::JmpCC, Node->getValueType(0), Values);
}

SDNode* FeroDAGToDAGISel::selectBranch(SDNode *Node, unsigned int TargetOp, FeroISD::CondCode Condition) {
  SDValue Val;
  unsigned int i = 0;
  for (auto It : Node->op_values()) {
    if (i++ == TargetOp) {
      Val = It;
      break;
    }
  }
  return CurDAG->SelectNodeTo(Node, FeroISD::Jmp, Node->getValueType(0), Val, CurDAG->getTargetConstant(Condition, SDLoc(Node), MVT::i16));
}

void FeroDAGToDAGISel::selectConstantAssignment(SDNode *Node) {
  auto Imm = cast<ConstantSDNode>(Node);
  CurDAG->SelectNodeTo(Node, Fero::LD, Node->getValueType(0), CurDAG->getTargetConstant(Imm->getZExtValue(), SDLoc(Node), MVT::i16));
}

void FeroDAGToDAGISel::selectFrameIndex(SDNode *Node) {
  SDLoc DL(Node);
  SDValue Imm = CurDAG->getTargetConstant(0, DL, MVT::i16);
  int FI = cast<FrameIndexSDNode>(Node)->getIndex();
  EVT VT = Node->getValueType(0);
  SDValue TFI = CurDAG->getTargetFrameIndex(FI, VT);
  unsigned Opc = Fero::ADDI;
  if (Node->hasOneUse()) {
    CurDAG->SelectNodeTo(Node, Opc, VT, TFI, Imm);
    return;
  }
  ReplaceNode(Node, CurDAG->getMachineNode(Opc, DL, VT, TFI, Imm));
}
