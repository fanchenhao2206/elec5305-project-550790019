AudioWorkletProcessor uses 128 sample blocks; sample rate is 48000 samples per second

all computed using highest note on sequencer (B4) +2 octaves (so B6)

https://ui.perfetto.dev/#!/record/share/6122465dbdd9bd2c19027f8db7463e8d89314ba9
[1] saw (no/yes aa) 54.59 vs 53.54 us
[2] squ (no/yes aa) 56.38 vs 56.98 us
[3] tri (no/yes aa) 57.01 vs 58.95 us

[.] saw (iir/zdf w 50Hz LFO) 57.76 vs 59.64µs
[.] saw (iir/zdf w 40Hz LFO) 56.82 vs 57.64 us
[.] saw (iir/zdf w 30Hz LFO) 57.32 vs 59.28 us
[4] saw (iir/zdf w 20Hz LFO) 57.46 vs 62.08 us
[5] saw (iir/zdf w 10Hz LFO) 56.76 vs 60.03 us
[6] saw (iir/zdf w 00Hz LFO) 59.20 vs 51.11 us

[7] recording no/yes aa and comparing fft and aliasing energy (A6 == 1760Hz)

[8] filter stability/instability; use the exploding obs capture (at 150Hz LFO)