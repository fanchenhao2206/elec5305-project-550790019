#include "picoDAW.h"
#include "IPlug_include_in_plug_src.h"
#include "LFO.h"
#include "IconsForkAwesome.h"
#include "IconsFontaudio.h"
#include "IPlugMidi.h"
#include "RootUIControl.h" // A root control that is invisible; holds data and funcs for drawing UI

#include <string>
#include <utility>

picoDAW::picoDAW(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  // Knob and slider controls use IParams; updates are communicated by us the IPlugWasmUI
  // to the DSP through the parameter pipeline; see IPlugWasmDSP::onParamChange...
  GetParam(kParamGain)->InitDouble("Gain", 100., 0., 100.0, 0.01, "%");
  GetParam(kParamAttack)->InitDouble("Attack", 10., 1., 1000., 0.1, "ms", IParam::kFlagsNone, "ADSR", IParam::ShapePowCurve(3.));
  GetParam(kParamDecay)->InitDouble("Decay", 10., 1., 1000., 0.1, "ms", IParam::kFlagsNone, "ADSR", IParam::ShapePowCurve(3.));
  GetParam(kParamSustain)->InitDouble("Sustain", 50., 0., 100., 1, "%", IParam::kFlagsNone, "ADSR");
  GetParam(kParamRelease)->InitDouble("Release", 10., 2., 1000., 0.1, "ms", IParam::kFlagsNone, "ADSR");
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
    IRECT keyboardBounds = b.GetFromBottom(120).GetReducedFromLeft(160).GetReducedFromRight(160);
    IRECT sequencerBounds = b.GetReducedFromBottom(120 + 5).GetReducedFromLeft(160).GetReducedFromRight(160).GetReducedFromTop(80 + 7.5);
    IRECT stepBounds = sequencerBounds.GetFromTop(20).GetTranslated(0, -20 - 5);
    IRECT cycleButtonBounds = keyboardBounds.GetCentredInside(160, 120).GetTranslated(-(320 + 80), 2.5);
    IRECT playButtonBounds = IRECT::MakeXYWH(0, 0, 160, 60).GetTranslated(800 -2.5, 2.5);
    IRECT soloButtonBounds = playButtonBounds.GetTranslated(-160, 0);
    IRECT titleBounds = IRECT(homeBounds.R, 0, soloButtonBounds.L, 60);

    // And then placing controls into those subrects; Notice that some occupy the same
    // subrect, the root control upon Update() will choose which one comes out on top

    // Common controls...
    auto root = new RootUIControl(pGraphics);
    auto title = new ITextControl(titleBounds, "SYN1 SEQ", IText(36.f, "Roboto-Regular"));
    auto homeButton = new IVButtonControl(homeBounds, [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenNavigation;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "picoDAW", style);
    auto step = new ITextControl(stepBounds, "INSERT CURRENT STEP IN PATTERN HERE", style.labelText, COLOR_GRAY);

    // Navigation controls...
    auto navigationSyn1Seq = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 0).SubRectVertical(2, 0), [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenSequencerSyn1;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN1 SEQ", style);
    auto navigationSyn2Seq = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 1).SubRectVertical(2, 0), [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenSequencerSyn2;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN2 SEQ", style);
    auto navigationSyn3Seq = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 2).SubRectVertical(2, 0), [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenSequencerSyn3;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN3 SEQ", style);
    auto navigationSyn4Seq = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 3).SubRectVertical(2, 0), [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenSequencerSyn4;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN4 SEQ", style);
    auto navigationSamp1Seq = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 4).SubRectVertical(2, 0), [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenSequencerSamp1;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SAMP1 SEQ", style.WithColor(kFG, COLOR_BLUE));
    auto navigationSamp2Seq = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 5).SubRectVertical(2, 0), [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenSequencerSamp2;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SAMP2 SEQ", style.WithColor(kFG, COLOR_BLUE));
    auto navigationSyn1Edit = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 0).SubRectVertical(2, 1), [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenEditorSyn1;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN1 EDIT", style);
    auto navigationSyn2Edit = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 1).SubRectVertical(2, 1), [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenEditorSyn2;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN2 EDIT", style);
    auto navigationSyn3Edit = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 2).SubRectVertical(2, 1), [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenEditorSyn3;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN3 EDIT", style);
    auto navigationSyn4Edit = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 3).SubRectVertical(2, 1), [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenEditorSyn4;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN4 EDIT", style);
    auto navigationSamp1Edit = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 4).SubRectVertical(2, 1), [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenEditorSamp1;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SAMP1 EDIT", style.WithColor(kFG, COLOR_BLUE));
    auto navigationSamp2Edit = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 5).SubRectVertical(2, 1), [pGraphics, root](IControl* pCaller) {
      root->mScreen = kScreenEditorSamp2;
      root->Update();
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SAMP2 EDIT", style.WithColor(kFG, COLOR_BLUE));

    // 2206: Mixer controls...
    auto mixer = new ITextControl(keyboardBounds, "MIXER", style.labelText, COLOR_GRAY);

    // Sequencer controls...
    auto midiSeq = new IVSequencerControl(sequencerBounds, 12, 16);
    auto gateSeq = new IVSequencerControl(keyboardBounds, 2, 16);
    auto keyboard = new IVKeyboardControl(keyboardBounds);
    auto cycleKeyboard = new IVButtonControl(cycleButtonBounds, [pGraphics, root](IControl* pCaller) {
      // Cycle through the sequencer's currently active sub-control
      KeyboardBoundsData &kbData = root->mKeyboardBoundsDatas[root->mScreen - kScreenSequencerSyn1];
      kbData.mKeyboardBoundsIdx += 1; 
      kbData.mKeyboardBoundsIdx %= kbData.mKeyboardBoundsTotal;
                  
      // Hide/Show controls so only the active one is displayed
      int sync = kbData.mKeyboardBoundsIdx;
      pGraphics->GetControlWithTag(kCtrlTagKeyboard)->Hide(sync != 0);
      pGraphics->GetControlWithTag(kCtrlTagGateSequencer)->Hide(sync != 1);

      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "TOGGLE", style);
    auto play = new IVToggleControl(playButtonBounds, [pGraphics, root, midiSeq, gateSeq](IControl* pCaller) {
      if (pCaller->GetValue() > 0.5) {
        // Is currently on, so was previously off; start playback wrt. current tempo
        root->mPlaybackTimer = Timer::Create([root, midiSeq, gateSeq](Timer &t) {
          IMidiMsg msg;
          int channelIdx = 0; // 2206: send to other channel if required
          int &stepIdx = root->mPlaybackStepIdx;
          int &prevNote = root->mPrevNote;
          const int nSteps = midiSeq->mNCols;

          // Recall that cells are given as a row index from top to bottom; matching graphics
          int midiCell = midiSeq->mCells[stepIdx];

          // But we want a row index from bottom to top; matching piano roll sequencers
          int midiStep = (midiSeq->mNRows - 1) - midiCell;

          // 2206: Playback is a mess, because currently midiSeq is global across all six instruments
          // TODO: don't just take from the first instrument [0], like we do here...
          // We take the octave from RootUIControl, which is relative to C4 == 60;
          // For example, if octave is -1, then we are in the C3 == 48 octave.
          int octave = root->mSynthDatas[0].mOctave;
          int currNote = (midiCell < 0) ? -1 : (60 + octave * 12) + midiStep;
          int currVelocity = 80;

          // For knowing whether to hold this note in legato with the previous identical note
          int gateCell = gateSeq->mCells[stepIdx];
          int currGate = (gateCell < 0) ? 0 : gateCell;

          // If at start of pattern, then assume (non-existent) prev gate is not legato; retrig!!
          int prevGate = (stepIdx == 0) ? 1 : gateSeq->mCells[stepIdx - 1];

          if (currNote < 0) {
            // Nothing to play this step, so turn off prev and that's it
            if (prevNote >= 0) {
              msg.MakeNoteOffMsg(prevNote, 0, channelIdx);
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
              msg.MakeNoteOffMsg(prevNote, 0, channelIdx);
              root->GetDelegate()->SendMidiMsgFromUI(msg);
              msg.MakeNoteOnMsg(currNote, currVelocity, 0, channelIdx);
              root->GetDelegate()->SendMidiMsgFromUI(msg);
            } /* else {} */ // Play legato with previous identical note
          } else /* if (currNote != prevNote) */ {
            // Something to play this step, and it's different than last step
            msg.MakeNoteOffMsg(prevNote, 0, channelIdx);
            root->GetDelegate()->SendMidiMsgFromUI(msg);
            msg.MakeNoteOnMsg(currNote, currVelocity, 0, channelIdx);
            root->GetDelegate()->SendMidiMsgFromUI(msg);
          }

          prevNote = currNote;
          stepIdx += 1;
          stepIdx %= nSteps; // to cycle around
        }, 125); // 2206: use current tempo, not hardcoded 120bpm
      } else {
        // Is currently off, so was previously on; stop playback
        IMidiMsg msg;
        msg.MakeNoteOffMsg(root->mPrevNote, 0, 0); // 2206: stop playback of other channels too
        root->GetDelegate()->SendMidiMsgFromUI(msg);
        root->mPrevNote = -1;
        delete root->mPlaybackTimer;
        root->mPlaybackTimer = nullptr;
        root->mPlaybackStepIdx = 0;
      }
    }, "", style, "PLAY", "PLAY"); // Although, this one appears on all screens...
    auto solo = new IVToggleControl(soloButtonBounds, [pGraphics](IControl* pCaller) {
      /* 2206: once mixer is ready... */
    }, "", style, "SOLO", "SOLO"); // And this one too... But, disabled except in SEQ or EDIT screens

    // 2206: Synth controls...
    // Break sequencerBounds into a 4x4 grid...

    auto coreSwitch = new IVSwitchControl(sequencerBounds.SubRectHorizontal(4, 0).SubRectVertical(4, 1), [pGraphics, root](IControl* pCaller) {
      // Which editor screen are we on? (Has to be one of the synth editors...)
      int synEditIdx = root->mScreen - kScreenEditorSyn1; // [0..3]

      // Modify the corresponding struct in root control...
      SynthData &synData = root->mSynthDatas[synEditIdx];
      synData.mCoreIdx += 1;
      synData.mCoreIdx %= synData.mCoreTotal;

      // Send arbitrary message to DSP. It needs to know which voice to modify and
      // what to modify it to. We need to send it synEditIdx and synData.mCoreIdx...
      // Maybe std::pair<int, int>? Let's try for now...
      root->GetDelegate()->SendArbitraryMsgFromUI(1, kCtrlTagCoreSwitch, sizeof(std::pair<int,int>), new std::pair<int,int>(synEditIdx, synData.mCoreIdx));

      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "CORE", style);
    auto octaveSwitch = new IVSwitchControl(sequencerBounds.SubRectHorizontal(4, 0).SubRectVertical(4, 2), [pGraphics, root](IControl* pCaller) {
      // Which editor screen are we on? (Has to be one of the synth editors...)
      int synEditIdx = root->mScreen - kScreenEditorSyn1; // [0..3]

      // Modify the corresponding struct in root control...
      SynthData &synData = root->mSynthDatas[synEditIdx];
      synData.mOctave += 1;
      if (synData.mOctave > 2) synData.mOctave = -2;

      // The playback button will use this mOctave value to generate the 
      // appropriate MIDI messages to send to DSP from us the UI; meaning
      // we don't need to send any arbitrary messages here...

      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "OCTAVE", style);
    auto gainKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 0).SubRectVertical(4, 3), kParamGain, "GAIN", style);

    auto attackKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 1).SubRectVertical(4, 0), kParamAttack, "ATTACK", style);
    auto decayKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 1).SubRectVertical(4, 1), kParamDecay, "DECAY", style);
    auto sustainKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 1).SubRectVertical(4, 2), kParamSustain, "SUSTAIN", style);
    auto releaseKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 1).SubRectVertical(4, 3), kParamRelease, "RELEASE", style);

    // 2206: Sampler controls...

    pGraphics->AttachControl(root, kCtrlTagRoot);
    pGraphics->AttachControl(title, kCtrlTagTitle);
    pGraphics->AttachControl(homeButton, kCtrlTagHomeButton);
    pGraphics->AttachControl(step, kCtrlTagStep);

    pGraphics->AttachControl(navigationSyn1Seq, kCtrlTagNavigationSyn1Seq);
    pGraphics->AttachControl(navigationSyn2Seq, kCtrlTagNavigationSyn2Seq);
    pGraphics->AttachControl(navigationSyn3Seq, kCtrlTagNavigationSyn3Seq);
    pGraphics->AttachControl(navigationSyn4Seq, kCtrlTagNavigationSyn4Seq);
    pGraphics->AttachControl(navigationSamp1Seq, kCtrlTagNavigationSamp1Seq);
    pGraphics->AttachControl(navigationSamp2Seq, kCtrlTagNavigationSamp2Seq);

    pGraphics->AttachControl(navigationSyn1Edit, kCtrlTagNavigationSyn1Edit);
    pGraphics->AttachControl(navigationSyn2Edit, kCtrlTagNavigationSyn2Edit);
    pGraphics->AttachControl(navigationSyn3Edit, kCtrlTagNavigationSyn3Edit);
    pGraphics->AttachControl(navigationSyn4Edit, kCtrlTagNavigationSyn4Edit);
    pGraphics->AttachControl(navigationSamp1Edit, kCtrlTagNavigationSamp1Edit);
    pGraphics->AttachControl(navigationSamp2Edit, kCtrlTagNavigationSamp2Edit);

    pGraphics->AttachControl(mixer, kCtrlTagMixer);

    pGraphics->AttachControl(coreSwitch, kCtrlTagCoreSwitch);
    pGraphics->AttachControl(octaveSwitch, kCtrlTagOctaveSwitch);
    pGraphics->AttachControl(gainKnob, kCtrlTagGainKnob);
    pGraphics->AttachControl(attackKnob, kCtrlTagAttackKnob);
    pGraphics->AttachControl(decayKnob, kCtrlTagDecayKnob);
    pGraphics->AttachControl(sustainKnob, kCtrlTagSustainKnob);
    pGraphics->AttachControl(releaseKnob, kCtrlTagReleaseKnob);

    pGraphics->AttachControl(midiSeq, kCtrlTagMidiSequencer);
    pGraphics->AttachControl(gateSeq, kCtrlTagGateSequencer);
    pGraphics->AttachControl(keyboard, kCtrlTagKeyboard);
    pGraphics->AttachControl(cycleKeyboard, kCtrlTagCycleKeyboardButton);
    pGraphics->AttachControl(play, kCtrlTagPlayToggle);
    pGraphics->AttachControl(solo, kCtrlTagSoloToggle);

    // Load home page and return to event loop...
    root->Update();
  };
