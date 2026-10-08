#include "picoDAW.h"
#include "IPlug_include_in_plug_src.h"
#include "LFO.h"
#include "IconsForkAwesome.h"
#include "IconsFontaudio.h"
#include "IPlugMidi.h"

#include <string>

#if IPLUG_EDITOR
// A root control that is invisible; holds data and funcs for drawing UI
class RootUIControl : public IControl {
public:
  RootUIControl(IGraphics *pGraphics)
  : IControl(IRECT()),
    mGraphics(pGraphics), mScreen(kScreenSyn1Seq),
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
      ctrlTag.second->Hide(true);
    }

    // And then update the controls behind the curtain
    auto midiSeq = mGraphics->GetControlWithTag(kCtrlTagMidiSequencer)->As<IVSequencerControl>();
    auto gateSeq = mGraphics->GetControlWithTag(kCtrlTagGateSequencer)->As<IVSequencerControl>();
    auto keyboard = mGraphics->GetControlWithTag(kCtrlTagKeyboard)->As<IVKeyboardControl>();
    auto title = mGraphics->GetControlWithTag(kCtrlTagTitle)->As<ITextControl>();
    auto cycle = mGraphics->GetControlWithTag(kCtrlTagCycleButton)->As<IVButtonControl>();
    auto home = mGraphics->GetControlWithTag(kCtrlTagHomeButton)->As<IVButtonControl>();
    auto play = mGraphics->GetControlWithTag(kCtrlTagPlayToggle)->As<IVToggleControl>();
    auto solo = mGraphics->GetControlWithTag(kCtrlTagSoloToggle)->As<IVToggleControl>();

    // 2206: Instead of worrying about adding extra methods to these controls, 
    // just write out the full code for those methods here; for now at least... (we hate oop!!)
    // keyboard->Update(mScreen);
    // midiSeq->Update(mScreen);
    // gateSeq->Update(mScreen);
    // title->Update(mScreen);
    /* title->Update(mScreen); */ {
      std::string str = "";
      switch (mScreen) {
        case kScreenHome:
          str = "HOME";
          break;
        case kScreenSyn1Seq:
          str = "SYN1 SEQ";
          break;
        case kScreenSyn1Edit:
          str = "SYN1 EDIT";
          break;
        default:
          str = "UNIMPLEMENTED";
          break;
      }
      title->SetStr(str.c_str());
    }

    // And lastly, unhide the updated controls that are relevant to the current screen
    home->Hide(false);
    title->Hide(false);
    if (mScreen == kScreenSyn1Seq) {
      int sync = mKeyboardBoundsIdx;
      keyboard->Hide(sync != 0);
      gateSeq->Hide(sync != 1);
      title->Hide(false);
      midiSeq->Hide(false);
      cycle->Hide(false);
      play->Hide(false);
      solo->Hide(false);
    } else {
      // navigation->Hide(false); // 2206: the "home" screen that shows buttons navigate screens
                                  // takes up the sequencerBounds
      // mixer->Hide(false);      // 2206: the mixer menu that shows knobs and mute/solo buttons
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
  mMakeGraphicsFunc = [this]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [](IGraphics* pGraphics) {
    pGraphics->EnableMouseOver(true);
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
        COLOR_RED, // Pressed
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

    // Making subrects of main window...
    const IRECT b = pGraphics->GetBounds().GetPadded(0);
    IRECT homeBounds = IRECT::MakeXYWH(0, 0, 160, 60).GetTranslated(2.5, 2.5);
    IRECT keyboardBounds = b.GetFromBottom(160).GetReducedFromLeft(160).GetReducedFromRight(160);
    IRECT sequencerBounds = b.GetReducedFromBottom(160 + 20).GetReducedFromLeft(160).GetReducedFromRight(160).GetReducedFromTop(80 - 20);
    IRECT cycleButtonBounds = keyboardBounds.GetCentredInside(160, 160).GetTranslated(-(320 + 80), 2.5);
    IRECT playButtonBounds = IRECT::MakeXYWH(0, 0, 160, 60).GetTranslated(800 -2.5, 2.5);
    IRECT soloButtonBounds = playButtonBounds.GetTranslated(-160, 0);
    IRECT titleBounds = IRECT(homeBounds.R, 0, soloButtonBounds.L, 60);

    // And then placing controls into those subrects; Notice that some occupy the same
    // subrect, the root control upon Update() will choose which one comes out on top 
    auto root = new RootUIControl(pGraphics); 
    auto midiSeq = new IVSequencerControl(sequencerBounds, 12, 16);
    auto gateSeq = new IVSequencerControl(keyboardBounds, 2, 16);
    auto keyboard = new IVKeyboardControl(keyboardBounds);
    auto title = new ITextControl(titleBounds, "SYN1 SEQ", IText(36.f, "Roboto-Regular"));
    auto homeButton = new IVButtonControl(homeBounds, [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenHome;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "picoDAW", style);
    auto cycleButton = new IVButtonControl(cycleButtonBounds, [pGraphics, root](IControl* pCaller) {
      // Cycle through its currently active sub-control
      root->mKeyboardBoundsIdx += 1; 
      root->mKeyboardBoundsIdx %= root->mKeyboardBoundsTotal;
                  
      // Hide/Show controls so only the active one is displayed
      int sync = root->mKeyboardBoundsIdx;
      pGraphics->GetControlWithTag(kCtrlTagKeyboard)->Hide(sync != 0);
      pGraphics->GetControlWithTag(kCtrlTagGateSequencer)->Hide(sync != 1);

      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "TOGGLE", style);
    auto playToggle = new IVToggleControl(playButtonBounds, [pGraphics, root, midiSeq, gateSeq](IControl* pCaller) {
      if (pCaller->GetValue() > 0.5) {
        // Is currently on, so was previously off; start playback wrt. current tempo
        root->mPlaybackTimer = Timer::Create([root, midiSeq, gateSeq](Timer &t) {
          IMidiMsg msg;
          int &stepIdx = root->mPlaybackStepIdx;
          int &prevNote = root->mPrevNote;
          const int nSteps = midiSeq->mNCols;

          // Recall that cells are given as a row index from top to bottom; matching graphics
          int midiCell = midiSeq->mCells[stepIdx];

          // But we want a row index from bottom to top; matching piano roll sequencers
          int midiStep = (midiSeq->mNRows - 1) - midiCell;

          // 2206: assumes we are in C3 octave (C3 === 48); also, velocity sequencer
          int currNote = (midiCell < 0) ? -1 : 48 + midiStep;
          int currVelocity = 80;

          // For knowing whether to hold this note in legato with the previous identical note
          int gateCell = gateSeq->mCells[stepIdx];
          int currGate = (gateCell < 0) ? 0 : gateCell;

          // If at start of pattern, then assume (non-existent) prev gate is not legato; retrig!!
          int prevGate = (stepIdx == 0) ? 1 : gateSeq->mCells[stepIdx - 1];

          if (currNote < 0) {
            // Nothing to play this step, so turn off prev and that's it
            if (prevNote >= 0) {
              msg.MakeNoteOffMsg(prevNote, 0);
              root->GetDelegate()->SendMidiMsgFromUI(msg);
            }
          } else if (currNote == prevNote) {
            // Something to play this step, but it's the same note as the previous step
            // Should this be played legato with previous step? Or retrigger envelope...
            // The gate sequencer should be used to decide that!!
            if (currGate == 1 || stepIdx == 0 || (prevGate == 1 && currGate == 0)) {
              // Retrigger envelope if requested on current step, or if at start of pattern,
              // or, if the current step is at the start of a (possible) chain of legato notes;
              // the case if the previous note was NOT legato, and the current note IS legato 
              // 
              // However, what if the next note, is instead not legato? Well, in that case, 
              // technically the current legato note still starts a chain of legato notes, 
              // but of course its length is just one... In this case though, lets retrigger the
              // current legato note anyways!! It just means that marking a note as legato is 
              // meaningless, if it's by its lonesome; only two or more legato identical notes 
              // in a row produces the legato effect, which is what we should expect of it!!
              msg.MakeNoteOffMsg(prevNote, 0);
              root->GetDelegate()->SendMidiMsgFromUI(msg);
              msg.MakeNoteOnMsg(currNote, currVelocity, 0);
              root->GetDelegate()->SendMidiMsgFromUI(msg);
            } /* else {} */ // Play legato with previous identical note
          } else /* if (currNote != prevNote) */ {
            // Something to play this step, and it's different than last step
            msg.MakeNoteOffMsg(prevNote, 0);
            root->GetDelegate()->SendMidiMsgFromUI(msg);
            msg.MakeNoteOnMsg(currNote, currVelocity, 0);
            root->GetDelegate()->SendMidiMsgFromUI(msg);
          }

          prevNote = currNote;
          stepIdx += 1;
          stepIdx %= nSteps; // to cycle around
        }, 125); // 2206: use current tempo, not hardcoded 120bpm
      } else {
        // Is currently off, so was previously on; stop playback
        IMidiMsg msg;
        msg.MakeNoteOffMsg(root->mPrevNote, 0);
        root->GetDelegate()->SendMidiMsgFromUI(msg);
        root->mPrevNote = -1;
        delete root->mPlaybackTimer;
        root->mPlaybackTimer = nullptr;
        root->mPlaybackStepIdx = 0;
      }
    }, "", style, "PLAY", "PLAY");
    auto soloToggle = new IVToggleControl(soloButtonBounds, [pGraphics](IControl* pCaller) {
      /* 2206: once mixer is ready... */
    }, "", style, "SOLO", "SOLO");

    pGraphics->AttachControl(root, kCtrlTagRoot);
    pGraphics->AttachControl(midiSeq, kCtrlTagMidiSequencer);
    pGraphics->AttachControl(title, kCtrlTagTitle);
    pGraphics->AttachControl(gateSeq, kCtrlTagGateSequencer);
    pGraphics->AttachControl(keyboard, kCtrlTagKeyboard);
    pGraphics->AttachControl(homeButton, kCtrlTagHomeButton);
    pGraphics->AttachControl(cycleButton, kCtrlTagCycleButton);
    pGraphics->AttachControl(playToggle, kCtrlTagPlayToggle);
    pGraphics->AttachControl(soloToggle, kCtrlTagSoloToggle);

    // Load home page and return to event loop...
    root->Update();
  };
#endif
}

#if IPLUG_DSP
void picoDAW::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  // We are given a (hopefully) empty buffer "outputs" to fill in with samples,
  // presumably played back through final audio output...

  // Fill in outputs with samples to play back; of course, if no MIDI KeyOn messages in queue,
  // probably nothing here right? (Not entirely true, because of the Release in ADSR envelope)
  // Point being, MIDI messages have been delivered to this DSP, and now we processes those 
  // messages; process to produce a block of samples, hence ProcessBlock()
  mInstrumentDSP.ProcessBlock(nullptr, outputs, 2, nFrames, mTimeInfo.mPPQPos, mTimeInfo.mTransportIsRunning);
  mMeterSender.ProcessBlock(outputs, nFrames, kCtrlTagMeter);
  mLFOVisSender.PushData({kCtrlTagLFOVis, {float(mInstrumentDSP.mLFO.GetLastOutput())}});
}

void picoDAW::OnIdle()
{
  // Send message from dlg, to the UI control specified by ctrlTag;
  // dlg interfaces between UI and DSP, so it essentially transmits
  // message received from any DSP's, to that specific UI control
  //
  // Specifically, these sender instances are a part of dlg (aka *this)
  // and they each maintain a queue of message datas that will be relevant
  // to specific UI controls; these senders know which specific UI controls
  // (on initialisation), so will remember to include the relevant ctrlTag
  // in the message datas it adds to the queue
  //
  // In that sense, these TransmitData are literal helper functions, emptying
  // the queue and sending messages to the addresses specified within them;
  // no thinking required; mMeterSender would be sending the levels produced
  // by mInstrumentDSP to the UI control given by kCtrlTagMeter; exactly what is done
  // through mMeterSender.ProcessBlock, for example
  mMeterSender.TransmitData(*this);
  mLFOVisSender.TransmitData(*this);
}

void picoDAW::OnReset()
{
  mInstrumentDSP.Reset(GetSampleRate(), GetBlockSize());
  mMeterSender.Reset(GetSampleRate());
}

// Received MIDI message from MIDI message maker, internal 
// (keyboard player, sequencer editor, playback generator),
// or external, like a normal USB MIDI controller
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
  // Route that message to the internal DSP's who need it
  mInstrumentDSP.ProcessMidiMsg(msg);
  // mInstrumentASP.ProcessMidiMsg(msg); // Send to all instruments, who will ignore the message 
  // mInstrumentBSP.ProcessMidiMsg(msg); // if not intended for them? Specified by channel which
  // mInstrumentCSP.ProcessMidiMsg(msg); // ranges from [0, 15]; picoDAW is just one instrument!
}

