//=== FeroCallingConv.td - Calling Conventions for Fero --*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for Fero architecture.
//===----------------------------------------------------------------------===//

def Fero_CCallingConv : CallingConv<[
  // Handles byval parameters. Tell LLVM what the size and alignment
  // requirements of each stack slot are
  CCIfByVal<CCPassByVal<2, 1>>,

  // Promote i8 to i16
  CCIfType<[i1, i8], CCPromoteToType<i16>>,

  // The first 4 arguments are passed in R0, R1, R2, R3
  CCIfType<[i16], CCAssignToReg<[R0, R1, R2, R3]>>,
]>;

// BeyondRISC C Return Convention
def Fero_CRetConv : CallingConv<[
  CCIfType<[i1, i8], CCPromoteToType<i16>>,

  // i16 are returned in registers R0, R1
  CCIfType<[i16], CCAssignToReg<[R0]>>,
]>;

def Fero_CalleeSavedRegs : CalleeSavedRegs<(add R4, R5, R6, R7, R8, R9, R10, R11, R12, R15)>;
