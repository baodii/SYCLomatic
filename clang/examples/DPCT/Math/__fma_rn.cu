__global__ void test(double d1, double d2, double d3) {
  // Start
  __fma_rn(d1 /*double*/, d2 /*double*/, d3 /*double*/);
  // End
}
