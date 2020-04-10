
clear;
clc;

play = 0;
write_file = 0;

eq = 1;
crossover = 0;

% Initial values
f0 = 500;
G = -9 ;
Q = 6;

f0_c = 1000;
Q_c = 6;

coef_amp = 1; %1/(2^2);
amp = 0.2;
% Open file
% file = 'noise.wav';
file = 'sweep_20_1000HZ.wav';
% file = 'sweep.wav';

input_folder = 'audio_files/';
output_folder = 'outputIIR/';
infile = strcat(input_folder,file);

[audio,fs] = audioread(infile);
audio = amp*audio(:,1);  
% sound(audio,fs);

% ---------------------------------
if eq == 1
    
    [a b] = eq_coef_calc(f0,G,Q,fs);
%     y = param_eq(a,b,audio,coef_amp);
    
    analise(b,a,3,1,0,0,fs)
    
    y = filter(b,a,audio);
    
    if write_file == 1
        save_file = strcat(input_folder,output_folder,'iir_EQ_');
        save_file = strcat(save_file,int2str(f0),file);
        audiowrite(save_file, y, fs);
    end

    if play ==1
        sound(y,fs);
    end

    if crossover == 1
        audio = y;
    end
    
    plot_fft(y,fs);
end

% --------------------------------------------

if crossover == 1
    
    N = length(audio);
    %audio_f_LP = zeros(1,N);
    %audio_f_HP = zeros(1,N);
    
    [lp, hp] = cros_coef_calc(f0_c,G,Q_c,fs);
%     stereo_mtx = cross(lp,hp,audio,coef_amp);
    
    bh = hp(1:3)
    ah = hp(4:6)
    audio_f_HP = filter(bh,ah,audio);
%     analise(bh, ah,1,1,0,0,fs)
    
    bl = lp(1:3) %*1.0e03
    al = lp(4:6)
    audio_f_LP = filter(bl,al,audio);
%     analise(bl,al,3,1,0,0,fs)
    
    [ftma_L,ftmf_L] = analise(bl,al,3,0,0,0,fs);
    [ftma_H,ftmf_H] = analise(bh,ah,3,0,0,0,fs);
    options = bodeoptions;
    options.FreqUnits = 'Hz'; % or 'rad/second', 'rpm', etc.
    options.Grid = 'on';
    options.Xlim = [20 20000]; 
%     options.Ylim = [-100 10];
    options.MagLowerLim = -100;
  
    bode(ftma_L,options);
    hold on;
    bode(ftma_H,options);
    
    stereo_mtx = [audio_f_HP(:), audio_f_LP(:)];

    if write_file == 1
        save_file = strcat(input_folder,output_folder,'iir_cross_');
        save_file = strcat(save_file,int2str(f0_c),file);
        audiowrite(save_file, stereo_mtx, fs);
    end

    if play ==1
        sound(stereo_mtx,fs);
    end
    
    if play == 2
        sound(audio_f_HP,fs);
        sound(audio_f_LP,fs);
    end
   
end

% Cálculo dos coeficientes

function [ca,cb] = eq_coef_calc(f0,G,Q,fs)

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
    
    ca = [1 a1 a2];
    cb = [b0 b1 b2];
end

function [lp,hp] = cros_coef_calc(f0,G,Q,fs)

    if Q==0
        Q = 0.001;
    end
    
    w0 = 2*pi*f0/fs;
    alpha = sin(w0)/(2.0*Q);
    a0 = 1 + alpha;
    
    lp(1) = ((1-cos(w0))/2);
    lp(2) = (1-cos(w0));
    lp(3) = lp(1);
    lp(4) = a0;
    lp(5) = (-2.0*cos(w0));
    lp(6) = (1 - alpha);
    
    hp(1) = ((1+cos(w0))/2);
    hp(2) = -(1+cos(w0));
    hp(3) = hp(1);
    hp(4) = a0;
    hp(5) = (-2.0*cos(w0));
    hp(6) = (1 - alpha);  
end 

