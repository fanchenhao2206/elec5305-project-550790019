
%% IIR vs ZDF Performance Analysis
% AudioWorklet: 48 kHz, 128 samples per block
% Oscillator: Saw
% Note: B6 (highest sequencer note)

clear; clc; close all;

%% Experimental configuration

Fs = 48000;
blockSize = 128;

blockTime = (blockSize/Fs)*1e6;

lfoRates = [0 10 20 30 40 50];

% Processing time (microseconds)
iir = [59.20 56.76 57.46 57.32 56.82 57.76];
zdf = [51.11 60.03 62.08 59.28 57.64 59.64];

% Calculate percentage overhead
overhead = ((zdf-iir)./iir)*100;

% Colours
colourIIR = [0.20 0.42 0.72];
colourZDF = [0.90 0.38 0.25];

%% IIR vs ZDF: Processing Cost and Real-Time Performance

figure('Color','w');

p1 = plot(lfoRates,iir,'-o', ...
    'Color',colourIIR,'LineWidth',2.2, ...
    'MarkerSize',9,'MarkerFaceColor',colourIIR);

hold on;

p2 = plot(lfoRates,zdf,'-s', ...
    'Color',colourZDF,'LineWidth',2.2, ...
    'MarkerSize',9,'MarkerFaceColor',colourZDF);

% Label IIR data points (below markers)
for k = 1:length(lfoRates)
    text(lfoRates(k),iir(k)-0.55, ...
        sprintf('%.2f',iir(k)), ...
        'Color',colourIIR, ...
        'FontSize',11, ...
        'HorizontalAlignment','center', ...
        'VerticalAlignment','top');
end

% Label ZDF data points (above markers)
for k = 1:length(lfoRates)
    text(lfoRates(k),zdf(k)+0.55, ...
        sprintf('%.2f',zdf(k)), ...
        'Color',colourZDF, ...
        'FontSize',11, ...
        'HorizontalAlignment','center', ...
        'VerticalAlignment','bottom');
end

xlabel('LFO Frequency (Hz)');
ylabel('Processing Time (\mus)');

title('IIR vs ZDF: Processing Cost and Real-Time Performance');

subtitle(sprintf('Sample Rate: %d Hz | Block Size: %d Samples', ...
    Fs,blockSize));

legend([p1 p2],{'IIR','ZDF'},'Location','northwest');

xticks(lfoRates);
xlim([-2 52]);
ylim([49 64]);

grid on;
box on;

% Performance budget annotation
annotation('textbox',[0.68 0.72 0.25 0.15], ...
    'String',sprintf(['Sample rate: %d Hz\n' ...
    'Block duration: %.2f ms\n' ...
    'Available budget: %.2f us\n' ...
    'Maximum usage: %.2f%%\n' ...
    'Remaining headroom: %.2f%%'], ...
    Fs,blockTime/1000,blockTime, ...
    max([iir zdf])/blockTime*100, ...
    100-max([iir zdf])/blockTime*100), ...
    'FitBoxToText','on', ...
    'BackgroundColor','white', ...
    'EdgeColor',[0.7 0.7 0.7], ...
    'FontSize',10);

set(gca,'FontSize',12,'LineWidth',1.1);

%%

exportgraphics(gcf, '[1]-comparison.png','Resolution',300);