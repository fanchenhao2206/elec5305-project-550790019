#pragma once

#include "MidiSynth.h"
#include "IPlugConstants.h"
#include "Smoothers.h"
#include "LFO.h"
#include "SineVoice.h"
#include "picoDAW_DSPConstants.h"

using namespace iplug;

class picoDAWDSP
{
public:
  picoDAWDSP()
  {
    mSynth.AddVoice(new SineVoice(), 0); // Default sine
    mSynth.AddVoice(new SineVoice(), 0);
    mSynth.AddVoice(new SineVoice(), 0);
    mSynth.AddVoice(new SineVoice(), 0);
    // mSynth.AddVoice(new SamplerVoice(), 0); // TODO: sampler
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
    mLFO.ProcessBlock(mModulations.GetList()[kModLFO], nFrames, qnPos, transportIsRunning, tempo);

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
    mLFO.SetSampleRate(sampleRate);
    mModulationsData.Resize(blockSize * kNumModulations);
    mModulations.Empty();
    
    for(int i = 0; i < kNumModulations; i++)
    {
      mModulations.Add(mModulationsData.Get() + (blockSize * i));
    }
  }

  void ProcessMidiMsg(const IMidiMsg& msg)
  {
    mSynth.AddMidiMsgToQueue(msg);
  }

  void SetParam(int paramIdx, double value)
  {
    using EEnvStage = ADSREnvelope<sample>::EStage;
    
    switch (paramIdx) {
      case kParamNoteGlideTime:
        mSynth.SetNoteGlideTime(value / 1000.);
        break;
      case kParamGain:
        mParamsToSmooth[kModGainSmoother] = (sample) value / 100.;
        break;
      case kParamSustain:
        mParamsToSmooth[kModSustainSmoother] = (sample) value / 100.;
        break;
      case kParamAttack:
      case kParamDecay:
      case kParamRelease:
      {
        EEnvStage stage = static_cast<EEnvStage>(EEnvStage::kAttack + (paramIdx - kParamAttack));
        mSynth.ForEachVoice([stage, value](SynthVoice& voice) {
          dynamic_cast<SineVoice&>(voice).mAMPEnv.SetStageTime(stage, value);
        });
        break;
      }
      case kParamLFODepth:
        mLFO.SetScalar(value / 100.);
        break;
      case kParamLFORateTempo:
        mLFO.SetQNScalarFromDivision(static_cast<int>(value));
        break;
      case kParamLFORateHz:
        mLFO.SetFreqCPS(value);
        break;
      case kParamLFORateMode:
        mLFO.SetRateMode(value > 0.5);
        break;
      case kParamLFOShape:
        mLFO.SetShape(static_cast<int>(value));
        break;
      default:
        break;
    }
  }
  
public:
  MidiSynth mSynth { VoiceAllocator::kPolyModePoly, MidiSynth::kDefaultBlockSize };
  WDL_TypedBuf<sample> mModulationsData; // Sample data for global modulations (e.g. smoothed sustain)
  WDL_PtrList<sample> mModulations; // Ptrlist for global modulations
  LogParamSmooth<sample, kNumModulations> mParamSmoother;
  sample mParamsToSmooth[kNumModulations];
  LFO<sample> mLFO;
};
