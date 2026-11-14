
clear;
close all;
clc;

%% Configuration
fundamental = 1760;
Fs = 48000;

files = {
    '[7]-output1.wav'
    '[7]-output2.wav'
};

names = {
    'Naive (No Anti-Aliasing)'
    'PolyBLEP'
};

%% Load audio
[x1, Fs1] = audioread(files{1});
[x2, Fs2] = audioread(files{2});

x1 = x1(:,1);
x2 = x2(:,1);

assert(Fs1 == Fs2, 'Sample rates do not match!');

Fs = Fs1;

%% Analyse both signals

results = cell(2,1);

for i = 1:2

    if i == 1
        x = x1;
    else
        x = x2;
    end

    N = length(x);

    % Windowing
    w = hann(N);
    X = fft(x .* w);

    % Single-sided spectrum
    P = abs(X(1:floor(N/2)+1)) / sum(w);
    P(2:end-1) = 2*P(2:end-1);

    f = Fs*(0:floor(N/2))/N;

    % dB spectrum
    PdB = 20*log10(P + eps);

    %% Calculate expected aliased harmonic frequencies
    %
    % Sawtooth contains harmonics at:
    % fundamental * n
    %
    % Harmonics above Nyquist fold back into
    % the sampled frequency range.

    maxN = 1000;

    n = (1:maxN)';

    harmonicFreq = n * fundamental;

    % Fold frequencies into [0, Fs/2]
    foldedFreq = abs(mod(harmonicFreq + Fs/2, Fs) - Fs/2);

    % Identify harmonics above Nyquist
    aliased = harmonicFreq > Fs/2;

    aliasFreqs = foldedFreq(aliased);

    % Exclude DC and frequencies coinciding with
    % an in-band harmonic.
    inBandHarmonics = harmonicFreq(harmonicFreq <= Fs/2);

    aliasMask = true(size(aliasFreqs));

    tolerance = max(Fs/N*3, 1);

    for j = 1:length(aliasFreqs)

        if any(abs(inBandHarmonics - aliasFreqs(j)) < tolerance)
            aliasMask(j) = false;
        end

    end

    aliasFreqs = aliasFreqs(aliasMask);

    %% Estimate energy around predicted alias frequencies

    aliasEnergy = 0;

    for j = 1:length(aliasFreqs)

        [~, idx] = min(abs(f - aliasFreqs(j)));

        search = max(1,idx-2):min(length(P),idx+2);

        aliasEnergy = aliasEnergy + sum(P(search).^2);

    end

    %% Store results

    results{i}.f = f;
    results{i}.P = P;
    results{i}.PdB = PdB;
    results{i}.aliasEnergy = aliasEnergy;

end

%% Calculate alias energy improvement

alias1 = results{1}.aliasEnergy;
alias2 = results{2}.aliasEnergy;

aliasImprovement = 10*log10((alias1 + eps)/(alias2 + eps));

%% Print results

fprintf('\n========== DSP ANALYSIS ==========\n');

for i = 1:2

    fprintf('\n%s\n', names{i});
    fprintf('Aliasing energy: %.8e\n', ...
        results{i}.aliasEnergy);

end

fprintf('\nPolyBLEP alias energy improvement: %.2f dB\n', ...
    aliasImprovement);

%% Plot FFT comparison

figure('Color','w');
% figure('Color','w','Position',[100 100 1200 900]);
subplot(2,1,1);

plot(results{1}.f, results{1}.PdB, ...
    'Color',[0.85 0.25 0.25], 'LineWidth',1);

hold on;

plot(results{2}.f, results{2}.PdB, ...
    'Color',[0.1 0.35 0.85], 'LineWidth',1);

title('FFT Spectrum Comparison');
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');
legend(names);
grid on;
xlim([0 Fs/2]);

%% Aliasing energy comparison

subplot(2,1,2);

bar([results{1}.aliasEnergy, results{2}.aliasEnergy]);

set(gca,'XTickLabel',names);

title(sprintf('Aliasing Energy Comparison (Improvement: %.2f dB)', ...
    aliasImprovement));

ylabel('Estimated Aliasing Energy');
grid on;

sgtitle('1760 Hz Oscillator - Naive vs PolyBLEP');

%% Export figure

exportgraphics(gcf,'[7]-comparison.png','Resolution',300);
