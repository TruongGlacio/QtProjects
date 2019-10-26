
/* qaudioplayer.q: a simple audio player
   $Id: qaudioplayer.q,v 1.11 2006/06/03 14:21:40 ag Exp $ */

/* TODO: mono file playback, better feedback when things go bonker */

import audio, sndfile, wave, ggi;
import dict, system;

/* Global data: the player and display threads, window id and geometry of the
   hosting frame, GGI visual and assorted parameters, and semaphores to
   communicate with the GUI and the player and display threads. */

def PLAYER = ref (), DISP = ref (),
  WINID = ref (), VIS = ref (), GEOM = ref (),
  FORE = ref (0,0,0), BACK = ref (0xffff,0xffff,0xffff),
  MSGS = semaphore, CTRL = semaphore, CMD = semaphore;

playing		= isthread H and then active H
		    where H = get PLAYER;

/* Choose the converter for doing on-the-fly sample rate conversion. See
   wave.q for the possible values and their meaning. The following default
   provides a reasonable compromise between conversion speed and output
   quality. */

def SRC_CONVERTER = SRC_SINC_FASTEST;

/* Master volume and (reciprocal of) playback speed. */

def GAIN = ref 1.0, SPEED = ref 1.0;

/* FFT parameters. */

def FMIN = ref 0, FMAX = ref 2500, SCALE = ref 5, FFT = ref false;

/* Callbacks. */

private load, play, stop, slider,
  init_display, update_display, redraw_display, resize_display;

rgb RGB		= (R,G,B)
		    where [B,G,R,A] = map (*0x101) (bytes (bytestr RGB));

initcb ID F B	= put WINID ID || put FORE F || put BACK B || init_display
		    where F = rgb F, B = rgb B;

finicb		= cancel (get PLAYER) || cancel (get DISP) || put VIS ();

fftcb ON	= put FFT ON || redraw_display;

fmincb VAL	= put FMIN VAL || redraw_display if VAL < get FMAX;

fmaxcb VAL	= put FMAX VAL || redraw_display if VAL > get FMIN;

scalecb VAL	= put SCALE VAL || redraw_display;

volumecb VAL	= put GAIN (VAL/100.0);

speedcb VAL	= put SPEED (100.0/VAL);

opencb NAME	= cancel (get PLAYER) || post MSGS (0.0,0) ||
		  put PLAYER (thread (load NAME));

playcb		= post CTRL play if playing;

stopcb		= post CTRL stop if playing;

slidercb POS	= post CTRL (slider POS) if playing;

redrawcb	= redraw_display;

resizecb WD HT	= resize_display WD HT;

/* The default audio device. We prefer Jack if it is available. Otherwise we
   just pick the first device with a nonzero number of output ports. */

find_loop N P []
		= -1;
find_loop N P [X|Xs]
		= N if P X;
		= find_loop (N+1) P Xs otherwise;

find P Xs	= find_loop 0 P Xs;

out_dev_ok (_,_,_,M,_)
		= M>0;
out_dev_ok ID:Int
		= out_dev_ok (audio_devices!ID);
out_dev_ok _	= false otherwise;

jack_devs	= cat (map (!2) DRVS)
		    where DRVS = filter ((=12).(!1)) audio_drivers;

default_out	= DEVS!ID if ID>=0
		    where DEVS = jack_devs, ID = find out_dev_ok DEVS;
		= find out_dev_ok audio_devices otherwise;

/* Audio setup. We assume a device capable of playing 16 bit stereo integer
   samples. The device number can be set via the Q_AUDIO_OUT environment
   variable. */

audio_out	= VAL where VAL:Int = val (getenv "Q_AUDIO_OUT");
		= default_out otherwise; // default

audio_setup	= (DRVNAME,DEVNAME,round RATE)
		    where (DEVNAME,DRV,_,_,RATE) = audio_devices!audio_out,
		      (DRVNAME|_) = audio_drivers!DRV;
		= ("None","None",()) otherwise;

def (DRIVER,DEVICE,RATE) = audio_setup, FORMAT = PA_INT16, BUFSIZE = 2048;

/* The FPS value determines the number of display frames to draw per second.
   FRAMESIZE is the corresponding number of audio frames which have to be
   drawn at each iteration. */

def FPS = 10, FRAMESIZE = round (RATE/FPS);

