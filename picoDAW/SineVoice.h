#pragma once

#include "IPlugConstants.h"
#include "Oscillator.h"
#include "ADSREnvelope.h"
#include "picoDAW_DSPConstants.h"

using namespace iplug;

class SineVoice : public SynthVoice
{
public:
  SineVoice()
  {
    // Make sure mOSC is a fastSinOsc
    mOSC = new FastSinOscillator<sample>();
  }

  bool GetBusy() const override
  {
    return mAMPEnv.GetBusy();
  }

  void Trigger(double level, bool isRetrigger) override
  {
    mOSC->Reset();
    
    if(isRetrigger)
      mAMPEnv.Retrigger(level);
    else
      mAMPEnv.Start(level);
  }
  
  void Release() override
  {
    mAMPEnv.Release();
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
    double osc1Freq = 440. * pow(2., pitch + pitchBend + inputs[kModLFO][0]);

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
    mOSC->SetSampleRate(sampleRate);
    mAMPEnv.SetSampleRate(sampleRate);
    
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