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
    // Hide all controls...
    for (auto ctrlTag : mGraphics->GetControlTags()) {
      ctrlTag.second->Hide(true); // Manually unhide later...
      ctrlTag.second->SetDisabled(false); // Manually disable later...
    }

    // Update controls...
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
    mGraphics->GetControlWithTag(kCtrlTagTitleText)->As<ITextControl>()->SetStr(str.c_str());

    // TODO: the remaining controls... (like sequencer and editor)
    //

    // Reveal only the controls relevant to the new screen we are changing to...
    mGraphics->GetControlWithTag(kCtrlTagHomeButton)->Hide(false);
    mGraphics->GetControlWithTag(kCtrlTagPlayToggle)->Hide(false);
    mGraphics->GetControlWithTag(kCtrlTagSoloToggle)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagSoloToggle)->SetDisabled(true); // TODO: remove this solo disable when mixer done...
    mGraphics->GetControlWithTag(kCtrlTagTitleText)->Hide(false);
    mGraphics->GetControlWithTag(kCtrlTagStepButton)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagStepButton)->SetDisabled(true); // TODO: remove this step disable when step done...
    if (screen == kScreenSequencerSyn1 || screen == kScreenSequencerSyn2 || screen == kScreenSequencerSyn3 || screen == kScreenSequencerSyn4 ||
        screen == kScreenSequencerSamp1 || screen == kScreenSequencerSamp2) {
      int sync = mKeyboardModeIdx;
      std::cout << sync << std::endl;
      mGraphics->GetControlWithTag(kCtrlTagKeyboard)->Hide(sync != 0);
      mGraphics->GetControlWithTag(kCtrlTagGateSequencer)->Hide(sync != 1);
      mGraphics->GetControlWithTag(kCtrlTagGateLabelText01)->Hide(sync != 1);
      mGraphics->GetControlWithTag(kCtrlTagGateLabelText02)->Hide(sync != 1);
      mGraphics->GetControlWithTag(kCtrlTagVelocitySequencer)->Hide(sync != 2);
      mGraphics->GetControlWithTag(kCtrlTagMidiSequencer)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagKeyboardModeSwitch)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagNoteLabelText01)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagNoteLabelText02)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagNoteLabelText03)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagNoteLabelText04)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagNoteLabelText05)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagNoteLabelText06)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagNoteLabelText07)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagNoteLabelText08)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagNoteLabelText09)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagNoteLabelText10)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagNoteLabelText11)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagNoteLabelText12)->Hide(false);
    } else if (screen == kScreenEditorSyn1 || screen == kScreenEditorSyn2 || screen == kScreenEditorSyn3 || screen == kScreenEditorSyn4) {
      mGraphics->GetControlWithTag(kCtrlTagSynthCoreSwitch)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagSynthCoreAntiAliasingToggle)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagVoiceOctaveSwitch)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagVoiceGainKnob)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagVoiceAttackKnob)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagVoiceDecayKnob)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagVoiceSustainKnob)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagVoiceReleaseKnob)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagFilterPanel)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagFilterCoreSwitch)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagFilterCutoffKnob)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagFilterResonanceKnob)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagFilterTypeSwitch)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagFilterAttackKnob)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagFilterDecayKnob)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagFilterSustainKnob)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagFilterReleaseKnob)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagFilterLFOKnob)->Hide(false);
      mGraphics->GetControlWithTag(kCtrlTagFilterCoreZDFToggle)->Hide(false);
    } else if (screen == kScreenEditorSamp1 || screen == kScreenEditorSamp2) {
      // sampler->Hide(false);    // TODO: the sampler screen, edit what sample is loaded
                                  // takes up the sequencerBounds
    } else /* if (screen == kScreenNavigation) */ {
      mGraphics->GetControlWithTag(kCtrlTagNavigationSyn1Seq)->Hide(false); // For now just syn1 sequencer...
      mGraphics->GetControlWithTag(kCtrlTagNavigationSyn2Seq)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagNavigationSyn2Seq)->SetDisabled(true);
      mGraphics->GetControlWithTag(kCtrlTagNavigationSyn3Seq)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagNavigationSyn3Seq)->SetDisabled(true);
      mGraphics->GetControlWithTag(kCtrlTagNavigationSyn4Seq)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagNavigationSyn4Seq)->SetDisabled(true);
      mGraphics->GetControlWithTag(kCtrlTagNavigationSamp1Seq)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagNavigationSamp1Seq)->SetDisabled(true);
      mGraphics->GetControlWithTag(kCtrlTagNavigationSamp2Seq)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagNavigationSamp2Seq)->SetDisabled(true);

      mGraphics->GetControlWithTag(kCtrlTagNavigationSyn1Edit)->Hide(false); // For now just syn1 edit...
      mGraphics->GetControlWithTag(kCtrlTagNavigationSyn2Edit)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagNavigationSyn2Edit)->SetDisabled(true);
      mGraphics->GetControlWithTag(kCtrlTagNavigationSyn3Edit)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagNavigationSyn3Edit)->SetDisabled(true);
      mGraphics->GetControlWithTag(kCtrlTagNavigationSyn4Edit)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagNavigationSyn4Edit)->SetDisabled(true);
      mGraphics->GetControlWithTag(kCtrlTagNavigationSamp1Edit)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagNavigationSamp1Edit)->SetDisabled(true);
      mGraphics->GetControlWithTag(kCtrlTagNavigationSamp2Edit)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagNavigationSamp2Edit)->SetDisabled(true);
      mGraphics->GetControlWithTag(kCtrlTagSoloToggle)->SetDisabled(true); // Except this disable!! Don't remove this disable!! Unless, can
                                     // use mScreen (aka prev actual screen) to judge which one to solo,
                                     // in which case solo button should be valid on navigation screen...
      mGraphics->GetControlWithTag(kCtrlTagMixerButton)->Hide(false); mGraphics->GetControlWithTag(kCtrlTagMixerButton)->SetDisabled(true); // TODO: remove all these disables when done...
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
