//==---------------- types.hpp --- SYCL types ------------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Implements vec and __swizzled_vec__ classes.

#pragma once

// Check if Clang's ext_vector_type attribute is available. Host compiler
// may not be Clang, and Clang may not be built with the extension.
#ifdef __clang__
#ifndef __has_extension
#define __has_extension(x) 0
#endif
#ifdef __HAS_EXT_VECTOR_TYPE__
#error "Undefine __HAS_EXT_VECTOR_TYPE__ macro"
#endif
#if __has_extension(attribute_ext_vector_type)
#define __HAS_EXT_VECTOR_TYPE__
#endif
#endif // __clang__

#if !defined(__HAS_EXT_VECTOR_TYPE__) && defined(__SYCL_DEVICE_ONLY__)
// This is a soft error. We expect the device compiler to have ext_vector_type
// support, but that should not be a hard requirement.
#error "SYCL device compiler is built without ext_vector_type support"
#endif // __HAS_EXT_VECTOR_TYPE__

#include <sycl/access/access.hpp>              // for decorated, address_space
#include <sycl/aliases.hpp>                    // for half, cl_char, cl_int
#include <sycl/detail/common.hpp>              // for ArrayCreator, RepeatV...
#include <sycl/detail/defines_elementary.hpp>  // for __SYCL2020_DEPRECATED
#include <sycl/detail/generic_type_lists.hpp>  // for vector_basic_list
#include <sycl/detail/generic_type_traits.hpp> // for is_sigeninteger, is_s...
#include <sycl/detail/iostream_proxy.hpp>      // for cout
#include <sycl/detail/memcpy.hpp>              // for memcpy
#include <sycl/detail/type_list.hpp>           // for is_contained
#include <sycl/detail/type_traits.hpp>         // for is_floating_point
#include <sycl/detail/vector_traits.hpp>       // for vector_alignment
#include <sycl/exception.hpp>                  // for make_error_code, errc
#include <sycl/half_type.hpp>                  // for StorageT, half, Vec16...
#include <sycl/marray.hpp>                     // for __SYCL_BINOP, __SYCL_...
#include <sycl/multi_ptr.hpp>                  // for multi_ptr

#include <array>       // for array
#include <assert.h>    // for assert
#include <cmath>       // for ceil, floor, rint, trunc
#include <cstddef>     // for size_t, NULL, byte
#include <cstdint>     // for uint8_t, int16_t, int...
#include <functional>  // for divides, multiplies
#include <iterator>    // for pair
#include <optional>    // for optional
#include <ostream>     // for operator<<, basic_ost...
#include <tuple>       // for tuple
#include <type_traits> // for enable_if_t, is_same
#include <utility>     // for index_sequence, make_...
#include <variant>     // for tuple, variant

#ifndef __SYCL_DEVICE_ONLY__
#include <cfenv> // for fesetround, fegetround
#endif

// 4.10.1: Scalar data types
// 4.10.2: SYCL vector types

namespace sycl {
inline namespace _V1 {

enum class rounding_mode { automatic = 0, rte = 1, rtz = 2, rtp = 3, rtn = 4 };
struct elem {
  static constexpr int x = 0;
  static constexpr int y = 1;
  static constexpr int z = 2;
  static constexpr int w = 3;
  static constexpr int r = 0;
  static constexpr int g = 1;
  static constexpr int b = 2;
  static constexpr int a = 3;
  static constexpr int s0 = 0;
  static constexpr int s1 = 1;
  static constexpr int s2 = 2;
  static constexpr int s3 = 3;
  static constexpr int s4 = 4;
  static constexpr int s5 = 5;
  static constexpr int s6 = 6;
  static constexpr int s7 = 7;
  static constexpr int s8 = 8;
  static constexpr int s9 = 9;
  static constexpr int sA = 10;
  static constexpr int sB = 11;
  static constexpr int sC = 12;
  static constexpr int sD = 13;
  static constexpr int sE = 14;
  static constexpr int sF = 15;
};

namespace detail {
// select_apply_cl_t selects from T8/T16/T32/T64 basing on
// sizeof(_IN).  expected to handle scalar types in _IN.
template <typename _IN, typename T8, typename T16, typename T32, typename T64>
using select_apply_cl_t = std::conditional_t<
    sizeof(_IN) == 1, T8,
    std::conditional_t<sizeof(_IN) == 2, T16,
                       std::conditional_t<sizeof(_IN) == 4, T32, T64>>>;

template <typename T> struct vec_helper {
  using RetType = T;
  static constexpr RetType get(T value) { return value; }
};
template <> struct vec_helper<bool> {
  using RetType = select_apply_cl_t<bool, std::int8_t, std::int16_t,
                                    std::int32_t, std::int64_t>;
  static constexpr RetType get(bool value) { return value; }
};

#if (!defined(_HAS_STD_BYTE) || _HAS_STD_BYTE != 0)
template <> struct vec_helper<std::byte> {
  using RetType = std::uint8_t;
  static constexpr RetType get(std::byte value) { return (RetType)value; }
  static constexpr std::byte get(std::uint8_t value) {
    return (std::byte)value;
  }
};
#endif

template <typename VecT, typename OperationLeftT, typename OperationRightT,
          template <typename> class OperationCurrentT, int... Indexes>
class SwizzleOp;

template <typename T, int N, typename V = void> struct VecStorage;

// Element type for relational operator return value.
template <typename DataT>
using rel_t = typename std::conditional_t<
    sizeof(DataT) == sizeof(opencl::cl_char), opencl::cl_char,
    typename std::conditional_t<
        sizeof(DataT) == sizeof(opencl::cl_short), opencl::cl_short,
        typename std::conditional_t<
            sizeof(DataT) == sizeof(opencl::cl_int), opencl::cl_int,
            typename std::conditional_t<sizeof(DataT) ==
                                            sizeof(opencl::cl_long),
                                        opencl::cl_long, bool>>>>;

// Special type indicating that SwizzleOp should just read value from vector -
// not trying to perform any operations. Should not be called.
template <typename T> class GetOp {
public:
  using DataT = T;
  DataT getValue(size_t) const { return (DataT)0; }
  DataT operator()(DataT, DataT) { return (DataT)0; }
};

// Special type for working SwizzleOp with scalars, stores a scalar and gives
// the scalar at any index. Provides interface is compatible with SwizzleOp
// operations
template <typename T> class GetScalarOp {
public:
  using DataT = T;
  GetScalarOp(DataT Data) : m_Data(Data) {}
  DataT getValue(size_t) const { return m_Data; }

private:
  DataT m_Data;
};

template <typename T> struct EqualTo {
  constexpr rel_t<T> operator()(const T &Lhs, const T &Rhs) const {
    return (Lhs == Rhs) ? -1 : 0;
  }
};

template <typename T> struct NotEqualTo {
  constexpr rel_t<T> operator()(const T &Lhs, const T &Rhs) const {
    return (Lhs != Rhs) ? -1 : 0;
  }
};

template <typename T> struct GreaterEqualTo {
  constexpr rel_t<T> operator()(const T &Lhs, const T &Rhs) const {
    return (Lhs >= Rhs) ? -1 : 0;
  }
};

template <typename T> struct LessEqualTo {
  constexpr rel_t<T> operator()(const T &Lhs, const T &Rhs) const {
    return (Lhs <= Rhs) ? -1 : 0;
  }
};

template <typename T> struct GreaterThan {
  constexpr rel_t<T> operator()(const T &Lhs, const T &Rhs) const {
    return (Lhs > Rhs) ? -1 : 0;
  }
};

template <typename T> struct LessThan {
  constexpr rel_t<T> operator()(const T &Lhs, const T &Rhs) const {
    return (Lhs < Rhs) ? -1 : 0;
  }
};

template <typename T> struct LogicalAnd {
  constexpr rel_t<T> operator()(const T &Lhs, const T &Rhs) const {
    return (Lhs && Rhs) ? -1 : 0;
  }
};

template <typename T> struct LogicalOr {
  constexpr rel_t<T> operator()(const T &Lhs, const T &Rhs) const {
    return (Lhs || Rhs) ? -1 : 0;
  }
};

template <typename T> struct RShift {
  constexpr T operator()(const T &Lhs, const T &Rhs) const {
    return Lhs >> Rhs;
  }
};

template <typename T> struct LShift {
  constexpr T operator()(const T &Lhs, const T &Rhs) const {
    return Lhs << Rhs;
  }
};

template <typename T, typename R>
using is_int_to_int = std::integral_constant<bool, std::is_integral_v<T> &&
                                                       std::is_integral_v<R>>;

template <typename T, typename R>
using is_sint_to_sint =
    std::integral_constant<bool, is_sigeninteger<T>::value &&
                                     is_sigeninteger<R>::value>;

template <typename T, typename R>
using is_uint_to_uint =
    std::integral_constant<bool, is_sugeninteger<T>::value &&
                                     is_sugeninteger<R>::value>;

template <typename T, typename R>
using is_sint_to_from_uint = std::integral_constant<
    bool, (is_sugeninteger<T>::value && is_sigeninteger<R>::value) ||
              (is_sigeninteger<T>::value && is_sugeninteger<R>::value)>;

template <typename T, typename R>
using is_sint_to_float = std::integral_constant<
    bool, std::is_integral_v<T> &&
              !(std::is_unsigned_v<T>)&&detail::is_floating_point<R>::value>;

template <typename T, typename R>
using is_uint_to_float =
    std::integral_constant<bool, std::is_unsigned_v<T> &&
                                     detail::is_floating_point<R>::value>;

template <typename T, typename R>
using is_int_to_float =
    std::integral_constant<bool, std::is_integral_v<T> &&
                                     detail::is_floating_point<R>::value>;

template <typename T, typename R>
using is_float_to_int =
    std::integral_constant<bool, detail::is_floating_point<T>::value &&
                                     std::is_integral_v<R>>;

template <typename T, typename R>
using is_float_to_float =
    std::integral_constant<bool, detail::is_floating_point<T>::value &&
                                     detail::is_floating_point<R>::value>;
template <typename T>
using is_standard_type =
    std::integral_constant<bool, detail::is_sgentype<T>::value>;

template <typename T, typename R, rounding_mode roundingMode, typename OpenCLT,
          typename OpenCLR>
std::enable_if_t<std::is_same_v<T, R>, R> convertImpl(T Value) {
  return Value;
}

#ifndef __SYCL_DEVICE_ONLY__

// Note for float to half conversions, static_cast calls the conversion operator
// implemented for host that takes care of the precision requirements.
template <typename T, typename R, rounding_mode roundingMode, typename OpenCLT,
          typename OpenCLR>
std::enable_if_t<!std::is_same_v<T, R> && (is_int_to_int<T, R>::value ||
                                           is_int_to_float<T, R>::value ||
                                           is_float_to_float<T, R>::value),
                 R>
convertImpl(T Value) {
  return static_cast<R>(Value);
}

// float to int
template <typename T, typename R, rounding_mode roundingMode, typename OpenCLT,
          typename OpenCLR>
std::enable_if_t<is_float_to_int<T, R>::value, R> convertImpl(T Value) {
  switch (roundingMode) {
    // Round to nearest even is default rounding mode for floating-point types
  case rounding_mode::automatic:
    // Round to nearest even.
  case rounding_mode::rte: {
    int OldRoundingDirection = std::fegetround();
    int Err = std::fesetround(FE_TONEAREST);
    if (Err)
      throw sycl::exception(make_error_code(errc::runtime),
                            "Unable to set rounding mode to FE_TONEAREST");
    R Result = std::rint(Value);
    Err = std::fesetround(OldRoundingDirection);
    if (Err)
      throw sycl::exception(make_error_code(errc::runtime),
                            "Unable to restore rounding mode.");
    return Result;
  }
    // Round toward zero.
  case rounding_mode::rtz:
    return std::trunc(Value);
    // Round toward positive infinity.
  case rounding_mode::rtp:
    return std::ceil(Value);
    // Round toward negative infinity.
  case rounding_mode::rtn:
    return std::floor(Value);
  };
  assert(false && "Unsupported rounding mode!");
  return static_cast<R>(Value);
}
#else

template <rounding_mode Mode>
using RteOrAutomatic = std::bool_constant<Mode == rounding_mode::automatic ||
                                          Mode == rounding_mode::rte>;

template <rounding_mode Mode>
using Rtz = std::bool_constant<Mode == rounding_mode::rtz>;

template <rounding_mode Mode>
using Rtp = std::bool_constant<Mode == rounding_mode::rtp>;

template <rounding_mode Mode>
using Rtn = std::bool_constant<Mode == rounding_mode::rtn>;

// convert types with an equal size and diff names
template <typename T, typename R, rounding_mode roundingMode, typename OpenCLT,
          typename OpenCLR>
std::enable_if_t<
    !std::is_same<T, R>::value && std::is_same<OpenCLT, OpenCLR>::value, R>
convertImpl(T Value) {
  return static_cast<R>(Value);
}

// signed to signed
#define __SYCL_GENERATE_CONVERT_IMPL(DestType)                                 \
  template <typename T, typename R, rounding_mode roundingMode,                \
            typename OpenCLT, typename OpenCLR>                                \
  std::enable_if_t<is_sint_to_sint<T, R>::value &&                             \
                       !std::is_same<OpenCLT, OpenCLR>::value &&               \
                       (std::is_same<OpenCLR, opencl::cl_##DestType>::value || \
                        (std::is_same<OpenCLR, signed char>::value &&          \
                         std::is_same<DestType, char>::value)),                \
                   R>                                                          \
  convertImpl(T Value) {                                                       \
    OpenCLT OpValue = sycl::detail::convertDataToType<T, OpenCLT>(Value);      \
    return __spirv_SConvert##_R##DestType(OpValue);                            \
  }

__SYCL_GENERATE_CONVERT_IMPL(char)
__SYCL_GENERATE_CONVERT_IMPL(short)
__SYCL_GENERATE_CONVERT_IMPL(int)
__SYCL_GENERATE_CONVERT_IMPL(long)

#undef __SYCL_GENERATE_CONVERT_IMPL

// unsigned to unsigned
#define __SYCL_GENERATE_CONVERT_IMPL(DestType)                                 \
  template <typename T, typename R, rounding_mode roundingMode,                \
            typename OpenCLT, typename OpenCLR>                                \
  std::enable_if_t<is_uint_to_uint<T, R>::value &&                             \
                       !std::is_same<OpenCLT, OpenCLR>::value &&               \
                       std::is_same<OpenCLR, opencl::cl_##DestType>::value,    \
                   R>                                                          \
  convertImpl(T Value) {                                                       \
    OpenCLT OpValue = sycl::detail::convertDataToType<T, OpenCLT>(Value);      \
    return __spirv_UConvert##_R##DestType(OpValue);                            \
  }

__SYCL_GENERATE_CONVERT_IMPL(uchar)
__SYCL_GENERATE_CONVERT_IMPL(ushort)
__SYCL_GENERATE_CONVERT_IMPL(uint)
__SYCL_GENERATE_CONVERT_IMPL(ulong)

#undef __SYCL_GENERATE_CONVERT_IMPL

// unsigned to (from) signed
template <typename T, typename R, rounding_mode roundingMode, typename OpenCLT,
          typename OpenCLR>
std::enable_if_t<is_sint_to_from_uint<T, R>::value &&
                     is_standard_type<OpenCLT>::value &&
                     is_standard_type<OpenCLR>::value,
                 R>
convertImpl(T Value) {
  return static_cast<R>(Value);
}

// sint to float
#define __SYCL_GENERATE_CONVERT_IMPL(SPIRVOp, DestType)                        \
  template <typename T, typename R, rounding_mode roundingMode,                \
            typename OpenCLT, typename OpenCLR>                                \
  std::enable_if_t<is_sint_to_float<T, R>::value &&                            \
                       (std::is_same<OpenCLR, DestType>::value ||              \
                        (std::is_same<OpenCLR, _Float16>::value &&             \
                         std::is_same<DestType, half>::value)),                \
                   R>                                                          \
  convertImpl(T Value) {                                                       \
    OpenCLT OpValue = sycl::detail::convertDataToType<T, OpenCLT>(Value);      \
    return __spirv_Convert##SPIRVOp##_R##DestType(OpValue);                    \
  }

__SYCL_GENERATE_CONVERT_IMPL(SToF, half)
__SYCL_GENERATE_CONVERT_IMPL(SToF, float)
__SYCL_GENERATE_CONVERT_IMPL(SToF, double)

#undef __SYCL_GENERATE_CONVERT_IMPL

// uint to float
#define __SYCL_GENERATE_CONVERT_IMPL(SPIRVOp, DestType)                        \
  template <typename T, typename R, rounding_mode roundingMode,                \
            typename OpenCLT, typename OpenCLR>                                \
  std::enable_if_t<is_uint_to_float<T, R>::value &&                            \
                       (std::is_same<OpenCLR, DestType>::value ||              \
                        (std::is_same<OpenCLR, _Float16>::value &&             \
                         std::is_same<DestType, half>::value)),                \
                   R>                                                          \
  convertImpl(T Value) {                                                       \
    OpenCLT OpValue = sycl::detail::convertDataToType<T, OpenCLT>(Value);      \
    return __spirv_Convert##SPIRVOp##_R##DestType(OpValue);                    \
  }

__SYCL_GENERATE_CONVERT_IMPL(UToF, half)
__SYCL_GENERATE_CONVERT_IMPL(UToF, float)
__SYCL_GENERATE_CONVERT_IMPL(UToF, double)

#undef __SYCL_GENERATE_CONVERT_IMPL

// float to float
#define __SYCL_GENERATE_CONVERT_IMPL(DestType, RoundingMode,                   \
                                     RoundingModeCondition)                    \
  template <typename T, typename R, rounding_mode roundingMode,                \
            typename OpenCLT, typename OpenCLR>                                \
  std::enable_if_t<is_float_to_float<T, R>::value &&                           \
                       !std::is_same<OpenCLT, OpenCLR>::value &&               \
                       (std::is_same<OpenCLR, DestType>::value ||              \
                        (std::is_same<OpenCLR, _Float16>::value &&             \
                         std::is_same<DestType, half>::value)) &&              \
                       RoundingModeCondition<roundingMode>::value,             \
                   R>                                                          \
  convertImpl(T Value) {                                                       \
    OpenCLT OpValue = sycl::detail::convertDataToType<T, OpenCLT>(Value);      \
    return __spirv_FConvert##_R##DestType##_##RoundingMode(OpValue);           \
  }

#define __SYCL_GENERATE_CONVERT_IMPL_FOR_ROUNDING_MODE(RoundingMode,           \
                                                       RoundingModeCondition)  \
  __SYCL_GENERATE_CONVERT_IMPL(double, RoundingMode, RoundingModeCondition)    \
  __SYCL_GENERATE_CONVERT_IMPL(float, RoundingMode, RoundingModeCondition)     \
  __SYCL_GENERATE_CONVERT_IMPL(half, RoundingMode, RoundingModeCondition)

__SYCL_GENERATE_CONVERT_IMPL_FOR_ROUNDING_MODE(rte, RteOrAutomatic)
__SYCL_GENERATE_CONVERT_IMPL_FOR_ROUNDING_MODE(rtz, Rtz)
__SYCL_GENERATE_CONVERT_IMPL_FOR_ROUNDING_MODE(rtp, Rtp)
__SYCL_GENERATE_CONVERT_IMPL_FOR_ROUNDING_MODE(rtn, Rtn)

#undef __SYCL_GENERATE_CONVERT_IMPL_FOR_ROUNDING_MODE
#undef __SYCL_GENERATE_CONVERT_IMPL

// float to int
#define __SYCL_GENERATE_CONVERT_IMPL(SPIRVOp, DestType, RoundingMode,          \
                                     RoundingModeCondition)                    \
  template <typename T, typename R, rounding_mode roundingMode,                \
            typename OpenCLT, typename OpenCLR>                                \
  std::enable_if_t<is_float_to_int<T, R>::value &&                             \
                       (std::is_same<OpenCLR, opencl::cl_##DestType>::value || \
                        (std::is_same<OpenCLR, signed char>::value &&          \
                         std::is_same<DestType, char>::value)) &&              \
                       RoundingModeCondition<roundingMode>::value,             \
                   R>                                                          \
  convertImpl(T Value) {                                                       \
    OpenCLT OpValue = sycl::detail::convertDataToType<T, OpenCLT>(Value);      \
    return __spirv_Convert##SPIRVOp##_R##DestType##_##RoundingMode(OpValue);   \
  }

