#if IPLUG_EDITOR
#include "IControl.h"

// A root control that is invisible; holds data and funcs for drawing UI
class RootUIControl : public IControl 
{
public:
  RootUIControl(IGraphics *pGraphics)
  : IControl(IRECT()),
    mGraphics(pGraphics), mScreen(kScreenNavigation),
    mKeyboardBoundsIdx(0), mKeyboardBoundsTotal(2),
    mPlaybackTimer(nullptr), mPlaybackStepIdx(0), mPrevNote(-1)
  {
    mIgnoreMouse = true; // Member of parent IControl
  }

  void Draw(IGraphics& g) override {}

  void Update()
  {
    // Hide all controls
    for (auto ctrlTag : mGraphics->GetControlTags()) {
      ctrlTag.second->Hide(true); // Manually unhide later...
      ctrlTag.second->SetDisabled(false); // Manually disable later...
    }

    // And then update the controls behind the curtain
    auto title = mGraphics->GetControlWithTag(kCtrlTagTitle)->As<ITextControl>();
    auto home = mGraphics->GetControlWithTag(kCtrlTagHomeButton)->As<IVButtonControl>();
    auto step = mGraphics->GetControlWithTag(kCtrlTagStep)->As<ITextControl>();

    auto navigation = mGraphics->GetControlWithTag(kCtrlTagNavigation);
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

    auto knob = mGraphics->GetControlWithTag(kCtrlTagKnobToggle)->As<IVToggleControl>();

    auto midiSeq = mGraphics->GetControlWithTag(kCtrlTagMidiSequencer)->As<IVSequencerControl>();
    auto gateSeq = mGraphics->GetControlWithTag(kCtrlTagGateSequencer)->As<IVSequencerControl>();
    auto keyboard = mGraphics->GetControlWithTag(kCtrlTagKeyboard)->As<IVKeyboardControl>();
    auto cycle = mGraphics->GetControlWithTag(kCtrlTagCycleButton)->As<IVButtonControl>();
    auto play = mGraphics->GetControlWithTag(kCtrlTagPlayToggle)->As<IVToggleControl>();
    auto solo = mGraphics->GetControlWithTag(kCtrlTagSoloToggle)->As<IVToggleControl>();

    // 2206: Instead of worrying about adding extra methods to these controls, 
    // just write out the full code for those methods here; for now at least...
    // (Maybe we will need oop... Or maybe not!! Hopefully not... Yeah, don't think we will need!!)

    // synth->Update(mScreen);
    // sampler->Update(mScreen);

    /* keyboard->Update(mScreen); */ {
      // 2206: How to update keyboard? What is keyboard?
    }
    /* midiSeq->Update(mScreen); */ {
      // 2206: How to update midiSeq? What is midiSeq?
    }
    /* gateSeq->Update(mScreen); */ {
      // 2206: How to update gateSeq? What is gateSeq?
    }

    /* title->Update(mScreen); */ {
      std::string str = "";
      switch (mScreen) {
        case kScreenNavigation:
          str = "HOME";
          break;

        case kScreenSyn1Seq:
          str = "SYN1 SEQUENCER";
          break;
        case kScreenSyn2Seq:
          str = "SYN2 SEQUENCER";
          break;
        case kScreenSyn3Seq:
          str = "SYN3 SEQUENCER";
          break;
        case kScreenSyn4Seq:
          str = "SYN4 SEQUENCER";
          break;

        case kScreenSyn1Edit:
          str = "SYN1 SETTINGS";
          break;
        case kScreenSyn2Edit:
          str = "SYN2 SETTINGS";
          break;
        case kScreenSyn3Edit:
          str = "SYN3 SETTINGS";
          break;
        case kScreenSyn4Edit:
          str = "SYN4 SETTINGS";
          break;

        case kScreenSamp1Seq:
          str = "SAMP1 SEQUENCER";
          break;
        case kScreenSamp2Seq:
          str = "SAMP2 SEQUENCER";
          break;

        case kScreenSamp1Edit:
          str = "SAMP1 SETTINGS";
          break;
        case kScreenSamp2Edit:
          str = "SAMP2 SETTINGS";
          break;

        default:
          str = "UNIMPLEMENTED";
          break;
      }
      title->SetStr(str.c_str());
    }

    // And lastly, unhide the updated controls that are relevant to the current screen
    home->Hide(false);
    play->Hide(false);
    solo->Hide(false);
    title->Hide(false);
    step->Hide(false);
    if (mScreen == kScreenSyn1Seq || mScreen == kScreenSyn2Seq || mScreen == kScreenSyn3Seq || mScreen == kScreenSyn4Seq ||
        mScreen == kScreenSamp1Seq || mScreen == kScreenSamp2Seq) {
      int sync = mKeyboardBoundsIdx;
      keyboard->Hide(sync != 0);
      gateSeq->Hide(sync != 1);
      midiSeq->Hide(false);
      cycle->Hide(false);
    } else if (mScreen == kScreenSyn1Edit || mScreen == kScreenSyn2Edit || mScreen == kScreenSyn3Edit || mScreen == kScreenSyn4Edit) {
      // 2206: These are very naive pseudocode; the way i've been designing the UI, these 
      // are unlikely to be single controls; instead, a mishmash of 42 different knobs and 
      // buttons; that would be ok, this will just be a long Update() function (that's ok)
      //
      // synth->Hide(false);      // 2206: the synth screen, edit the parameters of synth like ADSR
                                  // takes up the sequencerBounds
      knob->Hide(false);
    } else if (mScreen == kScreenSamp1Edit || mScreen == kScreenSamp2Edit) {
      // sampler->Hide(false);    // 2206: the sampler screen, edit what sample is loaded
                                  // takes up the sequencerBounds
    } else /* if (mScreen == kScreenNavigation) */ {
      navigation->Hide(false);
      navigationSyn1Seq->Hide(false);
      navigationSyn2Seq->Hide(false);
      navigationSyn3Seq->Hide(false);
      navigationSyn4Seq->Hide(false);
      navigationSamp1Seq->Hide(false);
      navigationSamp2Seq->Hide(false);

      navigationSyn1Edit->Hide(false);
      navigationSyn2Edit->Hide(false);
      navigationSyn3Edit->Hide(false);
      navigationSyn4Edit->Hide(false);
      navigationSamp1Edit->Hide(false);
      navigationSamp2Edit->Hide(false);
      solo->SetDisabled(true);
      mixer->Hide(false); // 2206: the mixer menu that shows knobs and mute/solo buttons
                          // takes up the keyboardBounds
    }
  }

  int mKeyboardBoundsIdx;
  const int mKeyboardBoundsTotal;

  Timer *mPlaybackTimer;
  IGraphics *mGraphics;

  int mPlaybackStepIdx;
  int mPrevNote;

  EScreens mScreen;
};
#endif
