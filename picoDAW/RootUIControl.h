#if IPLUG_EDITOR
#include "IControl.h"

struct SynthData
{
  SynthData()
  : mCoreIdx(0),
    mOctave(0), // A "UI-side" parameter that just determines, what type of
                // MIDI message to send to the DSP; it itself doesn't affect
                // the DSP especially, so we don't store it as an IParam...
    mGain(100.),
    mAttack(10.),
    mDecay(10.),
    mSustain(50.),
    mRelease(10.),
    mFilterCutoff(0.5),
    mFilterResonance(0.),

    mFilterAttack(0.),
    mFilterDecay(0.),
    mFilterSustain(1.),
    mFilterRelease(0.)
  {}

  int mCoreIdx; // Store the int representation of the underlying enum IParam
  int mOctave;
  double mGain;
  double mAttack;
  double mDecay;
  double mSustain;
  double mRelease;
  double mFilterCutoff;
  double mFilterResonance;
  double mFilterAttack;
  double mFilterDecay;
  double mFilterSustain;
  double mFilterRelease;
};

struct KeyboardBoundsData
{
  KeyboardBoundsData()
  : mKeyboardBoundsIdx(0),
    mKeyboardBoundsTotal(2)
  {}

  int mKeyboardBoundsIdx;
  int mKeyboardBoundsTotal;
};

// A root control that is invisible; holds data and funcs for drawing UI
class RootUIControl : public IControl 
{
public:
  RootUIControl(IGraphics *pGraphics)
  : IControl(IRECT()),
    mGraphics(pGraphics), mScreen(kScreenNavigation),
    mKeyboardBoundsDatas(6),
    mPlaybackTimer(nullptr), mPlaybackStepIdx(0), mPrevNote(-1),
    mSynthDatas(4)/*, mSamplerDatas(2)*/
  {
    mIgnoreMouse = true; // Member of parent IControl
  }

  void Draw(IGraphics& g) override {}

