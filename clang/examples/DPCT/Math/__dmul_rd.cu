__global__ void test(double d1, double d2) {
  // Start
  __dmul_rd(d1 /*double*/, d2 /*double*/);
  // End
}