#define __SYCL_GENERATE_CONVERT_IMPL_FOR_ROUNDING_MODE(RoundingMode,           \
                                                       RoundingModeCondition)  \
  __SYCL_GENERATE_CONVERT_IMPL(FToS, int, RoundingMode, RoundingModeCondition) \
  __SYCL_GENERATE_CONVERT_IMPL(FToS, char, RoundingMode,                       \
                               RoundingModeCondition)                          \
  __SYCL_GENERATE_CONVERT_IMPL(FToS, short, RoundingMode,                      \
                               RoundingModeCondition)                          \
  __SYCL_GENERATE_CONVERT_IMPL(FToS, long, RoundingMode,                       \
                               RoundingModeCondition)                          \
  __SYCL_GENERATE_CONVERT_IMPL(FToU, uint, RoundingMode,                       \
                               RoundingModeCondition)                          \
  __SYCL_GENERATE_CONVERT_IMPL(FToU, uchar, RoundingMode,                      \
                               RoundingModeCondition)                          \
  __SYCL_GENERATE_CONVERT_IMPL(FToU, ushort, RoundingMode,                     \
                               RoundingModeCondition)                          \
  __SYCL_GENERATE_CONVERT_IMPL(FToU, ulong, RoundingMode, RoundingModeCondition)

__SYCL_GENERATE_CONVERT_IMPL_FOR_ROUNDING_MODE(rte, RteOrAutomatic)
__SYCL_GENERATE_CONVERT_IMPL_FOR_ROUNDING_MODE(rtz, Rtz)
__SYCL_GENERATE_CONVERT_IMPL_FOR_ROUNDING_MODE(rtp, Rtp)
__SYCL_GENERATE_CONVERT_IMPL_FOR_ROUNDING_MODE(rtn, Rtn)

#undef __SYCL_GENERATE_CONVERT_IMPL_FOR_ROUNDING_MODE
#undef __SYCL_GENERATE_CONVERT_IMPL

// Back up
template <typename T, typename R, rounding_mode roundingMode, typename OpenCLT,
          typename OpenCLR>
std::enable_if_t<
    ((!is_standard_type<T>::value && !is_standard_type<OpenCLT>::value) ||
     (!is_standard_type<R>::value && !is_standard_type<OpenCLR>::value)) &&
        !std::is_same<OpenCLT, OpenCLR>::value,
    R>
convertImpl(T Value) {
  return static_cast<R>(Value);
}

#endif // __SYCL_DEVICE_ONLY__

// Forward declarations
template <typename TransformedArgType, int Dims, typename KernelType>
class RoundedRangeKernel;
template <typename TransformedArgType, int Dims, typename KernelType>
class RoundedRangeKernelWithKH;

} // namespace detail

template <typename T> using vec_data = detail::vec_helper<T>;

template <typename T>
using vec_data_t = typename detail::vec_helper<T>::RetType;