  void Update(EScreens screen)
  {
    // Save params of the old mScreen that we are changing from...
    if (mScreen == kScreenEditorSyn1 || mScreen == kScreenEditorSyn2 || mScreen == kScreenEditorSyn3 || mScreen == kScreenEditorSyn4) {
      auto &synData = mSynthDatas[mScreen - kScreenEditorSyn1];
      synData.mGain = GetParam(kParamGain)->Value();
      synData.mAttack = GetParam(kParamAttack)->Value();
      synData.mDecay = GetParam(kParamDecay)->Value();
      synData.mSustain = GetParam(kParamSustain)->Value();
      synData.mRelease = GetParam(kParamRelease)->Value();
      synData.mCoreIdx = GetParam(kParamCore)->Int();
    } else if (mScreen == kScreenSequencerSyn1 || mScreen == kScreenSequencerSyn2 || 
      mScreen == kScreenSequencerSyn3 || mScreen == kScreenSequencerSyn4 || 
      mScreen == kScreenSequencerSamp1 || mScreen == kScreenSequencerSamp2) {
      // TODO:
    } else if (mScreen == kScreenEditorSamp1 || mScreen == kScreenEditorSamp2) {
      // TODO:
    }

    // Load params of the new screen that we are changing to...
    if (screen == kScreenEditorSyn1 || screen == kScreenEditorSyn2 || screen == kScreenEditorSyn3 || screen == kScreenEditorSyn4) {
      auto &synData = mSynthDatas[screen - kScreenEditorSyn1];
      GetParam(kParamGain)->Set(synData.mGain);
      GetParam(kParamAttack)->Set(synData.mAttack);
      GetParam(kParamDecay)->Set(synData.mDecay);
      GetParam(kParamSustain)->Set(synData.mSustain);
      GetParam(kParamRelease)->Set(synData.mRelease);
      GetParam(kParamVoice)->Set(static_cast<double>(screen - kScreenEditorSyn1));
    } else if (screen == kScreenSequencerSyn1 || screen == kScreenSequencerSyn2 || 
      screen == kScreenSequencerSyn3 || screen == kScreenSequencerSyn4 || 
      screen == kScreenSequencerSamp1 || screen == kScreenSequencerSamp2) {
      // keyboard->Update(screen);
      // midiSeq->Update(screen);
      // gateSeq->Update(screen);
    } else if (screen == kScreenEditorSamp1 || screen == kScreenEditorSamp2) {
      // sampler->Update(screen);
    }

    // Hide all controls...
    for (auto ctrlTag : mGraphics->GetControlTags()) {
      ctrlTag.second->Hide(true); // Manually unhide later...
      ctrlTag.second->SetDisabled(false); // Manually disable later...
    }

    auto title = mGraphics->GetControlWithTag(kCtrlTagTitle)->As<ITextControl>();
    auto home = mGraphics->GetControlWithTag(kCtrlTagHomeButton)->As<IVButtonControl>();
    auto step = mGraphics->GetControlWithTag(kCtrlTagStep)->As<ITextControl>();

    auto navigationSyn1Seq = mGraphics->GetControlWithTag(kCtrlTagNavigationSyn1Seq);
    auto navigationSyn2Seq = mGraphics->GetControlWithTag(kCtrlTagNavigationSyn2Seq);
    auto navigationSyn3Seq = mGraphics->GetControlWithTag(kCtrlTagNavigationSyn3Seq);
    auto navigationSyn4Seq = mGraphics->GetControlWithTag(kCtrlTagNavigationSyn4Seq);
    auto navigationSamp1Seq = mGraphics->GetControlWithTag(kCtrlTagNavigationSamp1Seq);
    auto navigationSamp2Seq = mGraphics->GetControlWithTag(kCtrlTagNavigationSamp2Seq);
    auto navigationSyn1Edit = mGraphics->GetControlWithTag(kCtrlTagNavigationSyn1Edit);
    auto navigationSyn2Edit = mGraphics->GetControlWithTag(kCtrlTagNavigationSyn2Edit);
    auto navigationSyn3Edit = mGraphics->GetControlWithTag(kCtrlTagNavigationSyn3Edit);
    auto navigationSyn4Edit = mGraphics->GetControlWithTag(kCtrlTagNavigationSyn4Edit);
    auto navigationSamp1Edit = mGraphics->GetControlWithTag(kCtrlTagNavigationSamp1Edit);
    auto navigationSamp2Edit = mGraphics->GetControlWithTag(kCtrlTagNavigationSamp2Edit);

    auto mixer = mGraphics->GetControlWithTag(kCtrlTagMixer)->As<ITextControl>();

    auto coreSwitch = mGraphics->GetControlWithTag(kCtrlTagCoreSwitch)->As<IVSwitchControl>();
    auto octaveSwitch = mGraphics->GetControlWithTag(kCtrlTagOctaveSwitch)->As<IVSwitchControl>();
    auto gainKnob = mGraphics->GetControlWithTag(kCtrlTagGainKnob)->As<IVKnobControl>();
    auto attackKnob = mGraphics->GetControlWithTag(kCtrlTagAttackKnob)->As<IVKnobControl>();
    auto decayKnob = mGraphics->GetControlWithTag(kCtrlTagDecayKnob)->As<IVKnobControl>();
    auto sustainKnob = mGraphics->GetControlWithTag(kCtrlTagSustainKnob)->As<IVKnobControl>();
    auto releaseKnob = mGraphics->GetControlWithTag(kCtrlTagReleaseKnob)->As<IVKnobControl>();

    auto midiSeq = mGraphics->GetControlWithTag(kCtrlTagMidiSequencer)->As<IVSequencerControl>();
    auto gateSeq = mGraphics->GetControlWithTag(kCtrlTagGateSequencer)->As<IVSequencerControl>();
    auto keyboard = mGraphics->GetControlWithTag(kCtrlTagKeyboard)->As<IVKeyboardControl>();
    auto cycleKeyboard = mGraphics->GetControlWithTag(kCtrlTagCycleKeyboardButton)->As<IVButtonControl>();
    auto play = mGraphics->GetControlWithTag(kCtrlTagPlayToggle)->As<IVToggleControl>();
    auto solo = mGraphics->GetControlWithTag(kCtrlTagSoloToggle)->As<IVToggleControl>();

    // Update title control...
    std::string str = "";
    switch (screen) {
      case kScreenNavigation:
        str = "HOME";
        break;

      case kScreenSequencerSyn1:
        str = "SYN1 SEQUENCER";
        break;
      case kScreenSequencerSyn2:
        str = "SYN2 SEQUENCER";
        break;
      case kScreenSequencerSyn3:
        str = "SYN3 SEQUENCER";
        break;
      case kScreenSequencerSyn4:
        str = "SYN4 SEQUENCER";
        break;

      case kScreenEditorSyn1:
        str = "SYN1 SETTINGS";
        break;
      case kScreenEditorSyn2:
        str = "SYN2 SETTINGS";
        break;
      case kScreenEditorSyn3:
        str = "SYN3 SETTINGS";
        break;
      case kScreenEditorSyn4:
        str = "SYN4 SETTINGS";
        break;

      case kScreenSequencerSamp1:
        str = "SAMP1 SEQUENCER";
        break;
      case kScreenSequencerSamp2:
        str = "SAMP2 SEQUENCER";
        break;

      case kScreenEditorSamp1:
        str = "SAMP1 SETTINGS";
        break;
      case kScreenEditorSamp2:
        str = "SAMP2 SETTINGS";
        break;

      default:
        str = "UNIMPLEMENTED";
        break;
    }
    title->SetStr(str.c_str());

    // Reveal only the controls relevant to the new screen we are changing to...
    home->Hide(false);
    play->Hide(false);
    solo->Hide(false);
    title->Hide(false);
    step->Hide(false);
    if (screen == kScreenSequencerSyn1 || screen == kScreenSequencerSyn2 || screen == kScreenSequencerSyn3 || screen == kScreenSequencerSyn4 ||
        screen == kScreenSequencerSamp1 || screen == kScreenSequencerSamp2) {
      int sync = mKeyboardBoundsDatas[screen - kScreenSequencerSyn1].mKeyboardBoundsIdx;
      keyboard->Hide(sync != 0);
      gateSeq->Hide(sync != 1);
      midiSeq->Hide(false);
      cycleKeyboard->Hide(false);
    } else if (screen == kScreenEditorSyn1 || screen == kScreenEditorSyn2 || screen == kScreenEditorSyn3 || screen == kScreenEditorSyn4) {
      coreSwitch->Hide(false);
      octaveSwitch->Hide(false);
      gainKnob->Hide(false);
      attackKnob->Hide(false);
      decayKnob->Hide(false);
      sustainKnob->Hide(false);
      releaseKnob->Hide(false);
    } else if (screen == kScreenEditorSamp1 || screen == kScreenEditorSamp2) {
      // sampler->Hide(false);    // TODO: the sampler screen, edit what sample is loaded
                                  // takes up the sequencerBounds
    } else /* if (screen == kScreenNavigation) */ {
      navigationSyn1Seq->Hide(false); // For now just syn1 sequencer...
      navigationSyn2Seq->Hide(false); navigationSyn2Seq->SetDisabled(true);
      navigationSyn3Seq->Hide(false); navigationSyn3Seq->SetDisabled(true);
      navigationSyn4Seq->Hide(false); navigationSyn4Seq->SetDisabled(true);
      navigationSamp1Seq->Hide(false); navigationSamp1Seq->SetDisabled(true);
      navigationSamp2Seq->Hide(false); navigationSamp2Seq->SetDisabled(true);

      navigationSyn1Edit->Hide(false); // For now just syn1 edit...
      navigationSyn2Edit->Hide(false); navigationSyn2Edit->SetDisabled(true);
      navigationSyn3Edit->Hide(false); navigationSyn3Edit->SetDisabled(true);
      navigationSyn4Edit->Hide(false); navigationSyn4Edit->SetDisabled(true);
      navigationSamp1Edit->Hide(false); navigationSamp1Edit->SetDisabled(true);
      navigationSamp2Edit->Hide(false); navigationSamp2Edit->SetDisabled(true);
      solo->SetDisabled(true);
      mixer->Hide(false); // TODO: the mixer menu that shows knobs and mute/solo buttons
                          // takes up the keyboardBounds
    }

    // Finally, forget the previous mScreen, and transition to new screen...
    mScreen = screen;
  }

  std::vector<SynthData> mSynthDatas;
  std::vector<KeyboardBoundsData> mKeyboardBoundsDatas;

  Timer *mPlaybackTimer;
  IGraphics *mGraphics;

  int mPlaybackStepIdx;
  int mPrevNote;

  EScreens mScreen;
};
#endif
