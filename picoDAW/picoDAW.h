#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"
#include "IVSequencerControl.h"

const int kNumPresets = 1;

enum EParams
{
  // NOTE: good
  kParamRoot = 0,
  kParamKeyboard,
  kParamGateSequencer,
  kParamMidiSequencer,
  kParamVelocitySequencer,
  kParamCycleButton,
  kParamPlayButton,
  kParamSoloButton,

  // TODO: to remove 
  kParamGain,
  kParamNoteGlideTime,
  kParamAttack,
  kParamDecay,
  kParamSustain,
  kParamRelease,
  kParamLFOShape,
  kParamLFORateHz,
  kParamLFORateTempo,
  kParamLFORateMode,
  kParamLFODepth,
  kParamButton,

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
  kCtrlTagKeyboard,
  kCtrlTagGateSequencer,
  kCtrlTagMidiSequencer,
  kCtrlTagVelocitySequencer,
  kCtrlTagCycleButton,
  kCtrlTagPlayButton,
  kCtrlTagSoloButton,

  // TODO: to remove 
  kCtrlTagMeter,
  kCtrlTagLFOVis,
  kCtrlTagScope,
  kCtrlTagRTText,
  kCtrlTagBender,

  kNumCtrlTags
};

using namespace iplug;
using namespace igraphics;

class picoDAW final : public Plugin
{
public:
  picoDAW(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
public:
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
  void OnParamChangeUI(int paramIdx, EParamSource source) override;
  void OnIdle() override;
  bool OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) override;

private:
  picoDAWDSP<sample> mDSP {16};
  IPeakAvgSender<2> mMeterSender;
  ISender<1> mLFOVisSender;
#endif
};
