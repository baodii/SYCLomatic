// Option: --use-dpcpp-extensions=intel_device_math
__global__ void test(unsigned int u1, unsigned int u2) {
  // Start
  __vaddus2(u1 /*unsigned int*/, u2 /*unsigned int*/);
  // End
}