/// Provides a cross-patform vector class template that works efficiently on
/// SYCL devices as well as in host C++ code.
///
/// \ingroup sycl_api
template <typename Type, int NumElements> class vec {
  using DataT = Type;

  // This represent type of underlying value. There should be only one field
  // in the class, so vec<float, 16> should be equal to float16 in memory.
  using DataType = typename detail::VecStorage<DataT, NumElements>::DataType;

  // This represents HOW  we will approach the underlying value, so as to
  // benefit from vector speed improvements
  using VectorDataType =
      typename detail::VecStorage<DataT, NumElements>::VectorDataType;

  VectorDataType &getAsVector() {
    return *reinterpret_cast<VectorDataType *>(m_Data.data());
  }

  const VectorDataType &getAsVector() const {
    return *reinterpret_cast<const VectorDataType *>(m_Data.data());
  }

  static constexpr size_t AdjustedNum = (NumElements == 3) ? 4 : NumElements;
  static constexpr size_t Sz = sizeof(DataT) * AdjustedNum;
  static constexpr bool IsSizeGreaterThanMaxAlign =
      (Sz > detail::MaxVecAlignment);

  static constexpr bool IsHostHalf =
      std::is_same<DataT, sycl::detail::half_impl::half>::value &&
      std::is_same<sycl::detail::half_impl::StorageT,
                   sycl::detail::host_half_impl::half>::value;

  // TODO: There is no support for vector half type on host yet.
  // Also, when Sz is greater than alignment, we use std::array instead of
  // vector extension. This is for MSVC compatibility, which has a max alignment
  // of 64 for direct params. If we drop MSVC, we can have alignment the same as
  // size and use vector extensions for all sizes.
  static constexpr bool IsUsingArrayOnDevice =
      (IsHostHalf || IsSizeGreaterThanMaxAlign);

#if defined(__SYCL_DEVICE_ONLY__)
  static constexpr bool NativeVec = NumElements > 1 && !IsUsingArrayOnDevice;
  static constexpr bool IsUsingArrayOnHost =
      false; // we are not compiling for host.
#else
  static constexpr bool NativeVec = false;
  static constexpr bool IsUsingArrayOnHost =
      true; // host always uses std::array.
#endif

  static constexpr int getNumElements() { return NumElements; }

  // SizeChecker is needed for vec(const argTN &... args) ctor to validate args.
  template <int Counter, int MaxValue, class...>
  struct SizeChecker : std::conditional_t<Counter == MaxValue, std::true_type,
                                          std::false_type> {};

  template <int Counter, int MaxValue, typename DataT_, class... tail>
  struct SizeChecker<Counter, MaxValue, DataT_, tail...>
      : std::conditional_t<Counter + 1 <= MaxValue,
                           SizeChecker<Counter + 1, MaxValue, tail...>,
                           std::false_type> {};

  // Utility trait for creating an std::array from an vector argument.
  template <typename DataT_, typename T, std::size_t... Is>
  static constexpr std::array<DataT_, sizeof...(Is)>
  VecToArray(const vec<T, sizeof...(Is)> &V, std::index_sequence<Is...>) {
    return {static_cast<DataT_>(V.getValue(Is))...};
  }
  template <typename DataT_, typename T, int N, typename T2, typename T3,
            template <typename> class T4, int... T5, std::size_t... Is>
  static constexpr std::array<DataT_, sizeof...(Is)>
  VecToArray(const detail::SwizzleOp<vec<T, N>, T2, T3, T4, T5...> &V,
             std::index_sequence<Is...>) {
    return {static_cast<DataT_>(V.getValue(Is))...};
  }
  template <typename DataT_, typename T, int N, typename T2, typename T3,
            template <typename> class T4, int... T5, std::size_t... Is>
  static constexpr std::array<DataT_, sizeof...(Is)>
  VecToArray(const detail::SwizzleOp<const vec<T, N>, T2, T3, T4, T5...> &V,
             std::index_sequence<Is...>) {
    return {static_cast<DataT_>(V.getValue(Is))...};
  }
  template <typename DataT_, typename T, int N>
  static constexpr std::array<DataT_, N>
  FlattenVecArgHelper(const vec<T, N> &A) {
    return VecToArray<DataT_>(A, std::make_index_sequence<N>());
  }
  template <typename DataT_, typename T, int N, typename T2, typename T3,
            template <typename> class T4, int... T5>
  static constexpr std::array<DataT_, sizeof...(T5)> FlattenVecArgHelper(
      const detail::SwizzleOp<vec<T, N>, T2, T3, T4, T5...> &A) {
    return VecToArray<DataT_>(A, std::make_index_sequence<sizeof...(T5)>());
  }
  template <typename DataT_, typename T, int N, typename T2, typename T3,
            template <typename> class T4, int... T5>
  static constexpr std::array<DataT_, sizeof...(T5)> FlattenVecArgHelper(
      const detail::SwizzleOp<const vec<T, N>, T2, T3, T4, T5...> &A) {
    return VecToArray<DataT_>(A, std::make_index_sequence<sizeof...(T5)>());
  }
  template <typename DataT_, typename T>
  static constexpr auto FlattenVecArgHelper(const T &A) {
    return std::array<DataT_, 1>{vec_data<DataT_>::get(static_cast<DataT_>(A))};
  }
  template <typename DataT_, typename T> struct FlattenVecArg {
    constexpr auto operator()(const T &A) const {
      return FlattenVecArgHelper<DataT_>(A);
    }
  };

  // Alias for shortening the vec arguments to array converter.
  template <typename DataT_, typename... ArgTN>
  using VecArgArrayCreator =
      detail::ArrayCreator<DataT_, FlattenVecArg, ArgTN...>;

#define __SYCL_ALLOW_VECTOR_SIZES(num_elements)                                \
  template <int Counter, int MaxValue, typename DataT_, class... tail>         \
  struct SizeChecker<Counter, MaxValue, vec<DataT_, num_elements>, tail...>    \
      : std::conditional_t<                                                    \
            Counter + (num_elements) <= MaxValue,                              \
            SizeChecker<Counter + (num_elements), MaxValue, tail...>,          \
            std::false_type> {};                                               \
  template <int Counter, int MaxValue, typename DataT_, typename T2,           \
            typename T3, template <typename> class T4, int... T5,              \
            class... tail>                                                     \
  struct SizeChecker<                                                          \
      Counter, MaxValue,                                                       \
      detail::SwizzleOp<vec<DataT_, num_elements>, T2, T3, T4, T5...>,         \
      tail...>                                                                 \
      : std::conditional_t<                                                    \
            Counter + sizeof...(T5) <= MaxValue,                               \
            SizeChecker<Counter + sizeof...(T5), MaxValue, tail...>,           \
            std::false_type> {};                                               \
  template <int Counter, int MaxValue, typename DataT_, typename T2,           \
            typename T3, template <typename> class T4, int... T5,              \
            class... tail>                                                     \
  struct SizeChecker<                                                          \
      Counter, MaxValue,                                                       \
      detail::SwizzleOp<const vec<DataT_, num_elements>, T2, T3, T4, T5...>,   \
      tail...>                                                                 \
      : std::conditional_t<                                                    \
            Counter + sizeof...(T5) <= MaxValue,                               \
            SizeChecker<Counter + sizeof...(T5), MaxValue, tail...>,           \
            std::false_type> {};

  __SYCL_ALLOW_VECTOR_SIZES(1)
  __SYCL_ALLOW_VECTOR_SIZES(2)
  __SYCL_ALLOW_VECTOR_SIZES(3)
  __SYCL_ALLOW_VECTOR_SIZES(4)
  __SYCL_ALLOW_VECTOR_SIZES(8)
  __SYCL_ALLOW_VECTOR_SIZES(16)
#undef __SYCL_ALLOW_VECTOR_SIZES

  // TypeChecker is needed for vec(const argTN &... args) ctor to validate args.
  template <typename T, typename DataT_>
  struct TypeChecker : std::is_convertible<T, DataT_> {};
#define __SYCL_ALLOW_VECTOR_TYPES(num_elements)                                \
  template <typename DataT_>                                                   \
  struct TypeChecker<vec<DataT_, num_elements>, DataT_> : std::true_type {};   \
  template <typename DataT_, typename T2, typename T3,                         \
            template <typename> class T4, int... T5>                           \
  struct TypeChecker<                                                          \
      detail::SwizzleOp<vec<DataT_, num_elements>, T2, T3, T4, T5...>, DataT_> \
      : std::true_type {};                                                     \
  template <typename DataT_, typename T2, typename T3,                         \
            template <typename> class T4, int... T5>                           \
  struct TypeChecker<                                                          \
      detail::SwizzleOp<const vec<DataT_, num_elements>, T2, T3, T4, T5...>,   \
      DataT_> : std::true_type {};

  __SYCL_ALLOW_VECTOR_TYPES(1)
  __SYCL_ALLOW_VECTOR_TYPES(2)
  __SYCL_ALLOW_VECTOR_TYPES(3)
  __SYCL_ALLOW_VECTOR_TYPES(4)
  __SYCL_ALLOW_VECTOR_TYPES(8)
  __SYCL_ALLOW_VECTOR_TYPES(16)
#undef __SYCL_ALLOW_VECTOR_TYPES

  template <int... Indexes>
  using Swizzle =
      detail::SwizzleOp<vec, detail::GetOp<DataT>, detail::GetOp<DataT>,
                        detail::GetOp, Indexes...>;

  template <int... Indexes>
  using ConstSwizzle =
      detail::SwizzleOp<const vec, detail::GetOp<DataT>, detail::GetOp<DataT>,
                        detail::GetOp, Indexes...>;

  // Shortcuts for args validation in vec(const argTN &... args) ctor.
  template <typename... argTN>
  using EnableIfSuitableTypes = typename std::enable_if_t<
      std::conjunction<TypeChecker<argTN, DataT>...>::value>;

  template <typename... argTN>
  using EnableIfSuitableNumElements =
      typename std::enable_if_t<SizeChecker<0, NumElements, argTN...>::value>;

  template <size_t... Is>
  constexpr vec(const std::array<vec_data_t<DataT>, NumElements> &Arr,
                std::index_sequence<Is...>)
      : m_Data{vec_data_t<DataT>(static_cast<DataT>(Arr[Is]))...} {}

public:
  using element_type = DataT;
  using rel_t = detail::rel_t<DataT>;

#ifdef __SYCL_DEVICE_ONLY__
  using vector_t = VectorDataType;
#endif

  vec() = default;

  constexpr vec(const vec &Rhs) = default;

  constexpr vec(vec &&Rhs) = default;

  constexpr vec &operator=(const vec &Rhs) = default;

  // W/o this, things like "vec<char,*> = vec<signed char, *>" doesn't work.
  template <typename Ty = DataT>
  typename std::enable_if_t<!std::is_same_v<Ty, rel_t> &&
                                std::is_convertible_v<vec_data_t<Ty>, rel_t>,
                            vec &>
  operator=(const vec<rel_t, NumElements> &Rhs) {
    *this = Rhs.template as<vec>();
    return *this;
  }

  template <typename T = void>
  using EnableIfUsingArray =
      typename std::enable_if_t<IsUsingArrayOnDevice || IsUsingArrayOnHost, T>;

  template <typename T = void>
  using EnableIfNotUsingArray =
      typename std::enable_if_t<!IsUsingArrayOnDevice && !IsUsingArrayOnHost,
                                T>;

#ifdef __SYCL_DEVICE_ONLY__
  template <typename T = void>
  using EnableIfNotHostHalf = typename std::enable_if_t<!IsHostHalf, T>;

  template <typename T = void>
  using EnableIfHostHalf = typename std::enable_if_t<IsHostHalf, T>;

  template <typename T = void>
  using EnableIfUsingArrayOnDevice =
      typename std::enable_if_t<IsUsingArrayOnDevice, T>;

  template <typename T = void>
  using EnableIfNotUsingArrayOnDevice =
      typename std::enable_if_t<!IsUsingArrayOnDevice, T>;

  template <typename Ty = DataT>
  explicit constexpr vec(const EnableIfNotUsingArrayOnDevice<Ty> &arg)
      : m_Data{DataType(vec_data<Ty>::get(arg))} {}

  template <typename Ty = DataT>
  typename std::enable_if_t<
      std::is_fundamental_v<vec_data_t<Ty>> ||
          std::is_same_v<typename std::remove_const_t<Ty>, half>,
      vec &>
  operator=(const EnableIfNotUsingArrayOnDevice<Ty> &Rhs) {
    m_Data = (DataType)vec_data<Ty>::get(Rhs);
    return *this;
  }

  template <typename Ty = DataT>
  explicit constexpr vec(const EnableIfUsingArrayOnDevice<Ty> &arg)
      : vec{detail::RepeatValue<NumElements>(
                static_cast<vec_data_t<DataT>>(arg)),
            std::make_index_sequence<NumElements>()} {}

  template <typename Ty = DataT>
  typename std::enable_if_t<
      std::is_fundamental_v<vec_data_t<Ty>> ||
          std::is_same_v<typename std::remove_const_t<Ty>, half>,
      vec &>
  operator=(const EnableIfUsingArrayOnDevice<Ty> &Rhs) {
    for (int i = 0; i < NumElements; ++i) {
      setValue(i, Rhs);
    }
    return *this;
  }
#else
  explicit constexpr vec(const DataT &arg)
      : vec{detail::RepeatValue<NumElements>(
                static_cast<vec_data_t<DataT>>(arg)),
            std::make_index_sequence<NumElements>()} {}

  template <typename Ty = DataT>
  typename std::enable_if_t<
      std::is_fundamental<vec_data_t<Ty>>::value ||
          std::is_same<typename std::remove_const_t<Ty>, half>::value,
      vec &>
  operator=(const DataT &Rhs) {
    for (int i = 0; i < NumElements; ++i) {
      setValue(i, Rhs);
    }
    return *this;
  }
#endif

#ifdef __SYCL_DEVICE_ONLY__
  // Optimized naive constructors with NumElements of DataT values.
  // We don't expect compilers to optimize vararg recursive functions well.

  // Helper type to make specific constructors available only for specific
  // number of elements.
  template <int IdxNum, typename T = void>
  using EnableIfMultipleElems = typename std::enable_if_t<
      std::is_convertible_v<T, DataT> && NumElements == IdxNum, DataT>;
  template <typename Ty = DataT>
  constexpr vec(const EnableIfMultipleElems<2, Ty> Arg0,
                const EnableIfNotUsingArrayOnDevice<Ty> Arg1)
      : m_Data{vec_data<Ty>::get(Arg0), vec_data<Ty>::get(Arg1)} {}
  template <typename Ty = DataT>
  constexpr vec(const EnableIfMultipleElems<3, Ty> Arg0,
                const EnableIfNotUsingArrayOnDevice<Ty> Arg1, const DataT Arg2)
      : m_Data{vec_data<Ty>::get(Arg0), vec_data<Ty>::get(Arg1),
               vec_data<Ty>::get(Arg2)} {}
  template <typename Ty = DataT>
  constexpr vec(const EnableIfMultipleElems<4, Ty> Arg0,
                const EnableIfNotUsingArrayOnDevice<Ty> Arg1, const DataT Arg2,
                const Ty Arg3)
      : m_Data{vec_data<Ty>::get(Arg0), vec_data<Ty>::get(Arg1),
               vec_data<Ty>::get(Arg2), vec_data<Ty>::get(Arg3)} {}
  template <typename Ty = DataT>
  constexpr vec(const EnableIfMultipleElems<8, Ty> Arg0,
                const EnableIfNotUsingArrayOnDevice<Ty> Arg1, const DataT Arg2,
                const DataT Arg3, const DataT Arg4, const DataT Arg5,
                const DataT Arg6, const DataT Arg7)
      : m_Data{vec_data<Ty>::get(Arg0), vec_data<Ty>::get(Arg1),
               vec_data<Ty>::get(Arg2), vec_data<Ty>::get(Arg3),
               vec_data<Ty>::get(Arg4), vec_data<Ty>::get(Arg5),
               vec_data<Ty>::get(Arg6), vec_data<Ty>::get(Arg7)} {}
  template <typename Ty = DataT>
  constexpr vec(const EnableIfMultipleElems<16, Ty> Arg0,
                const EnableIfNotUsingArrayOnDevice<Ty> Arg1, const DataT Arg2,
                const DataT Arg3, const DataT Arg4, const DataT Arg5,
                const DataT Arg6, const DataT Arg7, const DataT Arg8,
                const DataT Arg9, const DataT ArgA, const DataT ArgB,
                const DataT ArgC, const DataT ArgD, const DataT ArgE,
                const DataT ArgF)
      : m_Data{vec_data<Ty>::get(Arg0), vec_data<Ty>::get(Arg1),
               vec_data<Ty>::get(Arg2), vec_data<Ty>::get(Arg3),
               vec_data<Ty>::get(Arg4), vec_data<Ty>::get(Arg5),
               vec_data<Ty>::get(Arg6), vec_data<Ty>::get(Arg7),
               vec_data<Ty>::get(Arg8), vec_data<Ty>::get(Arg9),
               vec_data<Ty>::get(ArgA), vec_data<Ty>::get(ArgB),
               vec_data<Ty>::get(ArgC), vec_data<Ty>::get(ArgD),
               vec_data<Ty>::get(ArgE), vec_data<Ty>::get(ArgF)} {}
#endif

  // Constructor from values of base type or vec of base type. Checks that
  // base types are match and that the NumElements == sum of lengths of args.
  template <typename... argTN, typename = EnableIfSuitableTypes<argTN...>,
            typename = EnableIfSuitableNumElements<argTN...>>
  constexpr vec(const argTN &...args)
      : vec{VecArgArrayCreator<vec_data_t<DataT>, argTN...>::Create(args...),
            std::make_index_sequence<NumElements>()} {}

  // TODO: Remove, for debug purposes only.
  void dump() const {
#ifndef __SYCL_DEVICE_ONLY__
    for (int I = 0; I < NumElements; ++I) {
      std::cout << "  " << I << ": " << getValue(I) << std::endl;
    }
    std::cout << std::endl;
#endif // __SYCL_DEVICE_ONLY__
  }

#ifdef __SYCL_DEVICE_ONLY__
  template <typename vector_t_ = vector_t,
            typename = typename std::enable_if_t<
                std::is_same<vector_t_, vector_t>::value &&
                !std::is_same<vector_t_, DataT>::value>>
  constexpr vec(vector_t openclVector) {
    if constexpr (!IsUsingArrayOnDevice) {
      m_Data = openclVector;
    } else {
      m_Data = bit_cast<DataType>(openclVector);
    }
  }

  operator vector_t() const {
    if constexpr (!IsUsingArrayOnDevice) {
      return m_Data;
    } else {
      auto ptr = bit_cast<const VectorDataType *>((&m_Data)->data());
      return *ptr;
    }
  }
#endif

  // Available only when: NumElements == 1
  template <int N = NumElements>
  operator typename std::enable_if_t<N == 1, DataT>() const {
    return vec_data<DataT>::get(m_Data);
  }

  __SYCL2020_DEPRECATED("get_count() is deprecated, please use size() instead")
  static constexpr size_t get_count() { return size(); }
  static constexpr size_t size() noexcept { return NumElements; }
  __SYCL2020_DEPRECATED(
      "get_size() is deprecated, please use byte_size() instead")
  static constexpr size_t get_size() { return byte_size(); }
  static constexpr size_t byte_size() noexcept { return sizeof(m_Data); }

  template <typename convertT,
            rounding_mode roundingMode = rounding_mode::automatic>
  vec<convertT, NumElements> convert() const {
    static_assert(std::is_integral<vec_data_t<convertT>>::value ||
                      detail::is_floating_point<convertT>::value,
                  "Unsupported convertT");
    vec<convertT, NumElements> Result;
    using OpenCLT = detail::ConvertToOpenCLType_t<vec_data_t<DataT>>;
    using OpenCLR = detail::ConvertToOpenCLType_t<vec_data_t<convertT>>;
    for (size_t I = 0; I < NumElements; ++I) {
      Result.setValue(
          I, vec_data<convertT>::get(
                 detail::convertImpl<vec_data_t<DataT>, vec_data_t<convertT>,
                                     roundingMode, OpenCLT, OpenCLR>(
                     vec_data<DataT>::get(getValue(I)))));
    }
    return Result;
  }

  template <typename asT> asT as() const {
    static_assert((sizeof(*this) == sizeof(asT)),
                  "The new SYCL vec type must have the same storage size in "
                  "bytes as this SYCL vec");
    static_assert(
        detail::is_contained<asT, detail::gtl::vector_basic_list>::value ||
            detail::is_contained<asT, detail::gtl::vector_bool_list>::value,
        "asT must be SYCL vec of a different element type and "
        "number of elements specified by asT");
    asT Result;
    detail::memcpy(&Result.m_Data, &m_Data, sizeof(decltype(Result.m_Data)));
    return Result;
  }

  template <int... SwizzleIndexes> Swizzle<SwizzleIndexes...> swizzle() {
    return this;
  }

  template <int... SwizzleIndexes>
  ConstSwizzle<SwizzleIndexes...> swizzle() const {
    return this;
  }

  // ext_vector_type is used as an underlying type for sycl::vec on device.
  // The problem is that for clang vector types the return of operator[] is a
  // temporary and not a reference to the element in the vector. In practice
  // reinterpret_cast<DataT *>(&m_Data)[i]; is working. According to
  // http://llvm.org/docs/GetElementPtr.html#can-gep-index-into-vector-elements
  // this is not disallowed now. But could probably be disallowed in the future.
  // That is why tests are added to check that behavior of the compiler has
  // not changed.
  //
  // Implement operator [] in the same way for host and device.
  // TODO: change host side implementation when underlying type for host side
  // will be changed to std::array.
  const DataT &operator[](int i) const {
    return reinterpret_cast<const DataT *>(&m_Data)[i];
  }

  DataT &operator[](int i) { return reinterpret_cast<DataT *>(&m_Data)[i]; }

  // Begin hi/lo, even/odd, xyzw, and rgba swizzles.
private:
  // Indexer used in the swizzles.def
  // Currently it is defined as a template struct. Replacing it with a constexpr
  // function would activate a bug in MSVC that is fixed only in v19.20.
  // Until then MSVC does not recognize such constexpr functions as const and
  // thus does not let using them in template parameters inside swizzle.def.
  template <int Index> struct Indexer {
    static constexpr int value = Index;
  };

public:
#ifdef __SYCL_ACCESS_RETURN
#error "Undefine __SYCL_ACCESS_RETURN macro"
#endif
#define __SYCL_ACCESS_RETURN this
#include "swizzles.def"
#undef __SYCL_ACCESS_RETURN
  // End of hi/lo, even/odd, xyzw, and rgba swizzles.

  template <access::address_space Space, access::decorated DecorateAddress>
  void load(size_t Offset, multi_ptr<const DataT, Space, DecorateAddress> Ptr) {
    for (int I = 0; I < NumElements; I++) {
      setValue(I, *multi_ptr<const DataT, Space, DecorateAddress>(
                      Ptr + Offset * NumElements + I));
    }
  }
  template <access::address_space Space, access::decorated DecorateAddress>
  void load(size_t Offset, multi_ptr<DataT, Space, DecorateAddress> Ptr) {
    multi_ptr<const DataT, Space, DecorateAddress> ConstPtr(Ptr);
    load(Offset, ConstPtr);
  }
  template <int Dimensions, access::mode Mode,
            access::placeholder IsPlaceholder, access::target Target,
            typename PropertyListT>
  void
  load(size_t Offset,
       accessor<DataT, Dimensions, Mode, Target, IsPlaceholder, PropertyListT>
           Acc) {
    multi_ptr<const DataT, detail::TargetToAS<Target>::AS,
              access::decorated::yes>
        MultiPtr(Acc);
    load(Offset, MultiPtr);
  }
  template <access::address_space Space, access::decorated DecorateAddress>
  void store(size_t Offset,
             multi_ptr<DataT, Space, DecorateAddress> Ptr) const {
    for (int I = 0; I < NumElements; I++) {
      *multi_ptr<DataT, Space, DecorateAddress>(Ptr + Offset * NumElements +
                                                I) = getValue(I);
    }
  }
  template <int Dimensions, access::mode Mode,
            access::placeholder IsPlaceholder, access::target Target,
            typename PropertyListT>
  void
  store(size_t Offset,
        accessor<DataT, Dimensions, Mode, Target, IsPlaceholder, PropertyListT>
            Acc) {
    multi_ptr<DataT, detail::TargetToAS<Target>::AS, access::decorated::yes>
        MultiPtr(Acc);
    store(Offset, MultiPtr);
  }

  void ConvertToDataT() {
    for (size_t i = 0; i < NumElements; ++i) {
      DataT tmp = getValue(i);
      setValue(i, tmp);
    }
  }

#ifdef __SYCL_BINOP
#error "Undefine __SYCL_BINOP macro"
#endif

#ifdef __SYCL_DEVICE_ONLY__
#define __SYCL_BINOP(BINOP, OPASSIGN, CONVERT)                                 \
  template <typename Ty = vec>                                                 \
  vec operator BINOP(const EnableIfNotUsingArrayOnDevice<Ty> &Rhs) const {     \
    vec Ret;                                                                   \
    Ret.m_Data = m_Data BINOP Rhs.m_Data;                                      \
    if constexpr (std::is_same<Type, bool>::value && CONVERT) {                \
      Ret.ConvertToDataT();                                                    \
    }                                                                          \
    return Ret;                                                                \
  }                                                                            \
  template <typename Ty = vec>                                                 \
  vec operator BINOP(const EnableIfUsingArrayOnDevice<Ty> &Rhs) const {        \
    vec Ret;                                                                   \
    for (size_t I = 0; I < NumElements; ++I) {                                 \
      Ret.setValue(I, (getValue(I) BINOP Rhs.getValue(I)));                    \
    }                                                                          \
    return Ret;                                                                \
  }                                                                            \
  template <typename T>                                                        \
  typename std::enable_if_t<                                                   \
      std::is_convertible<DataT, T>::value &&                                  \
          (std::is_fundamental<vec_data_t<T>>::value ||                        \
           std::is_same<typename std::remove_const_t<T>, half>::value),        \
      vec>                                                                     \
  operator BINOP(const T &Rhs) const {                                         \
    return *this BINOP vec(static_cast<const DataT &>(Rhs));                   \
  }                                                                            \
  vec &operator OPASSIGN(const vec &Rhs) {                                     \
    *this = *this BINOP Rhs;                                                   \
    return *this;                                                              \
  }                                                                            \
  template <int Num = NumElements>                                             \
  typename std::enable_if_t<Num != 1, vec &> operator OPASSIGN(                \
      const DataT &Rhs) {                                                      \
    *this = *this BINOP vec(Rhs);                                              \
    return *this;                                                              \
  }
#else // __SYCL_DEVICE_ONLY__
#define __SYCL_BINOP(BINOP, OPASSIGN, CONVERT)                                 \
  vec operator BINOP(const vec &Rhs) const {                                   \
    vec Ret{};                                                                 \
    if constexpr (NativeVec)                                                   \
      Ret.getAsVector() = getAsVector() BINOP Rhs.getAsVector();               \
    else                                                                       \
      for (size_t I = 0; I < NumElements; ++I)                                 \
        Ret.setValue(I, (DataT)(vec_data<DataT>::get(getValue(                 \
                            I)) BINOP vec_data<DataT>::get(Rhs.getValue(I)))); \
    return Ret;                                                                \
  }                                                                            \
  template <typename T>                                                        \
  typename std::enable_if_t<                                                   \
      std::is_convertible<DataT, T>::value &&                                  \
          (std::is_fundamental<vec_data_t<T>>::value ||                        \
           std::is_same<typename std::remove_const_t<T>, half>::value),        \
      vec>                                                                     \
  operator BINOP(const T &Rhs) const {                                         \
    return *this BINOP vec(static_cast<const DataT &>(Rhs));                   \
  }                                                                            \
  vec &operator OPASSIGN(const vec &Rhs) {                                     \
    *this = *this BINOP Rhs;                                                   \
    return *this;                                                              \
  }                                                                            \
  template <int Num = NumElements>                                             \
  typename std::enable_if_t<Num != 1, vec &> operator OPASSIGN(                \
      const DataT &Rhs) {                                                      \
    *this = *this BINOP vec(Rhs);                                              \
    return *this;                                                              \
  }
#endif // __SYCL_DEVICE_ONLY__

  __SYCL_BINOP(+, +=, true)
  __SYCL_BINOP(-, -=, true)
  __SYCL_BINOP(*, *=, false)
  __SYCL_BINOP(/, /=, false)

  // TODO: The following OPs are available only when: DataT != cl_float &&
  // DataT != cl_double && DataT != cl_half
  __SYCL_BINOP(%, %=, false)
  __SYCL_BINOP(|, |=, false)
  __SYCL_BINOP(&, &=, false)
  __SYCL_BINOP(^, ^=, false)
  __SYCL_BINOP(>>, >>=, false)
  __SYCL_BINOP(<<, <<=, true)
#undef __SYCL_BINOP
#undef __SYCL_BINOP_HELP

  // Note: vec<>/SwizzleOp logical value is 0/-1 logic, as opposed to 0/1 logic.
  // As far as CTS validation is concerned, 0/-1 logic also applies when
  // NumElements is equal to one, which is somewhat inconsistent with being
  // transparent with scalar data.
  // TODO: Determine if vec<, NumElements=1> is needed at all, remove this
  // inconsistency if not by disallowing one-element vectors (as in OpenCL)

#ifdef __SYCL_RELLOGOP
#error "Undefine __SYCL_RELLOGOP macro"
#endif
// Use __SYCL_DEVICE_ONLY__ macro because cast to OpenCL vector type is defined
// by SYCL device compiler only.
#ifdef __SYCL_DEVICE_ONLY__
#define __SYCL_RELLOGOP(RELLOGOP)                                              \
  vec<rel_t, NumElements> operator RELLOGOP(const vec &Rhs) const {            \
    auto Ret =                                                                 \
        vec<rel_t, NumElements>((typename vec<rel_t, NumElements>::vector_t)(  \
            m_Data RELLOGOP Rhs.m_Data));                                      \
    if (NumElements == 1) /*Scalar 0/1 logic was applied, invert*/             \
      Ret *= -1;                                                               \
    return Ret;                                                                \
  }                                                                            \
  template <typename T>                                                        \
  typename std::enable_if_t<std::is_convertible<T, DataT>::value &&            \
                                (std::is_fundamental<vec_data_t<T>>::value ||  \
                                 std::is_same<T, half>::value),                \
                            vec<rel_t, NumElements>>                           \
  operator RELLOGOP(const T &Rhs) const {                                      \
    return *this RELLOGOP vec(static_cast<const DataT &>(Rhs));                \
  }
#else
#define __SYCL_RELLOGOP(RELLOGOP)                                              \
  vec<rel_t, NumElements> operator RELLOGOP(const vec &Rhs) const {            \
    vec<rel_t, NumElements> Ret{};                                             \
    for (size_t I = 0; I < NumElements; ++I) {                                 \
      Ret.setValue(I, -(vec_data<DataT>::get(getValue(I))                      \
                            RELLOGOP vec_data<DataT>::get(Rhs.getValue(I))));  \
    }                                                                          \
    return Ret;                                                                \
  }                                                                            \
  template <typename T>                                                        \
  typename std::enable_if_t<std::is_convertible<T, DataT>::value &&            \
                                (std::is_fundamental<vec_data_t<T>>::value ||  \
                                 std::is_same<T, half>::value),                \
                            vec<rel_t, NumElements>>                           \
  operator RELLOGOP(const T &Rhs) const {                                      \
    return *this RELLOGOP vec(static_cast<const DataT &>(Rhs));                \
  }
#endif

  __SYCL_RELLOGOP(==)
  __SYCL_RELLOGOP(!=)
  __SYCL_RELLOGOP(>)
  __SYCL_RELLOGOP(<)
  __SYCL_RELLOGOP(>=)
  __SYCL_RELLOGOP(<=)
  // TODO: limit to integral types.
  __SYCL_RELLOGOP(&&)
  __SYCL_RELLOGOP(||)
#undef __SYCL_RELLOGOP

#ifdef __SYCL_UOP
#error "Undefine __SYCL_UOP macro"
#endif
#define __SYCL_UOP(UOP, OPASSIGN)                                              \
  vec &operator UOP() {                                                        \
    *this OPASSIGN vec_data<DataT>::get(1);                                    \
    return *this;                                                              \
  }                                                                            \
  vec operator UOP(int) {                                                      \
    vec Ret(*this);                                                            \
    *this OPASSIGN vec_data<DataT>::get(1);                                    \
    return Ret;                                                                \
  }

  __SYCL_UOP(++, +=)
  __SYCL_UOP(--, -=)
#undef __SYCL_UOP

  // operator~() available only when: dataT != float && dataT != double
  // && dataT != half
  template <typename T = DataT>
  typename std::enable_if_t<!std::is_floating_point_v<vec_data_t<T>> &&
                                (!IsUsingArrayOnDevice && !IsUsingArrayOnHost),
                            vec>
  operator~() const {
    vec Ret{(typename vec::DataType) ~m_Data};
    if constexpr (std::is_same<Type, bool>::value) {
      Ret.ConvertToDataT();
    }
    return Ret;
  }
  template <typename T = DataT>
  typename std::enable_if_t<!std::is_floating_point_v<vec_data_t<T>> &&
                                (IsUsingArrayOnDevice || IsUsingArrayOnHost),
                            vec>
  operator~() const {
    vec Ret{};
    for (size_t I = 0; I < NumElements; ++I) {
      Ret.setValue(I, ~getValue(I));
    }
    return Ret;
  }

  // operator!
  template <typename T = DataT, int N = NumElements>
  EnableIfNotUsingArray<vec<T, N>> operator!() const {
    return vec<T, N>{(typename vec<DataT, NumElements>::DataType) !m_Data};
  }

  // std::byte neither supports ! unary op or casting, so special handling is
  // needed. And, worse, Windows has a conflict with 'byte'.
#if (!defined(_HAS_STD_BYTE) || _HAS_STD_BYTE != 0)
  template <typename T = DataT, int N = NumElements>
  typename std::enable_if_t<std::is_same<std::byte, T>::value &&
                                (IsUsingArrayOnDevice || IsUsingArrayOnHost),
                            vec<T, N>>
  operator!() const {
    vec Ret{};
    for (size_t I = 0; I < NumElements; ++I) {
      Ret.setValue(I, std::byte{!vec_data<DataT>::get(getValue(I))});
    }
    return Ret;
  }

  template <typename T = DataT, int N = NumElements>
  typename std::enable_if_t<!std::is_same<std::byte, T>::value &&
                                (IsUsingArrayOnDevice || IsUsingArrayOnHost),
                            vec<T, N>>
  operator!() const {
    vec Ret{};
    for (size_t I = 0; I < NumElements; ++I)
      Ret.setValue(I, !vec_data<DataT>::get(getValue(I)));
    return Ret;
  }
#else
  template <typename T = DataT, int N = NumElements>
  EnableIfUsingArray<vec<T, N>> operator!() const {
    vec Ret{};
    for (size_t I = 0; I < NumElements; ++I)
      Ret.setValue(I, !vec_data<DataT>::get(getValue(I)));
    return Ret;
  }
#endif

  // operator +
  template <typename T = vec> EnableIfNotUsingArray<T> operator+() const {
    return vec{+m_Data};
  }

  template <typename T = vec> EnableIfUsingArray<T> operator+() const {
    vec Ret{};
    for (size_t I = 0; I < NumElements; ++I)
      Ret.setValue(I, vec_data<DataT>::get(+vec_data<DataT>::get(getValue(I))));
    return Ret;
  }

  // operator -
  template <typename T = vec> EnableIfNotUsingArray<T> operator-() const {
    return vec{-m_Data};
  }

  template <typename T = vec> EnableIfUsingArray<T> operator-() const {
    vec Ret{};
    for (size_t I = 0; I < NumElements; ++I)
      Ret.setValue(I, vec_data<DataT>::get(-vec_data<DataT>::get(getValue(I))));
    return Ret;
  }

  // OP is: &&, ||
  // vec<RET, NumElements> operatorOP(const vec<DataT, NumElements> &Rhs) const;
  // vec<RET, NumElements> operatorOP(const DataT &Rhs) const;

  // OP is: ==, !=, <, >, <=, >=
  // vec<RET, NumElements> operatorOP(const vec<DataT, NumElements> &Rhs) const;
  // vec<RET, NumElements> operatorOP(const DataT &Rhs) const;
private:
  // Generic method that execute "Operation" on underlying values.
#ifdef __SYCL_DEVICE_ONLY__
  template <template <typename> class Operation,
            typename Ty = vec<DataT, NumElements>>
  vec<DataT, NumElements>
  operatorHelper(const EnableIfNotUsingArrayOnDevice<Ty> &Rhs) const {
    vec<DataT, NumElements> Result;
    Operation<DataType> Op;
    Result.m_Data = Op(m_Data, Rhs.m_Data);
    return Result;
  }

  template <template <typename> class Operation,
            typename Ty = vec<DataT, NumElements>>
  vec<DataT, NumElements>
  operatorHelper(const EnableIfUsingArrayOnDevice<Ty> &Rhs) const {
    vec<DataT, NumElements> Result;
    Operation<DataT> Op;
    for (size_t I = 0; I < NumElements; ++I) {
      Result.setValue(I, Op(Rhs.getValue(I), getValue(I)));
    }
    return Result;
  }
#else  // __SYCL_DEVICE_ONLY__
  template <template <typename> class Operation>
  vec<DataT, NumElements>
  operatorHelper(const vec<DataT, NumElements> &Rhs) const {
    vec<DataT, NumElements> Result;
    Operation<DataT> Op;
    for (size_t I = 0; I < NumElements; ++I) {
      Result.setValue(I, Op(Rhs.getValue(I), getValue(I)));
    }
    return Result;
  }
#endif // __SYCL_DEVICE_ONLY

// setValue and getValue should be able to operate on different underlying
// types: enum cl_float#N , builtin vector float#N, builtin type float.
#ifdef __SYCL_DEVICE_ONLY__
  template <int Num = NumElements, typename Ty = int,
            typename = typename std::enable_if_t<1 != Num>>
  constexpr void setValue(EnableIfNotHostHalf<Ty> Index, const DataT &Value,
                          int) {
    m_Data[Index] = vec_data<DataT>::get(Value);
  }

  template <int Num = NumElements, typename Ty = int,
            typename = typename std::enable_if_t<1 != Num>>
  constexpr DataT getValue(EnableIfNotHostHalf<Ty> Index, int) const {
    return vec_data<DataT>::get(m_Data[Index]);
  }

  template <int Num = NumElements, typename Ty = int,
            typename = typename std::enable_if_t<1 != Num>>
  constexpr void setValue(EnableIfHostHalf<Ty> Index, const DataT &Value, int) {
    m_Data.s[Index] = vec_data<DataT>::get(Value);
  }

  template <int Num = NumElements, typename Ty = int,
            typename = typename std::enable_if_t<1 != Num>>
  constexpr DataT getValue(EnableIfHostHalf<Ty> Index, int) const {
    return vec_data<DataT>::get(m_Data.s[Index]);
  }
#else  // __SYCL_DEVICE_ONLY__
  template <int Num = NumElements,
            typename = typename std::enable_if_t<1 != Num>>
  constexpr void setValue(int Index, const DataT &Value, int) {
    m_Data[Index] = vec_data<DataT>::get(Value);
  }

  template <int Num = NumElements,
            typename = typename std::enable_if_t<1 != Num>>
  constexpr DataT getValue(int Index, int) const {
    return vec_data<DataT>::get(m_Data[Index]);
  }
#endif // __SYCL_DEVICE_ONLY__

  template <int Num = NumElements,
            typename = typename std::enable_if_t<1 == Num>>
  constexpr void setValue(int, const DataT &Value, float) {
    m_Data = vec_data<DataT>::get(Value);
  }

  template <int Num = NumElements,
            typename = typename std::enable_if_t<1 == Num>>
  DataT getValue(int, float) const {
    return vec_data<DataT>::get(m_Data);
  }

  // Special proxies as specialization is not allowed in class scope.
  constexpr void setValue(int Index, const DataT &Value) {
    if (NumElements == 1)
      setValue(Index, Value, 0);
    else
      setValue(Index, Value, 0.f);
  }

  DataT getValue(int Index) const {
    return (NumElements == 1) ? getValue(Index, 0) : getValue(Index, 0.f);
  }

  // fields
  // Alignment is the same as size, to a maximum size of 64.
  // detail::vector_alignment will return that value.
  alignas(detail::vector_alignment<DataT, NumElements>::value) DataType m_Data;

  // friends
  template <typename T1, typename T2, typename T3, template <typename> class T4,
            int... T5>
  friend class detail::SwizzleOp;
  template <typename T1, int T2> friend class vec;
};

