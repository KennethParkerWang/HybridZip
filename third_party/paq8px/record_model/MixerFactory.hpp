#pragma once

#include "Mixer.hpp"
#include "Shared.hpp"
#include "Mixer_Scalar.hpp"

class MixerFactory
{
private:
  const Shared* const shared;
public:
  MixerFactory(const Shared* const sh);
  Mixer* createMixer(int n, int m, int s, int promoted) const;
};
