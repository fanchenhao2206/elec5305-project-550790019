#if IPLUG_EDITOR
#include "IControl.h"

struct SynthData
{
  SynthData()
  : mCoreIdx(0),
    mCoreTotal(3),
    mOctave(0),

    mAttack(0.),
    mDecay(0.),
    mSustain(1.),
    mRelease(0.),

    mFilterCutoff(0.5),
    mFilterResonance(0.),

    mFilterAttack(0.),
    mFilterDecay(0.),
    mFilterSustain(1.),
    mFilterRelease(0.)
  {}

  int mCoreIdx; // [0..2], only three cores for now
  int mCoreTotal; // total three cores, for now
  int mOctave; // [-2..+2], relative to C4 octave (with C4 == 60)
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

  void Update()
  {
    ///////////////////////
    // Hide all controls //
    ///////////////////////

    for (auto ctrlTag : mGraphics->GetControlTags()) {
      ctrlTag.second->Hide(true); // Manually unhide later...
      ctrlTag.second->SetDisabled(false); // Manually disable later...
    }

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

    auto core = mGraphics->GetControlWithTag(kCtrlTagCoreSwitch)->As<IVSwitchControl>();

    auto midiSeq = mGraphics->GetControlWithTag(kCtrlTagMidiSequencer)->As<IVSequencerControl>();
    auto gateSeq = mGraphics->GetControlWithTag(kCtrlTagGateSequencer)->As<IVSequencerControl>();
    auto keyboard = mGraphics->GetControlWithTag(kCtrlTagKeyboard)->As<IVKeyboardControl>();
    auto cycleKeyboard = mGraphics->GetControlWithTag(kCtrlTagCycleKeyboardButton)->As<IVButtonControl>();
    auto play = mGraphics->GetControlWithTag(kCtrlTagPlayToggle)->As<IVToggleControl>();
    auto solo = mGraphics->GetControlWithTag(kCtrlTagSoloToggle)->As<IVToggleControl>();

    //////////////////////////////////////////////////////
    // So that can update the controls behind a curtain //
    //////////////////////////////////////////////////////

    // 2206: Instead of worrying about adding extra methods to these controls, 
    // just write out the full code for those methods here; for now at least...
    // (Maybe we will need oop... Or maybe not!! Hopefully not... Yeah, don't think we will need!!)
    if (mScreen == kScreenSequencerSyn1 || mScreen == kScreenSequencerSyn2 || mScreen == kScreenSequencerSyn3 || mScreen == kScreenSequencerSyn4 ||
        mScreen == kScreenSequencerSamp1 || mScreen == kScreenSequencerSamp2) {
      // keyboard->Update(mScreen);
      // midiSeq->Update(mScreen);
      // gateSeq->Update(mScreen);
      // TODO:
    } else if (mScreen == kScreenEditorSyn1 || mScreen == kScreenEditorSyn2 || mScreen == kScreenEditorSyn3 || mScreen == kScreenEditorSyn4) {
      // synth->Update(mScreen);

      ////////////////////
      // 2206: DO NOW!! //
      ////////////////////

      // core->Update(mScreen);

    } else if (mScreen == kScreenEditorSamp1 || mScreen == kScreenEditorSamp2) {
      // sampler->Update(mScreen);
      // TODO:
    }

    // title->Update(mScreen);
    std::string str = "";
    switch (mScreen) {
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

    ////////////////////////////////////////////////////////////////////////////////////
    // Then finally, reveal only the controls that are relevant to the current screen //
    ////////////////////////////////////////////////////////////////////////////////////

    home->Hide(false);
    play->Hide(false);
    solo->Hide(false);
    title->Hide(false);
    step->Hide(false);
    if (mScreen == kScreenSequencerSyn1 || mScreen == kScreenSequencerSyn2 || mScreen == kScreenSequencerSyn3 || mScreen == kScreenSequencerSyn4 ||
        mScreen == kScreenSequencerSamp1 || mScreen == kScreenSequencerSamp2) {
      int sync = mKeyboardBoundsDatas[mScreen - kScreenSequencerSyn1].mKeyboardBoundsIdx;
      keyboard->Hide(sync != 0);
      gateSeq->Hide(sync != 1);
      midiSeq->Hide(false);
      cycleKeyboard->Hide(false);
    } else if (mScreen == kScreenEditorSyn1 || mScreen == kScreenEditorSyn2 || mScreen == kScreenEditorSyn3 || mScreen == kScreenEditorSyn4) {
      // 2206: These are very naive pseudocode; the way i've been designing the UI, these 
      // are unlikely to be single controls; instead, a mishmash of 42 different knobs and 
      // buttons; that would be ok, this will just be a long Update() function (that's ok)
      core->Hide(false);
    } else if (mScreen == kScreenEditorSamp1 || mScreen == kScreenEditorSamp2) {
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

  std::vector<SynthData> mSynthDatas;
  std::vector<KeyboardBoundsData> mKeyboardBoundsDatas;

  Timer *mPlaybackTimer;
  IGraphics *mGraphics;

  int mPlaybackStepIdx;
  int mPrevNote;

  EScreens mScreen;
};
#endif
