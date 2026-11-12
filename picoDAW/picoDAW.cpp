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
  // Current instrument...
  GetParam(kParamVoiceIdx)->InitEnum("Voice", 0, { "SYNTH1", "SYNTH2", "SYNTH3", "SYNTH4", "SAMP1", "SAMP2" });

  // Knob and slider controls use IParams; updates are communicated by us the IPlugWasmUI
  // to them the DSP through the parameter pipeline; see IPlugWasmDSP::onParamChange...
  GetParam(kParamVoiceGain)->InitDouble("Gain", 100., 0., 100.0, 0.01, "%");
  GetParam(kParamVoiceOctave)->InitInt("Octave", 0, -2, 2);

  GetParam(kParamVoiceAttack)->InitDouble("Attack", 10., 1., 1000., 0.1, "ms", IParam::kFlagsNone, "ADSR", IParam::ShapePowCurve(3.));
  GetParam(kParamVoiceDecay)->InitDouble("Decay", 10., 1., 1000., 0.1, "ms", IParam::kFlagsNone, "ADSR", IParam::ShapePowCurve(3.));
  GetParam(kParamVoiceSustain)->InitDouble("Sustain", 50., 0., 100., 1, "%", IParam::kFlagsNone, "ADSR");
  GetParam(kParamVoiceRelease)->InitDouble("Release", 10., 2., 1000., 0.1, "ms", IParam::kFlagsNone, "ADSR");

  GetParam(kParamSynthCoreIdx)->InitEnum("Core", 0, { "SINE", "TRIANGLE", "SAW" });

  GetParam(kParamFilterCutoff)->InitDouble("Cutoff", 2000., 20., 20000., 1., "Hz", IParam::kFlagsNone, "Filter", IParam::ShapePowCurve(3.) );
  GetParam(kParamFilterResonance)->InitDouble("Resonance", 0.1, 0.1, 100., 0.1, "%", IParam::kFlagsNone, "Filter");

  GetParam(kParamFilterTypeIdx)->InitEnum("Type", 0, { "LOWPASS", "HIGHPASS", "BANDPASS" });

  GetParam(kParamFilterAttack)->InitDouble("Attack", 10., 1., 1000., 0.1, "ms", IParam::kFlagsNone, "ADSR", IParam::ShapePowCurve(3.));
  GetParam(kParamFilterDecay)->InitDouble("Decay", 10., 1., 1000., 0.1, "ms", IParam::kFlagsNone, "ADSR", IParam::ShapePowCurve(3.));
  GetParam(kParamFilterSustain)->InitDouble("Sustain", 50., 0., 100., 1, "%", IParam::kFlagsNone, "ADSR");
  GetParam(kParamFilterRelease)->InitDouble("Release", 10., 2., 1000., 0.1, "ms", IParam::kFlagsNone, "ADSR");

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
    auto titleText = new ITextControl(titleBounds, "SYN1 SEQ", IText(36.f, "Roboto-Regular"));
    auto homeButton = new IVButtonControl(homeBounds, [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenNavigation);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "picoDAW", style);

    // TODO: current step
    auto stepButton = new IVButtonControl(stepBounds, SplashAnimationFunc, "INSERT CURRENT STEP IN PATTERN HERE", style);

    // Navigation controls...
    auto navigationSyn1Seq = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 0).SubRectVertical(2, 0), [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenSequencerSyn1);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN1 SEQ", style);
    auto navigationSyn2Seq = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 1).SubRectVertical(2, 0), [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenSequencerSyn2);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN2 SEQ", style);
    auto navigationSyn3Seq = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 2).SubRectVertical(2, 0), [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenSequencerSyn3);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN3 SEQ", style);
    auto navigationSyn4Seq = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 3).SubRectVertical(2, 0), [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenSequencerSyn4);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN4 SEQ", style);
    auto navigationSamp1Seq = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 4).SubRectVertical(2, 0), [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenSequencerSamp1);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SAMP1 SEQ", style.WithColor(kFG, COLOR_BLUE));
    auto navigationSamp2Seq = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 5).SubRectVertical(2, 0), [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenSequencerSamp2);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SAMP2 SEQ", style.WithColor(kFG, COLOR_BLUE));
    auto navigationSyn1Edit = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 0).SubRectVertical(2, 1), [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenEditorSyn1);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN1 EDIT", style);
    auto navigationSyn2Edit = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 1).SubRectVertical(2, 1), [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenEditorSyn2);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN2 EDIT", style);
    auto navigationSyn3Edit = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 2).SubRectVertical(2, 1), [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenEditorSyn3);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN3 EDIT", style);
    auto navigationSyn4Edit = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 3).SubRectVertical(2, 1), [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenEditorSyn4);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SYN4 EDIT", style);
    auto navigationSamp1Edit = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 4).SubRectVertical(2, 1), [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenEditorSamp1);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SAMP1 EDIT", style.WithColor(kFG, COLOR_BLUE));
    auto navigationSamp2Edit = new IVButtonControl(sequencerBounds.SubRectHorizontal(6, 5).SubRectVertical(2, 1), [pGraphics, root](IControl* pCaller) {
      root->Update(EScreens::kScreenEditorSamp2);
      // Then animation...
      float x, y;
      pGraphics->GetMouseDownPoint(x, y);
      pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
      pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
    }, "SAMP2 EDIT", style.WithColor(kFG, COLOR_BLUE));

    // TODO: Mixer controls...
    auto mixerButton = new IVButtonControl(keyboardBounds, SplashAnimationFunc, "MIXER", style);

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
    auto playToggle = new IVToggleControl(playButtonBounds, [pGraphics, root, midiSeq, gateSeq](IControl* pCaller) {
      if (pCaller->GetValue() > 0.5) {
        // Is currently on, so was previously off; start playback wrt. current tempo
        root->mPlaybackTimer = Timer::Create([root, midiSeq, gateSeq](Timer &t) {
          for (int voiceIdx = 0; voiceIdx < 1; voiceIdx++) {
            IMidiMsg msg;
            int channelIdx = 0; // TODO: send to other channel if required(??)
            int &stepIdx = root->mPlaybackStepIdx;
            int &prevNote = root->mPrevNote;
            const int nSteps = midiSeq->mNCols;

            // Recall that cells are given as a row index from top to bottom; matching graphics
            int midiCell = midiSeq->mCells[stepIdx];

            // But we want a row index from bottom to top; matching piano roll sequencers
            int midiStep = (midiSeq->mNRows - 1) - midiCell;

            // TODO: Playback is a mess, because currently midiSeq is global across all six instruments
            // TODO: there is a root->mSeqDatas, and then midiStep := mSeqDatas[voiceIdx]->mMidiSeq, just like here for octave...
            // TODO: and, root->mPrevNote, should be moved into mSeqDatas instead... (the prevNote for this midi sequencer... right?)
            // NOTE: luckily, mPlaybackStepIdx is global to all sequencers, so is fine to leave in rootuicontrol...
            //
            // Relative to (C4 == 60) octave...
            int currNote = (midiCell < 0) ? -1 : 60 + midiStep;
            int currVelocity = 80; // TODO: velocity sequencer...

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
          }
        }, 125); // TODO: use current tempo, not hardcoded 120bpm
      } else {
        // Is currently off, so was previously on; stop playback
        IMidiMsg msg;
        msg.MakeNoteOffMsg(root->mPrevNote, 0, 0); // TODO: stop playback of other channels too(??)
        root->GetDelegate()->SendMidiMsgFromUI(msg);
        root->mPrevNote = -1;
        delete root->mPlaybackTimer;
        root->mPlaybackTimer = nullptr;
        root->mPlaybackStepIdx = 0;
      }
    }, "", style, "PLAY", "PLAY"); // Although, this one appears on all screens...
    auto soloToggle = new IVToggleControl(soloButtonBounds, [pGraphics](IControl* pCaller) {
      /* TODO: once mixer is ready... */
    }, "", style, "SOLO", "SOLO"); // And this one too... But, disabled except in SEQ or EDIT screens

    // Synth controls; Break sequencerBounds into a 4x4 grid...
    auto coreIdxSwitch = new IVSwitchControl(sequencerBounds.SubRectHorizontal(4, 0).SubRectVertical(4, 1), kParamSynthCoreIdx, "CORE", style);
    auto octaveSwitch = new IVSwitchControl(sequencerBounds.SubRectHorizontal(4, 0).SubRectVertical(4, 2), kParamVoiceOctave, "OCTAVE", style);
    auto gainKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 0).SubRectVertical(4, 3), kParamVoiceGain, "GAIN", style);

    auto attackKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 1).SubRectVertical(4, 0), kParamVoiceAttack, "ATTACK", style);
    auto decayKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 1).SubRectVertical(4, 1), kParamVoiceDecay, "DECAY", style);
    auto sustainKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 1).SubRectVertical(4, 2), kParamVoiceSustain, "SUSTAIN", style);
    auto releaseKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 1).SubRectVertical(4, 3), kParamVoiceRelease, "RELEASE", style);

    auto cutoffKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 2).SubRectVertical(4, 1), kParamFilterCutoff, "CUTOFF", style);
    auto resonanceKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 2).SubRectVertical(4, 2), kParamFilterResonance, "RESONANCE", style);
    auto typeIdxSwitch = new IVSwitchControl(sequencerBounds.SubRectHorizontal(4, 2).SubRectVertical(4, 3), kParamFilterTypeIdx, "TYPE", style);

    auto filterAttackKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 3).SubRectVertical(4, 0), kParamFilterAttack, "ATTACK", style);
    auto filterDecayKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 3).SubRectVertical(4, 1), kParamFilterDecay, "DECAY", style);
    auto filterSustainKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 3).SubRectVertical(4, 2), kParamFilterSustain, "SUSTAIN", style);
    auto filterReleaseKnob = new IVKnobControl(sequencerBounds.SubRectHorizontal(4, 3).SubRectVertical(4, 3), kParamFilterRelease, "RELEASE", style);

    // TODO: Sampler controls; Break sequencerBounds into a 4x4 grid...
    //

    pGraphics->AttachControl(root, kCtrlTagRoot);
    pGraphics->AttachControl(titleText, kCtrlTagTitleText);
    pGraphics->AttachControl(homeButton, kCtrlTagHomeButton);
    pGraphics->AttachControl(stepButton, kCtrlTagStepButton);

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

    pGraphics->AttachControl(mixerButton, kCtrlTagMixerButton);

    pGraphics->AttachControl(coreIdxSwitch, kCtrlTagSynthCoreSwitch);

    pGraphics->AttachControl(octaveSwitch, kCtrlTagVoiceOctaveSwitch);
    pGraphics->AttachControl(gainKnob, kCtrlTagVoiceGainKnob);
    pGraphics->AttachControl(attackKnob, kCtrlTagVoiceAttackKnob);
    pGraphics->AttachControl(decayKnob, kCtrlTagVoiceDecayKnob);
    pGraphics->AttachControl(sustainKnob, kCtrlTagVoiceSustainKnob);
    pGraphics->AttachControl(releaseKnob, kCtrlTagVoiceReleaseKnob);

    pGraphics->AttachControl(cutoffKnob, kCtrlTagFilterCutoffKnob);
    pGraphics->AttachControl(resonanceKnob, kCtrlTagFilterResonanceKnob);
    pGraphics->AttachControl(typeIdxSwitch, kCtrlTagFilterTypeSwitch);

    pGraphics->AttachControl(filterAttackKnob, kCtrlTagFilterAttackKnob);
    pGraphics->AttachControl(filterDecayKnob, kCtrlTagFilterDecayKnob);
    pGraphics->AttachControl(filterSustainKnob, kCtrlTagFilterSustainKnob);
    pGraphics->AttachControl(filterReleaseKnob, kCtrlTagFilterReleaseKnob);

    pGraphics->AttachControl(midiSeq, kCtrlTagMidiSequencer);
    pGraphics->AttachControl(gateSeq, kCtrlTagGateSequencer);
    pGraphics->AttachControl(keyboard, kCtrlTagKeyboard);
    pGraphics->AttachControl(cycleKeyboard, kCtrlTagCycleKeyboardButton);
    pGraphics->AttachControl(playToggle, kCtrlTagPlayToggle);
    pGraphics->AttachControl(soloToggle, kCtrlTagSoloToggle);

    // Load home page and return to event loop...
    root->Update(kScreenNavigation);
  };
