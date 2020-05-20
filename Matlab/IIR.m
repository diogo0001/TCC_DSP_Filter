%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%   Author: Diogo Tavares
%   Project: Variable 2 band crossover with equalization
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
clear;
clc;

% Options -----------------------------------------------------------------
play = 0;
write_file = 0; % 1 single or 2 for cascade crossover

% Filter parameters ----------------

% Equalizer ------------------------
eq = 0;
vari = 0;

f0 = [80,200,600,1500];
G = [9,3,-6,-12] ;
Q = [3,9,3,6];

% Crossover ------------------------
crossover = 1;

f0_c = 500;
plotfft = 1;
ax = [20 3000 -80 -40];
plot = 0;
fig = 1;

% type = 'Link'
type = 'Butt';

% Files -------------------------------------------------------------------
% file = 'noise.wav';
file = 'sweep_20_2000HZ.wav'
% file = 'sweep.wav';
amp = 0.6;  % reduce to aply eq gain

input_folder = 'audio_files/';
output_folder = 'outputIIR/';
infile = strcat(input_folder,file);
[audio,fs] = audioread(infile);
audio = amp*audio(:,1);  
file = strcat(type,file);

% Param EQ ----------------------------------------------------------------
if eq == 1
    
    y = audio;  % Preserves the audio input
    
    if vari == 1   
        file = strcat('_vari_',file);
        y = eq_variator(y,50,2000,fs,20);
    else    
        % For multiple frequencies
        for i=1:length(f0)
            [a b] = eq_coef_calc(f0(i),G(i),Q(i),fs);
            y = filter(b,a,y);
        end
    end
    
    if write_file == 1
        save_file = strcat(input_folder,output_folder,'_EQ_');
        save_file = strcat(save_file,int2str(f0),file);
        audiowrite(save_file, y, fs);
    end

    if play ==1
        sound(y,fs);
    end

    if crossover == 1
        file = strcat(file,'_EQ_');
        audio = y;
    end

end

% Crossover ---------------------------------------------------------------

if crossover == 1
    
    options = bodeoptions;
    options.FreqUnits = 'Hz';
    options.Grid = 'on';
    options.Xlim = [10 20000];
    options.MagLowerLimMode ='manual';
    options.MagLowerLim = -100;
    
    fig = fig + 1;
    N = length(audio);
    
    % Butterwoth 2nd order ---------------------------
    Q_c = 0.5;
    [lp, hp] = cros_coef_calc(f0_c,G,Q_c,fs);
    bl = lp(1:3) %*1.0e03
    al = lp(4:6)
    bh = hp(1:3)  % Inverted polarization
    ah = hp(4:6)

    if strcmp(type,'Butt') 
        audio_f_LP = filter(bl,al,audio);    % section 1 2nd order
        audio_f_HP = filter(bh,ah,audio);
    end

    [ftBt_L,ftmf_L] = analise(bl,al,fig,0,0,0,fs);
    [ftBt_H,ftmf_H] = analise(bh,ah,fig,0,0,0,fs);
    
    if plot ==1
        figure(1)
        bode(ftBt_L,options);
        hold on;
        bode(ftBt_H,options);  
        bode(ftBt_L+ftBt_H,options); 
    end

    % Linkwitz-Riley 4th order ---------------------
    Q_c = 0.707;
    [lp, hp] = cros_coef_calc(f0_c,G,Q_c,fs);
    bl = lp(1:3) %*1.0e03
    al = lp(4:6)
    bh = hp(1:3)
    ah = hp(4:6)

    if strcmp(type,'Link')
        audio_f_LP1 = filter(bl,al,audio);       % section 1 2nd order
        audio_f_HP1 = filter(bh,ah,audio);
        audio_f_LP = filter(bl,al,audio_f_LP1);  % section 2 2nd order   
        audio_f_HP = filter(bh,ah,audio_f_HP1);
    end

    [ftLR_L,ftmf_L] = analise(bl,al,fig,0,0,0,fs);
    [ftLR_H,ftmf_H] = analise(bh,ah,fig,0,0,0,fs);
    ftLR_L = ftLR_L*ftLR_L;
    ftLR_H = ftLR_H*ftLR_H;
    
    if plot ==1
        figure(2)
        bode(ftLR_L,options);
        hold on;
        bode(ftLR_H,options);  
        bode(ftLR_L+ftLR_H,options); 

        figure(3)
        bode(ftLR_L,options);
        hold on;
        bode(ftLR_H,options);  
        bode(ftLR_L+ftLR_H,options);

        bode(ftBt_L,options);
        bode(ftBt_H,options);  
        bode(ftBt_L+ftBt_H,options); 
    end
    
%     [nx,dx] = tfdata(ftLR_L);
%     [n,d]=freqz(nx{1},dx{1});
%     hold on;
%     [nx,dx] = tfdata(ftLR_H);
%     freqz(nx{1},dx{1});
%     [nx,dx] = tfdata(ftLR_L+ftLR_H);
%     freqz(nx{1},dx{1});
%     plot(nx{1});
    
    stereo_mtx = [audio_f_HP(:), audio_f_LP(:)];
    ch_sum = audio_f_HP + audio_f_LP;
    
    if plotfft ==1
        plot_fft(ch_sum,fs,ax);
        hold on;
        plot_fft(audio,fs,ax);
        plot_fft(audio_f_HP,fs,ax);
        plot_fft(audio_f_LP,fs,ax);
    end
    
    if write_file == 1 || write_file == 2
        save_file = strcat(input_folder,output_folder,'_stereo_cross_');
        save_file = strcat(save_file,int2str(f0_c),file);
        audiowrite(save_file, stereo_mtx, fs);
        
        save_file = strcat(input_folder,output_folder,'_sum_cross_');
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

        hp(1) = -((1+cos(w0))/2); % Invert 2nd order polarization
        hp(2) = (1+cos(w0));      % Doesn't affect 4th order (using biquad)
        hp(3) = hp(1);             
        hp(4) = a0;
        hp(5) = (-2.0*cos(w0));
        hp(6) = (1 - alpha);  

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
        options.MagLowerLim = -80;
        figure(fig)
        bode(ftma,options);
    end
end

function buffer = eq_variator(sig,f_ini,f_end,fs,inc_f)

    G = 12;
    Q = 6;
    sig = sig';
    size = length(sig);
    step_f = -(f_ini - f_end);
    step_f = floor(step_f/inc_f);
    step_t = floor(size/abs(step_f)); 
    buffer = [];
    f0 = f_ini;
        
    for i=1:step_t:size
        if size-i <= step_t
            break
        end
        f0 = f0 + inc_f;
        [a,b] = eq_coef_calc(f0,G,Q,fs);
        buffer = [buffer filter(b,a,sig(i:i+step_t-1))];
    end
    
    buffer = [buffer filter(b,a,sig(i:end))];
    buffer = buffer';
end

% Função criada para uso generico de plotagem de graficos FFT
function plot_fft(sig,fs,ax)
    
	L = length(sig); 
    Y = fft(sig);
    P2 = abs(Y/L);
    P1 = P2(1:L/2+1);
    P1(2:end-1) = 2*P1(2:end-1);
    f = fs*(0:(L/2))/L;

    semilogx(f,20*log10(P1));
    axis(ax); 
    grid on
    title('Resposta em Frequência')
    xlabel('Frequencia (Hz)')
    ylabel('Amplitude (dB)')

end
