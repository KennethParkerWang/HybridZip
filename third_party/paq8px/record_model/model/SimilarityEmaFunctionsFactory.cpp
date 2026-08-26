#include "SimilarityEmaFunctionsFactory.hpp"

SimilarityEmaUpdateFunction SimilarityEmaFunctionsFactory::getEmaUpdateFunction(
    const Shared* const /*shared*/) {
  return &SimilarityEmaFunctions_Scalar::update_and_find;
}
