// Option: --use-dpcpp-extensions=intel_device_math
#include "cuda_bf16.h"
#include "cuda_fp16.h"

__global__ void test(__half h1, __half h2, __nv_bfloat16 b1, __nv_bfloat16 b2) {
  // Start
  __hmul_sat(h1 /*__half*/, h2 /*__half*/);
  __hmul_sat(b1 /*__nv_bfloat16*/, b2 /*__nv_bfloat16*/);
  // End
}
