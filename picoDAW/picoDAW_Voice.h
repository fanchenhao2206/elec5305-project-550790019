//
// Tim Fan <fanchenhao2206@icloud.com>

#pragma once

#include "IPlugConstants.h"
#include "picoDAW_Oscillator.h"
#include "picoDAW_Filter.h"
#include "picoDAW_DSPConstants.h"
#include "ADSREnvelope.h"

using namespace iplug;

class picoDAW_Voice : public SynthVoice
{
public:
  picoDAW_Voice()
  {
    mSineCore = new SinOscillator<sample>();
    mTriangleCore = new TriangleOscillator<sample>();
    mSawCore = new SawOscillator<sample>();

    // mBiquadIIRLPFilter = new BiquadIIRLPFilter<sample>();
    // mBiquadIIRHPFilter = new BiquadIIRHPFilter<sample>();
    // mBiquadIIRBPFilter = new BiquadIIRBPFilter<sample>();

    // mBilinearTransformLPFilter = new BilinearTransformLPFilter<sample>();
    // mBilinearTransformHPFilter = new BilinearTransformHPFilter<sample>();
    // mBilinearTransformBPFilter = new BilinearTransformBPFilter<sample>();

    // mStateVariableFilter = new StateVariableFilter<sample>();
    // mFilter = mStateVariableFilter; FIXME: also buggy...

    mFilter = new picoDAW_Filter<sample>(); // TODO: remove

    mOSC = mSineCore; // FIXME: This is pretty buggy...
                      // Open app, press core switch, and then
                      // press play; expect to hear triangle,
                      // but instead hear sine. And press switch
                      // again, do hear saw, as expect. I'm not 
                      // sure what the solution is, but also a 
                      // very minor bug, for now, because at 
                      // least it doesn't crash the plugin!!

  }

  void SetSynthCore(int coreIdx) override
  {
    if (coreIdx == 0) {
      mOSC = mSineCore;
    } else if (coreIdx == 1) {
      mOSC = mTriangleCore;
    } else if (coreIdx == 2) {
      mOSC = mSawCore;
    }
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
    // inputs to the synthesizer can just fetch a value every block, like this:
//      double gate = mInputs[kVoiceControlGate].endValue;
    double pitch = mInputs[kVoiceControlPitch].endValue;
    double pitchBend = mInputs[kVoiceControlPitchBend].endValue;

    // or write the entire control ramp to a buffer, like this, to get sample-accurate ramps:
    mInputs[kVoiceControlTimbre].Write(mTimbreBuffer.Get(), startIdx, nFrames);
    
    // convert from "1v/oct" pitch space to frequency in Hertz
    double osc1Freq = 440. * pow(2., pitch + pitchBend + static_cast<double>(mOctave));

    for(auto i = startIdx; i < startIdx + nFrames; i++)
    {
      float noise = mTimbreBuffer.Get()[i] * Rand();
      // an MPE synth can use pressure here in addition to gain
      outputs[0][i] += (mOSC->Process(osc1Freq) + noise) * mAMPEnv.Process(inputs[kModSustainSmoother][i]) * mGain;
      outputs[1][i] = outputs[0][i];
    }
  }

  void SetSampleRateAndBlockSize(double sampleRate, int blockSize) override
  {
    mSineCore->SetSampleRate(sampleRate);
    mTriangleCore->SetSampleRate(sampleRate);
    mSawCore->SetSampleRate(sampleRate);

    // mStateVariableFilter->SetSampleRate(sampleRate); // TODO: uncomment and add more...
    
    mAMPEnv.SetSampleRate(sampleRate);
    mFilterEnv.SetSampleRate(sampleRate);
    
    mTimbreBuffer.Resize(blockSize);
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
  IOscillator<sample> *mSineCore;
  IOscillator<sample> *mSawCore; // Custom cores implementing IOscillator interface...
  IOscillator<sample> *mTriangleCore;

  picoDAW_Filter<sample> *mFilter; // picoDAW_Filter because there is no IFilter interface...

  int mOctave;

  WDL_TypedBuf<float> mTimbreBuffer;

  // noise generator for test
  uint32_t mRandSeed = 0;
  
  // return single-precision floating point number on [-1, 1]
  float Rand()
  {
    mRandSeed = mRandSeed * 0x0019660D + 0x3C6EF35F;
    uint32_t temp = ((mRandSeed >> 9) & 0x007FFFFF) | 0x3F800000;
    return (*reinterpret_cast<float*>(&temp))*2.f - 3.f;
  }

};