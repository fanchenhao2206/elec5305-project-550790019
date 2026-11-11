#pragma once

#include "MidiSynth.h"
#include "IPlugConstants.h"
#include "Smoothers.h"
#include "LFO.h"
#include "picoDAW_Voice.h"
#include "picoDAW_DSPConstants.h"

using namespace iplug;

class picoDAWDSP
{
public:
  picoDAWDSP() : mNVoices(1) // Just one voice for now...
  {
    mSynth.AddVoice(new picoDAWVoice(), 0);
    // mSynth.AddVoice(new picoDAWVoice(), 0);
    // mSynth.AddVoice(new picoDAWVoice(), 0);
    // mSynth.AddVoice(new picoDAWVoice(), 0);
    // mSynth.AddVoice(new SamplerVoice(), 0); // TODO: sampler
    // mSynth.AddVoice(new SamplerVoice(), 0);
  }

  void ProcessBlock(sample** inputs, sample** outputs, int nOutputs, int nFrames, double qnPos = 0., bool transportIsRunning = false, double tempo = 120.)
  {
    // TODO: add signals into outputs; e.g. outputs[0][s] += sig1[s] + sig2[s];
    // where sig1 is the output from the synth1, and sig2 for synth2; making a mix

    // Clear outputs, just in case not already empty...
    for(auto i = 0; i < nOutputs; i++) {
      memset(outputs[i], 0, nFrames * sizeof(sample));
    }

    // Inputs is empty; picoDAWDSP is the first DSP plugin in the chain!!

    // First job is to generate the individual frame-by-frame smoothed gain and sustain values,
    // from the single gain and sustain values given to us in mParamsToSmooth via UI updates etc.
    mParamSmoother.ProcessBlock(mParamsToSmooth, mModulations.GetList(), nFrames); 

    // Now we pass in that list of individual frame-by-frame smoothed sustain values, to the 
    // instrument. Inside mSynth, already contains the A, D, and R durations, and note frequency
    // too; these are all single values that don't require smoothing in between ProcessBlocks...
    // outputs is the almost-ready final output signal...
    mSynth.ProcessBlock(mModulations.GetList(), outputs, 0, nOutputs, nFrames);
    
    // The last step, is to use the individual frame-by-frame smoothed gain values, to make sure
    // that any gain change (e.g. via mixer knob) isn't too sudden on the listener's ears...
    for(int i = 0; i < nFrames; i++) {
      sample smoothedGain = mModulations.GetList()[kModGainSmoother][i];
      outputs[0][i] *= smoothedGain;
      outputs[1][i] *= smoothedGain;
    }
  }

  void Reset(double sampleRate, int blockSize)
  {
    // This function is called once on plugin load...
    mSynth.SetSampleRateAndBlockSize(sampleRate, blockSize);
    mSynth.Reset();

    // Initialise mModulations/Data, which ProcessBlock calls will use as buffer...
    mModulationsData.Resize(blockSize * kNumModulations);
    mModulations.Empty();
    for(int i = 0; i < kNumModulations; i++) {
      mModulations.Add(mModulationsData.Get() + (blockSize * i));
    }
  }

  void SetParam(int paramIdx, double value, int voiceIdx)
  {
    // Notice that Attack, Decay, and Release can currently be set on a voice-by-voice
    // basis, but Sustain and Gain cannot be. TODO: change mParamsToSmooth very slightly,
    // into a 2D array, of nModulations by nVoices.
    switch (paramIdx) {
      case kParamVoiceGain:
        mParamsToSmooth[kModGainSmoother] = (sample) value / 100.;
        break;
      case kParamVoiceOctave:
        mSynth.GetVoice(voiceIdx)->SetOctave(static_cast<int>(value));
        break;
      case kParamVoiceAttack:
        mSynth.GetVoice(voiceIdx)->mAMPEnv.SetStageTime(ADSREnvelope<sample>::EStage::kAttack, value); 
        break;
      case kParamVoiceDecay:
        mSynth.GetVoice(voiceIdx)->mAMPEnv.SetStageTime(ADSREnvelope<sample>::EStage::kDecay, value); 
        break;
      case kParamVoiceSustain:
        mParamsToSmooth[kModSustainSmoother] = (sample) value / 100.;
        break;
      case kParamVoiceRelease:
        mSynth.GetVoice(voiceIdx)->mAMPEnv.SetStageTime(ADSREnvelope<sample>::EStage::kRelease, value); 
        break;
      case kParamSynthCoreIdx:
        mSynth.GetVoice(voiceIdx)->SetSynthCore(static_cast<int>(value));
        break;
      case kParamFilterCutoff:
        mSynth.GetVoice(voiceIdx)->SetFilterCutoff(value);
        break;
      case kParamFilterResonance:
        mSynth.GetVoice(voiceIdx)->SetFilterResonance(value);
        break;
      case kParamFilterTypeIdx:
        // TODO:
        break;
      case kParamFilterAttack:
        // TODO:
        break;
      case kParamFilterDecay:
        // TODO:
        break;
      case kParamFilterSustain:
        // TODO:
        break;
      case kParamFilterRelease:
        // TODO:
        break;
      default:
        break;
    }
  }

  void ProcessMidiMsg(const IMidiMsg& msg)
  {
    mSynth.AddMidiMsgToQueue(msg);
  }
  
public:
  int mNVoices;
  MidiSynth mSynth { VoiceAllocator::kPolyModeMono, MidiSynth::kDefaultBlockSize };
  WDL_TypedBuf<sample> mModulationsData; // Sample data for global modulations (e.g. smoothed sustain)
  WDL_PtrList<sample> mModulations; // Ptrlist for global modulations
  LogParamSmooth<sample, kNumModulations> mParamSmoother;
  sample mParamsToSmooth[kNumModulations];
};
