// Migration desc: The API is Removed.
void test(cudaLimit l, size_t s) {
  // Start
  cudaDeviceSetLimit(l /*cudaLimit*/, s /*size_t*/);
  // End
}
