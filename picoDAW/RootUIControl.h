#if IPLUG_EDITOR
#include "IControl.h"
#include <vector>
#include <iostream>
// A root control that is invisible; holds data and funcs for drawing UI
class RootUIControl : public IControl 
{
public:
  RootUIControl(IGraphics *pGraphics)
  : IControl(IRECT()),
    mGraphics(pGraphics), mScreen(kScreenNavigation),
    mPlaybackTimer(nullptr), mPlaybackStepIdx(0), mPrevPlaybackNote(-1)
  {
    mIgnoreMouse = true; // Member of parent IControl
  }

  void Draw(IGraphics& g) override {}

  void Update(EScreens screen)
  {
    auto titleText = mGraphics->GetControlWithTag(kCtrlTagTitleText)->As<ITextControl>();
    auto homeButton = mGraphics->GetControlWithTag(kCtrlTagHomeButton)->As<IVButtonControl>();
    auto stepButton = mGraphics->GetControlWithTag(kCtrlTagStepButton)->As<IVButtonControl>();

    auto noteLabelText01 = mGraphics->GetControlWithTag(kCtrlTagNoteLabelText01)->As<ITextControl>();
    auto noteLabelText02 = mGraphics->GetControlWithTag(kCtrlTagNoteLabelText02)->As<ITextControl>();
    auto noteLabelText03 = mGraphics->GetControlWithTag(kCtrlTagNoteLabelText03)->As<ITextControl>();
    auto noteLabelText04 = mGraphics->GetControlWithTag(kCtrlTagNoteLabelText04)->As<ITextControl>();
    auto noteLabelText05 = mGraphics->GetControlWithTag(kCtrlTagNoteLabelText05)->As<ITextControl>();
    auto noteLabelText06 = mGraphics->GetControlWithTag(kCtrlTagNoteLabelText06)->As<ITextControl>();
    auto noteLabelText07 = mGraphics->GetControlWithTag(kCtrlTagNoteLabelText07)->As<ITextControl>();
    auto noteLabelText08 = mGraphics->GetControlWithTag(kCtrlTagNoteLabelText08)->As<ITextControl>();
    auto noteLabelText09 = mGraphics->GetControlWithTag(kCtrlTagNoteLabelText09)->As<ITextControl>();
    auto noteLabelText10 = mGraphics->GetControlWithTag(kCtrlTagNoteLabelText10)->As<ITextControl>();
    auto noteLabelText11 = mGraphics->GetControlWithTag(kCtrlTagNoteLabelText11)->As<ITextControl>();
    auto noteLabelText12 = mGraphics->GetControlWithTag(kCtrlTagNoteLabelText12)->As<ITextControl>();

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
    auto coreAntiAliasToggle = mGraphics->GetControlWithTag(kCtrlTagSynthCoreAntiAliasingToggle)->As<IVToggleControl>();
    auto octaveSwitch = mGraphics->GetControlWithTag(kCtrlTagVoiceOctaveSwitch)->As<IVSwitchControl>();
    auto gainKnob = mGraphics->GetControlWithTag(kCtrlTagVoiceGainKnob)->As<IVKnobControl>();

    auto attackKnob = mGraphics->GetControlWithTag(kCtrlTagVoiceAttackKnob)->As<IVKnobControl>();
    auto decayKnob = mGraphics->GetControlWithTag(kCtrlTagVoiceDecayKnob)->As<IVKnobControl>();
    auto sustainKnob = mGraphics->GetControlWithTag(kCtrlTagVoiceSustainKnob)->As<IVKnobControl>();
    auto releaseKnob = mGraphics->GetControlWithTag(kCtrlTagVoiceReleaseKnob)->As<IVKnobControl>();

    auto filterPanel = mGraphics->GetControlWithTag(kCtrlTagFilterPanel)->As<IPanelControl>();
    auto filterIdxSwitch = mGraphics->GetControlWithTag(kCtrlTagFilterCoreSwitch)->As<IVSwitchControl>();
    auto cutoffKnob = mGraphics->GetControlWithTag(kCtrlTagFilterCutoffKnob)->As<IVKnobControl>();
    auto resonanceKnob = mGraphics->GetControlWithTag(kCtrlTagFilterResonanceKnob)->As<IVKnobControl>();
    auto typeIdxSwitch = mGraphics->GetControlWithTag(kCtrlTagFilterTypeSwitch)->As<IVSwitchControl>();

    auto filterAttackKnob = mGraphics->GetControlWithTag(kCtrlTagFilterAttackKnob)->As<IVKnobControl>();
    auto filterDecayKnob = mGraphics->GetControlWithTag(kCtrlTagFilterDecayKnob)->As<IVKnobControl>();
    auto filterSustainKnob = mGraphics->GetControlWithTag(kCtrlTagFilterSustainKnob)->As<IVKnobControl>();
    auto filterReleaseKnob = mGraphics->GetControlWithTag(kCtrlTagFilterReleaseKnob)->As<IVKnobControl>();
    auto filterLFOKnob = mGraphics->GetControlWithTag(kCtrlTagFilterLFOKnob)->As<IVKnobControl>();
    auto filterCoreZDFToggle = mGraphics->GetControlWithTag(kCtrlTagFilterCoreZDFToggle)->As<IVToggleControl>();

    auto midiSeq = mGraphics->GetControlWithTag(kCtrlTagMidiSequencer)->As<IVNoteSequencerControl>();
    auto gateSeq = mGraphics->GetControlWithTag(kCtrlTagGateSequencer)->As<IVStepValueControl>();
    auto gateLabelText01 = mGraphics->GetControlWithTag(kCtrlTagGateLabelText01)->As<ITextControl>();
    auto gateLabelText02 = mGraphics->GetControlWithTag(kCtrlTagGateLabelText02)->As<ITextControl>();
    auto velocitySeq = mGraphics->GetControlWithTag(kCtrlTagVelocitySequencer)->As<IVStepValueControl>();
    auto keyboard = mGraphics->GetControlWithTag(kCtrlTagKeyboard)->As<IVKeyboardControl>();
    auto keyboardModeSwitch = mGraphics->GetControlWithTag(kCtrlTagKeyboardModeSwitch)->As<IVSwitchControl>();
    auto playToggle = mGraphics->GetControlWithTag(kCtrlTagPlayToggle)->As<IVToggleControl>();
    auto soloToggle = mGraphics->GetControlWithTag(kCtrlTagSoloToggle)->As<IVToggleControl>();

    // Hide all controls...
    for (auto ctrlTag : mGraphics->GetControlTags()) {
      ctrlTag.second->Hide(true); // Manually unhide later...
      ctrlTag.second->SetDisabled(false); // Manually disable later...
    }

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
      int sync = mKeyboardModeIdx;
      std::cout << sync << std::endl;
      keyboard->Hide(sync != 0);
      gateSeq->Hide(sync != 1);
      gateLabelText01->Hide(sync != 1);
      gateLabelText02->Hide(sync != 1);
      velocitySeq->Hide(sync != 2);
      midiSeq->Hide(false);
      keyboardModeSwitch->Hide(false);
      noteLabelText01->Hide(false);
      noteLabelText02->Hide(false);
      noteLabelText03->Hide(false);
      noteLabelText04->Hide(false);
      noteLabelText05->Hide(false);
      noteLabelText06->Hide(false);
      noteLabelText07->Hide(false);
      noteLabelText08->Hide(false);
      noteLabelText09->Hide(false);
      noteLabelText10->Hide(false);
      noteLabelText11->Hide(false);
      noteLabelText12->Hide(false);
    } else if (screen == kScreenEditorSyn1 || screen == kScreenEditorSyn2 || screen == kScreenEditorSyn3 || screen == kScreenEditorSyn4) {
      coreIdxSwitch->Hide(false);
      coreAntiAliasToggle->Hide(false);
      octaveSwitch->Hide(false);
      gainKnob->Hide(false);
      attackKnob->Hide(false);
      decayKnob->Hide(false);
      sustainKnob->Hide(false);
      releaseKnob->Hide(false);
      filterPanel->Hide(false);
      filterIdxSwitch->Hide(false);
      cutoffKnob->Hide(false);
      resonanceKnob->Hide(false);
      typeIdxSwitch->Hide(false);
      filterAttackKnob->Hide(false);
      filterDecayKnob->Hide(false);
      filterSustainKnob->Hide(false);
      filterReleaseKnob->Hide(false);
      filterLFOKnob->Hide(false);
      filterCoreZDFToggle->Hide(false);
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

  Timer *mPlaybackTimer;
  IGraphics *mGraphics;

  int mPlaybackStepIdx;
  int mPrevPlaybackNote;

  int mKeyboardModeIdx;

  EScreens mScreen;
};
#endif
