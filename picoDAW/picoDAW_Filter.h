// Implementing custom filters...
// Tim Fan <fanchenhao2206@icloud.com>

#pragma once

#include "IPlugPlatform.h"
#include "Filter.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

BEGIN_IPLUG_NAMESPACE

template <typename T, int NC = 1>
class picoDAWFilter : public IFilter<T>
{
public:
  picoDAWFilter()
  : IFilter<T>()
  {
  }

  virtual ~picoDAWFilter() {}

  void SetFreqCPS(double freq) override { mFreq = Clip(freq, 5., 0.49 * mSampleRate); mDirty = true; }
  void SetQ(double Q) override { mQ = Clip(Q, 0.1, 100.); mDirty = true; }
  void SetGain(double gainDB) { mGain = Clip(gainDB, -36., 36.); mDirty = true; }

  void SetSampleRate(double sampleRate) override
  {
    mSampleRate = sampleRate > 0. ? sampleRate : 44100.;
    OnSampleRateChanged();
    mDirty = true;
  }

  void ProcessBlock(T** inputs, T** outputs, int nChans, int nFrames) override
  {
    assert(nChans <= NC);
    Refresh();

    for (auto c = 0; c < nChans; c++)
      for (auto s = 0; s < nFrames; s++)
        outputs[c][s] = this->Process(inputs[c][s], c);
  }

protected:
  virtual void UpdateCoefficients() = 0;
  virtual void OnSampleRateChanged() {}

  inline void Refresh()
  {
    if (mDirty)
    {
      mDirty = false;
      UpdateCoefficients();
    }
  }

  double mFreq = 1000.;
  double mQ = 0.707;
  double mGain = 0.; // dB
  double mSampleRate = 44100.;
  bool mDirty = true;
};

enum class EBiquadMode
{
  LowPass = 0,
  HighPass,
  BandPass,
  Notch,
  AllPass,
  Peaking,
  LowShelf,
  HighShelf,
  NumModes
};

template <typename T = double, int NC = 1>
class BiquadIIRFilter : public picoDAWFilter<T, NC>
{
public:
  BiquadIIRFilter(EBiquadMode mode = EBiquadMode::LowPass, double freqCPS = 1000.)
  {
    mMode = mode;
    this->mFreq = freqCPS;
  }

  void SetMode(EBiquadMode mode) { mMode = mode; this->mDirty = true; }

  void SetMode(EMode mode) override
  {
    switch (mode)
    {
      case kLowPass:       SetMode(EBiquadMode::LowPass); break;
      case kHighPass:      SetMode(EBiquadMode::HighPass); break;
      case kBandPass:      SetMode(EBiquadMode::BandPass); break;
      case kNotch:         SetMode(EBiquadMode::Notch); break;
      case kPeak:
      case kBell:          SetMode(EBiquadMode::Peaking); break;
      case kLowPassShelf:  SetMode(EBiquadMode::LowShelf); break;
      case kHighPassShelf: SetMode(EBiquadMode::HighShelf); break;
      default: break;
    }
  }

  inline T Process(T input, int channel = 0) override
  {
    assert(channel < NC);
    this->Refresh();

    const double x = static_cast<double>(input);
    const double y = mB0 * x + mZ1[channel];
    mZ1[channel] = mB1 * x - mA1 * y + mZ2[channel];
    mZ2[channel] = mB2 * x - mA2 * y;
    return static_cast<T>(y);
  }

