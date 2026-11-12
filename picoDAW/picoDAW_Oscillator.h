//
// Tim Fan <fanchenhao2206@icloud.com>

#pragma once

#include "IPlugPlatform.h"
#include "Oscillator.h"
#include <iostream>

BEGIN_IPLUG_NAMESPACE

template <typename T>
class picoDAWOscillator : public IOscillator<T>
{
public:
  picoDAWOscillator(double startPhase = 0., double startFreq = 1.)
  : IOscillator<T>(startPhase, startFreq)
  {
  }
};

template <typename T>
class TriangleOscillator : public picoDAWOscillator<T>
{
public:
  TriangleOscillator(double startPhase = 0.,
                     double startFreq = 1.)
    : picoDAWOscillator<T>(startPhase, startFreq)
  {}

  inline T Process()
  {
    auto &phase = picoDAWOscillator<T>::mPhase;

    phase += picoDAWOscillator<T>::mPhaseIncr;
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
        << picoDAWOscillator<T>::mPhaseIncr
        << " Sample rate: "
        << picoDAWOscillator<T>::mSampleRate
        << std::endl;
      counter = 0;
    }
    picoDAWOscillator<T>::SetFreqCPS(freqHz);
    return Process();
  }
};

template <typename T>
class SawOscillator : public picoDAWOscillator<T>
{
public:
  SawOscillator(double startPhase = 0.,
                double startFreq = 1.)
    : picoDAWOscillator<T>(startPhase, startFreq)
  {}

  inline T Process()
  {
    auto &phase = picoDAWOscillator<T>::mPhase;

    phase += picoDAWOscillator<T>::mPhaseIncr;
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
        << picoDAWOscillator<T>::mPhaseIncr
        << " Sample rate: "
        << picoDAWOscillator<T>::mSampleRate
        << std::endl;
      counter = 0;
    }
    picoDAWOscillator<T>::SetFreqCPS(freqHz);
    return Process();
  }
};

template <typename T>
class SquareOscillator : public picoDAWOscillator<T>
{
public:
  SquareOscillator(double startPhase = 0.,
                   double startFreq = 1.)
    : picoDAWOscillator<T>(startPhase, startFreq)
  {}

  inline T Process()
  {
    auto &phase = picoDAWOscillator<T>::mPhase;

    phase += picoDAWOscillator<T>::mPhaseIncr;
    phase -= std::floor(phase);

    return T(phase < 0.5 ? 1.0 : -1.0);
  }

  inline T Process(double freqHz) override
  {
    static int counter = 0;
    if (++counter == 1000)
    {
      std::cout
        << "SQUARE Frequency: " << freqHz
        << " Phase increment: "
        << picoDAWOscillator<T>::mPhaseIncr
        << " Sample rate: "
        << picoDAWOscillator<T>::mSampleRate
        << std::endl;
      counter = 0;
    }

    picoDAWOscillator<T>::SetFreqCPS(freqHz);
    return Process();
  }
};

END_IPLUG_NAMESPACE
