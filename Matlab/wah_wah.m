% wah_wah.m   state variable band pass
% written by Ronan O'Malley
% October 2nd 2005
% 
% BP filter with narrow pass band, Fc_HP oscillates up and down the spectrum
% Difference equation taken from DAFX chapter 2
%
% Changing this from a BP to a BS (notch instead of a bandpass) converts this effect to a phaser
%
% yl_HP(n) = F1*yb_HP(n) + yl_HP(n-1)
% yb_HP(n) = F1*yh_HP(n) + yb_HP(n-1)
% yh_HP(n) = x(n) - yl_HP(n-1) - Q1*yb_HP(n-1)
%
% vary Fc_HP from 500 to 5000 Hz
% 44100 samples per sec

clear all;
close all;

infile = 'noise.wav';

% read in wav sample
[x, Fs] = audioread(infile);

%%%%%%% EFFECT COEFFICIENTS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% damping factor
% lower the damping factor the smaller the pass band
damp = 0.3;

% min and max centre cutoff frequency of variable bandpass filter
minf_HP=1000;
maxf_HP=20000;

minf_LP=40;
maxf_LP=3000;

% wah frequency, how many Hz per second are cycled through
Fw = 200; 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% change in centre frequency per sample (Hz)
%delta=0.1;
delta = Fw/Fs;
%0.1 => at 44100 samples per second should mean  4.41kHz Fc_HP shift per sec

% create triangle wave of centre frequency values
Fc_HP=minf_HP:delta:maxf_HP;
Fc_LP=minf_LP:delta:maxf_LP;

while(length(Fc_HP) < length(x) )
    Fc_HP= [ Fc_HP (maxf_HP:-delta:minf_HP) ];
    Fc_HP= [ Fc_HP (minf_HP:delta:maxf_HP) ]; 
end

while(length(Fc_LP) < length(x) )
    Fc_LP= [ Fc_LP (maxf_LP:-delta:minf_LP) ];
    Fc_LP= [ Fc_LP (minf_LP:delta:maxf_LP) ]; 
end

% trim tri wave to size of input
Fc_HP = Fc_HP(1:length(x));
Fc_LP = Fc_LP(1:length(x));

% difference equation coefficients
F1 = 2*sin((pi*Fc_HP(1))/Fs);  % must be recalculated each time Fc_HP changes
F2 = 2*sin((pi*Fc_LP(1))/Fs);

Q1 = 2*damp;                % this dictates size of the pass bands

yh_HP=zeros(size(x));          % create emptly out vectors
yb_HP=zeros(size(x));
yl_HP=zeros(size(x));

yh_LP=zeros(size(x));          % create emptly out vectors
yb_LP=zeros(size(x));
yl_LP=zeros(size(x));

% first sample, to avoid referencing of negative signals
yh_HP(1) = x(1);
yb_HP(1) = F1*yh_HP(1);
yl_HP(1) = F1*yb_HP(1);

yh_LP(1) = x(1);
yb_LP(1) = F1*yh_LP(1);
yl_LP(1) = F1*yb_LP(1);


% apply difference equation to the sample
for n=2:length(x)
    
    yh_HP(n) = x(n) - yl_HP(n-1) - Q1*yb_HP(n-1);
    yb_HP(n) = F1*yh_HP(n) + yb_HP(n-1);
    yl_HP(n) = F1*yb_HP(n) + yl_HP(n-1);
    
    yh_LP(n) = x(n) - yl_LP(n-1) - Q1*yb_LP(n-1);
    yb_LP(n) = F1*yh_LP(n) + yb_LP(n-1);
    yl_LP(n) = F1*yb_LP(n) + yl_LP(n-1);
    
    F1 = 2*sin((pi*Fc_HP(n))/Fs);
%     F2 = 2*sin((pi*Fc_LP(n))/Fs);
end

%normalise

maxyb_HP = max(abs(yb_HP));
yb_HP = yb_HP/maxyb_HP;

maxyb_LP = max(abs(yl_LP));
yl_LP = yl_LP/maxyb_LP;

output = [yb_HP(:),yl_LP(:)];
sound(output,Fs);
% write output wav files

% wavwrite(yb_HP, Fs, N, 'out_wah.wav');

% figure(1)
% hold on
% plot(x,'r');
% plot(yb_HP,'b');
% title('Wah-wah and original Signal');