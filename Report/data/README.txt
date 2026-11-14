note that ui.perfetto.dev was previously known as chrome://tracing

https://ui.perfetto.dev/#!/record/share/6122465dbdd9bd2c19027f8db7463e8d89314ba9

[1.0]-no-aa-saw-chrome-2026106-15044.pftrace

first trace obtained from running ui.perfetto.dev onto picoDAW, saw wave, NO anti-aliasing...  

[2.0]-yes-aa-saw-chrome-2026106-15044.pftrace

first trace obtained from running ui.perfetto.dev onto picoDAW, saw wave, YES anti-aliasing...

disturbingly, the NO anti-aliasing takes a longer time on average to complete AudioWorkletProcessor::Process,
40.78us versus 35.73us, see [1.0].png vs [2.0].png

[1.1]-no-aa-saw-chrome-2026106-15044.pftrace

even after rerunning the NO anti-aliasing test, it is not shorter; takes even longer actually!! 42.38us
see [1.1].png

then, a quick inspection of the NO anti-alias versus YES anti-alias cores, reveals that debugging 
statements for the NO anti-alias cores are still in their process() methods, while in the YES anti-alias
cores the debug statements had been already removed; an unfair comparison.

// NO anti-aa core, for [1.0] and [1.1]
template <typename T>
class SawOscillator : public picoDAWOscillator<T>
{
public:
  SawOscillator(double startPhase = 0.,
                double startFreq = 1.)
    : picoDAWOscillator<T>(startPhase, startFreq)
  {}

  inline T Process()
  {
    auto &phase = picoDAWOscillator<T>::mPhase;

    phase += picoDAWOscillator<T>::mPhaseIncr;
    phase -= std::floor(phase);

    return T(2.0 * phase - 1.0);
  }

  inline T Process(double freqHz) override
  {
    static int counter = 0;
    if (++counter == 1000)
    {
      std::cout
        << "SAW Frequency: " << freqHz
        << " Phase increment: "
        << picoDAWOscillator<T>::mPhaseIncr
        << " Sample rate: "
        << picoDAWOscillator<T>::mSampleRate
        << std::endl;
      counter = 0;
    }
    picoDAWOscillator<T>::SetFreqCPS(freqHz);
    return Process();
  }
};

[1.2]-no-aa-saw-chrome-2026106-15044.pftrace

// NO anti-aa core, for [1.2]
template <typename T>
class SawOscillator : public picoDAWOscillator<T>
{
public:
  SawOscillator(double startPhase = 0.,
                double startFreq = 1.)
    : picoDAWOscillator<T>(startPhase, startFreq)
  {}

  inline T Process()
  {
    auto &phase = picoDAWOscillator<T>::mPhase;

    phase += picoDAWOscillator<T>::mPhaseIncr;
    phase -= std::floor(phase);

    return T(2.0 * phase - 1.0);
  }

  inline T Process(double freqHz) override
  {
    picoDAWOscillator<T>::SetFreqCPS(freqHz);
    return Process();
  }
};

additionally, the test was streamlined so that throughout the whole 30second
duration, audio is playing; then the only data captured is the target 
AudioWorkletProcessor::Process, while audio is always playing (samples always
being generated throughout the 30second period)

[2.2]-yes-aa-saw-chrome-2026106-15044.pftrace

the YES anti-aa core was unchanged, because again its debugging statements were 
already removed. the additional trace, is so that both [1.2] and [2.2] follow the
same testing methodology...

but even with that change, the comparision seen in [1.2].png vs [2.2].png, the
the difference is marginal; in fact, the no-aa still takes longer on average
for some reason; 54.59 vs 53.54

