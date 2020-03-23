fs = 8000; %frequency sample rate
i=1/fs; % interval
t = 0:i:2; % time
phi = 0;
A = 1;
% F = 220Hz
f = 220;
x = A*sin((2*pi*f*t) + phi);
% F = 240Hz
f = 221;
y = A*sin((2*pi*f*t) + phi);
stereo_mtx = [x(:), y(:)];
audiowrite('stereo sound.wav', stereo_mtx, fs);