% Implementação
function [y] = param_eq(a,b,audio,coef_amp)

    a = coef_amp*a;
    b = coef_amp*b;
    
    y(1) = b(1)*audio(1);
    y(2) = b(1)*audio(2) + b(2)*audio(1) - a(1)*y(1);
    y(3) = b(1)*audio(3) + b(2)*audio(2) + b(3)*audio(1) - a(1)*y(2) - a(2)*y(1);

    N = length(audio);

    for n=4:N
         y(n) = b(1)*audio(n) + b(2)*audio(n-1) + b(3)*audio(n-2) - a(1)*y(n-1) - a(2)*y(n-2); 
    end

    y = y';
end


function [y] = cross(lp,hp,audio,coef_amp)

    N = length(audio);
    audio_f_LP = zeros(1,N);
    audio_f_HP = zeros(1,N);
        
    lp = coef_amp*lp;
    hp = coef_amp*hp;

    % HP ----------------------------
    b = hp(1:3)
    a = hp(4:5)

    audio_f_HP(1) = b(1)*audio(1);
    audio_f_HP(2) = b(1)*audio(2) + b(2)*audio(1) - a(1)*audio_f_HP(1);
    audio_f_HP(3) = b(1)*audio(3) + b(2)*audio(2) + b(3)*audio(1) - a(1)*audio_f_HP(2) - a(2)*audio_f_HP(1);

    for n=4:N
        audio_f_HP(n) = b(1)*audio(n) + b(2)*audio(n-1) + b(3)*audio(n-2) - a(1)*audio_f_HP(n-1) - a(2)*audio_f_HP(n-2); 
    end
    % -------------------------------
    
    % LP ----------------------------
    b = lp(1:3)
    a = lp(4:5)
    
    audio_f_LP(1) = b(1)*audio(1);
    audio_f_LP(2) = b(1)*audio(2) + b(2)*audio(1) - a(1)*audio_f_LP(1);
    audio_f_LP(3) = b(1)*audio(3) + b(2)*audio(2) + b(3)*audio(1) - a(1)*audio_f_LP(2) - a(2)*audio_f_LP(1);

    for n=4:N
        audio_f_LP(n) = b(1)*audio(n) + b(2)*audio(n-1) + b(3)*audio(n-2) - a(1)*audio_f_LP(n-1) - a(2)*audio_f_LP(n-2); 
    end
    % -------------------------------
    
    y = [audio_f_HP(:), audio_f_LP(:)];

end

function [ftma,ftmf] = analise(num, den,fig,bod,rlocus,step,fs)
    
    % Transfer functions
    ftma = tf(num,den,1/fs)
    ftmf = feedback(ftma,1)
    
    if rlocus~=0
        figure(fig)
        rlocus(ftmf);
        hold on;
        fig = fig+1;
    end
    
    if step~=0
        figure(fig)
        hold on
        step(ftmf);
        fig = fig+1;
    end
    
    if bod~=0
        f1 = 2*pi*20;
        f2 = 2*pi*20000;
        w = logspace(f1,f2,f2*2) ;
        options = bodeoptions;
        options.FreqUnits = 'Hz'; % or 'rad/second', 'rpm', etc.
        options.FreqScale = 'log';
        options.PhaseUnits= 'deg';
        options.Grid = 'on';
        options.Xlim = [20 20000]; 
%    options.Ylim = [-100 10];
        options.MagLowerLim = -100;
        figure(fig)
        bode(ftma,options);
    end
end

function plot_fft(sig,fs)
n = [0:29];
x = cos(2*pi*n/10);

N1 = 64;
N2 = 128;
N3 = 256;
X1 = abs(fft(x,N1));
X2 = abs(fft(x,N2));
X3 = abs(fft(x,N3));
    
F1 = [0 : N1 - 1]/N1;
F2 = [0 : N2 - 1]/N2;
F3 = [0 : N3 - 1]/N3;

subplot(3,1,1)
plot(F1,X1,'-x'),axis([0 1 0 20])
subplot(3,1,2)
plot(F2,X2,'-x'),axis([0 1 0 20])
subplot(3,1,3)
plot(F3,X3,'-x'),axis([0 1 0 20])

end