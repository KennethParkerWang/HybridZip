#include "OLS_factory.hpp"
#include "OLS_float_Scalar.hpp"
#include "OLS_double_Scalar.hpp"

std::unique_ptr<OLS_float> create_OLS_float(SIMDType simd, size_t n, size_t solveInterval, float lambda, float nu) {
  static_cast<void>(simd);
  return std::make_unique<OLS_float_Scalar>(n, solveInterval, lambda, nu);
}

std::unique_ptr<OLS_double> create_OLS_double(SIMDType simd, size_t n, size_t solveInterval, double lambda, double nu) {
  static_cast<void>(simd);
  return std::make_unique<OLS_double_Scalar>(n, solveInterval, lambda, nu);
}