/* Check system for available realtime thread priorities. */

private maxprio;
def POL = SCHED_FIFO, PRIO = maxprio;

testprio PRIO		= setsched this_thread 0 0 || true
			    where () = setsched this_thread POL PRIO;
			= false otherwise;

maxprio			= until (neg testprio) (+1) 1 - 1;

/* Execute a task in realtime. */

special realtime X;

realtime X		= setsched TH 0 0 || Y
			    where TH = this_thread,
			      Y = setsched TH POL PRIO || X;

/* "Ping" a sound file. Check that the file exists and that libsndfile can
   open it. Return a tuple (NAME,RATE,CHANNELS,FORMAT,SUBTYPE) if everything
   is ok, fail otherwise. The meaning of the returned fields is as follows:

   NAME:	the basename of the file (stripping the path)
   RATE:	the sample rate
   CHANNELS:	the number of channels
   FORMAT:	the major format of the file, as a string
   SUBTYPE:	the subtype, as a string */

fst2 (X,Y|_)		= (X,Y);

sf_ping NAME		= (NAME,RATE,CHANNELS,FORMAT,SUBTYPE)
  where SF:SndFile = sf_open NAME SFM_READ (),
    (RATE,CHANNELS,FORMAT) = sf_info SF,
    SUBTYPE = FORMAT and SF_FORMAT_SUBMASK,
    FORMAT = FORMAT and SF_FORMAT_TYPEMASK,
    FORMATS = dict (map fst2 sf_major_formats),
    SUBTYPES = dict sf_subtypes,
    FORMAT:String = FORMATS!FORMAT,
    SUBTYPE:String = SUBTYPES!SUBTYPE,
    NAME:String = last (split "/" NAME);

/* Read a number of samples from the input file, performing sample rate
   conversion on the fly. */

read_wave SF SRC RATIO NFRAMES
			= WAV
			    where WAV:ByteStr = sf_read_double SF NFRAMES,
			      WAV:ByteStr = src_process SRC RATIO WAV;

/* The play loop. In each iteration we read FRAMESIZE samples from the sound
   file, draw them on the screen and play them back. This is executed as a
   background task. */

private play_loop, init_state, update_state;

load NAME		= realtime (play_loop (init_state NAME));

play_loop STATE:Tuple	= play_loop (update_state STATE);
play_loop STATE		= () otherwise;

/* Initialize the player thread. The state of the thread is kept in a tuple
   with the following components:

   - NAME: The name of the soundfile.
   - SF: The soundfile.
   - OUT: The audio output stream.
   - SRC: Sample rate converter state
   - RATIO: Nominal sample rate conversion ratio
   - POS: Current position of the slider.
   - I: Current audio frame number.
   - N: Total number of audio frames.
   - WAV: Current part of the wave being played.
   - PLAY: Flag indicating whether we are currently playing back.

   The following parameters are kept as globals, as they are modified outside
   the loop:

   - VIS: The ggi visual on which the wave is rendered.

   - GEOM: The geometry of the visual, consisting of:

   - WD, HT: The dimensions of the visual.
   - WH: Height of the wave display for each channel.
   - LT, LC, RT, RC: Top and center Y values of the display for the left
     and right channel, respectively. */

private next_state;

init_state NAME		= update_display WAV ||
			  (NAME,SF,SF_RATE,OUT,SRC,RATIO,0.0,0,
			   sf_frames SF,WAV,false)
  where OUT:AudioStream =
    open_audio_stream audio_out PA_WRITE (RATE,CHANNELS,FORMAT,BUFSIZE),
    SRC:SRCState = src_new SRC_CONVERTER CHANNELS,
    RATIO:Num = RATE/SF_RATE,
    ACT_RATIO = RATIO*get SPEED,
    NFRAMES = round (FRAMESIZE/ACT_RATIO),
    WAV:ByteStr = read_wave SF SRC ACT_RATIO NFRAMES
  if CHANNELS = 2 // FIXME: only stereo waves for now
  where SF:SndFile = sf_open NAME SFM_READ (),
    (SF_RATE,CHANNELS,_) = sf_info SF;

init_state NAME		= update_display () || fail otherwise;

/* Update the thread state. */

