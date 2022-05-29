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
#include "FeroISelLowering.h"
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
    default: break;
  }

  // Select the default instruction
  SelectCode(Node);
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
