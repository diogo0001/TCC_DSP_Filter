
clear;
clc;

play = 0;
write_file = 1;

eq = 0;
crossover = 1;

% Initial values
f0 = 500;
G = -9 ;
Q = 6;

f0_c = 2500;
Q_c = 9;

coef_amp = 1; %1/(2^2);
amp = 0.2;
% Open file
% file = 'noise.wav';
% file = 'sweep_20_1000HZ.wav';
file = 'sweep.wav';

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
    
    bl = lp(1:3) %*1.0e03
    al = lp(4:6)
    audio_f_LP = filter(bl,al,audio);
    
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

% plot(x)
% figure(2);
% plot(y);



%% Cálculo dos coeficientes

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

