#if IPLUG_EDITOR
#include "IControl.h"

struct SynthData
{
  int mSynthCoreIdx;
  int mVoiceOctave;
  double mVoiceGain;
  double mVoiceAttack;
  double mVoiceDecay;
  double mVoiceSustain;
  double mVoiceRelease;
  int mFilterTypeIdx;
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
      synData.mVoiceGain = GetParam(kParamVoiceGain)->Value();
      synData.mVoiceAttack = GetParam(kParamVoiceAttack)->Value();
      synData.mVoiceDecay = GetParam(kParamVoiceDecay)->Value();
      synData.mVoiceSustain = GetParam(kParamVoiceSustain)->Value();
      synData.mVoiceRelease = GetParam(kParamVoiceRelease)->Value();
      synData.mSynthCoreIdx = GetParam(kParamSynthCoreIdx)->Int();
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
      GetParam(kParamVoiceGain)->Set(synData.mVoiceGain);
      GetParam(kParamVoiceAttack)->Set(synData.mVoiceAttack);
      GetParam(kParamVoiceDecay)->Set(synData.mVoiceDecay);
      GetParam(kParamVoiceSustain)->Set(synData.mVoiceSustain);
      GetParam(kParamVoiceRelease)->Set(synData.mVoiceRelease);
      GetParam(kParamVoiceIdx)->Set(static_cast<double>(screen - kScreenEditorSyn1));
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

    auto titleText = mGraphics->GetControlWithTag(kCtrlTagTitleText)->As<ITextControl>();
    auto homeButton = mGraphics->GetControlWithTag(kCtrlTagHomeButton)->As<IVButtonControl>();
    auto stepButton = mGraphics->GetControlWithTag(kCtrlTagStepButton)->As<IVButtonControl>();

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

    auto mixerButton = mGraphics->GetControlWithTag(kCtrlTagMixerButton)->As<IVButtonControl>();

    auto coreIdxSwitch = mGraphics->GetControlWithTag(kCtrlTagSynthCoreSwitch)->As<IVSwitchControl>();
    auto octaveSwitch = mGraphics->GetControlWithTag(kCtrlTagVoiceOctaveSwitch)->As<IVSwitchControl>();
    auto gainKnob = mGraphics->GetControlWithTag(kCtrlTagVoiceGainKnob)->As<IVKnobControl>();

    auto attackKnob = mGraphics->GetControlWithTag(kCtrlTagVoiceAttackKnob)->As<IVKnobControl>();
    auto decayKnob = mGraphics->GetControlWithTag(kCtrlTagVoiceDecayKnob)->As<IVKnobControl>();
    auto sustainKnob = mGraphics->GetControlWithTag(kCtrlTagVoiceSustainKnob)->As<IVKnobControl>();
    auto releaseKnob = mGraphics->GetControlWithTag(kCtrlTagVoiceReleaseKnob)->As<IVKnobControl>();

    auto cutoffKnob = mGraphics->GetControlWithTag(kCtrlTagFilterCutoffKnob)->As<IVKnobControl>();
    auto resonanceKnob = mGraphics->GetControlWithTag(kCtrlTagFilterResonanceKnob)->As<IVKnobControl>();
    auto typeIdxSwitch = mGraphics->GetControlWithTag(kCtrlTagFilterTypeSwitch)->As<IVSwitchControl>();

    auto filterAttackKnob = mGraphics->GetControlWithTag(kCtrlTagFilterAttackKnob)->As<IVKnobControl>();
    auto filterDecayKnob = mGraphics->GetControlWithTag(kCtrlTagFilterDecayKnob)->As<IVKnobControl>();
    auto filterSustainKnob = mGraphics->GetControlWithTag(kCtrlTagFilterSustainKnob)->As<IVKnobControl>();
    auto filterReleaseKnob = mGraphics->GetControlWithTag(kCtrlTagFilterReleaseKnob)->As<IVKnobControl>();

    auto midiSeq = mGraphics->GetControlWithTag(kCtrlTagMidiSequencer)->As<IVSequencerControl>();
    auto gateSeq = mGraphics->GetControlWithTag(kCtrlTagGateSequencer)->As<IVSequencerControl>();
    auto keyboard = mGraphics->GetControlWithTag(kCtrlTagKeyboard)->As<IVKeyboardControl>();
    auto cycleKeyboard = mGraphics->GetControlWithTag(kCtrlTagCycleKeyboardButton)->As<IVButtonControl>();
    auto playToggle = mGraphics->GetControlWithTag(kCtrlTagPlayToggle)->As<IVToggleControl>();
    auto soloToggle = mGraphics->GetControlWithTag(kCtrlTagSoloToggle)->As<IVToggleControl>();

    // Update titleText control...
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
    titleText->SetStr(str.c_str());

    // Reveal only the controls relevant to the new screen we are changing to...
    homeButton->Hide(false);
    playToggle->Hide(false);
    soloToggle->Hide(false); soloToggle->SetDisabled(true); // TODO: remove this solo disable when mixer done...
    titleText->Hide(false);
    stepButton->Hide(false); stepButton->SetDisabled(true); // TODO: remove this step disable when step done...
    if (screen == kScreenSequencerSyn1 || screen == kScreenSequencerSyn2 || screen == kScreenSequencerSyn3 || screen == kScreenSequencerSyn4 ||
        screen == kScreenSequencerSamp1 || screen == kScreenSequencerSamp2) {
      int sync = mKeyboardBoundsDatas[screen - kScreenSequencerSyn1].mKeyboardBoundsIdx;
      keyboard->Hide(sync != 0);
      gateSeq->Hide(sync != 1);
      midiSeq->Hide(false);
      cycleKeyboard->Hide(false);
    } else if (screen == kScreenEditorSyn1 || screen == kScreenEditorSyn2 || screen == kScreenEditorSyn3 || screen == kScreenEditorSyn4) {
      coreIdxSwitch->Hide(false);
      octaveSwitch->Hide(false);
      gainKnob->Hide(false);
      attackKnob->Hide(false);
      decayKnob->Hide(false);
      sustainKnob->Hide(false);
      releaseKnob->Hide(false);
      cutoffKnob->Hide(false);
      resonanceKnob->Hide(false);
      typeIdxSwitch->Hide(false);
      filterAttackKnob->Hide(false);
      filterDecayKnob->Hide(false);
      filterSustainKnob->Hide(false);
      filterReleaseKnob->Hide(false);
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
      soloToggle->SetDisabled(true); // Except this disable!! Don't remove this disable!! Unless, can
                                     // use mScreen (aka prev actual screen) to judge which one to solo,
                                     // in which case solo button should be valid on navigation screen...
      mixerButton->Hide(false); mixerButton->SetDisabled(true); // TODO: remove all these disables when done...
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