  void Reset() override
  {
    for (auto c = 0; c < NC; c++) { mZ1[c] = 0.; mZ2[c] = 0.; }
  }

protected:
  void UpdateCoefficients() override
  {
    const double w0 = 2. * PI * this->mFreq / this->mSampleRate;
    const double cw = std::cos(w0);
    const double sw = std::sin(w0);
    const double alpha = sw / (2. * this->mQ);
    const double A = std::pow(10., this->mGain / 40.);

    double b0 = 1., b1 = 0., b2 = 0., a0 = 1., a1 = 0., a2 = 0.;

    switch (mMode)
    {
      case EBiquadMode::LowPass:
        b0 = (1. - cw) * 0.5; b1 = 1. - cw; b2 = b0;
        a0 = 1. + alpha; a1 = -2. * cw; a2 = 1. - alpha;
        break;
      case EBiquadMode::HighPass:
        b0 = (1. + cw) * 0.5; b1 = -(1. + cw); b2 = b0;
        a0 = 1. + alpha; a1 = -2. * cw; a2 = 1. - alpha;
        break;
      case EBiquadMode::BandPass: // constant 0 dB peak gain
        b0 = alpha; b1 = 0.; b2 = -alpha;
        a0 = 1. + alpha; a1 = -2. * cw; a2 = 1. - alpha;
        break;
      case EBiquadMode::Notch:
        b0 = 1.; b1 = -2. * cw; b2 = 1.;
        a0 = 1. + alpha; a1 = -2. * cw; a2 = 1. - alpha;
        break;
      case EBiquadMode::AllPass:
        b0 = 1. - alpha; b1 = -2. * cw; b2 = 1. + alpha;
        a0 = 1. + alpha; a1 = -2. * cw; a2 = 1. - alpha;
        break;
      case EBiquadMode::Peaking:
        b0 = 1. + alpha * A; b1 = -2. * cw; b2 = 1. - alpha * A;
        a0 = 1. + alpha / A; a1 = -2. * cw; a2 = 1. - alpha / A;
        break;
      case EBiquadMode::LowShelf:
      {
        const double t = 2. * std::sqrt(A) * alpha;
        b0 = A * ((A + 1.) - (A - 1.) * cw + t);
        b1 = 2. * A * ((A - 1.) - (A + 1.) * cw);
        b2 = A * ((A + 1.) - (A - 1.) * cw - t);
        a0 = (A + 1.) + (A - 1.) * cw + t;
        a1 = -2. * ((A - 1.) + (A + 1.) * cw);
        a2 = (A + 1.) + (A - 1.) * cw - t;
        break;
      }
      case EBiquadMode::HighShelf:
      {
        const double t = 2. * std::sqrt(A) * alpha;
        b0 = A * ((A + 1.) + (A - 1.) * cw + t);
        b1 = -2. * A * ((A - 1.) + (A + 1.) * cw);
        b2 = A * ((A + 1.) + (A - 1.) * cw - t);
        a0 = (A + 1.) - (A - 1.) * cw + t;
        a1 = 2. * ((A - 1.) - (A + 1.) * cw);
        a2 = (A + 1.) - (A - 1.) * cw - t;
        break;
      }
      default: break;
    }

    const double inv = 1. / a0;
    mB0 = b0 * inv; mB1 = b1 * inv; mB2 = b2 * inv;
    mA1 = a1 * inv; mA2 = a2 * inv;
  }

private:
  EBiquadMode mMode = EBiquadMode::LowPass;
  double mB0 = 1., mB1 = 0., mB2 = 0., mA1 = 0., mA2 = 0.;
  double mZ1[NC] = {};
  double mZ2[NC] = {};
};

enum class ELadderMode
{
  LowPass24 = 0,
  HighPass24,
  BandPass,
  LowPass12,
  NumModes
};

template <typename T = double, int NC = 1>
class MoogLadderFilter : public picoDAWFilter<T, NC>
{
public:
  MoogLadderFilter(ELadderMode mode = ELadderMode::LowPass24, double freqCPS = 1000.)
  {
    mMode = mode;
    this->mFreq = freqCPS;
  }

  void SetMode(ELadderMode mode) { mMode = mode; }

  void SetMode(EMode mode) override
  {
    switch (mode)
    {
      case kHighPass:
      case kHighPassShelf: SetMode(ELadderMode::HighPass24); break;
      case kBandPass:
      case kPeak:
      case kBell:          SetMode(ELadderMode::BandPass); break;
      default:             SetMode(ELadderMode::LowPass24); break;
    }
  }

  void SetDrive(double drive) { mDrive = Clip(drive, 0.1, 10.); }

  inline T Process(T input, int channel = 0) override
  {
    assert(channel < NC);
    this->Refresh();

    double* s = mS[channel];
    double* t = mT[channel];
    const double in = static_cast<double>(input);
    const double x = mDrive * in - mK * s[3];

    s[0] += mG * (std::tanh(x) - t[0]); t[0] = std::tanh(s[0]);
    s[1] += mG * (t[0] - t[1]);         t[1] = std::tanh(s[1]);
    s[2] += mG * (t[1] - t[2]);         t[2] = std::tanh(s[2]);
    s[3] += mG * (t[2] - t[3]);         t[3] = std::tanh(s[3]);

    double y = 0.;
    switch (mMode)
    {
      case ELadderMode::LowPass24:  y = s[3]; break;
      case ELadderMode::LowPass12:  y = s[1]; break;
      case ELadderMode::BandPass:   y = 2. * (s[1] - s[3]); break;
      case ELadderMode::HighPass24: y = x - 4. * s[0] + 6. * s[1] - 4. * s[2] + s[3]; break;
      default: break;
    }
    return static_cast<T>(y);
  }