#ifdef __cpp_deduction_guides
// all compilers supporting deduction guides also support fold expressions
template <class T, class... U,
          class = std::enable_if_t<(std::is_same_v<T, U> && ...)>>
vec(T, U...) -> vec<T, sizeof...(U) + 1>;
#endif

namespace detail {

// SwizzleOP represents expression templates that operate on vec.
// Actual computation performed on conversion or assignment operators.
template <typename VecT, typename OperationLeftT, typename OperationRightT,
          template <typename> class OperationCurrentT, int... Indexes>
class SwizzleOp {
  using DataT = typename VecT::element_type;
  using CommonDataT = std::common_type_t<typename OperationLeftT::DataT,
                                         typename OperationRightT::DataT>;
  static constexpr int getNumElements() { return sizeof...(Indexes); }

  using rel_t = detail::rel_t<DataT>;
  using vec_t = vec<DataT, sizeof...(Indexes)>;
  using vec_rel_t = vec<rel_t, sizeof...(Indexes)>;

  template <typename OperationRightT_,
            template <typename> class OperationCurrentT_, int... Idx_>
  using NewLHOp = SwizzleOp<VecT,
                            SwizzleOp<VecT, OperationLeftT, OperationRightT,
                                      OperationCurrentT, Indexes...>,
                            OperationRightT_, OperationCurrentT_, Idx_...>;