#endif
}

#if IPLUG_DSP
void picoDAW::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  // Inputs to mDSP is empty, because it is the first DSP plugin in the chain!!
  mDSP.ProcessBlock(nullptr, outputs, 2, nFrames, mTimeInfo.mPPQPos, mTimeInfo.mTransportIsRunning);

  // mDSP generates outputs, that we then pass BACK to UI through mMeterSender, for visualisation
  mMeterSender.ProcessBlock(outputs, nFrames, kCtrlTagMeter);
}

void picoDAW::OnIdle()
{
  // Whenever the DSP has time (finished ProcessBlock early, for example), it'll send control
  // messages back to UI, through ISenders like mMeterSender, letting the UI know of the current
  // status of the DSP output, or any other changes to send. Specifically, the DSP does this by
  // filling up IPeakAvgSender::mQueue through PushData, from which the UI takes via TransmitData
  mMeterSender.TransmitData(*this);
}

void picoDAW::OnReset()
{
  mDSP.Reset(GetSampleRate(), GetBlockSize());
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
  mDSP.ProcessMidiMsg(msg);
  // mASP.ProcessMidiMsg(msg); // Send to all instruments, who will ignore the message 
  // mBSP.ProcessMidiMsg(msg); // if not intended for them? Specified by channel which
  // mCSP.ProcessMidiMsg(msg); // ranges from [0, 15]; picoDAW is just one instrument!
                               // See... event.mAddress (.mChannel?)
}

