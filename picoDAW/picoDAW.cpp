#include "picoDAW.h"
#include "IPlug_include_in_plug_src.h"
#include "LFO.h"
#include "IconsForkAwesome.h"
#include "IconsFontaudio.h"
#include "IPlugMidi.h"

#include <string>
/** A root control that is invisible and does nothing; just holds data for drawing UI */
class RootUIControl : public IControl {
public:
  int keyboardBoundsIdx = 0;
  const int keyboardBoundsTotal = 2;

  Timer *mPlaybackTimer = nullptr; // Notice pointer; this will be allocated on heap!!
  int mPlaybackStep = 0;
  int mPrevNote = -1;

  RootUIControl() : IControl(IRECT()) { mIgnoreMouse = true; }
  void Draw(IGraphics& g) override {}
};

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

    // Making subrects of main window
    const IRECT b = pGraphics->GetBounds().GetPadded(0);
    IRECT keyboardBounds = b.GetFromBottom(160).GetReducedFromLeft(160).GetReducedFromRight(160);
    IRECT sequencerBounds = b.GetReducedFromBottom(160 + 20).GetReducedFromLeft(160).GetReducedFromRight(160).GetReducedFromTop(80 - 20);
    IRECT cycleButtonBounds = keyboardBounds.GetCentredInside(160, 160).GetTranslated(-(320 + 80), 2.5);
    IRECT playButtonBounds = IRECT(160 + 640, 0, 960, 60).GetTranslated(-2.5, 2.5);
    IRECT soloButtonBounds = IRECT(160 + 640, 0, 960, 60).GetTranslated(-2.5 - 160, 2.5);

    // The root control; its members are the global variables necessary to control UI
    pGraphics->AttachControl(new RootUIControl(), kCtrlTagRoot);

    // Placing controls into those subrects
    pGraphics->AttachControl(new ITextControl(b.GetPadded(-5), "picoDAW", IText(36.f, "Roboto-Regular").WithAlign(EAlign::Near).WithVAlign(EVAlign::Top)), kCtrlTagTitle);
    pGraphics->AttachControl(new IVSequencerControl(sequencerBounds, 12, 16), kCtrlTagMidiSequencer);

    // Notice that these occupy the same subrect, so every control but one is manually hidden on 
    // boot; a seperate button will then be pressed to cycle through which control is visible
    pGraphics->AttachControl(new IVKeyboardControl(keyboardBounds), kCtrlTagKeyboard);
    pGraphics->AttachControl(new IVSequencerControl(keyboardBounds, 2, 16), kCtrlTagGateSequencer);

    // References to those controls that can easily pass into the lambda functions below
    auto root = pGraphics->GetControlWithTag(kCtrlTagRoot)->As<RootUIControl>(); 
    auto title = pGraphics->GetControlWithTag(kCtrlTagTitle)->As<ITextControl>();
    auto midiSeq = pGraphics->GetControlWithTag(kCtrlTagMidiSequencer)->As<IVSequencerControl>();
    auto gateSeq = pGraphics->GetControlWithTag(kCtrlTagGateSequencer)->As<IVSequencerControl>();
    auto keyboard = pGraphics->GetControlWithTag(kCtrlTagKeyboard)->As<IVKeyboardControl>();

    // Hide/Show controls so only the active one is displayed
    {
      int sync = root->keyboardBoundsIdx;
      keyboard->Hide(sync != 0);
      gateSeq->Hide(sync != 1);
    }

    // Button to cycle through what is displayed in keyboardBounds
    pGraphics->AttachControl(new IVButtonControl(cycleButtonBounds, 
      [pGraphics, root](IControl* pCaller) {
        // Cycle through its currently active sub-control
        root->keyboardBoundsIdx += 1; 
        root->keyboardBoundsIdx %= root->keyboardBoundsTotal;
                    
        // Hide/Show controls so only the active one is displayed
        int sync = root->keyboardBoundsIdx;
        pGraphics->GetControlWithTag(kCtrlTagKeyboard)->Hide(sync != 0);
        pGraphics->GetControlWithTag(kCtrlTagGateSequencer)->Hide(sync != 1);

        // Then animation...
        float x, y;
        pGraphics->GetMouseDownPoint(x, y);
        pCaller->As<IVectorBase>()->SetSplashPoint(x, y);
        pCaller->SetAnimation(SplashAnimationFunc, DEFAULT_ANIMATION_DURATION);
      }, "TOGGLE", style), kCtrlTagCycleButton);
  
    // Button to playback the whole pattern
    pGraphics->AttachControl(new IVToggleControl(playButtonBounds, 
      [pGraphics, root, midiSeq](IControl* pCaller) {
        if (pCaller->GetValue() > 0.5) {
          // Is currently on, so was previously off; start playback wrt. current tempo
          root->mPlaybackTimer = Timer::Create(
            [root, midiSeq](Timer &t) {
              IMidiMsg msg;
              auto &stepIdx = root->mPlaybackStep;
              auto &prevNote = root->mPrevNote;

              // Cell is given as a row index from top to bottom; matching graphics
              int cell = midiSeq->mCells[stepIdx];

              // But we want a row index from bottom to top; matching piano roll sequencers
              int step = (midiSeq->mNRows - 1) - cell;

              // 2206: assumes we are in C3 octave (C3 === 48); also, velocity sequencer
              int currNote = (cell < 0) ? -1 : 48 + step;
              int currVelocity = 80;

              if (currNote < 0) {
                // Nothing to play this step, so turn off prev and that's it
                if (prevNote >= 0) {
                  msg.MakeNoteOffMsg(prevNote, 0);
                  root->GetDelegate()->SendMidiMsgFromUI(msg);
                }
              } else if (currNote != prevNote) {
                // Something to play this step, and it's different than last step
                msg.MakeNoteOffMsg(prevNote, 0);
                root->GetDelegate()->SendMidiMsgFromUI(msg);
                msg.MakeNoteOnMsg(currNote, currVelocity, 0);
                root->GetDelegate()->SendMidiMsgFromUI(msg);
              } else {
                // Something to play this step, but it's the same as last step
                // 2206: Should this be played legato with previous step? Or retrigger envelope...
                // The gate sequencer should be used to decide that!! Along with e.g. 50% of step
                msg.MakeNoteOffMsg(prevNote, 0);
                root->GetDelegate()->SendMidiMsgFromUI(msg);
                msg.MakeNoteOnMsg(currNote, currVelocity, 0);
                root->GetDelegate()->SendMidiMsgFromUI(msg);
              }

              prevNote = currNote;
              stepIdx += 1;
              stepIdx %= midiSeq->mNCols; // to cycle around
            }, 125); // 2206: use current tempo, not hardcoded 120bpm
        } else {
          // Is currently off, so was previously on; stop playback
          IMidiMsg msg;
          msg.MakeNoteOffMsg(root->mPrevNote, 0);
          root->GetDelegate()->SendMidiMsgFromUI(msg);
          root->mPrevNote = -1;
          delete root->mPlaybackTimer;
          root->mPlaybackTimer = nullptr;
          root->mPlaybackStep = 0;
        }
      }, "", style, "PLAY", "PLAY"), kCtrlTagPlayButton);
    
    // Button to solo this single instrument
    pGraphics->AttachControl(new IVToggleControl(soloButtonBounds, 
      [pGraphics](IControl* pCaller) {
        /* TODO: once mixer is ready... */
      }, "", style, "SOLO", "SOLO"), kCtrlTagSoloButton);

    // Allow playing synth by QWERTY keyboard; polyphonic unlike the monoponic of MIDI sequencer
    pGraphics->SetQwertyMidiKeyHandlerFunc(
      [pGraphics](const IMidiMsg& msg) {
        pGraphics->GetControlWithTag(kCtrlTagKeyboard)->As<IVKeyboardControl>()->SetNoteFromMidi(
          msg.NoteNumber(), msg.StatusMsg() == IMidiMsg::kNoteOn);
      });
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