  void Reset() override
  {
    for (auto c = 0; c < NC; c++)
      for (auto i = 0; i < 4; i++) { mS[c][i] = 0.; mT[c][i] = 0.; }
  }

protected:
  void UpdateCoefficients() override
  {
    mG = std::min(1. - std::exp(-2. * PI * this->mFreq / this->mSampleRate), 0.99);
    const double res = Clip((this->mQ - 0.707) / (20. - 0.707), 0., 1.);
    mK = 4.05 * res; // ~4 = self-oscillation
  }

private:
  ELadderMode mMode = ELadderMode::LowPass24;
  double mG = 0.;
  double mK = 0.;
  double mDrive = 1.;
  double mS[NC][4] = {};
  double mT[NC][4] = {}; // cached tanh of each stage
};

enum class EZDFLadderMode
{
  LowPass24 = 0,
  LowPass12,
  HighPass24,
  BandPass,
  NumModes
};
 
template <typename T = double, int NC = 1>
class ZDFLadderFilter : public picoDAWFilter<T, NC>
{
public:
  ZDFLadderFilter(EZDFLadderMode mode = EZDFLadderMode::LowPass24, double freqCPS = 1000.)
  {
    mMode = mode;
    this->mFreq = freqCPS;
  }
 
  void SetMode(EZDFLadderMode mode) { mMode = mode; }
 
  void SetMode(EMode mode) override
  {
    switch (mode)
    {
      case kHighPass:
      case kHighPassShelf: SetMode(EZDFLadderMode::HighPass24); break;
      case kBandPass:
      case kPeak:
      case kBell:          SetMode(EZDFLadderMode::BandPass); break;
      default:             SetMode(EZDFLadderMode::LowPass24); break;
    }
  }
 
  void SetDrive(double drive) { mDrive = Clip(drive, 0.1, 10.); }
 
  inline T Process(T input, int channel = 0) override
  {
    assert(channel < NC);
    this->Refresh();
 
    double* s = mS[channel];
    const double in = mDrive * static_cast<double>(input);
 
    // Closed-form solve of the feedback loop:
    //   y4 = G^4 * u + S,  u = in - k * y4
    const double S = mG3b * s[0] + mG2b * s[1] + mGb * s[2] + mB * s[3];
    const double y4lin = (mG4 * in + S) * mInvDen;
    double u = in - mK * y4lin;
    u = std::tanh(u);
 
    // Run the four TPT one-poles with the resolved input
    double y[4];
    double x = u;
    for (auto i = 0; i < 4; i++)
    {
      const double v = (x - s[i]) * mG;
      y[i] = v + s[i];
      s[i] = y[i] + v;
      x = y[i];
    }
 
    double out = 0.;
    switch (mMode)
    {
      case EZDFLadderMode::LowPass24:  out = y[3]; break;
      case EZDFLadderMode::LowPass12:  out = y[1]; break;
      case EZDFLadderMode::BandPass:   out = 2. * (y[1] - y[3]); break;
      case EZDFLadderMode::HighPass24: out = u - 4. * y[0] + 6. * y[1] - 4. * y[2] + y[3]; break;
      default: break;
    }
    return static_cast<T>(out);
  }
 
  void Reset() override
  {
    for (auto c = 0; c < NC; c++)
      for (auto i = 0; i < 4; i++) mS[c][i] = 0.;
  }
 
protected:
  void UpdateCoefficients() override
  {
    const double g = std::tan(PI * this->mFreq / this->mSampleRate);
    mG = g / (1. + g);   // one-pole gain
    mB = 1. - mG;        // state feed-through, = 1 / (1 + g)
 
    const double G2 = mG * mG;
    mG4 = G2 * G2;
    mG3b = G2 * mG * mB;
    mG2b = G2 * mB;
    mGb = mG * mB;
 
    const double res = Clip((this->mQ - 0.707) / (20. - 0.707), 0., 1.);
    mK = 4. * res; // 4 = self-oscillation (bounded by the tanh)
    mInvDen = 1. / (1. + mK * mG4);
  }
 
private:
  EZDFLadderMode mMode = EZDFLadderMode::LowPass24;
  double mG = 0., mB = 1., mG4 = 0., mG3b = 0., mG2b = 0., mGb = 0.;
  double mK = 0., mInvDen = 1.;
  double mDrive = 1.;
  double mS[NC][4] = {};
};

END_IPLUG_NAMESPACE