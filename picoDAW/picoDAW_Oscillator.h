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

// 2-point PolyBLEP residual for a unit-height (+/-1 => jump of 2) step.
// t  = phase in [0,1) measured from the discontinuity
// dt = phase increment per sample
inline double PolyBLEP(double t, double dt)
{
  if (t < dt)
  {
    t /= dt;
    return t + t - t * t - 1.;
  }
  else if (t > 1. - dt)
  {
    t = (t - 1.) / dt;
    return t * t + t + t + 1.;
  }
  return 0.;
}

// 2-point PolyBLAMP residual for a slope change of 1 sample-unit
// (the integral of the step residual, derived from PolyBLEP above).
// Multiply by the slope change *per sample*.
inline double PolyBLAMP(double t, double dt)
{
  if (t < dt)
  {
    const double d = 1. - t / dt;
    return d * d * d / 6.;
  }
  else if (t > 1. - dt)
  {
    const double e = 1. - (1. - t) / dt;   // = 1 - distance before the corner
    return e * e * e / 6.;
  }
  return 0.;
}

inline double Wrap(double x) { return x - std::floor(x); }

template <typename T>
class BLOscillatorBase : public picoDAWOscillator<T>
{
public:
  using Base = picoDAWOscillator<T>;
 
  BLOscillatorBase(double startPhase = 0., double startFreq = 1.)
  : Base(startPhase, startFreq)
  {
  }
 
protected:
  // advance and return the phase after the increment; dt is clamped below Nyquist
  inline double Advance(double& dt)
  {
    dt = Base::mPhaseIncr;
    if (dt > 0.49) dt = 0.49;
    else if (dt < -0.49) dt = -0.49;
    if (dt < 0.) dt = -dt; // negative frequency: treat as |f| for the correction width
 
    double& phase = Base::mPhase;
    phase += Base::mPhaseIncr;
    phase -= std::floor(phase);
    return phase;
  }
};
 
template <typename T>
class BLSawOscillator : public BLOscillatorBase<T>
{
public:
  BLSawOscillator(double startPhase = 0., double startFreq = 1.)
  : BLOscillatorBase<T>(startPhase, startFreq) {}
 
  inline T Process()
  {
    double dt;
    const double t = this->Advance(dt);
    return T(2. * t - 1. - PolyBLEP(t, dt));
  }
 
  inline T Process(double freqHz) override
  {
    this->SetFreqCPS(freqHz);
    return Process();
  }
};
 
template <typename T>
class BLSquareOscillator : public BLOscillatorBase<T>
{
public:
  BLSquareOscillator(double startPhase = 0., double startFreq = 1., double pulseWidth = 0.5)
  : BLOscillatorBase<T>(startPhase, startFreq)
  {
    SetPulseWidth(pulseWidth);
  }
 
  void SetPulseWidth(double pw) { mPW = pw < 0.01 ? 0.01 : (pw > 0.99 ? 0.99 : pw); }
 
  inline T Process()
  {
    double dt;
    const double t = this->Advance(dt);
    double y = t < mPW ? 1. : -1.;
    y += PolyBLEP(t, dt);                          // rising edge at t = 0
    y -= PolyBLEP(Wrap(t + 1. - mPW), dt); // falling edge at t = pw
    return T(y);
  }
 
  inline T Process(double freqHz) override
  {
    this->SetFreqCPS(freqHz);
    return Process();
  }
 
private:
  double mPW = 0.5;
};
 
// Triangle: only the slope is discontinuous, so correct the two corners with PolyBLAMP.
template <typename T>
class BLTriangleOscillator : public BLOscillatorBase<T>
{
public:
  BLTriangleOscillator(double startPhase = 0., double startFreq = 1.)
  : BLOscillatorBase<T>(startPhase, startFreq) {}
 
  inline T Process()
  {
    double dt;
    const double t = this->Advance(dt);
    double y = 1. - 4. * std::abs(t - 0.5);
    // slope jumps by +8*dt per sample at t=0 and by -8*dt at t=0.5
    y += 8. * dt * (PolyBLAMP(t, dt) - PolyBLAMP(Wrap(t + 0.5), dt));
    return T(y);
  }
 
  inline T Process(double freqHz) override
  {
    this->SetFreqCPS(freqHz);
    return Process();
  }
};

END_IPLUG_NAMESPACE