update_state STATE	= get CTRL STATE if #CTRL > 0;
update_state (NAME,SF,SF_RATE,OUT,SRC,RATIO,POS,I,N,WAV,PLAY)
			= update_display WAV ||
			  post MSGS (POS,I div SF_RATE) ||
			  write_audio_stream OUT
			  (wave_to FORMAT (prd_wave (get GAIN) WAV)) ||
			  next_state
			  (NAME,SF,SF_RATE,OUT,SRC,RATIO,POS,I,N,WAV,PLAY)
			    where POS = I/N if PLAY;
update_state STATE	= get CTRL STATE otherwise;

next_state (NAME,SF,SF_RATE,OUT,SRC,RATIO,POS,I,N,WAV,PLAY)
			= (NAME,SF,SF_RATE,OUT,SRC,RATIO,POS,
			   I+NFRAMES,N,WAV1,PLAY)
			    if #WAV1 > 0
			    where ACT_RATIO = RATIO*get SPEED,
			      NFRAMES = round (FRAMESIZE/ACT_RATIO),
			      WAV1:ByteStr = read_wave SF SRC ACT_RATIO NFRAMES;
			= (NAME,SF,SF_RATE,OUT,SRC,RATIO,POS,I,N,WAV,
			   false)
			    otherwise;

/* Process control messages. */

play (NAME,SF,SF_RATE,OUT,SRC,RATIO,POS,I,N,WAV,PLAY)
			= (NAME,SF,SF_RATE,OUT,SRC,RATIO,POS,I,N,WAV,
			   true);

stop (NAME,SF,SF_RATE,OUT,SRC,RATIO,POS,I,N,WAV,PLAY)
			= update_display WAV ||
			  post MSGS (POS,I div SF_RATE) ||
			  (NAME,SF,SF_RATE,OUT,SRC,RATIO,POS,I,N,WAV,
			   false)
			    where POS = I/N if PLAY;
stop STATE		= STATE otherwise;

trim N K		= N div K * K;

slider POS1 (NAME,SF,SF_RATE,OUT,SRC,RATIO,POS,I,N,WAV,PLAY)
			= update_display WAV ||
			  post MSGS (I div SF_RATE) ||
			  (NAME,SF,SF_RATE,OUT,SRC,RATIO,
			   POS1,I,N,WAV,PLAY)
			    where ACT_RATIO = RATIO*get SPEED,
			      NFRAMES = round (FRAMESIZE/ACT_RATIO),
			      I = trim (round (POS1*N)) NFRAMES,
 			      WAV:ByteStr = sf_seek SF I SEEK_SET ||
			      read_wave SF SRC ACT_RATIO NFRAMES
			    if POS1<>POS;
slider _ STATE		= STATE otherwise;

/* Initialize the display. */

private display_loop, show_wave;

geom (WD,HT)		= (WD,HT,WH,LT,LC,RT,RC)
  where
    WH = (HT-20) div 2,
    LT = 10, LC = LT+WH div 2,
    RT = LT+WH, RC = RT+WH div 2;

init_display		= put qaudioplayer::VIS VIS ||
			  put GEOM (geom (WD,HT)) ||
			  put DISP (thread (display_loop ())) ||
			  update_display ()
  where
    VIS = ggi_open (sprintf "x:-keepcursor -inwin=%d" (get WINID)),
    (WD,HT) = ggi_set_mode VIS "" || sscanf (ggi_get_mode VIS) "%dx%d",
    _ = ggi_set_background VIS (get BACK) ||
        ggi_set_foreground VIS (get FORE) ||
        ggi_set_flags VIS GGI_FLAG_ASYNC;

/* The display loop. */

display_loop WAV	= display_loop (get CMD WAV);

show WAV _		= show_wave WAV || ggi_flush (get VIS) || WAV;
redraw WAV		= show_wave WAV || ggi_flush (get VIS) || WAV;

/* Update the display. */

update_display WAV	= post CMD (show WAV);

/* Draw the wave in a box, centered around a horizontal zero line. This is
   done once for each channel in a stereo signal. If FFT mode is enabled, the
   magnitude spectrum is drawn instead of the wave itself. */

private show_fft;

show_wave WAV		= show_fft WAV if get FFT;