  template <typename OperationRightT_,
            template <typename> class OperationCurrentT_, int... Idx_>
  using NewRelOp = SwizzleOp<vec<rel_t, VecT::getNumElements()>,
                             SwizzleOp<VecT, OperationLeftT, OperationRightT,
                                       OperationCurrentT, Indexes...>,
                             OperationRightT_, OperationCurrentT_, Idx_...>;

  template <typename OperationLeftT_,
            template <typename> class OperationCurrentT_, int... Idx_>
  using NewRHOp = SwizzleOp<VecT, OperationLeftT_,
                            SwizzleOp<VecT, OperationLeftT, OperationRightT,
                                      OperationCurrentT, Indexes...>,
                            OperationCurrentT_, Idx_...>;

  template <int IdxNum, typename T = void>
  using EnableIfOneIndex = typename std::enable_if_t<
      1 == IdxNum && SwizzleOp::getNumElements() == IdxNum, T>;

  template <int IdxNum, typename T = void>
  using EnableIfMultipleIndexes = typename std::enable_if_t<
      1 != IdxNum && SwizzleOp::getNumElements() == IdxNum, T>;

  template <typename T>
  using EnableIfScalarType = typename std::enable_if_t<
      std::is_convertible_v<DataT, T> &&
      (std::is_fundamental_v<vec_data_t<T>> ||
       std::is_same_v<typename std::remove_const_t<T>, half>)>;

  template <typename T>
  using EnableIfNoScalarType = typename std::enable_if_t<
      !std::is_convertible_v<DataT, T> ||
      !(std::is_fundamental_v<vec_data_t<T>> ||
        std::is_same_v<typename std::remove_const_t<T>, half>)>;

  template <int... Indices>
  using Swizzle =
      SwizzleOp<VecT, GetOp<DataT>, GetOp<DataT>, GetOp, Indices...>;

  template <int... Indices>
  using ConstSwizzle =
      SwizzleOp<const VecT, GetOp<DataT>, GetOp<DataT>, GetOp, Indices...>;

public:
  using element_type = DataT;

