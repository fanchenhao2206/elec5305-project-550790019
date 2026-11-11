//
// Tim Fan <fanchenhao2206@icloud.com>

#pragma once

#include "IPlugPlatform.h"
#include <iostream>

BEGIN_IPLUG_NAMESPACE

template <typename T>
class picoDAW_Oscillator : public IOscillator<T>
{
public:
  picoDAW_Oscillator(double startPhase = 0., double startFreq = 1.)
  : IOscillator<T>(startPhase, startFreq)
  {
  }
};

template <typename T>
class TriangleOscillator : public picoDAW_Oscillator<T>
{
public:
  TriangleOscillator(double startPhase = 0.,
                     double startFreq = 1.)
    : picoDAW_Oscillator<T>(startPhase, startFreq)
  {}

  inline T Process()
  {
    auto &phase = picoDAW_Oscillator<T>::mPhase;

    phase += picoDAW_Oscillator<T>::mPhaseIncr;
    phase -= std::floor(phase);

    return T(1.0 - 4.0 * std::abs(phase - 0.5));
  }

  inline T Process(double freqHz) override
  {
    static int counter = 0;
    if (++counter == 1000)
    {
      std::cout
        << "TRIANGLE Frequency: " << freqHz
        << " Phase increment: "
        << picoDAW_Oscillator<T>::mPhaseIncr
        << " Sample rate: "
        << picoDAW_Oscillator<T>::mSampleRate
        << std::endl;
      counter = 0;
    }
    picoDAW_Oscillator<T>::SetFreqCPS(freqHz);
    return Process();
  }
};

template <typename T>
class SawOscillator : public picoDAW_Oscillator<T>
{
public:
  SawOscillator(double startPhase = 0.,
                double startFreq = 1.)
    : picoDAW_Oscillator<T>(startPhase, startFreq)
  {}

  inline T Process()
  {
    auto &phase = picoDAW_Oscillator<T>::mPhase;

    phase += picoDAW_Oscillator<T>::mPhaseIncr;
    phase -= std::floor(phase);

    return T(2.0 * phase - 1.0);
  }

  inline T Process(double freqHz) override
  {
    static int counter = 0;
    if (++counter == 1000)
    {
      std::cout
        << "SAW Frequency: " << freqHz
        << " Phase increment: "
        << picoDAW_Oscillator<T>::mPhaseIncr
        << " Sample rate: "
        << picoDAW_Oscillator<T>::mSampleRate
        << std::endl;
      counter = 0;
    }
    picoDAW_Oscillator<T>::SetFreqCPS(freqHz);
    return Process();
  }
};

END_IPLUG_NAMESPACE