show_wave ()		= ggi_clear VIS ||
			  ggi_draw_hline VIS (0,LC) WD ||
			  ggi_draw_hline VIS (0,RC) WD ||
			  ggi_puts VIS (5,LT) "L" ||
			  ggi_puts VIS (5,RT) "R"
  where VIS = get qaudioplayer::VIS,
    (WD,HT,WH,LT,LC,RT,RC) = get GEOM;

show_wave WAV		= ggi_clear VIS ||
			  ggi_draw_hline VIS (0,LC) WD ||
			  ggi_draw_hline VIS (0,RC) WD ||
			  draw_wave VIS (0,LT) (WD,WH) L ||
			  ggi_puts VIS (5,LT) "L" ||
			  draw_wave VIS (0,RT) (WD,WH) R ||
			  ggi_puts VIS (5,RT) "R"
  where VIS = get qaudioplayer::VIS,
    (WD,HT,WH,LT,LC,RT,RC) = get GEOM,
    [L,R] = split_wave 2 WAV;

/* FFT display. */

draw_tick VIS X Y	= ggi_draw_vline VIS (X,Y) 10;
draw_val VIS WD X Y S	= ggi_puts VIS (X,Y+10) S if X=0;
			= ggi_puts VIS (X-OFFS,Y+10) S
			    where (OFFS,_) = ggi_get_string_size VIS S,
			      OFFS = ifelse (X>=WD-1) OFFS (OFFS div 2);

freqval I		= get FMIN+(I/10*(get FMAX-get FMIN));

show_fft ()		= ggi_clear VIS ||
			  ggi_draw_hline VIS (0,LC) WD ||
			  do (flip (draw_tick VIS) LC) TICKS ||
			  do (uncurry (flip (draw_val VIS WD) LC))
			  (zip TICKS S) ||
			  ggi_draw_hline VIS (0,RC) WD ||
			  do (flip (draw_tick VIS) RC) TICKS ||
			  do (uncurry (flip (draw_val VIS WD) RC))
			  (zip TICKS S) ||
			  ggi_puts VIS (5,LT) "L" ||
			  ggi_puts VIS (5,RT) "R"
  where VIS = get qaudioplayer::VIS,
    (WD,HT,WH,LT,LC,RT,RC) = get GEOM,
    TICKS = map (round.((WD/10)*)) [0..9] ++ [WD-1],
    S = map (sprintf "%.0f".freqval) [0..10];

show_fft WAV		= ggi_clear VIS ||
			  ggi_draw_hline VIS (0,LC) WD ||
			  do (flip (draw_tick VIS) LC) TICKS ||
			  do (uncurry (flip (draw_val VIS WD) LC))
			  (zip TICKS S) ||
			  ggi_draw_hline VIS (0,RC) WD ||
			  do (flip (draw_tick VIS) RC) TICKS ||
			  do (uncurry (flip (draw_val VIS WD) RC))
			  (zip TICKS S) ||
			  draw_wave VIS (0,LT) (WD,WH) MAG1 ||
			  ggi_puts VIS (5,LT) "L" ||
			  draw_wave VIS (0,RT) (WD,WH) MAG2 ||
			  ggi_puts VIS (5,RT) "R"
  where VIS = get qaudioplayer::VIS,
    (WD,HT,WH,LT,LC,RT,RC) = get GEOM,
    [L,R] = split_wave 2 WAV,
    MAG1 = prd_wave (get SCALE) (fft L!0),
    MAG2 = prd_wave (get SCALE) (fft R!0),
    N = wave_size MAG1-1,
    N1 = round (N*(get FMIN*2/RATE)),
    N2 = round (N*(get FMAX*2/RATE)),
    MAG1 = sub_wave MAG1 N1 N2, MAG2 = sub_wave MAG2 N1 N2,
    TICKS = map (round.((WD/10)*)) [0..9] ++ [WD-1],
    S = map (sprintf "%.0f".freqval) [0..10];

/* Redraw the display. */

redraw_display		= post CMD redraw;

/* Resize the display. */

resize_display _ _	= put GEOM (geom (WD,HT)) ||
			  redraw_display
  where
    VIS = get qaudioplayer::VIS,
    (WD,HT) = ggi_set_mode VIS "" || sscanf (ggi_get_mode VIS) "%dx%d",
    _ = ggi_set_background VIS (get BACK) ||
        ggi_set_foreground VIS (get FORE);
