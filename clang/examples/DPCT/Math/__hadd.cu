// Option: --use-dpcpp-extensions=intel_device_math
#include "cuda_bf16.h"
#include "cuda_fp16.h"

__global__ void test(__half h1, __half h2, __nv_bfloat16 b1, __nv_bfloat16 b2,
                     int i1, int i2) {
  // Start
  __hadd(h1 /*__half*/, h2 /*__half*/);
  __hadd(b1 /*__nv_bfloat16*/, b2 /*__nv_bfloat16*/);
  __hadd(i1 /*int*/, i2 /*int*/);
  // End
}
