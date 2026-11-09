#pragma once

#include "MidiSynth.h"
#include "IPlugConstants.h"
#include "Smoothers.h"
#include "LFO.h"
#include "picoDAWVoice.h"
#include "picoDAW_DSPConstants.h"

using namespace iplug;

class picoDAWDSP
{
public:
  picoDAWDSP()
  {
    mSynth.AddVoice(new picoDAWVoice(), 0);
    // mSynth.AddVoice(new picoDAWVoice(), 0);
    // mSynth.AddVoice(new picoDAWVoice(), 0);
    // mSynth.AddVoice(new picoDAWVoice(), 0);
    // mSynth.AddVoice(new SamplerVoice(), 0); // 2206: sampler
    // mSynth.AddVoice(new SamplerVoice(), 0);
  }

  void ProcessBlock(sample** inputs, sample** outputs, int nOutputs, int nFrames, double qnPos = 0., bool transportIsRunning = false, double tempo = 120.)
  {
    // clear outputs
    for(auto i = 0; i < nOutputs; i++)
    {
      memset(outputs[i], 0, nFrames * sizeof(sample));
    }

    // 2206: now add signals into outputs; e.g. outputs[0][s] += sig1[s] + sig2[s];
    // where sig1 is the output from the synth1, and sig2 for synth2; making a mix
    
    mParamSmoother.ProcessBlock(mParamsToSmooth, mModulations.GetList(), nFrames); 
    // We give the paramSmoother the paramsToSmooth array that we built; specifically the sustain and gain values...

    // 2206: Clearly this fills in outputs with the final audio-ready output, 
    // but what actually plays back that audio? answer, probably javascript
    // tldr, midiSynth generates the outputs, it contains the algorithm for 
    // generating e.g. sine waves wrt midi notes
    mSynth.ProcessBlock(mModulations.GetList(), outputs, 0, nOutputs, nFrames);
    
    for(int s=0; s < nFrames;s++)
    {
      sample smoothedGain = mModulations.GetList()[kModGainSmoother][s];
      outputs[0][s] *= smoothedGain;
      outputs[1][s] *= smoothedGain;
    }
  }

  void Reset(double sampleRate, int blockSize)
  {
    mSynth.SetSampleRateAndBlockSize(sampleRate, blockSize);
    mSynth.Reset();
    mModulationsData.Resize(blockSize * kNumModulations);
    mModulations.Empty();
    
    for(int i = 0; i < kNumModulations; i++)
    {
      mModulations.Add(mModulationsData.Get() + (blockSize * i));
    }

    // Gain default, as in mixer...
    mParamsToSmooth[kModGainSmoother] = (sample) 100. / 100.;

    // ADSR defaults, as in editor...
    sample attack_value = 10.;         // Attack default (ms)
    sample decay_value = 10.;          // Decay default (ms)
    sample sustain_value = 50. / 100.; // Sustain default (%)
    sample release_value = 10.;        // Release default (ms)

    for (int i = 0; i < 6; i++) {
      auto voice = mSynth.GetVoice(i);
      voice->mAMPEnv.SetStageTime(ADSREnvelope<sample>::EStage::kAttack, attack_value); 
      voice->mAMPEnv.SetStageTime(ADSREnvelope<sample>::EStage::kDecay, decay_value);
      mParamsToSmooth[kModSustainSmoother] = sustain_value; // The sustain value to use for the next ProcessBlock,
                                                            // then passed to mParamSmoother who does magic... and
                                                            // generates the mModulations.GetList() that contains 
                                                            // the final individual frame-by-frame sustain values
      voice->mAMPEnv.SetStageTime(ADSREnvelope<sample>::EStage::kRelease, release_value);
    }
  }

  void ProcessMidiMsg(const IMidiMsg& msg)
  {
    mSynth.AddMidiMsgToQueue(msg);
  }
  
public:
  MidiSynth mSynth { VoiceAllocator::kPolyModeMono, MidiSynth::kDefaultBlockSize };
  WDL_TypedBuf<sample> mModulationsData; // Sample data for global modulations (e.g. smoothed sustain)
  WDL_PtrList<sample> mModulations; // Ptrlist for global modulations
  LogParamSmooth<sample, kNumModulations> mParamSmoother;
  sample mParamsToSmooth[kNumModulations];
};
