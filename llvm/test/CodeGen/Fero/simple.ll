; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=fero %s -o - | FileCheck %s

define i16 @or(i16 %a, i16 %b) {
entry:
  ; CHECK-LABEL: or:
  ; CHECK: or r1, r0
  ; CHECK-NEXT: mov r0, r1
  ; CHECK-NEXT: ret
  %or = or i16 %b, %a
  ret i16 %or
}

define i16 @mov_indirect(i16* %a) {
entry:
  ; CHECK-LABEL: mov_indirect:
  ; CHECK: mov r0, [r0]
  ; CHECK-NEXT: ret
  %0 = load i16, i16* %a
  ret i16 %0
}

define void @str(i16* nocapture noundef writeonly %a, i16 noundef %b) {
entry:
  ; CHECK-LABEL: str:
  ; CHECK: str [r0], r1
  ; CHECK-NEXT: ret
  store i16 %b, i16* %a
  ret void
}

define i16 @add(i16 %a, i16 %b) {
entry:
  ; CHECK-LABEL: add:
  ; CHECK: add r0, r1
  ; CHECK-NEXT: ret
  %0 = add i16 %a, %b
  ret i16 %0
}

define i16 @sub(i16 %a, i16 %b) {
entry:
  ; CHECK-LABEL: sub:
  ; CHECK: sub r0, r1
  ; CHECK-NEXT: ret
  %0 = sub i16 %a, %b
  ret i16 %0
}

define i16 @setcc_eq(i16 %a, i16 %b) {
entry:
; CHECK-LABEL: setcc_eq:
; CHECK: tst  r0
; CHECK-NEXT: mov.Z	r0, r1
; CHECK-NEXT: ret
  %cmp = icmp eq i16 %a, 0
  %b.a = select i1 %cmp, i16 %b, i16 %a
  ret i16 %b.a
}

define i16 @set(i16 %a, i16 %b) {
entry:
; CHECK-LABEL: set:
; CHECK: tst  r0
; CHECK-NEXT: set	r0, 2
; CHECK-NEXT: ret
  %cmp = icmp ne i16 %a, 0
  %. = zext i1 %cmp to i16
  ret i16 %.
}

define i16 @inc(i16 %a) {
entry:
  ; CHECK-LABEL: inc:
  ; CHECK: inc r0
  ; CHECK-NEXT: ret
  %0 = add i16 %a, 1
  ret i16 %0
}

define i16 @dec(i16 %a) {
entry:
  ; CHECK-LABEL: dec:
  ; CHECK: dec r0
  ; CHECK-NEXT: ret
  %0 = sub i16 %a, 1
  ret i16 %0
}

define void @jmp_notZ(i16 %a, i16* %b) {
entry:
; CHECK-LABEL: jmp_notZ:
; CHECK: tst  r0
; CHECK-NEXT: jmp.notZ	.LBB9_2
; CHECK-NEXT: %bb.1:                                ; %if.then
; CHECK-NEXT: ld	r0, 0
; CHECK-NEXT: str	[r1], r0
; CHECK-NEXT: .LBB9_2:
; CHECK-NEXT: ret
  %cmp = icmp eq i16 %a, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i16 0, i16* %b
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

define i16 @and(i16 %a, i16 %b) {
entry:
; CHECK-LABEL: and:
; CHECK: and  r1, r0
; CHECK-NEXT: mov r0, r1
; CHECK-NEXT: ret
  %and = and i16 %b, %a
  ret i16 %and
}

define i16 @shl(i16 %a, i16 %b) {
entry:
; CHECK-LABEL: shl:
; CHECK: shl  r0, r1
; CHECK-NEXT: ret
  %shl = shl i16 %a, %b
  ret i16 %shl
}
