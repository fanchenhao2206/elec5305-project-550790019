//
// Tim Fan <fanchenhao2206@icloud.com>

#pragma once

#include "IPlugConstants.h"
#include "picoDAW_Oscillator.h"
#include "picoDAW_Filter.h"
#include "picoDAW_DSPConstants.h"
#include "ADSREnvelope.h"

#include <cassert>

using namespace iplug;

class picoDAWVoice : public SynthVoice
{
public:
  picoDAWVoice()
  {
    mSineCore = new SinOscillator<sample>();

    mTriangleCore = new TriangleOscillator<sample>();
    mSawCore = new SawOscillator<sample>();
    mSquareCore = new SquareOscillator<sample>();

    mBLTriangleCore = new BLTriangleOscillator<sample>();
    mBLSawCore = new BLSawOscillator<sample>();
    mBLSquareCore = new BLSquareOscillator<sample>();

    mOSC = mSineCore; // FIXME: This is pretty buggy...
                      // Open app, press core switch, and then
                      // press play; expect to hear triangle,
                      // but instead hear sine. And press switch
                      // again, do hear saw, as expect. I'm not 
                      // sure what the solution is, but also a 
                      // very minor bug, for now, because at 
                      // least it doesn't crash the plugin!!

    mStateVariableFilter = new StateVariableFilter<sample>();
    mBiquadIIRFilter = new BiquadIIRFilter<sample>();
    mMoogLadderFilter = new MoogLadderFilter<sample>();
    mZDFLadderFilter = new ZDFLadderFilter<sample>();

    mFilter = mStateVariableFilter; // FIXME: also buggy...

  }

  void UpdateSynthCore()
  {
    if (!mAntiAliasing) {  
      if (mSynthCoreIdx == 0) {
        mOSC = mSineCore; // Non-AA cores...
      } else if (mSynthCoreIdx == 1) {
        mOSC = mTriangleCore;
      } else if (mSynthCoreIdx == 2) {
        mOSC = mSawCore;
      } else if (mSynthCoreIdx == 3) {
        mOSC = mSquareCore;
      } else {
        // TODO: throw error...
        assert(false);
      }
    } else {
      if (mSynthCoreIdx == 0) {
        mOSC = mSineCore; // No-op; Pure sine has no harmonics...
      } else if (mSynthCoreIdx == 1) {
        mOSC = mBLTriangleCore;
      } else if (mSynthCoreIdx == 2) {
        mOSC = mBLSawCore;
      } else if (mSynthCoreIdx == 3) {
        mOSC = mBLSquareCore;
      } else {
        // TODO: throw error...
        assert(false);
      }
    } 
  }

  void SetSynthCore(int synthCoreIdx) override
  {
    mSynthCoreIdx = synthCoreIdx;
    UpdateSynthCore();
  }

  void SetSynthCoreAntiAliasing(bool antiAliasing) override
  {
    mAntiAliasing = antiAliasing;
    UpdateSynthCore();
  }

  void UpdateFilterCore()
  {
    if (!mIsZDF) {  
      if (mFilterCoreIdx == 0) {
        mFilter = mBiquadIIRFilter; // Normal IIR
      } else if (mFilterCoreIdx == 1) {
        mFilter = mMoogLadderFilter; // Ladder IIR
      } else {
        // TODO: throw error...
        assert(false);
      }
    } else {
      if (mFilterCoreIdx == 0) {
        mFilter = mStateVariableFilter; // Normal ZDF
      } else if (mFilterCoreIdx == 1) {
        mFilter = mZDFLadderFilter; // Ladder ZDF
      } else {
        // TODO: throw error...
        assert(false);
      }
    } 
  }

  void SetFilterCore(int filterCoreIdx) override
  {
    mFilterCoreIdx = filterCoreIdx;
    UpdateFilterCore();
  }

  void SetFilterCoreZDF(bool isZDF) override
  {
    mIsZDF = isZDF;
    UpdateFilterCore();
  }

  void SetFilterType(int filterTypeIdx) override
  {
    mFilterTypeIdx = filterTypeIdx;
  }