#endif
}

#if IPLUG_DSP
void picoDAW::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  // Inputs to mDSP is empty, because it is the first DSP plugin in the chain!!
  mDSP.ProcessBlock(nullptr, outputs, 2, nFrames, mTimeInfo.mPPQPos, mTimeInfo.mTransportIsRunning);

  // mDSP generates outputs, that we then pass BACK to UI, through mMeterSender, for visualisation
  // The sending is done through TransmitData, and the notification to ProcessBlock is done here...
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
// into "picoDAW-ui.js", and loaded into a <script> tag in "IPlugWasmBundle.js.template". And
// a secondary AudioWorklet worker thread, spawned by the main thread before entering the UI loop, 
// endlessly calling its "process" function, specified in the AudioWorkletProcessor API and defined
// in "IPlugWasmProcessor.js.template". Each invocation calls "processBlock", which is bound to
// IPlugWasmDSP's "processBlock" C++ code, the transpiled javascript code located inside the
// "picoDAW-dsp.js" DSP module provided to the AudioWorklet thread on initialisation...
//
// How do the two threads communicate? They communicate via the javascript functions
// "setParam", and "onParam", for communicating parameter changes for example. What I 
// mean is, in the IPlugWasmUI StartMainLoopTimer loop whenever it runs into a C++ call
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
// our IPlugWasmDP::OnParamChange (aka picoDAW::OnParamChange) here...
void picoDAW::OnParamChange(int paramIdx)
{
  // For DSP-side paramater changes...
  mDSP.SetParam(paramIdx, GetParam(paramIdx)->Value(), GetParam(kParamVoiceIdx)->Int());
}
#endif

