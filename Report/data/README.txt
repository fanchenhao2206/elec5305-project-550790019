note that ui.perfetto.dev was previously known as chrome://tracing

[1.0]-no-aa-saw-chrome-2026106-15044.pftrace

first trace obtained from running ui.perfetto.dev onto picoDAW, saw wave, NO anti-aliasing...  

[2.0]-yes-aa-saw-chrome-2026106-15044.pftrace

first trace obtained from running ui.perfetto.dev onto picoDAW, saw wave, YES anti-aliasing...

disturbingly, the NO anti-aliasing takes a longer time on average to complete AudioWorkletProcessor::Process,
40.78us versus 35.73us, see [1.0].png vs [2.0].png

[1.1]-no-aa-saw-chrome-2026106-15044.pftrace

even after rerunning the NO anti-aliasing test, it is not shorter; takes even longer actually!! 42.38us
see [1.1].png

