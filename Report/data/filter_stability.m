close all;

% Load the MOV video
videoFile = '[8]-output-2.mov';

% Create VideoReader object
v = VideoReader(videoFile);

% Read audio data
[audioData, Fs] = audioread(videoFile);

% Calculate time vector
t = (0:length(audioData)-1) / Fs;

% Select time interval
idx = (t >= 1.95) & (t <= 2.5);

% Plot audio signal
figure;
plot(t(idx), audioData(idx,:));

xlabel('Time (seconds)');
ylabel('Amplitude');
title('Audio Signal');
xlim([1.95 2.5]);
grid on;

%% 
close all;

% Extract ZDF section (1.95 - 2.07 seconds)
idx_ZDF = (t >= 1.95) & (t <= 2.07);
audio_ZDF = audioData(idx_ZDF,:);

% Extract IIR section (2.08 - 2.50 seconds)
idx_IIR = (t >= 2.08) & (t <= 2.50);
audio_IIR = audioData(idx_IIR,:);

% Create individual time vectors
t_ZDF = (0:length(audio_ZDF)-1) / Fs;
t_IIR = (0:length(audio_IIR)-1) / Fs;

% Plot
figure('Color','w','Position',[100 100 1100 650]);

subplot(2,1,1);
plot(t_ZDF, audio_ZDF, 'b');
title('ZDF Filter Response');
ylabel('Amplitude');
grid on;
box on;

subplot(2,1,2);
plot(t_IIR, audio_IIR, 'r');
title('IIR Filter Response');
xlabel('Time (seconds)');
ylabel('Amplitude');
grid on;
box on;

sgtitle({'IIR vs ZDF: Response to Extreme Cutoff Modulation', ...
    'LFO Rate: 150 Hz | Cutoff: 57 Hz | Resonance: 100%'});


%% Export figure

exportgraphics(gcf,'[8]-comparison.png','Resolution',300);