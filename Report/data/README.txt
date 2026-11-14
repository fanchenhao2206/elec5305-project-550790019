AudioWorkletProcessor uses 128 sample blocks; sample rate is 48000 samples per second

all computed using highest note on sequencer +2 octaves
[1] saw (no/yes aa) 54.59 vs 53.54 us
[2] squ (no/yes aa) 56.38 vs 56.98 us
[3] tri (no/yes aa) 57.01 vs 58.95 us

[4] saw (iir/zdf w 20Hz LFO) 57.46 vs 62.08 us
[5] saw (iir/zdf w 10Hz LFO) 56.76 vs 60.03 us
[6] saw (iir/zdf w 00Hz LFO) 59.20 vs 51.11 us

[7] recording no/yes aa and comparing fft and aliasing energy

[8] filter stability/instability; use the exploding obs capture