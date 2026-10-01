#include "picoDAW.h"
#include "IPlug_include_in_plug_src.h"
#include "LFO.h"
#include "IconsForkAwesome.h"
#include "IconsFontaudio.h"

#include <string>
/** A root control that is invisible and does nothing; just holds data for drawing UI */
class RootUIControl : public IControl {
public:
  int keyboardBoundsIdx = 0;
  const int keyboardBoundsTotal = 2;
  
  RootUIControl() : IControl(IRECT()) { mIgnoreMouse = true; }
  void Draw(IGraphics& g) override {}
};

picoDAW::picoDAW(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kParamGain)->InitDouble("Gain", 100., 0., 100.0, 0.01, "%");
  GetParam(kParamNoteGlideTime)->InitMilliseconds("Note Glide Time", 0., 0.0, 30.);
  GetParam(kParamAttack)->InitDouble("Attack", 10., 1., 1000., 0.1, "ms", IParam::kFlagsNone, "ADSR", IParam::ShapePowCurve(3.));
  GetParam(kParamDecay)->InitDouble("Decay", 10., 1., 1000., 0.1, "ms", IParam::kFlagsNone, "ADSR", IParam::ShapePowCurve(3.));
  GetParam(kParamSustain)->InitDouble("Sustain", 50., 0., 100., 1, "%", IParam::kFlagsNone, "ADSR");
  GetParam(kParamRelease)->InitDouble("Release", 10., 2., 1000., 0.1, "ms", IParam::kFlagsNone, "ADSR");
  GetParam(kParamLFOShape)->InitEnum("LFO Shape", LFO<>::kTriangle, {LFO_SHAPE_VALIST});
  GetParam(kParamLFORateHz)->InitFrequency("LFO Rate", 1., 0.01, 40.);
  GetParam(kParamLFORateTempo)->InitEnum("LFO Rate", LFO<>::k1, {LFO_TEMPODIV_VALIST});
  GetParam(kParamLFORateMode)->InitBool("LFO Sync", true);
  GetParam(kParamLFODepth)->InitPercentage("LFO Depth");
    
#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachPanelBackground(COLOR_LIGHT_GRAY);

    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->LoadFont("ForkAwesome", FORK_AWESOME_FN);
    pGraphics->LoadFont("Fontaudio", FONTAUDIO_FN);

    const IVStyle style {
      true, // Show label
      true, // Show value
      {
        DEFAULT_BGCOLOR, // Background
        DEFAULT_FGCOLOR, // Foreground
        DEFAULT_PRCOLOR, // Pressed
        COLOR_BLACK, // Frame
        DEFAULT_HLCOLOR, // Highlight
        DEFAULT_SHCOLOR, // Shadow
        COLOR_BLACK, // Extra 1
        DEFAULT_X2COLOR, // Extra 2
        DEFAULT_X3COLOR  // Extra 3
      }, // Colours
      IText(24.f, "Roboto-Regular"), // Label text
      IText(16.f, "Roboto-Regular") // Value text
    };

    //
    // The main window; and the title control in top left
    //
    const IRECT b = pGraphics->GetBounds().GetPadded(0);
    pGraphics->AttachControl(new ITextControl(b.GetPadded(-5), "picoDAW", IText(
      36.f, "Roboto-Regular").WithAlign(EAlign::Near).WithVAlign(EVAlign::Top)));

    //
    // The root control; contains the global variables necessary to control UI
    //
    pGraphics->AttachControl(new RootUIControl(), kCtrlTagRoot);

    //
    // Making subrects of main window
    //
    IRECT keyboardBounds = b.GetFromBottom( // 160px gap above bottom of screen
      160).GetReducedFromLeft(160).GetReducedFromRight(160);
    IRECT sequencerBounds = b.GetReducedFromBottom( // 20px gap above top of keyboard
      160 + 20).GetReducedFromLeft(160).GetReducedFromRight(160).GetReducedFromTop(80 - 20);
    IRECT cycleButtonBounds = keyboardBounds.GetCentredInside(120, 120).GetTranslated(-(320 + 80), 0);

    //
    // Placing controls into those subrects
    //
    pGraphics->AttachControl(new IVSequencerControl<12,16>(sequencerBounds, kParamMidiSequencer));

    // Notice that these occupy the same subrect, so every control but one is manually hidden on 
    // boot; a seperate button will then be pressed to cycle through which control is visible
    pGraphics->AttachControl(new IVKeyboardControl(keyboardBounds, kParamKeyboard), kCtrlTagKeyboard);
    pGraphics->AttachControl(new IVSequencerControl<5,16>(keyboardBounds, kParamGateSequencer));

    // Hide/Show controls so only the active one is displayed
    int sync = pGraphics->GetControlWithTag(kCtrlTagRoot)->As<RootUIControl>()->keyboardBoundsIdx;
    pGraphics->HideControl(kParamKeyboard, sync != 0);
    pGraphics->HideControl(kParamGateSequencer, sync != 1);

    // Button to cycle through what is displayed in keyboardBounds
    pGraphics->AttachControl(new IVButtonControl(cycleButtonBounds, 
      [&](IControl* pCaller) {
        // Grab the root control
        auto ui = pCaller->GetUI();
        auto root = ui->GetControlWithTag(kCtrlTagRoot)->As<RootUIControl>(); 

        // Cycle through its currently active sub-control
        root->keyboardBoundsIdx += 1; 
        root->keyboardBoundsIdx %= root->keyboardBoundsTotal;
                    
        // Hide/Show controls so only the active one is displayed
        int sync = root->keyboardBoundsIdx;
        ui->HideControl(kParamKeyboard, sync != 0);
        ui->HideControl(kParamGateSequencer, sync != 1);

        // Then animation...
        float x, y;
        ui->GetMouseDownPoint(x, y);
        pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
        pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);

        // ...Done
      }, "TOGGLE", style), kCtrlTagCycleButton);
  };
