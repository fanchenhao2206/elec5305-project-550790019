#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"
#include "IVSequencerControl.h"
#include "IPlugTimer.h"

const int kNumPresets = 1;

enum EParams
{
  // NOTE: good
  kParamRoot = 0,

  kParamKeyboardModeIdx,

  kParamVoiceIdx,

  kParamVoiceGain,
  kParamVoiceOctave,

  kParamVoiceAttack,
  kParamVoiceDecay,
  kParamVoiceSustain,
  kParamVoiceRelease,

  kParamSynthCoreIdx,

  kParamFilterCutoff,
  kParamFilterResonance,

  kParamFilterAttack,
  kParamFilterDecay,
  kParamFilterSustain,
  kParamFilterRelease,

  kParamFilterTypeIdx,

  kNumParams
};

#if IPLUG_DSP
// will use EParams in picoDAW_DSP.h
#include "picoDAW_DSP.h"
#endif

enum EControlTags
{
  // NOTE: good
  kCtrlTagRoot = 0,
  kCtrlTagHomeButton,
  kCtrlTagTitleText,
  kCtrlTagStepButton,

  kCtrlTagNoteLabelText01,
  kCtrlTagNoteLabelText02,
  kCtrlTagNoteLabelText03,
  kCtrlTagNoteLabelText04,
  kCtrlTagNoteLabelText05,
  kCtrlTagNoteLabelText06,
  kCtrlTagNoteLabelText07,
  kCtrlTagNoteLabelText08,
  kCtrlTagNoteLabelText09,
  kCtrlTagNoteLabelText10,
  kCtrlTagNoteLabelText11,
  kCtrlTagNoteLabelText12,

  kCtrlTagKeyboard,
  kCtrlTagGateSequencer,
  kCtrlTagGateLabelText01,
  kCtrlTagGateLabelText02,
  kCtrlTagMidiSequencer,
  kCtrlTagVelocitySequencer,
  kCtrlTagKeyboardModeSwitch,
  kCtrlTagPlayToggle,
  kCtrlTagSoloToggle,

  kCtrlTagNavigation,
  kCtrlTagNavigationSyn1Seq,
  kCtrlTagNavigationSyn2Seq,
  kCtrlTagNavigationSyn3Seq,
  kCtrlTagNavigationSyn4Seq,
  kCtrlTagNavigationSamp1Seq,
  kCtrlTagNavigationSamp2Seq,
  kCtrlTagNavigationSyn1Edit,
  kCtrlTagNavigationSyn2Edit,
  kCtrlTagNavigationSyn3Edit,
  kCtrlTagNavigationSyn4Edit,
  kCtrlTagNavigationSamp1Edit,
  kCtrlTagNavigationSamp2Edit,

  kCtrlTagMixerButton,

  kCtrlTagSynthCoreSwitch,

  kCtrlTagVoiceOctaveSwitch,
  kCtrlTagVoiceGainKnob,
  kCtrlTagVoiceAttackKnob,
  kCtrlTagVoiceDecayKnob,
  kCtrlTagVoiceSustainKnob,
  kCtrlTagVoiceReleaseKnob,

  kCtrlTagFilterCutoffKnob,
  kCtrlTagFilterResonanceKnob,
  kCtrlTagFilterTypeSwitch,
  kCtrlTagFilterAttackKnob,
  kCtrlTagFilterDecayKnob,
  kCtrlTagFilterSustainKnob,
  kCtrlTagFilterReleaseKnob,

  // TODO: to remove 
  kCtrlTagMeter,
  kCtrlTagLFOVis,
  kCtrlTagScope,
  kCtrlTagRTText,
  kCtrlTagBender,

  kNumCtrlTags
};

enum EScreens
{
  kScreenNavigation = 0,

  kScreenSequencerSyn1,
  kScreenSequencerSyn2,
  kScreenSequencerSyn3,
  kScreenSequencerSyn4,

  kScreenSequencerSamp1,
  kScreenSequencerSamp2,

  kScreenEditorSyn1,
  kScreenEditorSyn2,
  kScreenEditorSyn3,
  kScreenEditorSyn4,

  kScreenEditorSamp1,
  kScreenEditorSamp2,

  kNumScreens
};

using namespace iplug;
using namespace igraphics;

class picoDAW final : public Plugin
{
public:
  picoDAW(const InstanceInfo& info);

#if IPLUG_EDITOR
  void OnParamChangeUI(int paramIdx, EParamSource source) override;
#endif

#if IPLUG_DSP // http://bit.ly/2S64BDd
public:
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
  void OnReset() override;
  void OnIdle() override;
  void OnParamChange(int paramIdx) override;

private:
  picoDAWDSP mDSP;
  IPeakAvgSender<2> mMeterSender;
#endif
};
