// RUN: mlir-opt %s -allow-unregistered-dialect -split-input-file -test-vector-warp-distribute=rewrite-warp-ops-to-scf-if | FileCheck %s --check-prefix=CHECK-SCF-IF
// RUN: mlir-opt %s -allow-unregistered-dialect -split-input-file -test-vector-warp-distribute="hoist-uniform" | FileCheck --check-prefixes=CHECK-HOIST %s
// RUN: mlir-opt %s -allow-unregistered-dialect -split-input-file -test-vector-warp-distribute="hoist-uniform distribute-transfer-write" | FileCheck --check-prefixes=CHECK-D %s

// CHECK-SCF-IF-DAG: memref.global "private" @__shared_32xf32 : memref<32xf32, 3>
// CHECK-SCF-IF-DAG: memref.global "private" @__shared_64xf32 : memref<64xf32, 3>
// CHECK-SCF-IF-DAG: memref.global "private" @__shared_128xf32 : memref<128xf32, 3>
// CHECK-SCF-IF-DAG: memref.global "private" @__shared_256xf32 : memref<256xf32, 3>

// CHECK-SCF-IF-LABEL: func @rewrite_warp_op_to_scf_if(
//  CHECK-SCF-IF-SAME:     %[[laneid:.*]]: index,
//  CHECK-SCF-IF-SAME:     %[[v0:.*]]: vector<4xf32>, %[[v1:.*]]: vector<8xf32>)
func.func @rewrite_warp_op_to_scf_if(%laneid: index,
                                %v0: vector<4xf32>, %v1: vector<8xf32>) {
//   CHECK-SCF-IF-DAG:   %[[c0:.*]] = arith.constant 0 : index
//   CHECK-SCF-IF-DAG:   %[[c2:.*]] = arith.constant 2 : index
//   CHECK-SCF-IF-DAG:   %[[c4:.*]] = arith.constant 4 : index
//   CHECK-SCF-IF-DAG:   %[[c8:.*]] = arith.constant 8 : index
//       CHECK-SCF-IF:   %[[is_lane_0:.*]] = arith.cmpi eq, %[[laneid]], %[[c0]]

//       CHECK-SCF-IF:   %[[buffer_v0:.*]] = memref.get_global @__shared_128xf32
//       CHECK-SCF-IF:   %[[s0:.*]] = arith.muli %[[laneid]], %[[c4]]
//       CHECK-SCF-IF:   vector.store %[[v0]], %[[buffer_v0]][%[[s0]]]
//       CHECK-SCF-IF:   %[[buffer_v1:.*]] = memref.get_global @__shared_256xf32
//       CHECK-SCF-IF:   %[[s1:.*]] = arith.muli %[[laneid]], %[[c8]]
//       CHECK-SCF-IF:   vector.store %[[v1]], %[[buffer_v1]][%[[s1]]]

//   CHECK-SCF-IF-DAG:   gpu.barrier
//   CHECK-SCF-IF-DAG:   %[[buffer_def_0:.*]] = memref.get_global @__shared_32xf32
//   CHECK-SCF-IF-DAG:   %[[buffer_def_1:.*]] = memref.get_global @__shared_64xf32

//       CHECK-SCF-IF:   scf.if %[[is_lane_0]] {
  %r:2 = vector.warp_execute_on_lane_0(%laneid)[32]
      args(%v0, %v1 : vector<4xf32>, vector<8xf32>) -> (vector<1xf32>, vector<2xf32>) {
    ^bb0(%arg0: vector<128xf32>, %arg1: vector<256xf32>):
//       CHECK-SCF-IF:     %[[arg1:.*]] = vector.load %[[buffer_v1]][%[[c0]]] : memref<256xf32, 3>, vector<256xf32>
//       CHECK-SCF-IF:     %[[arg0:.*]] = vector.load %[[buffer_v0]][%[[c0]]] : memref<128xf32, 3>, vector<128xf32>
//       CHECK-SCF-IF:     %[[def_0:.*]] = "some_def"(%[[arg0]]) : (vector<128xf32>) -> vector<32xf32>
//       CHECK-SCF-IF:     %[[def_1:.*]] = "some_def"(%[[arg1]]) : (vector<256xf32>) -> vector<64xf32>
    %2 = "some_def"(%arg0) : (vector<128xf32>) -> vector<32xf32>
    %3 = "some_def"(%arg1) : (vector<256xf32>) -> vector<64xf32>
//       CHECK-SCF-IF:     vector.store %[[def_0]], %[[buffer_def_0]][%[[c0]]]
//       CHECK-SCF-IF:     vector.store %[[def_1]], %[[buffer_def_1]][%[[c0]]]
    vector.yield %2, %3 : vector<32xf32>, vector<64xf32>
  }
//       CHECK-SCF-IF:   }
//       CHECK-SCF-IF:   gpu.barrier
//       CHECK-SCF-IF:   %[[o1:.*]] = arith.muli %[[laneid]], %[[c2]]
//       CHECK-SCF-IF:   %[[r1:.*]] = vector.load %[[buffer_def_1]][%[[o1]]] : memref<64xf32, 3>, vector<2xf32>
//       CHECK-SCF-IF:   %[[r0:.*]] = vector.load %[[buffer_def_0]][%[[laneid]]] : memref<32xf32, 3>, vector<1xf32>
//       CHECK-SCF-IF:   "some_use"(%[[r0]]) : (vector<1xf32>) -> ()
//       CHECK-SCF-IF:   "some_use"(%[[r1]]) : (vector<2xf32>) -> ()
  "some_use"(%r#0) : (vector<1xf32>) -> ()
  "some_use"(%r#1) : (vector<2xf32>) -> ()
  return
}

// -----

// CHECK-D-DAG: #[[MAP1:.*]] = affine_map<()[s0] -> (s0 * 2 + 32)>

// CHECK-ALL-LABEL: func @warp(
// CHECK-HOIST: memref.subview
// CHECK-HOIST: memref.subview
// CHECK-HOIST: memref.subview
// CHECK-HOIST: vector.warp_execute_on_lane_0

//     CHECK-D: %[[R:.*]]:2 = vector.warp_execute_on_lane_0(%{{.*}})[32] -> (vector<2xf32>, vector<1xf32>) {
//     CHECK-D:   arith.addf {{.*}} : vector<32xf32>
//     CHECK-D:   arith.addf {{.*}} : vector<64xf32>
//     CHECK-D:   vector.yield %{{.*}}, %{{.*}} : vector<64xf32>, vector<32xf32>
// CHECK-D-DAG: vector.transfer_write %[[R]]#1, %{{.*}}[%{{.*}}] {in_bounds = [true]} : vector<1xf32>, memref<128xf32
// CHECK-D-DAG: %[[ID1:.*]] = affine.apply #[[MAP1]]()[%{{.*}}]
// CHECK-D-DAG: vector.transfer_write %[[R]]#0, %2[%[[ID1]]] {in_bounds = [true]} : vector<2xf32>, memref<128xf32

// CHECK-ALL-NOT: vector.warp_execute_on_lane_0
// CHECK-ALL: vector.transfer_read {{.*}} vector<1xf32>
// CHECK-ALL: vector.transfer_read {{.*}} vector<1xf32>
// CHECK-ALL: vector.transfer_read {{.*}} vector<2xf32>
// CHECK-ALL: vector.transfer_read {{.*}} vector<2xf32>
// CHECK-ALL: arith.addf {{.*}} : vector<1xf32>
// CHECK-ALL: arith.addf {{.*}} : vector<2xf32>
// CHECK-ALL: vector.transfer_write {{.*}} : vector<1xf32>
// CHECK-ALL: vector.transfer_write {{.*}} : vector<2xf32>

#map0 =  affine_map<(d0)[s0] -> (d0 + s0)>
func.func @warp(%laneid: index, %arg1: memref<1024xf32>, %arg2: memref<1024xf32>,
           %arg3: memref<1024xf32>, %gid : index) {
  vector.warp_execute_on_lane_0(%laneid)[32] {
    %sa = memref.subview %arg1[%gid] [128] [1] : memref<1024xf32> to memref<128xf32, #map0>
    %sb = memref.subview %arg2[%gid] [128] [1] : memref<1024xf32> to memref<128xf32, #map0>
    %sc = memref.subview %arg3[%gid] [128] [1] : memref<1024xf32> to memref<128xf32, #map0>
    %c0 = arith.constant 0 : index
    %c32 = arith.constant 32 : index
    %cst = arith.constant 0.000000e+00 : f32
    %2 = vector.transfer_read %sa[%c0], %cst : memref<128xf32, #map0>, vector<32xf32>
    %3 = vector.transfer_read %sa[%c32], %cst : memref<128xf32, #map0>, vector<32xf32>
    %4 = vector.transfer_read %sb[%c0], %cst : memref<128xf32, #map0>, vector<64xf32>
    %5 = vector.transfer_read %sb[%c32], %cst : memref<128xf32, #map0>, vector<64xf32>
    %6 = arith.addf %2, %3 : vector<32xf32>
    %7 = arith.addf %4, %5 : vector<64xf32>
    vector.transfer_write %6, %sc[%c0] : vector<32xf32>, memref<128xf32, #map0>
    vector.transfer_write %7, %sc[%c32] : vector<64xf32>, memref<128xf32, #map0>
  }
  return
}

// -----

// CHECK-D-LABEL: func @warp_extract(
//       CHECK-D:   %[[WARPOP:.*]] = vector.warp_execute_on_lane_0(%{{.*}})[32] -> (vector<1xf32>)
//       CHECK-D:     "test.dummy_op"
//       CHECK-D:     vector.yield %{{.*}} : vector<1xf32>
//       CHECK-D:   }
//       CHECK-D:   vector.warp_execute_on_lane_0(%{{.*}})[32] {
//       CHECK-D:     vector.transfer_write %[[WARPOP]], %{{.*}}[%{{.*}}] {{.*}} : vector<1xf32>
//       CHECK-D:   }

#map2 =  affine_map<(d0)[s0] -> (d0 + s0)>

func.func @warp_extract(%laneid: index, %arg1: memref<1024xf32>, %gid : index) {
  vector.warp_execute_on_lane_0(%laneid)[32] {
    %sa = memref.subview %arg1[%gid] [128] [1] : memref<1024xf32> to memref<128xf32, #map2>
    %c0 = arith.constant 0 : index
    %v = "test.dummy_op"() : () -> (vector<1xf32>)
    vector.transfer_write %v, %sa[%c0] : vector<1xf32>, memref<128xf32, #map2>
  }
  return
}