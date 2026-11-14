
clear;
close all;
clc;

%% Load audio
[x1, Fs1] = audioread('[7]-output1.wav');
[x2, Fs2] = audioread('[7]-output2.wav');

% Select first channel
x1 = x1(:,1);
x2 = x2(:,1);

% Ensure sample rates match
assert(Fs1 == Fs2, 'Sample rates do not match!');

Fs = Fs1;

%% FFT analysis
N1 = length(x1);
N2 = length(x2);

% Apply Hann windows to reduce spectral leakage
w1 = hann(N1);
w2 = hann(N2);

X1 = fft(x1 .* w1);
X2 = fft(x2 .* w2);

% Single-sided amplitude spectra
P1 = abs(X1(1:floor(N1/2)+1)) / sum(w1);
P2 = abs(X2(1:floor(N2/2)+1)) / sum(w2);

P1(2:end-1) = 2*P1(2:end-1);
P2(2:end-1) = 2*P2(2:end-1);

% Frequency axes
f1 = Fs*(0:floor(N1/2))/N1;
f2 = Fs*(0:floor(N2/2))/N2;

% Convert to dB
P1_dB = 20*log10(P1 + eps);
P2_dB = 20*log10(P2 + eps);

%% Find spectral peaks

% Tune MinPeakProminence if too many or too few peaks appear
[pks1, locs1] = findpeaks(P1_dB, f1, ...
    'MinPeakProminence', 6, ...
    'MinPeakDistance', 100);

[pks2, locs2] = findpeaks(P2_dB, f2, ...
    'MinPeakProminence', 6, ...
    'MinPeakDistance', 100);

%% Plot
figure('Color', 'w', 'Position', [100 100 1200 800]);

% Output 1: Without anti-aliasing
subplot(2,1,1);

plot(f1, P1_dB, 'b', 'LineWidth', 1);
hold on;

plot(locs1, pks1, 'rv', ...
    'MarkerFaceColor', 'r', ...
    'MarkerSize', 5);

xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
title('Output 1 - Without Anti-Aliasing');
grid on;
xlim([0 Fs/2]);

for k = 1:length(pks1)
    text(locs1(k), pks1(k)+3, ...
        sprintf('%.0f Hz', locs1(k)), ...
        'FontSize', 8, ...
        'HorizontalAlignment', 'center');
end

% Output 2: With anti-aliasing
subplot(2,1,2);

plot(f2, P2_dB, 'b', 'LineWidth', 1);
hold on;

plot(locs2, pks2, 'rv', ...
    'MarkerFaceColor', 'r', ...
    'MarkerSize', 5);

xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
title('Output 2 - With Anti-Aliasing');
grid on;
xlim([0 Fs/2]);

for k = 1:length(pks2)
    text(locs2(k), pks2(k)+3, ...
        sprintf('%.0f Hz', locs2(k)), ...
        'FontSize', 8, ...
        'HorizontalAlignment', 'center');
end

sgtitle('1760 Hz Waveform at 48kHz - FFT Comparison');

%% Export PNG
exportgraphics(gcf, '[7]-FFT_Comparison.png', ...
    'Resolution', 300);

disp('FFT comparison exported to FFT_Comparison.png');