  void SetFilterCutoff(double freq) override
  {
    mCutoffFreq = freq;
  }

  void SetFilterResonance(double percentage) override
  {
    mResonancePercentage = percentage;
  }

  void SetOctave(int octave) override
  {
    mOctave = octave;
  }

  bool GetBusy() const override
  {
    return mAMPEnv.GetBusy() || mFilterEnv.GetBusy();
  }

  void Trigger(double level, bool isRetrigger) override
  {
    mOSC->Reset();
    mFilter->Reset();
    
    if(isRetrigger) {
      mAMPEnv.Retrigger(level);
      mFilterEnv.Retrigger(level);
    } else {
      mAMPEnv.Start(level);
      mFilterEnv.Start(level);
    }
  }
  
  void Release() override
  {
    mAMPEnv.Release();
    mFilterEnv.Release();
  }

  void ProcessSamplesAccumulating(sample** inputs, sample** outputs, int nInputs, int nOutputs, int startIdx, int nFrames) override
  {
    double pitch = mInputs[kVoiceControlPitch].endValue;
    double osc1Freq = 440. * pow(2., pitch + static_cast<double>(mOctave));

    const double lfoDepth = 2.0;

    for (int i = startIdx; i < startIdx + nFrames; i++) {
      mFilter->SetMode(static_cast<EMode>(mFilterTypeIdx));
      mFilter->SetFreqCPS(mCutoffFreq *
        pow(2., mFilterEnv.Process(inputs[kModFilterSustainSmoother][i]) + lfoDepth * inputs[kModLFOSmoother][i]));
      mFilter->SetQ(mResonancePercentage);

      outputs[0][i] += mFilter->Process(mOSC->Process(osc1Freq) * mAMPEnv.Process(inputs[kModSustainSmoother][i]) * mGain);
      outputs[1][i] = outputs[0][i];
    }
  }

  void SetSampleRateAndBlockSize(double sampleRate, int blockSize) override
  {
    mSineCore->SetSampleRate(sampleRate);
    mTriangleCore->SetSampleRate(sampleRate);
    mSawCore->SetSampleRate(sampleRate);
    mSquareCore->SetSampleRate(sampleRate);

    mBLTriangleCore->SetSampleRate(sampleRate);
    mBLSawCore->SetSampleRate(sampleRate);
    mBLSquareCore->SetSampleRate(sampleRate);

    mStateVariableFilter->SetSampleRate(sampleRate);
    mBiquadIIRFilter->SetSampleRate(sampleRate);
    mMoogLadderFilter->SetSampleRate(sampleRate);
    mZDFLadderFilter->SetSampleRate(sampleRate);
    
    mAMPEnv.SetSampleRate(sampleRate);
    mFilterEnv.SetSampleRate(sampleRate);
  }

  void SetProgramNumber(int pgm) override
  {
    //TODO:
  }

  // this is called by the VoiceAllocator to set generic control values.
  void SetControl(int controlNumber, float value) override
  {
    //TODO:
  }

private:
  IOscillator<sample> *mSineCore; // Default core provided by IPlug...
  IOscillator<sample> *mSawCore; // Custom cores implementing IOscillator interface...
  IOscillator<sample> *mTriangleCore;
  IOscillator<sample> *mSquareCore;

  IOscillator<sample> *mBLSawCore; // Custom cores with anti-aliasing (aka band-limiting)
  IOscillator<sample> *mBLTriangleCore;
  IOscillator<sample> *mBLSquareCore;

  IFilter<sample> *mStateVariableFilter; // Default filter provided by IPlug...
  IFilter<sample> *mBiquadIIRFilter; // Custom filters implementing IFilter interface...
  IFilter<sample> *mMoogLadderFilter;
  IFilter<sample> *mZDFLadderFilter;

  int mSynthCoreIdx;
  bool mAntiAliasing;

  int mFilterCoreIdx;
  bool mIsZDF;

  int mOctave;
  int mFilterTypeIdx;
  double mCutoffFreq;
  double mResonancePercentage;
};