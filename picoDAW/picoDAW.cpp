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
    auto gainKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 0).SubRectVertical(4, 3), /* kParamGain */[pGraphics, root](IControl* pCaller) {
      // Turns out this is an onclick function; useless to me!! Maybe i need params...
      // which are synced with their copies in rootuicontrol. and then, on screen switch,
      // the IParam's values are updated to the corresponding values in rootuicontrol...
    }, "GAIN", style);
    gainKnob->SetValue(1.); // As the initial value. BUT, instead a param should be used,
                            // whose initial value is 1., so that don't need this line...
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

// First of all, we know that picoDAW represents both an IPlugWasmUI and IPlugWasmDSP,
// with a portion of methods dedicated to one, and the rest the other, by preprocessor
// #ifdef guards. And, that both are IEditorDelegates. (The difference is that IPlugWasmUI 
// has access to a graphics API; it's an IGEditorDelegate that inherits the IEditorDelegate 
// base class). The path for this is a bit of a mess to see. In both .h files, e.g. 
// IPlugWasmUI.h you will see a #include "IPlugAPIBase.h", which itself includes a
// IPlugPluginBase.h, which then includes a IPlugDelegate_select.h, which like its name 
// implies, sets the delegate class to be either IEditorDelegate or IGEditorDelegate; 
// both with base class of IEditorDelegate!!
//
// The idea behind this seperation, is that when compiled into a WASM build, the two run
// in seperate javascript threads. So, a user launches picoDAW in their browser, and two
// threads are spawned. There's the main <script> thread, included in the index.HTML that
// spins in a loop drawing UI, and then there's a seperate AudioWorklet worker thread that
// deals with DSP in a loop, also spawned on load.
// 
// (i) Through iPlug2, what runs in the two threads is essentially C++ code. The IPlugWasmUI
// C++ code runs in the UI thread in a loop. Specifically, the "int main()" of IPlugWasmUI 
// included from "IPlug_include_in_plug_src.h", is compiled into a program with a proper 
// entry point. This entry point calls "StartMainLoopTimer", which eventually starts the loop
//
// The resulting program --- with an "int main()"-type entry point --- is called "picoDAW-ui.js",
// compiled by emcc into javascript through emmake make --makefile picoDAW-wasm-ui.mk in the 
// "makedist-wasm.sh" script; see also "picoDAW-wasm-ui.mk". This program is included in a <script> 
// tag in the main index.HTML in the "_initUI" function of "IPlugWasmBundle.js.template", so that
// the program runs in the main javascript thread when the "_initUI" function is reached
//
// (ii) The IPlugWasmDSP C++ code runs inside the AudioWorklet thread, also in a loop, but is a
// bit more subtle. This is because the AudioWorklet is quite explictly its own thread. It is a
// thread spawned by the main javascript thread. The only way it has access to our custom C++
// DSP code, converted to javascript, is through the "process" function specified in the 
// "AudioWorkletProcessor" API, custom-defined in the "IPlugWasmProcessor.js.template". The idea
// is that on every call made to "process" by the AudioWorklet thread, it will call our 
// corresponding "processBlock" C++ code that is a part of the IPlugWasmDSP delegate. This 
// is why the IPlugWasmDSP delegate doesn't have a "int main()" entry point; it's only 
// contribution to the AudioWorklet thread is its "processBlock" function, the AudioWorklet 
// internals instead contains the inevitable loop that runs in its thread
//
// So, the resulting binary, called "picoDAW-dsp.js", doesn't have a "int main()"-type entry point.
// Instead, the resulting "picoDAW-dsp.js" --- which can instead be thought of as a library of DSP 
// related functions; a DSP module --- is provided to to the AudioWorklet thread on spawning, which
// runs the actual DSP loop; "audioWorklet.addModule(dspModuleUrl)" in "IPlugWasmBundle.js.template".
// The binding for the javascript invocation of "processBlock" in "IPlugWasmProcessor.js.template",
// to the C++ "processBlock" inside the DSP module "picoDAW-dsp.js", is found in "IPlugWasmDSP.cpp"
// And the compilation into the DSP module, is found in "makedist-wasm.sh" through 
// "emmake make --makefile picoDAW-wasm-dsp.mk"; see also "picoDAW-wasm-dsp.mk"
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
// tapping onto the AudioWorklet thread's this.port, specified in the AudioWorklet
// API, with type 'param'. And you can see that in the corresponding AudioWorklet thread
// initialisation of "IPlugWasmProcessor.js.template", every time a message is passed to 
// the AudioWorklet through this.port, the "_onMessage" function inside it is called. It 
// checks the type of message, and if it is 'param', it calls the javascript "onParam" 
// function, which is bound through emscripten in "IPlugWasmDSP.cpp" to the 
// IPlugWasmDSP::OnParamMessage (aka picoDAW::OnParamMessage) function.
void picoDAW::OnParamChange(int paramIdx)
{
  // Sync our DSP with the change to the given IParam...
  // mDSP.SetParam(paramIdx, GetParam(paramIdx)->Value());
}

// As an example, we can look at the IVKnobControl UI element, initialised and owned
// by the main UI thread who is running IPlugWasmUI's StartMainLoopTimer on infinite
// inside a <script> HTML tag in the _initUI of "IPlugWasmBundle.js.template"; specifically
// it's running the "picoDAW-ui.js" script. (At this point it's worth pointing out that
// the IPlugWasmUI's StartMainLoopTimer is compiled by emcc into "picoDAW-ui.js" through
// emmake make --makefile picoDAW-wasm-ui.mk in the "makedist-wasm.sh" script; see also
// "picoDAW-wasm-ui.mk")
//
// The knob's value is changed by clicking and then dragging on it, upward for increasing 
// and downward for decreasing. And you can see in IKnobControlBase::OnMouseDrag, the 
// base class of IVKnobControl, that the change is reflected in the UI by first
// IControl::SetValue(v) of course, and then also IControl::SetDirty(true), so that:
// (i) the knob control is redrawn on the next frame, and (ii) the change is communicated
// to the AudioWorklet thread by first calling IPlugWasmUI::SendParamterValueFromUI, 
// which as above makes the javascript invocation "window.iPlugController.setParam", 
// which calls "this.workletNode.port.postMessage({ type: 'param' ... })", which finally
// results in the AudioWorklet's "_onMessage" being triggered, because it is bound to 
// its "this.port.onmessage" in "IPlugWasmProcessor.js.template". The "_onMessage" does
// a switch case on the message's type, and if 'param', calls the javascript function
// "onParam", which as above makes the C++ invocation "_onParam" in "IPlugWasmDSP.cpp",
// which finally calls this IPlugWasmDSP::OnParamMessage (aka picoDAW::OnParamMessage).
// The remaining UI updates are communicated to the DSP, through arbitrary messages
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
