
clear;
clc;
% Options ------------
play = 0;
write_file = 0;

eq = 0;
crossover = 1;

% Filter parameters -------------------------------------------------------

% Equalizer ----------
f0 = 500;
G = -9 ;
Q = 6;

% Crossover ----------
f0_c = 200;
Q_c = 0.85;   %0.7109;   -3db   0.707 -> Butterrworth 2th order

types = ['butt-2nd', 'butt-4th', 'link-riley-2nd', 'link-riley-4th'];
type = 'butt-2nd';

amp = 0.6;

% Files --------------
% file = 'noise.wav';
file = 'sweep_20_1000HZ.wav';
% file = 'sweep.wav';

input_folder = 'audio_files/';
output_folder = 'outputIIR/';
infile = strcat(input_folder,file);

[audio,fs] = audioread(infile);
audio = amp*audio(:,1);  
% sound(audio,fs);

% Param EQ ----------------------------------------------------------------
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

end

% Crossover ---------------------------------------------------------------

if crossover == 1
    
    N = length(audio);
    [lp, hp] = cros_coef_calc(f0_c,G,Q_c,fs,type);
    bl = lp(1:3) %*1.0e03
    al = lp(4:6)
    bh = hp(1:3)
    ah = hp(4:6)
    
    audio_f_LP = filter(bl,al,audio);    
    audio_f_HP = filter(bh,ah,audio);
    
% Transfer function analisis -----------------------------
    [ftma_L,ftmf_L] = analise(bl,al,3,0,0,0,fs)
    [ftma_H,ftmf_H] = analise(bh,ah,3,0,0,0,fs)
    options = bodeoptions;
    options.FreqUnits = 'Hz';
    options.Grid = 'on';
    options.Xlim = [10 20000];
    options.MagLowerLimMode ='manual';
%     options.Ylim = [-100 10];
    options.MagLowerLim = -100;
  
    bode(ftma_L,options);
    hold on;
    bode(ftma_H,options);

    stereo_mtx = [audio_f_HP(:), audio_f_LP(:)];
    ch_sum = audio_f_HP + audio_f_LP;

    if write_file == 1
        save_file = strcat(input_folder,output_folder,'iir_st_cross_');
        save_file = strcat(save_file,int2str(f0_c),file);
        audiowrite(save_file, stereo_mtx, fs);
        
        save_file = strcat(input_folder,output_folder,'iir_sum_cross_');
        save_file = strcat(save_file,int2str(f0_c),file);
        audiowrite(save_file, ch_sum, fs);
    end

    if play ==1
        sound(stereo_mtx,fs);
    end
    
    if play == 2
        sound(audio_f_HP,fs);
        sound(audio_f_LP,fs);
    end
   
end

%% Cálculo dos coeficientes -----------------------------------------------

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

function [lp,hp] = cros_coef_calc(f0,G,Q,fs,type)

    if Q==0
        Q = 0.001;
    end
       
    switch type
        case 'butt-2nd'
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

        case 'butt-4th'
        case 'link-riley-2nd'
        case 'link-riley-4th'
        otherwise
            lp = 0;
            hp = 0;
    end 
end

function [ftma,ftmf] = analise(num, den,fig,bod,rlocus,step,fs)
    
    % Transfer functions
    ftma = tf(num,den,1/fs);
    ftmf = feedback(ftma,1);
    
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
        options.FreqUnits = 'Hz';
        options.FreqScale = 'log';
        options.PhaseUnits= 'deg';
        options.Grid = 'on';
        options.MagLowerLimMode ='manual'
        options.Xlim = [20 20000]; 
%    options.Ylim = [-100 10];
        options.MagLowerLim = -80;
        figure(fig)
        bode(ftma,options);
    end
end
