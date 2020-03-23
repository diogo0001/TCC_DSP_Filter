Fs = 44100; % Input sample rate
% Define a bandpass variable bandwidth FIR filter:
vbw = dsp.VariableBandwidthFIRFilter('FilterType','Bandpass',...
    'FilterOrder',300,...
    'SampleRate',Fs,...
    'CenterFrequency',1e4,...
    'Bandwidth',4e3);
tfe = dsp.TransferFunctionEstimator('FrequencyRange','onesided');
aplot = dsp.ArrayPlot('PlotType','Line',...
    'XOffset',0,...
    'YLimits',[-120 5], ...
    'SampleIncrement', 44100/1024,...
    'YLabel','Frequency Response (dB)',...
    'XLabel','Frequency (Hz)',...
    'Title','System Transfer Function');
FrameLength = 1024;
sine = dsp.SineWave('SamplesPerFrame',FrameLength);
for i=1:1000
    % Generate input
    x = sine() + randn(FrameLength,1);
    % Pass input through the filter
    y = vbw(x);
    % Transfer function estimation
    h = tfe(x,y);
    % Plot transfer function
    aplot(20*log10(abs(h)))
    % Tune bandwidth and center frequency of the FIR filter
    if (i>25)
        vbw.CenterFrequency = i*10;
        vbw.Bandwidth = 200;
    end
%     pause(0.05);
end