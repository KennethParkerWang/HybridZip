#include "MixerFactory.hpp"

MixerFactory::MixerFactory(const Shared* const sh) : shared(sh) {}

Mixer* MixerFactory::createMixer(const int n, const int m, const int s, const int promoted) const {
  return new Mixer_Scalar(shared, n, m, s, promoted);
}