#if IPLUG_EDITOR
// Notice that on the UI side, after IPlugWasmUI::SendParameterValueFromUI in "IPlugWasmUI.cpp" 
// is called to invoke the javascript "setParam", it also calls IPlugAPIBase::SendParameterValueFromUI,
// itself, specifically, its parent's implementation of the same method name. This method 
// calls its child's IPlugWasmUI::SetParameterValue, which calls a no-op IPlugWasmUI::OnParamChange, 
// before finally calling IPluginBase::SendParameterValueFromUI, which again is the same method
// name, but this time is for the grandparent of IPlugWasmUI; the parent of IPlugAPIBase, aka 
// IEditorDelegate::SendParameterValueFromUI is what is called... And this function, finally, is
// what calls IPlugWasmUI::OnParamChangeUI, triggering the UI itself to respond to the parameter
// change caused by itself. The key to all of this working though, is that IPlugWasmUI::OnParamChange 
// is a no-op, IPlugWasmDSP::OnParamChangeUI is a no-op, and IPlugWasmUI::OnParamChangeUI and
// IPlugWasmDSP::OnParamChange do what they need to do for their respective sides...
void picoDAW::OnParamChangeUI(int paramIdx, EParamSource source)
{
  if (auto pGraphics = GetUI()) {
    int voiceIdx = GetParam(kParamVoiceIdx)->Int();
    double value = GetParam(paramIdx)->Value();

    auto root = pGraphics->GetControlWithTag(kCtrlTagRoot)->As<RootUIControl>();
    switch (paramIdx) {
      // TODO: better solution is have a mVoiceDatas, where the first four are synthdatas, and the last two are samplerdatas...
      case kParamVoiceGain:
        // TODO: obviously wrong once sampler comes into mix too; unless change param to SynthGain...
        root->mSynthDatas[voiceIdx].mVoiceGain = value;
        break;
      case kParamVoiceOctave:
        // TODO: obviously wrong once sampler comes into mix too; unless change param to SynthOctave...
        root->mSynthDatas[voiceIdx].mVoiceOctave = value;
        break;
      case kParamVoiceAttack:
        // TODO: obviously wrong once sampler comes into mix too; unless change param to SynthAttack...
        root->mSynthDatas[voiceIdx].mVoiceAttack = value;
        break;
      case kParamVoiceDecay:
        // TODO: obviously wrong once sampler comes into mix too; unless change param to SynthDecay...
        root->mSynthDatas[voiceIdx].mVoiceDecay = value;
        break;
      case kParamVoiceSustain:
        // TODO: ...
        root->mSynthDatas[voiceIdx].mVoiceSustain = value;
        break;
      case kParamVoiceRelease:
        root->mSynthDatas[voiceIdx].mVoiceRelease = value;
        break;
      case kParamSynthCoreIdx:
        root->mSynthDatas[voiceIdx].mSynthCoreIdx = static_cast<int>(value);
        break;
      case kParamFilterCutoff:
        root->mSynthDatas[voiceIdx].mFilterCutoff = value;
        break;
      case kParamFilterResonance:
        root->mSynthDatas[voiceIdx].mFilterResonance = value;
        break;
      case kParamFilterTypeIdx:
        root->mSynthDatas[voiceIdx].mFilterTypeIdx = static_cast<int>(value);
        break;
      case kParamFilterAttack:
        root->mSynthDatas[voiceIdx].mFilterAttack = value;
        break;
      case kParamFilterDecay:
        root->mSynthDatas[voiceIdx].mFilterDecay = value;
        break;
      case kParamFilterSustain:
        root->mSynthDatas[voiceIdx].mFilterSustain = value;
        break;
      case kParamFilterRelease:
        root->mSynthDatas[voiceIdx].mFilterRelease = value;
        break;
      default:
        break;
    }
  }
}
#endif
