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
  kCtrlTagHomeButton,
  kCtrlTagTitle,

  kCtrlTagKeyboard,
  kCtrlTagGateSequencer,
  kCtrlTagMidiSequencer,
  kCtrlTagVelocitySequencer,
  kCtrlTagCycleButton,
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

  kCtrlTagMixer,

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

  kScreenSyn1Seq,
  kScreenSyn2Seq,
  kScreenSyn3Seq,
  kScreenSyn4Seq,

  kScreenSyn1Edit,
  kScreenSyn2Edit,
  kScreenSyn3Edit,
  kScreenSyn4Edit,

  kScreenSamp1Seq,
  kScreenSamp2Seq,

  kScreenSamp1Edit,
  kScreenSamp2Edit,

  kNumScreens
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
  picoDAWDSP<sample> mInstrumentDSP {16};
  IPeakAvgSender<2> mMeterSender;
  ISender<1> mLFOVisSender;
#endif
};