// There are two threads in the WASM build of picoDAW. The main javascript thread that will 
// spin in an infinite loop running the "int main()" function of IPlugWasmUI, compiled by emcc
// into "picoDAW-ui.js". And a secondary AudioWorklet worker thread, spawned by the main thread 
// before entering the UI loop, endlessly calling its "process" function, specified in the
// AudioWorkletProcessor API and defined in "IPlugWasmProcessor.js.template". Each invocation
// calls "processBlock", which is bound to IPlugWasmDSP's "processBlock" C++ code, the transpiled
// javascript code located inside the "picoDAW-dsp.js" module provided on thread initialisation
//
// How do the two threads communicate? They communicate via the javascript functions
// "setParam", and "onParam", for communicating parameter changes for example. What I 
// mean is, in the IPlugWasmUI StartMainLoopTimer loop, whenever it runs into a C++ call
// to IPlugWasmUI::SendParameterValueFromUI in "IPlugWasmUI.cpp", it makes the javascript
// invocation "window.iPlugController.setParam", aka, calling IPlugWasmController.setParam
// from "IPlugWasmBundle.js.template":
//
// setParam(paramIdx, value) {
//   this.workletNode?.port.postMessage({ type: 'param', paramIdx, value });
// }
//
// tapping onto the AudioWorklet thread's "this.port", specified in the AudioWorklet
// API, with type 'param'. And you can see that in the corresponding AudioWorklet thread
// initialisation of "IPlugWasmProcessor.js.template", every time a message is passed to 
// the AudioWorklet through this.port, the "_onMessage" function inside it is called. It 
// checks the type of message, and if it is 'param', it calls the javascript "onParam" 
// function, which is bound through emscripten in "IPlugWasmDSP.cpp" to the IPlugWasmDSP::
// OnParamMessage function, which after calling IPlugAPIBase::SetParameterValue, calls
// our picoDAW::OnParamChange here...
void picoDAW::OnParamChange(int paramIdx)
{
  // Sync our DSP with the change to the given IParam...
  mDSP.SetParam(paramIdx, GetParam(paramIdx)->Value());
}

