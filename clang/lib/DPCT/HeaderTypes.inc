//===--------------- HeaderTypes.inc --------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef HEADER
#define HEADER(Name, Spelling)
#endif

#ifndef STD_HEADER
#define STD_HEADER(Name, Spelling) HEADER(Name, Spelling)
#endif

#ifndef ONEDPL_HEADER
#define ONEDPL_HEADER(Name, Spelling) HEADER(DPL_ ## Name, Spelling)
#endif

#ifndef MKL_HEADER
#define MKL_HEADER(Name, Spelling) HEADER(MKL_ ## Name, Spelling)
#endif

#ifndef DPCT_HEADER
#define DPCT_HEADER(Name, Spelling) HEADER(DPCT_ ## Name, Spelling)
#endif

STD_HEADER(SYCL, "<sycl/sycl.hpp>")
STD_HEADER(Math, "<cmath>")
STD_HEADER(Algorithm, "<algorithm>")
STD_HEADER(Complex, "<complex>")
STD_HEADER(Thread, "<thread>")
STD_HEADER(Future, "<future>")
STD_HEADER(Time, "<time.h>")
STD_HEADER(Stdlib, "<cstdlib>")
STD_HEADER(Chrono, "<chrono>")
STD_HEADER(Limits, "<limits>")
STD_HEADER(Numeric, "<numeric>")
STD_HEADER(Functional, "<functional>")
STD_HEADER(Array, "<array>")
STD_HEADER(Tuple, "<tuple>")
STD_HEADER(SYCL_Math, "<sycl/ext/intel/math.hpp>")
#ifdef _WIN32
STD_HEADER(DL, "<libloaderapi.h>")
#else
STD_HEADER(DL, "<dlfcn.h>")
#endif

ONEDPL_HEADER(Algorithm, "<oneapi/dpl/algorithm>")
ONEDPL_HEADER(Execution, "<oneapi/dpl/execution>")
ONEDPL_HEADER(Iterator, "<oneapi/dpl/iterator>")
ONEDPL_HEADER(Async, "<oneapi/dpl/async>")
ONEDPL_HEADER(Memory, "<oneapi/dpl/memory>")
ONEDPL_HEADER(Random, "<oneapi/dpl/random>")

MKL_HEADER(Mkl, "<oneapi/mkl.hpp>")
MKL_HEADER(RNG, "<oneapi/mkl/rng/device.hpp>")

DPCT_HEADER(Dpct,  "<dpct/dpct.hpp>")
DPCT_HEADER(DPL_Utils, "<dpct/dpl_utils.hpp>")
DPCT_HEADER(FFT_Utils, "<dpct/fft_utils.hpp>")
DPCT_HEADER(RNG_Utils, "<dpct/rng_utils.hpp>")
DPCT_HEADER(CCL_Utils, "<dpct/ccl_utils.hpp>")
DPCT_HEADER(BLAS_Utils, "<dpct/blas_utils.hpp>")
DPCT_HEADER(DNNL_Utils, "<dpct/dnnl_utils.hpp>")
DPCT_HEADER(LAPACK_Utils, "<dpct/lapack_utils.hpp>")
DPCT_HEADER(COMMON_Utils, "<dpct/lib_common_utils.hpp>")
DPCT_HEADER(Atomic, "<dpct/atomic.hpp>")
DPCT_HEADER(SPBLAS_Utils, "<dpct/sparse_utils.hpp>")
DPCT_HEADER(Math, "<dpct/math.hpp>")

#undef MKL_HEADER
#undef ONEDPL_HEADER
#undef STD_HEADER
#undef HEADER
