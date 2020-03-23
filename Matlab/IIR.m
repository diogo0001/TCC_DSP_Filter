%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
%
%%%%%%%%%%%%%%%%%%%%%%%%%%

clear;
clc;

% Open file
% file = 'sweep_20_1000Hz.wav';
file = 'noise.wav';
save_file = strcat('iir_cross_',file);
[x,fs] = audioread(file);
x = x(:,1);

% Initial values
f0 = 200;
G = -30;
Q = 8;

prev_f0 = f0;
prev_G = G;
prev_Q = Q;

[a b] = coef_calc(f0,G,Q,fs);

y(1) = b(1)*x(1);
y(2) = b(1)*x(2) + b(2)*x(1) - a(1)*y(1);
y(3) = b(1)*x(3) + b(2)*x(2) + b(3)*x(1) - a(1)*y(2) - a(2)*y(1);

i = 1;
N = length(x);
for n=4:N
    
    i = i+1;
    
    if i> N/60
        i = 1;
        f0 = f0 + 100;
    end
    
    
    if prev_f0 ~= f0 | prev_G ~= G | prev_Q ~= Q
        [a b] = coef_calc(f0,G,Q,fs);
        prev_f0 = f0;
        prev_G = G;
        prev_Q = Q;
    end
    
     y(n) = b(1)*x(n) + b(2)*x(n-1) + b(3)*x(n-2) - a(1)*y(n-1) - a(2)*y(n-2); 
end

y = y';

sound(y,fs);

% plot(x)
% figure(2);
% plot(y);

if prev_f0 ~= f0 | prev_G ~= G | prev_Q ~= Q
    [a b] = coef_calc(f0,G,Q,fs);
    prev_f0 = f0;
    prev_G = G;
    prev_Q = Q;
end


%%
function [ca cb] = coef_calc(f0,G,Q,fs)

    B = f0/Q;
    e = G/20;

    b = -cos(2*pi*(f0/fs));
    a = (1-tan(pi*B/fs))/((1+tan(pi*B/fs)));
    K = 10^e;
    
    b0 = (1+a+K-K*a)*0.5;
    b1 = (b+b*a);
    b2 = (1+a-K+K*a)*0.5;
    a1 = b1;
    a2 = a;
    
    ca = [a1 a2];
    cb = [b0 b1 b2];
end