//===-- FeroRegisterInfo.td - Fero Register defs ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the RISC-W register file
//===----------------------------------------------------------------------===//

let Namespace = "Fero" in {
class FeroReg<bits<5> Enc, string n, list<string> alt = []> : Register<n> {
  let HWEncoding{4-0} = Enc;
  let AltNames = alt;
}
} // end Namespace

def R0  : FeroReg<0, "r0", []>, DwarfRegNum<[0]>;
def R1  : FeroReg<1, "r1", []>, DwarfRegNum<[1]>;
def R2  : FeroReg<2, "r2", []>, DwarfRegNum<[2]>;
def R3  : FeroReg<3, "r3", []>, DwarfRegNum<[3]>;
def R4  : FeroReg<4, "r4", []>, DwarfRegNum<[4]>;
def R5  : FeroReg<5, "r5", []>, DwarfRegNum<[5]>;
def R6  : FeroReg<6, "r6", []>, DwarfRegNum<[6]>;
def R7  : FeroReg<7, "r7", []>, DwarfRegNum<[7]>;
def R8  : FeroReg<8, "r8", []>, DwarfRegNum<[8]>;
def R9  : FeroReg<9, "r9", []>, DwarfRegNum<[9]>;
def R10 : FeroReg<10,"r10", []>, DwarfRegNum<[10]>;
def R11 : FeroReg<11,"r11", []>, DwarfRegNum<[11]>;
def R12 : FeroReg<12,"r12", []>, DwarfRegNum<[12]>;
def R13 : FeroReg<13,"r13", ["tid"]>, DwarfRegNum<[13]>;
def R14 : FeroReg<14,"r14", ["sp"]>, DwarfRegNum<[14]>;
def R15 : FeroReg<15,"r15", ["lr"]>, DwarfRegNum<[15]>;

// The order of registers represents the preferred allocation sequence.
// Registers are listed in the order caller-save, callee-save, specials.
def GPR : RegisterClass<"Fero", [i16], 16, (add
    (sequence "R%u", 4, 12),
    (sequence "R%u", 0, 3),
    (sequence "R%u", 13, 15)
  )>;

def SP : RegisterClass<"Fero", [i16], 16, (add R14)>;