// Here below you'll see though, that an alternative is to communicate to the DSP backend
// through arbitrary messages. These would work well. But because the iPlug2-provided knobs
// and sliders already integrate so well with IParams, it's more work --- specifically to
// modify the provided controls --- to use arbitrary messages, instead of the existing
// "sending" of parameter change message from the IPlugWasmUI to the IPlugWasmDSP, by 
// calling IPlugWasmUI::SendParameterValueFromUI, which eventually leads to the above
// IPlugWasmDSP::OnParamChange. So, the change to make to the knob and slider etc. controls,
// to make them use arbitrary messages instead, is to change the IControl::SetDirty function
// to call IPlugWasmUI::SendArbitraryMsgFromUI instead of IPlugWasmUI::SendParameterValueFromUI;
// again, pretty pointless because the parameter change approach is essentially equivalent.
// Arbitrary messages are used for these cases here though, because, in my opinion, they are
// more appropriate. But, TODO: they can definitely be converted to IParams too, through
// InitEnums, probably...
bool picoDAW::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData)
{
  if (ctrlTag == kCtrlTagCoreSwitch && msgTag == 1) {
    // Received arbitrary message from core switch. It gave us which voice to 
    // modify and what to modify it to. We received synEditIdx at .first, and
    // synData.mCoreIdx at .second. Using std::pair is certainly better than 
    // re-inventing a new two-member struct...
    const std::pair<int,int> msg = *static_cast<const std::pair<int,int>*>(pData);
    int synEditIdx = msg.first;
    int coreIdx = msg.second;
    mDSP.mSynth.mVoiceAllocator.mVoicePtrs[synEditIdx]->SetSynthCore(coreIdx);
    delete static_cast<const std::pair<int,int>*>(pData); // This makes you feel like you're
                                                          // actually programming doesn't it
  }
  
  return false;
}
#endif
