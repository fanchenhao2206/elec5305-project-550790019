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
  kParamAttack,
  kParamDecay,
  kParamSustain,
  kParamRelease,
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
  kCtrlTagStep,

  kCtrlTagKeyboard,
  kCtrlTagGateSequencer,
  kCtrlTagMidiSequencer,
  kCtrlTagVelocitySequencer,
  kCtrlTagCycleKeyboardButton,
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

  kCtrlTagCoreSwitch,

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

#if IPLUG_DSP // http://bit.ly/2S64BDd
public:
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
  void OnReset() override;
  // void OnParamChange(int paramIdx) override;
  // void OnParamChangeUI(int paramIdx, EParamSource source) override;
  void OnIdle() override;
  bool OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) override;

private:
  picoDAWDSP mDSP;
  IPeakAvgSender<2> mMeterSender;
#endif
};