void picoDAW::OnParamChange(int paramIdx)
{
  mInstrumentDSP.SetParam(paramIdx, GetParam(paramIdx)->Value());
}

void picoDAW::OnParamChangeUI(int paramIdx, EParamSource source)
{
  #if IPLUG_EDITOR
  if (auto pGraphics = GetUI())
  {
    // 2206: the value tied to this paramIdx has changed; if the paramIdx
    // was for the play button, for example, then its value changing means...
    // the user pressed the play button! Well, let's first double check its value
    // is true; GetParam(kParamPlayButton)->Bool() that is. And if it is, then 
    // the user must have pressed it, so we leave it on, until the user either 
    // presses it again---triggering its value to turn to false---or the song 
    // finishes and we are the ones to turn it off; we'll do that somewhere.

    // So, if the value is on, and previously was off; start playback!
    // Ideally, still allow the rest of the UI to be interacted with, 
    // and any other DSP modules to keep running. In other words, DO NOT 
    // hang here for the song duration... Trigger the flag for starting playback,
    // and exit. Notably, a mPlaybackDSP module will receive this flag change
    // and it is what starts real playback. So we need a seperate mPlaybackDSP 
    // module (probably); and the current mInstrumentDSP is really mKeyboardDSP, or 
    // mSequencerEditDSP (playing the test sounds as draw on sequencer map).
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
    mInstrumentDSP.mSynth.SetPitchBendRange(bendRange);
  }
  
  return false;
}
#endif
