
file = 'audio_files/noise.wav';
[noise,fs] = audioread(file);
% noise = noise(:,1);

[b,a] = sos2tf(SOS,G);

f = filter(b,a,noise);

sound(f,fs)