  const DataT &operator[](int i) const {
    std::array<int, getNumElements()> Idxs{Indexes...};
    return (*m_Vector)[Idxs[i]];
  }

  template <typename _T = VecT>
  std::enable_if_t<!std::is_const_v<_T>, DataT> &operator[](int i) {
    std::array<int, getNumElements()> Idxs{Indexes...};
    return (*m_Vector)[Idxs[i]];
  }

  __SYCL2020_DEPRECATED("get_count() is deprecated, please use size() instead")
  size_t get_count() const { return size(); }
  size_t size() const noexcept { return getNumElements(); }

  template <int Num = getNumElements()>
  __SYCL2020_DEPRECATED(
      "get_size() is deprecated, please use byte_size() instead")
  size_t get_size() const {
    return byte_size<Num>();
  }

  template <int Num = getNumElements()> size_t byte_size() const noexcept {
    return sizeof(DataT) * (Num == 3 ? 4 : Num);
  }

  template <typename T, int IdxNum = getNumElements(),
            typename = EnableIfOneIndex<IdxNum>,
            typename = EnableIfScalarType<T>>
  operator T() const {
    return getValue(0);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  friend NewRHOp<GetScalarOp<T>, std::multiplies, Indexes...>
  operator*(const T &Lhs, const SwizzleOp &Rhs) {
    return NewRHOp<GetScalarOp<T>, std::multiplies, Indexes...>(
        Rhs.m_Vector, GetScalarOp<T>(Lhs), Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  friend NewRHOp<GetScalarOp<T>, std::plus, Indexes...>
  operator+(const T &Lhs, const SwizzleOp &Rhs) {
    return NewRHOp<GetScalarOp<T>, std::plus, Indexes...>(
        Rhs.m_Vector, GetScalarOp<T>(Lhs), Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  friend NewRHOp<GetScalarOp<T>, std::divides, Indexes...>
  operator/(const T &Lhs, const SwizzleOp &Rhs) {
    return NewRHOp<GetScalarOp<T>, std::divides, Indexes...>(
        Rhs.m_Vector, GetScalarOp<T>(Lhs), Rhs);
  }

  // TODO: Check that Rhs arg is suitable.
#ifdef __SYCL_OPASSIGN
#error "Undefine __SYCL_OPASSIGN macro."
#endif
#define __SYCL_OPASSIGN(OPASSIGN, OP)                                          \
  SwizzleOp &operator OPASSIGN(const DataT &Rhs) {                             \
    operatorHelper<OP>(vec_t(Rhs));                                            \
    return *this;                                                              \
  }                                                                            \
  template <typename RhsOperation>                                             \
  SwizzleOp &operator OPASSIGN(const RhsOperation &Rhs) {                      \
    operatorHelper<OP>(Rhs);                                                   \
    return *this;                                                              \
  }

  __SYCL_OPASSIGN(+=, std::plus)
  __SYCL_OPASSIGN(-=, std::minus)
  __SYCL_OPASSIGN(*=, std::multiplies)
  __SYCL_OPASSIGN(/=, std::divides)
  __SYCL_OPASSIGN(%=, std::modulus)
  __SYCL_OPASSIGN(&=, std::bit_and)
  __SYCL_OPASSIGN(|=, std::bit_or)
  __SYCL_OPASSIGN(^=, std::bit_xor)
  __SYCL_OPASSIGN(>>=, RShift)
  __SYCL_OPASSIGN(<<=, LShift)
#undef __SYCL_OPASSIGN

#ifdef __SYCL_UOP
#error "Undefine __SYCL_UOP macro"
#endif
#define __SYCL_UOP(UOP, OPASSIGN)                                              \
  SwizzleOp &operator UOP() {                                                  \
    *this OPASSIGN static_cast<DataT>(1);                                      \
    return *this;                                                              \
  }                                                                            \
  vec_t operator UOP(int) {                                                    \
    vec_t Ret = *this;                                                         \
    *this OPASSIGN static_cast<DataT>(1);                                      \
    return Ret;                                                                \
  }

  __SYCL_UOP(++, +=)
  __SYCL_UOP(--, -=)
#undef __SYCL_UOP

  template <typename T = DataT>
  typename std::enable_if_t<std::is_integral_v<vec_data_t<T>>, vec_t>
  operator~() {
    vec_t Tmp = *this;
    return ~Tmp;
  }

  vec_rel_t operator!() {
    vec_t Tmp = *this;
    return !Tmp;
  }

  vec_t operator+() {
    vec_t Tmp = *this;
    return +Tmp;
  }

  vec_t operator-() {
    vec_t Tmp = *this;
    return -Tmp;
  }

  template <int IdxNum = getNumElements(),
            typename = EnableIfMultipleIndexes<IdxNum>>
  SwizzleOp &operator=(const vec<DataT, IdxNum> &Rhs) {
    std::array<int, IdxNum> Idxs{Indexes...};
    for (size_t I = 0; I < Idxs.size(); ++I) {
      m_Vector->setValue(Idxs[I], Rhs.getValue(I));
    }
    return *this;
  }

  template <int IdxNum = getNumElements(), typename = EnableIfOneIndex<IdxNum>>
  SwizzleOp &operator=(const DataT &Rhs) {
    std::array<int, IdxNum> Idxs{Indexes...};
    m_Vector->setValue(Idxs[0], Rhs);
    return *this;
  }

  template <int IdxNum = getNumElements(),
            EnableIfMultipleIndexes<IdxNum, bool> = true>
  SwizzleOp &operator=(const DataT &Rhs) {
    std::array<int, IdxNum> Idxs{Indexes...};
    for (auto Idx : Idxs) {
      m_Vector->setValue(Idx, Rhs);
    }
    return *this;
  }

  template <int IdxNum = getNumElements(), typename = EnableIfOneIndex<IdxNum>>
  SwizzleOp &operator=(DataT &&Rhs) {
    std::array<int, IdxNum> Idxs{Indexes...};
    m_Vector->setValue(Idxs[0], Rhs);
    return *this;
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewLHOp<GetScalarOp<T>, std::multiplies, Indexes...>
  operator*(const T &Rhs) const {
    return NewLHOp<GetScalarOp<T>, std::multiplies, Indexes...>(
        m_Vector, *this, GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewLHOp<RhsOperation, std::multiplies, Indexes...>
  operator*(const RhsOperation &Rhs) const {
    return NewLHOp<RhsOperation, std::multiplies, Indexes...>(m_Vector, *this,
                                                              Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewLHOp<GetScalarOp<T>, std::plus, Indexes...> operator+(const T &Rhs) const {
    return NewLHOp<GetScalarOp<T>, std::plus, Indexes...>(m_Vector, *this,
                                                          GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewLHOp<RhsOperation, std::plus, Indexes...>
  operator+(const RhsOperation &Rhs) const {
    return NewLHOp<RhsOperation, std::plus, Indexes...>(m_Vector, *this, Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewLHOp<GetScalarOp<T>, std::minus, Indexes...>
  operator-(const T &Rhs) const {
    return NewLHOp<GetScalarOp<T>, std::minus, Indexes...>(m_Vector, *this,
                                                           GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewLHOp<RhsOperation, std::minus, Indexes...>
  operator-(const RhsOperation &Rhs) const {
    return NewLHOp<RhsOperation, std::minus, Indexes...>(m_Vector, *this, Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewLHOp<GetScalarOp<T>, std::divides, Indexes...>
  operator/(const T &Rhs) const {
    return NewLHOp<GetScalarOp<T>, std::divides, Indexes...>(
        m_Vector, *this, GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewLHOp<RhsOperation, std::divides, Indexes...>
  operator/(const RhsOperation &Rhs) const {
    return NewLHOp<RhsOperation, std::divides, Indexes...>(m_Vector, *this,
                                                           Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewLHOp<GetScalarOp<T>, std::modulus, Indexes...>
  operator%(const T &Rhs) const {
    return NewLHOp<GetScalarOp<T>, std::modulus, Indexes...>(
        m_Vector, *this, GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewLHOp<RhsOperation, std::modulus, Indexes...>
  operator%(const RhsOperation &Rhs) const {
    return NewLHOp<RhsOperation, std::modulus, Indexes...>(m_Vector, *this,
                                                           Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewLHOp<GetScalarOp<T>, std::bit_and, Indexes...>
  operator&(const T &Rhs) const {
    return NewLHOp<GetScalarOp<T>, std::bit_and, Indexes...>(
        m_Vector, *this, GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewLHOp<RhsOperation, std::bit_and, Indexes...>
  operator&(const RhsOperation &Rhs) const {
    return NewLHOp<RhsOperation, std::bit_and, Indexes...>(m_Vector, *this,
                                                           Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewLHOp<GetScalarOp<T>, std::bit_or, Indexes...>
  operator|(const T &Rhs) const {
    return NewLHOp<GetScalarOp<T>, std::bit_or, Indexes...>(
        m_Vector, *this, GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewLHOp<RhsOperation, std::bit_or, Indexes...>
  operator|(const RhsOperation &Rhs) const {
    return NewLHOp<RhsOperation, std::bit_or, Indexes...>(m_Vector, *this, Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewLHOp<GetScalarOp<T>, std::bit_xor, Indexes...>
  operator^(const T &Rhs) const {
    return NewLHOp<GetScalarOp<T>, std::bit_xor, Indexes...>(
        m_Vector, *this, GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewLHOp<RhsOperation, std::bit_xor, Indexes...>
  operator^(const RhsOperation &Rhs) const {
    return NewLHOp<RhsOperation, std::bit_xor, Indexes...>(m_Vector, *this,
                                                           Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewLHOp<GetScalarOp<T>, RShift, Indexes...> operator>>(const T &Rhs) const {
    return NewLHOp<GetScalarOp<T>, RShift, Indexes...>(m_Vector, *this,
                                                       GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewLHOp<RhsOperation, RShift, Indexes...>
  operator>>(const RhsOperation &Rhs) const {
    return NewLHOp<RhsOperation, RShift, Indexes...>(m_Vector, *this, Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewLHOp<GetScalarOp<T>, LShift, Indexes...> operator<<(const T &Rhs) const {
    return NewLHOp<GetScalarOp<T>, LShift, Indexes...>(m_Vector, *this,
                                                       GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewLHOp<RhsOperation, LShift, Indexes...>
  operator<<(const RhsOperation &Rhs) const {
    return NewLHOp<RhsOperation, LShift, Indexes...>(m_Vector, *this, Rhs);
  }

  template <
      typename T1, typename T2, typename T3, template <typename> class T4,
      int... T5,
      typename = typename std::enable_if_t<sizeof...(T5) == getNumElements()>>
  SwizzleOp &operator=(const SwizzleOp<T1, T2, T3, T4, T5...> &Rhs) {
    std::array<int, getNumElements()> Idxs{Indexes...};
    for (size_t I = 0; I < Idxs.size(); ++I) {
      m_Vector->setValue(Idxs[I], Rhs.getValue(I));
    }
    return *this;
  }

  template <
      typename T1, typename T2, typename T3, template <typename> class T4,
      int... T5,
      typename = typename std::enable_if_t<sizeof...(T5) == getNumElements()>>
  SwizzleOp &operator=(SwizzleOp<T1, T2, T3, T4, T5...> &&Rhs) {
    std::array<int, getNumElements()> Idxs{Indexes...};
    for (size_t I = 0; I < Idxs.size(); ++I) {
      m_Vector->setValue(Idxs[I], Rhs.getValue(I));
    }
    return *this;
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewRelOp<GetScalarOp<T>, EqualTo, Indexes...> operator==(const T &Rhs) const {
    return NewRelOp<GetScalarOp<T>, EqualTo, Indexes...>(NULL, *this,
                                                         GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewRelOp<RhsOperation, EqualTo, Indexes...>
  operator==(const RhsOperation &Rhs) const {
    return NewRelOp<RhsOperation, EqualTo, Indexes...>(NULL, *this, Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewRelOp<GetScalarOp<T>, NotEqualTo, Indexes...>
  operator!=(const T &Rhs) const {
    return NewRelOp<GetScalarOp<T>, NotEqualTo, Indexes...>(
        NULL, *this, GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewRelOp<RhsOperation, NotEqualTo, Indexes...>
  operator!=(const RhsOperation &Rhs) const {
    return NewRelOp<RhsOperation, NotEqualTo, Indexes...>(NULL, *this, Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewRelOp<GetScalarOp<T>, GreaterEqualTo, Indexes...>
  operator>=(const T &Rhs) const {
    return NewRelOp<GetScalarOp<T>, GreaterEqualTo, Indexes...>(
        NULL, *this, GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewRelOp<RhsOperation, GreaterEqualTo, Indexes...>
  operator>=(const RhsOperation &Rhs) const {
    return NewRelOp<RhsOperation, GreaterEqualTo, Indexes...>(NULL, *this, Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewRelOp<GetScalarOp<T>, LessEqualTo, Indexes...>
  operator<=(const T &Rhs) const {
    return NewRelOp<GetScalarOp<T>, LessEqualTo, Indexes...>(
        NULL, *this, GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewRelOp<RhsOperation, LessEqualTo, Indexes...>
  operator<=(const RhsOperation &Rhs) const {
    return NewRelOp<RhsOperation, LessEqualTo, Indexes...>(NULL, *this, Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewRelOp<GetScalarOp<T>, GreaterThan, Indexes...>
  operator>(const T &Rhs) const {
    return NewRelOp<GetScalarOp<T>, GreaterThan, Indexes...>(
        NULL, *this, GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewRelOp<RhsOperation, GreaterThan, Indexes...>
  operator>(const RhsOperation &Rhs) const {
    return NewRelOp<RhsOperation, GreaterThan, Indexes...>(NULL, *this, Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewRelOp<GetScalarOp<T>, LessThan, Indexes...> operator<(const T &Rhs) const {
    return NewRelOp<GetScalarOp<T>, LessThan, Indexes...>(NULL, *this,
                                                          GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewRelOp<RhsOperation, LessThan, Indexes...>
  operator<(const RhsOperation &Rhs) const {
    return NewRelOp<RhsOperation, LessThan, Indexes...>(NULL, *this, Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewRelOp<GetScalarOp<T>, LogicalAnd, Indexes...>
  operator&&(const T &Rhs) const {
    return NewRelOp<GetScalarOp<T>, LogicalAnd, Indexes...>(
        NULL, *this, GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewRelOp<RhsOperation, LogicalAnd, Indexes...>
  operator&&(const RhsOperation &Rhs) const {
    return NewRelOp<RhsOperation, LogicalAnd, Indexes...>(NULL, *this, Rhs);
  }

  template <typename T, typename = EnableIfScalarType<T>>
  NewRelOp<GetScalarOp<T>, LogicalOr, Indexes...>
  operator||(const T &Rhs) const {
    return NewRelOp<GetScalarOp<T>, LogicalOr, Indexes...>(NULL, *this,
                                                           GetScalarOp<T>(Rhs));
  }

  template <typename RhsOperation,
            typename = EnableIfNoScalarType<RhsOperation>>
  NewRelOp<RhsOperation, LogicalOr, Indexes...>
  operator||(const RhsOperation &Rhs) const {
    return NewRelOp<RhsOperation, LogicalOr, Indexes...>(NULL, *this, Rhs);
  }

  // Begin hi/lo, even/odd, xyzw, and rgba swizzles.
private:
  // Indexer used in the swizzles.def.
  // Currently it is defined as a template struct. Replacing it with a constexpr
  // function would activate a bug in MSVC that is fixed only in v19.20.
  // Until then MSVC does not recognize such constexpr functions as const and
  // thus does not let using them in template parameters inside swizzle.def.
  template <int Index> struct Indexer {
    static constexpr int IDXs[sizeof...(Indexes)] = {Indexes...};
    static constexpr int value = IDXs[Index >= getNumElements() ? 0 : Index];
  };

public:
#ifdef __SYCL_ACCESS_RETURN
#error "Undefine __SYCL_ACCESS_RETURN macro"
#endif
#define __SYCL_ACCESS_RETURN m_Vector
#include "swizzles.def"
#undef __SYCL_ACCESS_RETURN
  // End of hi/lo, even/odd, xyzw, and rgba swizzles.

  // Leave store() interface to automatic conversion to vec<>.
  // Load to vec_t and then assign to swizzle.
  template <access::address_space Space, access::decorated DecorateAddress>
  void load(size_t offset, multi_ptr<DataT, Space, DecorateAddress> ptr) {
    vec_t Tmp;
    Tmp.template load(offset, ptr);
    *this = Tmp;
  }

  template <typename convertT, rounding_mode roundingMode>
  vec<convertT, sizeof...(Indexes)> convert() const {
    // First materialize the swizzle to vec_t and then apply convert() to it.
    vec_t Tmp = *this;
    return Tmp.template convert<convertT, roundingMode>();
  }

  template <typename asT> asT as() const {
    // First materialize the swizzle to vec_t and then apply as() to it.
    vec_t Tmp = *this;
    static_assert((sizeof(Tmp) == sizeof(asT)),
                  "The new SYCL vec type must have the same storage size in "
                  "bytes as this SYCL swizzled vec");
    static_assert(
        detail::is_contained<asT, detail::gtl::vector_basic_list>::value ||
            detail::is_contained<asT, detail::gtl::vector_bool_list>::value,
        "asT must be SYCL vec of a different element type and "
        "number of elements specified by asT");
    return Tmp.template as<asT>();
  }

private:
  SwizzleOp(const SwizzleOp &Rhs)
      : m_Vector(Rhs.m_Vector), m_LeftOperation(Rhs.m_LeftOperation),
        m_RightOperation(Rhs.m_RightOperation) {}

  SwizzleOp(VecT *Vector, OperationLeftT LeftOperation,
            OperationRightT RightOperation)
      : m_Vector(Vector), m_LeftOperation(LeftOperation),
        m_RightOperation(RightOperation) {}

  SwizzleOp(VecT *Vector) : m_Vector(Vector) {}

  SwizzleOp(SwizzleOp &&Rhs)
      : m_Vector(Rhs.m_Vector), m_LeftOperation(std::move(Rhs.m_LeftOperation)),
        m_RightOperation(std::move(Rhs.m_RightOperation)) {}

  // Either performing CurrentOperation on results of left and right operands
  // or reading values from actual vector. Perform implicit type conversion when
  // the number of elements == 1

  template <int IdxNum = getNumElements()>
  CommonDataT getValue(EnableIfOneIndex<IdxNum, size_t> Index) const {
    if (std::is_same<OperationCurrentT<DataT>, GetOp<DataT>>::value) {
      std::array<int, getNumElements()> Idxs{Indexes...};
      return m_Vector->getValue(Idxs[Index]);
    }
    auto Op = OperationCurrentT<vec_data_t<CommonDataT>>();
    return vec_data<CommonDataT>::get(
        Op(vec_data<CommonDataT>::get(m_LeftOperation.getValue(Index)),
           vec_data<CommonDataT>::get(m_RightOperation.getValue(Index))));
  }

  template <int IdxNum = getNumElements()>
  DataT getValue(EnableIfMultipleIndexes<IdxNum, size_t> Index) const {
    if (std::is_same<OperationCurrentT<DataT>, GetOp<DataT>>::value) {
      std::array<int, getNumElements()> Idxs{Indexes...};
      return m_Vector->getValue(Idxs[Index]);
    }
    auto Op = OperationCurrentT<vec_data_t<DataT>>();
    return vec_data<DataT>::get(
        Op(vec_data<DataT>::get(m_LeftOperation.getValue(Index)),
           vec_data<DataT>::get(m_RightOperation.getValue(Index))));
  }

  template <template <typename> class Operation, typename RhsOperation>
  void operatorHelper(const RhsOperation &Rhs) {
    Operation<vec_data_t<DataT>> Op;
    std::array<int, getNumElements()> Idxs{Indexes...};
    for (size_t I = 0; I < Idxs.size(); ++I) {
      DataT Res = vec_data<DataT>::get(
          Op(vec_data<DataT>::get(m_Vector->getValue(Idxs[I])),
             vec_data<DataT>::get(Rhs.getValue(I))));
      m_Vector->setValue(Idxs[I], Res);
    }
  }

  // fields
  VecT *m_Vector;

  OperationLeftT m_LeftOperation;
  OperationRightT m_RightOperation;

  // friends
  template <typename T1, int T2> friend class sycl::vec;

  template <typename T1, typename T2, typename T3, template <typename> class T4,
            int... T5>
  friend class SwizzleOp;
};
} // namespace detail

// scalar BINOP vec<>
// scalar BINOP SwizzleOp
// vec<> BINOP SwizzleOp
#ifdef __SYCL_BINOP
#error "Undefine __SYCL_BINOP macro"
#endif
#define __SYCL_BINOP(BINOP)                                                    \
  template <typename T, int Num>                                               \
  typename std::enable_if_t<                                                   \
      std::is_fundamental<vec_data_t<T>>::value ||                             \
          std::is_same<typename std::remove_const_t<T>, half>::value,          \
      vec<T, Num>>                                                             \
  operator BINOP(const T &Lhs, const vec<T, Num> &Rhs) {                       \
    return vec<T, Num>(Lhs) BINOP Rhs;                                         \
  }                                                                            \
  template <typename VecT, typename OperationLeftT, typename OperationRightT,  \
            template <typename> class OperationCurrentT, int... Indexes,       \
            typename T, typename T1 = typename VecT::element_type,             \
            int Num = sizeof...(Indexes)>                                      \
  typename std::enable_if_t<                                                   \
      std::is_convertible<T, T1>::value &&                                     \
          (std::is_fundamental<vec_data_t<T>>::value ||                        \
           std::is_same<typename std::remove_const_t<T>, half>::value),        \
      vec<T1, Num>>                                                            \
  operator BINOP(                                                              \
      const T &Lhs,                                                            \
      const detail::SwizzleOp<VecT, OperationLeftT, OperationRightT,           \
                              OperationCurrentT, Indexes...> &Rhs) {           \
    vec<T1, Num> Tmp = Rhs;                                                    \
    return Lhs BINOP Tmp;                                                      \
  }                                                                            \
  template <typename VecT, typename OperationLeftT, typename OperationRightT,  \
            template <typename> class OperationCurrentT, int... Indexes,       \
            typename T = typename VecT::element_type,                          \
            int Num = sizeof...(Indexes)>                                      \
  vec<T, Num> operator BINOP(                                                  \
      const vec<T, Num> &Lhs,                                                  \
      const detail::SwizzleOp<VecT, OperationLeftT, OperationRightT,           \
                              OperationCurrentT, Indexes...> &Rhs) {           \
    vec<T, Num> Tmp = Rhs;                                                     \
    return Lhs BINOP Tmp;                                                      \
  }

__SYCL_BINOP(+)
__SYCL_BINOP(-)
__SYCL_BINOP(*)
__SYCL_BINOP(/)
__SYCL_BINOP(%)
__SYCL_BINOP(&)
__SYCL_BINOP(|)
__SYCL_BINOP(^)
__SYCL_BINOP(>>)
__SYCL_BINOP(<<)
#undef __SYCL_BINOP

// scalar RELLOGOP vec<>
// scalar RELLOGOP SwizzleOp
// vec<> RELLOGOP SwizzleOp
#ifdef __SYCL_RELLOGOP
#error "Undefine __SYCL_RELLOGOP macro"
#endif
#define __SYCL_RELLOGOP(RELLOGOP)                                              \
  template <typename T, typename DataT, int Num>                               \
  typename std::enable_if_t<                                                   \
      std::is_convertible<T, DataT>::value &&                                  \
          (std::is_fundamental<vec_data_t<T>>::value ||                        \
           std::is_same<typename std::remove_const_t<T>, half>::value),        \
      vec<detail::rel_t<DataT>, Num>>                                          \
  operator RELLOGOP(const T &Lhs, const vec<DataT, Num> &Rhs) {                \
    return vec<T, Num>(static_cast<T>(Lhs)) RELLOGOP Rhs;                      \
  }                                                                            \
  template <typename VecT, typename OperationLeftT, typename OperationRightT,  \
            template <typename> class OperationCurrentT, int... Indexes,       \
            typename T, typename T1 = typename VecT::element_type,             \
            int Num = sizeof...(Indexes)>                                      \
  typename std::enable_if_t<                                                   \
      std::is_convertible<T, T1>::value &&                                     \
          (std::is_fundamental<vec_data_t<T>>::value ||                        \
           std::is_same<typename std::remove_const_t<T>, half>::value),        \
      vec<detail::rel_t<T1>, Num>>                                             \
  operator RELLOGOP(                                                           \
      const T &Lhs,                                                            \
      const detail::SwizzleOp<VecT, OperationLeftT, OperationRightT,           \
                              OperationCurrentT, Indexes...> &Rhs) {           \
    vec<T1, Num> Tmp = Rhs;                                                    \
    return Lhs RELLOGOP Tmp;                                                   \
  }                                                                            \
  template <typename VecT, typename OperationLeftT, typename OperationRightT,  \
            template <typename> class OperationCurrentT, int... Indexes,       \
            typename T = typename VecT::element_type,                          \
            int Num = sizeof...(Indexes)>                                      \
  vec<detail::rel_t<T>, Num> operator RELLOGOP(                                \
      const vec<T, Num> &Lhs,                                                  \
      const detail::SwizzleOp<VecT, OperationLeftT, OperationRightT,           \
                              OperationCurrentT, Indexes...> &Rhs) {           \
    vec<T, Num> Tmp = Rhs;                                                     \
    return Lhs RELLOGOP Tmp;                                                   \
  }

__SYCL_RELLOGOP(==)
__SYCL_RELLOGOP(!=)
__SYCL_RELLOGOP(>)
__SYCL_RELLOGOP(<)
__SYCL_RELLOGOP(>=)
__SYCL_RELLOGOP(<=)
// TODO: limit to integral types.
__SYCL_RELLOGOP(&&)
__SYCL_RELLOGOP(||)
#undef __SYCL_RELLOGOP

namespace detail {

// Vectors of size 1 are handled separately and therefore 1 is not included in
// the check below.
constexpr bool isValidVectorSize(int N) {
  return N == 2 || N == 3 || N == 4 || N == 8 || N == 16;
}
template <typename T, int N, typename V> struct VecStorage {
  static_assert(
      isValidVectorSize(N) || N == 1,
      "Incorrect number of elements for sycl::vec: only 1, 2, 3, 4, 8 "
      "or 16 are supported");
  static_assert(!std::is_same_v<V, void>, "Incorrect data type for sycl::vec");
};

#ifdef __SYCL_DEVICE_ONLY__
// device always has ext vector support, but for huge vectors
// we switch to std::array, so that we can use a smaller alignment (64)
// this is to support MSVC, which has a max of 64 for direct params.
template <typename T, int N> struct VecStorageImpl {
  static constexpr size_t Num = (N == 3) ? 4 : N;
  static constexpr size_t Sz = Num * sizeof(T);
  using DataType =
      typename std::conditional<Sz <= 64, T __attribute__((ext_vector_type(N))),
                                std::array<T, Num>>::type;
  using VectorDataType = T __attribute__((ext_vector_type(N)));
};
#else // __SYCL_DEVICE_ONLY__

template <typename T, int N> struct VecStorageImpl;
#define __SYCL_DEFINE_VECSTORAGE_IMPL(type, cl_type, num)                      \
  template <> struct VecStorageImpl<type, num> {                               \
    using DataType = std::array<type, (num == 3) ? 4 : num>;                   \
    using VectorDataType = ::cl_##cl_type##num;                                \
  };

#define __SYCL_DEFINE_VECSTORAGE_IMPL_FOR_TYPE(type, cl_type)                  \
  __SYCL_DEFINE_VECSTORAGE_IMPL(type, cl_type, 2)                              \
  __SYCL_DEFINE_VECSTORAGE_IMPL(type, cl_type, 3)                              \
  __SYCL_DEFINE_VECSTORAGE_IMPL(type, cl_type, 4)                              \
  __SYCL_DEFINE_VECSTORAGE_IMPL(type, cl_type, 8)                              \
  __SYCL_DEFINE_VECSTORAGE_IMPL(type, cl_type, 16)

__SYCL_DEFINE_VECSTORAGE_IMPL_FOR_TYPE(std::int8_t, char)
__SYCL_DEFINE_VECSTORAGE_IMPL_FOR_TYPE(std::int16_t, short)
__SYCL_DEFINE_VECSTORAGE_IMPL_FOR_TYPE(std::int32_t, int)
__SYCL_DEFINE_VECSTORAGE_IMPL_FOR_TYPE(std::int64_t, long)
__SYCL_DEFINE_VECSTORAGE_IMPL_FOR_TYPE(std::uint8_t, uchar)
__SYCL_DEFINE_VECSTORAGE_IMPL_FOR_TYPE(std::uint16_t, ushort)
__SYCL_DEFINE_VECSTORAGE_IMPL_FOR_TYPE(std::uint32_t, uint)
__SYCL_DEFINE_VECSTORAGE_IMPL_FOR_TYPE(std::uint64_t, ulong)
__SYCL_DEFINE_VECSTORAGE_IMPL_FOR_TYPE(float, float)
__SYCL_DEFINE_VECSTORAGE_IMPL_FOR_TYPE(double, double)

#undef __SYCL_DEFINE_VECSTORAGE_IMPL_FOR_TYPE
#undef __SYCL_DEFINE_VECSTORAGE_IMPL
#endif // __SYCL_DEVICE_ONLY__
// Single element bool
template <> struct VecStorage<bool, 1, void> {
  using DataType = bool;
  using VectorDataType = bool;
};
// Multiple element bool
template <int N>
struct VecStorage<bool, N, typename std::enable_if_t<isValidVectorSize(N)>> {
  using DataType =
      typename VecStorageImpl<select_apply_cl_t<bool, std::int8_t, std::int16_t,
                                                std::int32_t, std::int64_t>,
                              N>::DataType;
  using VectorDataType =
      typename VecStorageImpl<select_apply_cl_t<bool, std::int8_t, std::int16_t,
                                                std::int32_t, std::int64_t>,
                              N>::VectorDataType;
};
// Single element signed integers
template <typename T>
struct VecStorage<T, 1, typename std::enable_if_t<is_sigeninteger<T>::value>> {
  using DataType = select_apply_cl_t<T, std::int8_t, std::int16_t, std::int32_t,
                                     std::int64_t>;
  using VectorDataType = DataType;
};
// Single element unsigned integers
template <typename T>
struct VecStorage<T, 1, typename std::enable_if_t<is_sugeninteger<T>::value>> {
  using DataType = select_apply_cl_t<T, std::uint8_t, std::uint16_t,
                                     std::uint32_t, std::uint64_t>;
  using VectorDataType = DataType;
};
// Single element floating-point (except half)
template <typename T>
struct VecStorage<
    T, 1,
    typename std::enable_if_t<!is_half<T>::value && is_sgenfloat<T>::value>> {
  using DataType =
      select_apply_cl_t<T, std::false_type, std::false_type, float, double>;
  using VectorDataType = DataType;
};
// Multiple elements signed/unsigned integers and floating-point (except half)
template <typename T, int N>
struct VecStorage<T, N,
                  typename std::enable_if_t<isValidVectorSize(N) &&
                                            (is_sgeninteger<T>::value ||
                                             (is_sgenfloat<T>::value &&
                                              !is_half<T>::value))>> {
  using DataType =
      typename VecStorageImpl<typename VecStorage<T, 1>::DataType, N>::DataType;
  using VectorDataType =
      typename VecStorageImpl<typename VecStorage<T, 1>::DataType,
                              N>::VectorDataType;
};
// Single element half
template <> struct VecStorage<half, 1, void> {
  using DataType = sycl::detail::half_impl::StorageT;
  using VectorDataType = sycl::detail::half_impl::StorageT;
};
// Multiple elements half
#define __SYCL_DEFINE_HALF_VECSTORAGE(Num)                                     \
  template <> struct VecStorage<half, Num, void> {                             \
    using DataType = sycl::detail::half_impl::Vec##Num##StorageT;              \
    using VectorDataType = sycl::detail::half_impl::Vec##Num##StorageT;        \
  };
__SYCL_DEFINE_HALF_VECSTORAGE(2)
__SYCL_DEFINE_HALF_VECSTORAGE(3)
__SYCL_DEFINE_HALF_VECSTORAGE(4)
__SYCL_DEFINE_HALF_VECSTORAGE(8)
__SYCL_DEFINE_HALF_VECSTORAGE(16)
#undef __SYCL_DEFINE_HALF_VECSTORAGE
} // namespace detail

/// This macro must be defined to 1 when SYCL implementation allows user
/// applications to explicitly declare certain class types as device copyable
/// by adding specializations of is_device_copyable type trait class.
#define SYCL_DEVICE_COPYABLE 1

/// is_device_copyable is a user specializable class template to indicate
/// that a type T is device copyable, which means that SYCL implementation
/// may copy objects of the type T between host and device or between two
/// devices.
/// Specializing is_device_copyable such a way that
/// is_device_copyable_v<T> == true on a T that does not satisfy all
/// the requirements of a device copyable type is undefined behavior.
template <typename T, typename = void>
struct is_device_copyable : std::false_type {};

// NOTE: this specialization is a candidate for all T such that T is trivially
// copyable, including std::array<T, N>, std::optional<T>, std::variant<T>,
// sycl::marray<T> and T[N]. Thus, specializations for all these mentioned
// types are guarded by `std::enable_if_t<!std::is_trivially_copyable<...>>`
// so that they are candidates only for non-trivially-copyable types.
// Otherwise, there are several candidates and the compiler can't decide.
template <typename T>
struct is_device_copyable<T, std::enable_if_t<std::is_trivially_copyable_v<T>>>
    : std::true_type {};

template <typename T>
inline constexpr bool is_device_copyable_v = is_device_copyable<T>::value;

// std::array<T, 0> is implicitly device copyable type.
template <typename T>
struct is_device_copyable<std::array<T, 0>> : std::true_type {};

// std::array<T, N> is implicitly device copyable type if T is device copyable
template <typename T, std::size_t N>
struct is_device_copyable<
    std::array<T, N>,
    std::enable_if_t<!std::is_trivially_copyable_v<std::array<T, N>>>>
    : is_device_copyable<T> {};

// std::optional<T> is implicitly device copyable type if T is device copyable
template <typename T>
struct is_device_copyable<
    std::optional<T>,
    std::enable_if_t<!std::is_trivially_copyable_v<std::optional<T>>>>
    : is_device_copyable<T> {};

// std::pair<T1, T2> is implicitly device copyable type if T1 and T2 are device
// copyable
template <typename T1, typename T2>
struct is_device_copyable<
    std::pair<T1, T2>,
    std::enable_if_t<!std::is_trivially_copyable_v<std::pair<T1, T2>>>>
    : std::bool_constant<is_device_copyable<T1>::value &&
                         is_device_copyable<T2>::value> {};

// std::tuple<> is implicitly device copyable type.
template <> struct is_device_copyable<std::tuple<>> : std::true_type {};

// std::tuple<Ts...> is implicitly device copyable type if each type T of Ts...
// is device copyable.
template <typename T, typename... Ts>
struct is_device_copyable<
    std::tuple<T, Ts...>,
    std::enable_if_t<!std::is_trivially_copyable_v<std::tuple<T, Ts...>>>>
    : std::bool_constant<is_device_copyable<T>::value &&
                         is_device_copyable<std::tuple<Ts...>>::value> {};

// std::variant<> is implicitly device copyable type
template <> struct is_device_copyable<std::variant<>> : std::true_type {};

// std::variant<Ts...> is implicitly device copyable type if each type T of
// Ts... is device copyable
template <typename... Ts>
struct is_device_copyable<
    std::variant<Ts...>,
    std::enable_if_t<!std::is_trivially_copyable_v<std::variant<Ts...>>>>
    : std::bool_constant<(is_device_copyable<Ts>::value && ...)> {};

// marray is device copyable if element type is device copyable and it is also
// not trivially copyable (if the element type is trivially copyable, the marray
// is device copyable by default).
template <typename T, std::size_t N>
struct is_device_copyable<sycl::marray<T, N>,
                          std::enable_if_t<is_device_copyable<T>::value &&
                                           !std::is_trivially_copyable_v<T>>>
    : std::true_type {};

// array is device copyable if element type is device copyable
template <typename T, std::size_t N>
struct is_device_copyable<T[N],
                          std::enable_if_t<!std::is_trivially_copyable_v<T>>>
    : is_device_copyable<T> {};

template <typename T>
struct is_device_copyable<
    T, std::enable_if_t<!std::is_trivially_copyable_v<T> &&
                        (std::is_const_v<T> || std::is_volatile_v<T>)>>
    : is_device_copyable<std::remove_cv_t<T>> {};

namespace detail {
template <typename T, typename = void>
struct IsDeprecatedDeviceCopyable : std::false_type {};

// TODO: using C++ attribute [[deprecated]] or the macro __SYCL2020_DEPRECATED
// does not produce expected warning message for the type 'T'.
template <typename T>
struct __SYCL2020_DEPRECATED("This type isn't device copyable in SYCL 2020")
    IsDeprecatedDeviceCopyable<
        T, std::enable_if_t<std::is_trivially_copy_constructible_v<T> &&
                            std::is_trivially_destructible_v<T> &&
                            !is_device_copyable<T>::value>> : std::true_type {};

template <typename T, int N>
struct __SYCL2020_DEPRECATED("This type isn't device copyable in SYCL 2020")
    IsDeprecatedDeviceCopyable<T[N]> : IsDeprecatedDeviceCopyable<T> {};

#ifdef __SYCL_DEVICE_ONLY__
// Checks that the fields of the type T with indices 0 to (NumFieldsToCheck -
// 1) are device copyable.
template <typename T, unsigned NumFieldsToCheck>
struct CheckFieldsAreDeviceCopyable
    : CheckFieldsAreDeviceCopyable<T, NumFieldsToCheck - 1> {
  using FieldT = decltype(__builtin_field_type(T, NumFieldsToCheck - 1));
  static_assert(is_device_copyable<FieldT>::value ||
                    detail::IsDeprecatedDeviceCopyable<FieldT>::value,
                "The specified type is not device copyable");
};

template <typename T> struct CheckFieldsAreDeviceCopyable<T, 0> {};

// Checks that the base classes of the type T with indices 0 to
// (NumFieldsToCheck - 1) are device copyable.
template <typename T, unsigned NumBasesToCheck>
struct CheckBasesAreDeviceCopyable
    : CheckBasesAreDeviceCopyable<T, NumBasesToCheck - 1> {
  using BaseT = decltype(__builtin_base_type(T, NumBasesToCheck - 1));
  static_assert(is_device_copyable<BaseT>::value ||
                    detail::IsDeprecatedDeviceCopyable<BaseT>::value,
                "The specified type is not device copyable");
};

template <typename T> struct CheckBasesAreDeviceCopyable<T, 0> {};

// All the captures of a lambda or functor of type FuncT passed to a kernel
// must be is_device_copyable, which extends to bases and fields of FuncT.
// Fields are captures of lambda/functors and bases are possible base classes
// of functors also allowed by SYCL.
// The SYCL-2020 implementation must check each of the fields & bases of the
// type FuncT, only one level deep, which is enough to see if they are all
// device copyable by using the result of is_device_copyable returned for them.
// At this moment though the check also allowes using types for which
// (is_trivially_copy_constructible && is_trivially_destructible) returns true
// and (is_device_copyable) returns false. That is the deprecated behavior and
// is currently/temporarily supported only to not break older SYCL programs.
template <typename FuncT>
struct CheckDeviceCopyable
    : CheckFieldsAreDeviceCopyable<FuncT, __builtin_num_fields(FuncT)>,
      CheckBasesAreDeviceCopyable<FuncT, __builtin_num_bases(FuncT)> {};

// Below are two specializations for CheckDeviceCopyable when a kernel lambda
// is wrapped after range rounding optimization.
template <typename TransformedArgType, int Dims, typename KernelType>
struct CheckDeviceCopyable<
    RoundedRangeKernel<TransformedArgType, Dims, KernelType>>
    : CheckDeviceCopyable<KernelType> {};

template <typename TransformedArgType, int Dims, typename KernelType>
struct CheckDeviceCopyable<
    RoundedRangeKernelWithKH<TransformedArgType, Dims, KernelType>>
    : CheckDeviceCopyable<KernelType> {};

#endif // __SYCL_DEVICE_ONLY__
} // namespace detail

} // namespace _V1
} // namespace sycl

#undef __SYCL_ALIGNED_VAR