#endif
}

#if IPLUG_DSP
void picoDAW::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  mDSP.ProcessBlock(nullptr, outputs, 2, nFrames, mTimeInfo.mPPQPos, mTimeInfo.mTransportIsRunning);
  mMeterSender.ProcessBlock(outputs, nFrames, kCtrlTagMeter);
  mLFOVisSender.PushData({kCtrlTagLFOVis, {float(mDSP.mLFO.GetLastOutput())}});
}

void picoDAW::OnIdle()
{
  mMeterSender.TransmitData(*this);
  mLFOVisSender.TransmitData(*this);
}

void picoDAW::OnReset()
{
  mDSP.Reset(GetSampleRate(), GetBlockSize());
  mMeterSender.Reset(GetSampleRate());
}

void picoDAW::ProcessMidiMsg(const IMidiMsg& msg)
{
  TRACE;
  
  int status = msg.StatusMsg();
  
  switch (status)
  {
    case IMidiMsg::kNoteOn:
    case IMidiMsg::kNoteOff:
    case IMidiMsg::kPolyAftertouch:
    case IMidiMsg::kControlChange:
    case IMidiMsg::kProgramChange:
    case IMidiMsg::kChannelAftertouch:
    case IMidiMsg::kPitchWheel:
    {
      goto handle;
    }
    default:
      return;
  }
  
handle:
  mDSP.ProcessMidiMsg(msg);
  SendMidiMsg(msg);
}

void picoDAW::OnParamChange(int paramIdx)
{
  mDSP.SetParam(paramIdx, GetParam(paramIdx)->Value());
}

void picoDAW::OnParamChangeUI(int paramIdx, EParamSource source)
{
  #if IPLUG_EDITOR
  if (auto pGraphics = GetUI())
  {
    if (paramIdx == kParamLFORateMode)
    {
      const auto sync = GetParam(kParamLFORateMode)->Bool();
      pGraphics->HideControl(kParamLFORateHz, sync);
      pGraphics->HideControl(kParamLFORateTempo, !sync);
    }
  }
  #endif
}

bool picoDAW::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData)
{
  if(ctrlTag == kCtrlTagBender && msgTag == IWheelControl::kMessageTagSetPitchBendRange)
  {
    const int bendRange = *static_cast<const int*>(pData);
    mDSP.mSynth.SetPitchBendRange(bendRange);
  }
  
  return false;
}
#endif
