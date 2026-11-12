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

    mOSC = mSineCore; // FIXME: This is pretty buggy...
                      // Open app, press core switch, and then
                      // press play; expect to hear triangle,
                      // but instead hear sine. And press switch
                      // again, do hear saw, as expect. I'm not 
                      // sure what the solution is, but also a 
                      // very minor bug, for now, because at 
                      // least it doesn't crash the plugin!!

    mStateVariableFilter = new StateVariableFilter<sample>();
    // mBiquadIIRFilter = new BiquadIIRLPFilter<sample>();
    // mBilinearTransformFilter = new BilinearTransformLPFilter<sample>();

    mFilter = mStateVariableFilter; // FIXME: also buggy...

  }

  void SetSynthCore(int synthCoreIdx) override
  {
    if (synthCoreIdx == 0) {
      mOSC = mSineCore;
    } else if (synthCoreIdx == 1) {
      mOSC = mTriangleCore;
    } else if (synthCoreIdx == 2) {
      mOSC = mSawCore;
    } else if (synthCoreIdx == 3) {
      mOSC = mSquareCore;
    } else {
      // TODO: throw error...
      assert(false);
    }
  }

  void SetFilterCore(int filterCoreIdx) override
  {
    if (filterCoreIdx == 0) {
      mFilter = mStateVariableFilter;
    } else {
      // TODO: throw error...
      assert(false);
    }
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

    for (int i = startIdx; i < startIdx + nFrames; i++) {
      mFilter->SetMode(static_cast<EMode>(mFilterTypeIdx));
      mFilter->SetFreqCPS(mCutoffFreq * pow(100., mFilterEnv.Process(inputs[kModFilterSustainSmoother][i])));
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

    mStateVariableFilter->SetSampleRate(sampleRate);
    
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

  IFilter<sample> *mStateVariableFilter; // Default filter provided by IPlug...
  // IFilter<sample> *mBiquadIIRFilter; // Custom filters implementing IFilter interface...
  // IFilter<sample> *mBilinearTransformFilter;

  int mOctave;
  int mFilterTypeIdx;
  double mCutoffFreq;
  double mResonancePercentage;
};