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

% Filter parameters -------------------------------------------------------

% Equalizer ------------------------
eq = 1;
vari = 0;

f0 = [500];
G = [9];
Q = [3];

% Crossover ------------------------
crossover = 0;
f0_c = 500;
pol = 1;        % invert -> -1

type = 'Link';
% type = 'Butt';

% Plot -----------------------------
plot = 1;
plotfft = 2;
bode = 0;
ax = [20 20000 -80 -40];  
fig = 1; 

% plot_type = 'All';
% plot_type = 'Butt';
plot_type = 'Link';

% Files -----------------------------
% file = 'noise.wav';
% file = 'sweep_20_2000HZ.wav'
file = 'sweep2.wav';
amp = 3.08;  % reduce to apply eq gain if necessary

input_folder = 'audio_files/';
output_folder = 'outputIIR/';
infile = strcat(input_folder,file);

[audio,fs] = audioread(infile);
audio = amp*audio(:,1);

file = strcat(type,file);

scale = fs/(2*pi); 

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
    
    % Plot    
    if plot ==1
        axx = findall(gcf, 'Type', 'axes');
        set(axx, 'XScale', 'log');
        set(axx, 'XLim', [0 20000]);

        figure(fig);
        [hl, wl] = freqz(b,a,fs);
        subplot(2,1,1)
        semilogx(wl*scale,20*log10(abs(hl)));
        title('Função de transferência do equalizador');
        ylabel('Magnitude (dB)');
        hold on; grid on;        
        subplot(2,1,2)
        semilogx(wl*scale,phase(hl)*180/pi);
        xlabel('Frequência (Hz)');
        ylabel('Fase (graus)');
        hold on; grid on;
    end
    
    if plotfft == 2
        fig = fig + 1;
        figure(fig)
        plot_fft(y,fs,ax);
        hold on;
        plot_fft(audio,fs,ax);
        title('Resposta em frequência do equalizador com sweep');
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
    
    N = length(audio);
    
    % Butterwoth 2nd order ---------------------------
    Q_c = 0.5;
    [lp, hp] = cros_coef_calc(f0_c,G,Q_c,fs,pol);
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
    
    if plot == 1
        if bode == 1 
            figure(fig); %1
            bode(ftBt_L,options);
            hold on;
            bode(ftBt_H,options);  
            bode(ftBt_L+ftBt_H,options); 
            
        elseif strcmp(plot_type,'Butt') || strcmp(plot_type,'All')
            axx = findall(gcf, 'Type', 'axes');
            set(axx, 'XScale', 'log');
            set(axx, 'XLim', [20 20000]);

            figure(fig); %1
            [hl, wl] = freqz(bl,al,fs);     
            subplot(2,1,1)
            semilogx(wl*scale,20*log10(abs(hl)),'b');   % LP butt freq
            title('Função de transferência do crossover');
            ylabel('Magnitude (dB)');
            hold on; grid on;        
            subplot(2,1,2)                  
            semilogx(wl*scale,angle(hl)*180/pi,'b');    % LP butt phase
            xlabel('Frequência (Hz)');
            ylabel('Fase (graus)');
            hold on; grid on;

            subplot(2,1,1)
            [hh, wh] = freqz(bh,ah,fs);     
            semilogx(wh*scale,20*log10(abs(hh)),'r');   % HP butt freq
            subplot(2,1,2)
            semilogx(wh*scale,angle(hh)*180/pi,'r');    % HP butt phase
           
            subplot(2,1,1)                  
            [nx,dx] = tfdata(ftBt_L+ftBt_H,'v');    
            [ht,wt]=freqz(nx,dx,fs);
            semilogx(wt*scale,20*log10(abs(ht)),'y');   % Sum butt freq
%             subplot(2,1,2)
%             semilogx(wt*scale,angle(ht)*180/pi);    % Sum butt phase
        end
    end

    % Linkwitz-Riley - 4th order ---------------------
    Q_c = 0.707;
    [lp, hp] = cros_coef_calc(f0_c,G,Q_c,fs,pol);
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
    
    if plot == 1
        if bode == 1
            fig = fig + 1; %2
            figure(fig)
            bode(ftLR_L,options);
            hold on;
            bode(ftLR_H,options);  
            bode(ftLR_L+ftLR_H,options); 
            
            bode(ftLR_L,options);
            hold on;
            bode(ftLR_H,options);  
            bode(ftLR_L+ftLR_H,options);

            bode(ftBt_L,options);
            bode(ftBt_H,options);  
            bode(ftBt_L+ftBt_H,options); 
            
         elseif strcmp(plot_type,'Link') || strcmp(plot_type,'All') 
            ax = findall(gcf, 'Type', 'axes');
            set(ax, 'XScale', 'log');
            set(ax, 'XLim', [20 20000]);
       
            figure(fig)
            subplot(2,1,1);
            [nx,dx] = tfdata(ftLR_L,'v');          
            [hl, wl] = freqz(nx,dx,fs);
            semilogx(wl*scale,20*log10(abs(hl)),'m');   % LP link freq
            title('Função de transferência do equalizador');
            ylabel('Magnitude (dB)');
            hold on; grid on;   
            subplot(2,1,2)
            semilogx(wl*scale,phase(hl)*180/pi,'m');    % LP link phase
            xlabel('Frequência (Hz)');
            ylabel('Fase (graus)');
            hold on; grid on;

            subplot(2,1,1);        
            [nx,dx] = tfdata(ftLR_H,'v');
            [hh, wh] = freqz(nx,dx,fs);
            semilogx(wh*scale,20*log10(abs(hh)),'g');   % HP link freq
            subplot(2,1,2)
            semilogx(wh*scale,360+phase(hh)*180/pi,'g');    % HP link phase

            subplot(2,1,1);
            [nx,dx] = tfdata(ftLR_L+ftLR_H,'v');
            [ht,wt] = freqz(nx,dx,fs);
            semilogx(wt*scale,20*log10(abs(ht)),'c');   % Sum link freq
%             subplot(2,1,2)
%             semilogx(wt*scale,angle(ht)*180/pi);    % Sum link phase
        end 
    end
    
    stereo_mtx = [audio_f_HP(:), audio_f_LP(:)];
    ch_sum = audio_f_HP + audio_f_LP;
    
    if plotfft ==1
        fig = fig + 1; %3
        figure(fig)
        plot_fft(ch_sum,fs,ax);
        hold on;
%         plot_fft(audio,fs,ax);
%         plot_fft(audio_f_HP,fs,ax);
%         plot_fft(audio_f_LP,fs,ax);
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
    K = 10^e;
    
    if G<0
        B = B/K;
    end
    
    b = -cos(2*pi*(f0/fs));
    a = (1-tan(pi*B/fs))/((1+tan(pi*B/fs)));
    
    b0 = (1+a+K-K*a)*0.5;
    b1 = (b+b*a);
    b2 = (1+a-K+K*a)*0.5;
    a1 = b1;
    a2 = a;
    
    ca = [1 a1 a2];
    cb = [b0 b1 b2];
end

function [lp,hp] = cros_coef_calc(f0,G,Q,fs,pol)

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

        hp(1) = ((1+cos(w0))/2)*pol; % Invert 2nd order polarization
        hp(2) = -(1+cos(w0))*pol;      % Doesn't affect 4th order (using biquad)
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
    xlabel('Frequência (Hz)')
    ylabel('Amplitude (dB)')

end
