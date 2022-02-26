#
#include	<sstream>
#include	<unoevent.h>
#include	<iunoplugincontroller.h>
#include	<vector>
#include	<sstream>
#include	<chrono>
#include        <Windows.h>
#include        <mutex>

#include	"SDRunoPlugin_cw.h"
#include	"SDRunoPlugin_cwUi.h"
#include	"utilities.h"
#include	"up-filter.h"

#define CWError         1000
#define CWNewLetter     1001
#define CWCarrierUp     1002
#define CWCarrierDown   1005
#define CWStroke        1003
#define CWDot           1004
#define CWendofword     1006
#define CWFailing       1007


#define USECS_PER_SEC           1000000
#define MODE_IDLE               0100
#define MODE_IN_TONE            0200
#define MODE_AFTER_TONE         0300
#define MODE_END_OF_LETTER      0400

#define CW_DOT_REPRESENTATION   '.'
#define CW_DASH_REPRESENTATION  '_'
#define CW_IF                   0

/*
 *      The standard morse wordlength (i.e.
 *      PARIS) is 50 bits, then for a wpm of 1,
 *      one bit takes 1.2 second or 1.2 x 10^6  microseconds
 */
#define DOT_MAGIC               1200000

#define  _USE_MATH_DEFINES
#include        <math.h>

#define	NO_OFFSET_FOUND	-500
#define	SEARCH_WIDTH	400
#define	CW_RATE	2000
#define	IN_RATE	192000

SDRunoPlugin_cw::SDRunoPlugin_cw (IUnoPluginController& controller) :
	                                           IUnoPlugin (controller),
	                                           m_form (*this, controller),
	                                           m_worker (nullptr),
	                                           cwBuffer (16 * 32768),
	                                           cwAudioBuffer (16 * 32768),
	                                           passbandFilter (35,
		                                                   -CW_RATE / 2,
		                                                   +CW_RATE / 2,
		                                                   IN_RATE),
	                                           theMixer (IN_RATE),
	                                           theDecimator (IN_RATE, 2000),
	                                           cwToneBuffer (128),
	                                           newFFT (CW_RATE, 0, CW_RATE - 1) {
	m_controller = &controller;
	//	cw specifics
	cw_IF		= CW_IF;
	SmoothenSamples = new cwAverage(4);
	thresholdFilter = new cwAverage(8);
	spaceFilter	= new cwAverage(16);
	cwTracking	= false;
	cwDefaultSpeed	= 30;
	agc_peak	= 0;
	noiseLevel	= 0;
	SquelchValue.store(5);
	cwState		= MODE_IDLE;
	cwPreviousState = MODE_IDLE;
	lengthofPreviousTone = 0;
	cwRange		= cwDefaultSpeed / 2;
	lowerBoundWPM	= cwDefaultSpeed - cwRange;
	upperBoundWPM	= cwDefaultSpeed + cwRange;
	cwSpeed		= cwDefaultSpeed;       /* initial    */

	cwDefaultDotLength = DOT_MAGIC / cwDefaultSpeed;
	cwDotLength	= cwDefaultDotLength; // in usec
	cwSpaceLength	= cwDefaultDotLength; // in usec
	cwDotLengthMax	= DOT_MAGIC / lowerBoundWPM;
	cwDotLengthMin	= DOT_MAGIC / upperBoundWPM;

	cwDashLength	= 3 * cwDotLength;
	cwDashLengthMax = 3 * cwDotLengthMax;
	cwDashLengthMin = 3 * cwDotLengthMin;

	cw_adaptive_threshold = 2 * cwDefaultDotLength;
	cwNoiseSpike	= cwDotLength / 3;
	cwStartTimestamp = 0;
	cwEndTimestamp	= 0;
	cwAudioRate	= 0;
	cwFilterDegree	= 5;
	cw_BandPassFilter = new bandpassFilter (2 * cwFilterDegree + 1,
	                                        cw_IF - 500,
	                                        cw_IF + 500,
		                                WORKING_RATE);
	cw_finalFilter	= new bandpassFilter (2 * cwFilterDegree - 1,
	                                      cw_IF - 50,
	                                      cw_IF + 50, WORKING_RATE);
	                                      
//


	searchRange	= SEARCH_WIDTH;
	m_controller	-> RegisterStreamProcessor (0, this);
	m_controller    -> RegisterAudioProcessor (0, this);
	m_controller    -> SetDemodulatorType (0,
                                 IUnoPluginController::DemodulatorIQOUT);

	selectedFrequency
	                = m_controller -> GetVfoFrequency (0);
	centerFrequency = m_controller -> GetCenterFrequency (0);
	cwAudioRate	= m_controller -> GetAudioSampleRate (0);
	cwError		= false;

	cwToneBuffer. resize (cwAudioRate);
	for (int i = 0; i < cwAudioRate; i++) {
	   float term = (float)i / cwAudioRate * 2 * M_PI;
	   cwToneBuffer [i] = std::complex<float> (cos (term),
	                                           sin (term));
	} 
	cwTonePhase	= 0;
	m_form. cw_audioRate (cwAudioRate);
	audioFilter	= new upFilter (25, WORKING_RATE, cwAudioRate);
	m_worker        = new std::thread (&SDRunoPlugin_cw::WorkerFunction,
	                                                               this);
}

	SDRunoPlugin_cw::~SDRunoPlugin_cw () {	
	running. store (false);
	m_worker -> join ();
//	m_controller	-> UnregisterStreamProcessor (0, this);
	m_controller    -> UnregisterAudioProcessor (0, this);

	delete m_worker;
	m_worker = nullptr;
	delete cw_BandPassFilter;
	delete cw_finalFilter;
	delete audioFilter;
}
//
//	The assumption is that the inputstream has a rate
//	of 2000000 / 32
void    SDRunoPlugin_cw::StreamProcessorProcess (channel_t channel,
	                                         Complex *buffer,
	                                         int length,
	                                         bool& modified) { 
	(void)channel; (void)buffer; (void)length;
	modified = false;
}

void    SDRunoPlugin_cw::AudioProcessorProcess (channel_t channel,
	                                        float* buffer,
	                                        int length,
	                                        bool& modified) {
//	/      Handling IQ input, note that SDRuno interchanges I and Q elements
        if (!modified) {
           for (int i = 0; i < length; i++) {
              std::complex<float> sample =
                           std::complex<float>(buffer [2 * i +  1],
                                               buffer [2 * i]);
              sample = passbandFilter.Pass (sample);
//      since Offset == 0, we do not need a shift here
//            sample = theMixer.do_shift (sample, theOffset);
              if (theDecimator.Pass (sample, &sample))
                 cwBuffer.putDataIntoBuffer (&sample, 1);
           }
        }

	if (cwAudioBuffer. GetRingBufferReadAvailable () >= length * 2) {
	   cwAudioBuffer. getDataFromBuffer (buffer, length * 2);
	}
	modified = true;
}

void	SDRunoPlugin_cw::HandleEvent (const UnoEvent& ev) {
	switch (ev. GetType ()) {
	   case UnoEvent::FrequencyChanged:
	      break;

	   case UnoEvent::CenterFrequencyChanged:
	      break;

//	always use VRX0 for hardware related parameters
	   default:
	      m_form. HandleEvent (ev);	
	      break;
	}
}

static inline
float   clamp (float X, float Min, float Max) {
	if (X > Max)
	   return Max;
	if (X < Min)
	   return Min;
	return X;
}

#define	BUFFER_SIZE	4096
void	SDRunoPlugin_cw::WorkerFunction () {
std::complex<float> buffer [BUFFER_SIZE];
int	cycleCount	= 0;

	running. store (true);
	while (running. load ()) {
	   while (running. load () &&
	              (cwBuffer. GetRingBufferReadAvailable () <  BUFFER_SIZE))
	      Sleep (1);
	   if (!running. load ())
	      break;
	   cwBuffer. getDataFromBuffer (buffer, BUFFER_SIZE);
	   for (int i = 0; i < BUFFER_SIZE; i ++)
	      processSample (buffer [i]);
	   cycleCount += BUFFER_SIZE;
	   if (cycleCount > IN_RATE) {
	      cycleCount		= 0;
	      m_form. cw_audioRate	(cwAudioRate);
	      cw_showdotLength		(cwDotLength);
	      cw_showspaceLength	(cwSpaceLength);
	      cw_showspeed		(cwSpeed);
	      cw_showagcpeak		(clamp (agc_peak * 1000.0, 0.0, 100.0));
	      cw_shownoiseLevel		(clamp (noiseLevel * 1000.0, 0.0, 100.0));
	   }
	}
	m_form.set_cwText ("end of worker function");
	Sleep(1000);
}
static inline
std::complex<float> cmul(std::complex<float> v, float p) {
	return std::complex<float>(real(v) * p, imag(v) * p);
}
//
static int fftTeller = 0;
void	SDRunoPlugin_cw::processSample (std::complex<float> z) {
std::complex<float>	ret;
int32_t	lengthOfTone;
int32_t	lengthOfSilence;
int32_t	t;
char	buffer [4];
std::complex<float> res;
static
float	value = 0;
std::complex<float> outV [2000];
std::vector<std::complex<float>> tone (cwAudioRate / WORKING_RATE);

//	cw_BandPassFilter is supposed to be "wide"
	std::complex<float> s	= cw_BandPassFilter	-> Pass (z);
	audioFilter -> Filter (cmul (s, 20), tone. data ());
	for (int i = 0; i < tone. size (); i ++) {
	   tone [i] *= cwToneBuffer [cwTonePhase];
	   cwTonePhase = (cwTonePhase + 801) % cwAudioRate;
	}
	cwAudioBuffer.putDataIntoBuffer (tone. data (), tone. size () * 2);

	if (value >= agc_peak / 2) {
	   newFFT. do_FFT (z, outV);
	   fftTeller ++;
	   if (fftTeller > CW_RATE / 2) {
	      int offs = offset (outV);
	      if ((offs != NO_OFFSET_FOUND) && (abs (offs) >= 3)) {
	         if ((- searchRange / 2 < cw_IF + offs) &&
	             (cw_IF + offs / 2 < searchRange)) {
                    cw_IF += offs / 2;
	            cw_finalFilter -> update (cw_IF - 50, cw_IF + 50);
                    updateFrequency (offs / 2);
                  
	         }
              }
		  m_form.cw_showIF(cw_IF);
	      fftTeller = 0;
	      newFFT. reset ();
	   }
	}
	      
	s	= cw_finalFilter -> Pass (s);
	value = abs(s);
	if (value > agc_peak)
	   agc_peak = decayingAverage (agc_peak, value, 50.0);
	else
	   agc_peak = decayingAverage (agc_peak, value, 500.0);

	currentTime += USECS_PER_SEC / WORKING_RATE;
	switch (cwState) {
	   case MODE_IDLE:
	      if ((value > 0.67 * agc_peak) &&
	          (value > SquelchValue. load () * noiseLevel)) { 
/*
 *	we seem to start a new tone
 */
	         cwState		= MODE_IN_TONE;
	         currentTime		= 0;
	         cwCurrent		= 0;
	         cwStartTimestamp	= currentTime;
	         cwPreviousState	= cwState;
	      }
	      else 
	         noiseLevel		= decayingAverage (noiseLevel,
	                                                   value, 500.0);
	      break;				/* just wait	*/

	   case MODE_IN_TONE:
/*
 *	we are/were receiving a tone, either continue
 *	or stop it, depending on some threshold value.
 */
	      if (value > 0.33 * agc_peak)
	         break;			/* just go on	*/
/*
 *	if we are here, the tone has ended
 */
	      cwEndTimestamp	= currentTime;
	      lengthOfTone	= currentTime - cwStartTimestamp;

	      if (lengthOfTone < cwNoiseSpike) {
	         cwState = cwPreviousState;
	         break;
	      }

	      noiseLevel	= decayingAverage (noiseLevel, value, 500.0);

	      if (lengthOfTone <= cw_adaptive_threshold)
	         dataBuffer [cwCurrent ++] = CW_DOT_REPRESENTATION;
	      else
	         dataBuffer [cwCurrent ++] = CW_DASH_REPRESENTATION;

/*
 *	if we gathered too many dots and dashes, we end up
 *	with garbage.
 */
	      if (cwCurrent >= CW_RECEIVE_CAPACITY) {
	         cwCurrent = 0;
	         cwState = MODE_IDLE;
	         break;
	      }

	      dataBuffer [cwCurrent] = 0;
	      cw_showSymbol (dataBuffer);
	      cwState = MODE_AFTER_TONE;

	      if (cwTracking) {
	         t = newThreshold (lengthofPreviousTone, lengthOfTone);
	         if (t > 0) {
	            cw_adaptive_threshold = thresholdFilter -> filter (t);
	            cwDotLength		= cw_adaptive_threshold / 2;
	            cwDashLength	= 3 * cwDotLength;
	            cwSpeed		= DOT_MAGIC / cwDotLength;
	            cwNoiseSpike	= cwDotLength / 3;
	         }
	      }

	      lengthofPreviousTone = lengthOfTone;
	      break;

	   case MODE_AFTER_TONE:
/*
 *	following the end of the tone, we might go back for the
 *	next dash or dot, or we might decide that we reached
 *	the end of the letter
 */
	      if ((value > 0.67 * agc_peak) &&
	          (value > SquelchValue. load () * noiseLevel)) {
	         int t = currentTime - cwEndTimestamp;
	         cwSpaceLength		= spaceFilter -> filter (t);
	         cwState		= MODE_IN_TONE;
	         cwStartTimestamp	= currentTime;
	         cwPreviousState	= cwState;
	         break;
	      }	
// 	no tone, adjust noiselevel and handle silence
	      noiseLevel	= decayingAverage (noiseLevel, value, 500.0);
	      lengthOfSilence	= currentTime - cwEndTimestamp;

	      if ((lengthOfSilence >= 2 * (cwDotLength + cwSpaceLength) / 2)) {
	         lookupToken (dataBuffer, buffer);
	         cwCurrent = 0;
	         cwState = MODE_END_OF_LETTER;
	         cw_addText (buffer [0]);
	      }
//	otherwise, silence too short, do nothing as yet
	      break;
/*
 * 	end_of_letter may be followed by another letter or an end
 * 	of word indication
 */
	   case MODE_END_OF_LETTER:	/* almost MODE_IDLE	*/
	      if ((value > 0.67 * agc_peak) &&
	          (value > SquelchValue. load () * noiseLevel)) {
	         cwState		= MODE_IN_TONE;
	         cwStartTimestamp	= currentTime;
	         cwPreviousState	= cwState;
	      }
	      else {
/*
 *	still silence, look what to do
 */
	         noiseLevel		= decayingAverage (noiseLevel,
	                                                   value, 500.0);
	         lengthOfSilence = currentTime - cwEndTimestamp;
	         if (lengthOfSilence > 4.0 * (cwSpaceLength + cwDotLength) / 2) {
	            cw_addText (' ');	/* word space	*/
	            cwState = MODE_IDLE;
	         }
	      }
	      break;

	   default:
	      break;
	}
}

const char * const codetable[] = {
	"A._",
	"B_...",
	"C_._.",
	"D_..",
	"E.",
	"F.._.",
	"G__.",
	"H....",
	"I..",
	"J.___",
	"K_._",
	"L._..",
	"M__",
	"N_.",
	"O___",
	"P.__.",
	"Q__._",
	"R._.",
	"S...",
	"T_",
	"U.._",
	"V..._",
	"W.__",
	"X_.._",
	"Y_.__",
	"Z__..",
	"0_____",
	"1.____",
	"2..___",
	"3...__",
	"4...._",
	"5.....",
	"6_....",
	"7__...",
	"8___..",
	"9____.",
	".._._._",
	",__..__",
	"?..__..",
	"~"
	};

void	SDRunoPlugin_cw::lookupToken (char *in, char *out) {
int	i;

	for (i = 0; codetable [i][0] != '~'; i ++) {
	   if (strcmp (&codetable [i][1], in) == 0) {
	      out [0] = codetable [i][0];
	      return;
	   }
	}
	out [0] = '*';
}
/*
 * 	check for dot/dash or dash/dot sequence to adapt the
 * 	threshold.
 */
int32_t	SDRunoPlugin_cw::newThreshold (int32_t prev_element,
	                                     int32_t curr_element) {
	if ((prev_element > 0) && (curr_element > 0)) {
	   if ((curr_element > 2 * prev_element) &&
	       (curr_element < 4 * prev_element))
	      return  getMeanofDotDash (prev_element, curr_element);
/*
 * 	check for dash dot sequence
 */
	   if ((prev_element > 2 * curr_element) &&
	       (prev_element < 4 * curr_element))
	      return getMeanofDotDash (curr_element, prev_element);
	}
	return -1;
}
/*
 * 	if we have a dot/dash sequence, we might adapt
 * 	the adaptive threshold if both dot and dash length
 * 	are reasonable.
 */
int	SDRunoPlugin_cw::getMeanofDotDash (int idot, int idash) {
int	dot, dash;

	if ((idot > cwDotLengthMin) && (idot < cwDotLengthMax))
	   dot = idot;
	else
	   dot = cwDotLength;

	if ((idash > cwDashLengthMin) && (idash < cwDashLengthMax))
	   dash = idash;
	else
	   dash = cwDashLength;

	return (dash + dot) / 2;
}

void	SDRunoPlugin_cw::cw_showSymbol		(char *v) {
	m_form. cw_showSymbol (v);
}

void	SDRunoPlugin_cw::cw_showdotLength	(int v) {
	m_form. cw_showdotLength (v);
}

void	SDRunoPlugin_cw::cw_showspaceLength	(int v) {
	m_form. cw_showspaceLength (v);
}

void	SDRunoPlugin_cw::cw_showspeed		(int v) {
	m_form. cw_showspeed (v);
}

void	SDRunoPlugin_cw::cw_showagcpeak		(int v) {
	m_form. cw_showagcpeak (v);
}

void	SDRunoPlugin_cw::cw_shownoiseLevel	(int v) {
	m_form. cw_shownoiseLevel (v);
}

static	std::string textLine;
void	SDRunoPlugin_cw::cw_clrText			()	  {
	m_form.  set_cwText ("");
	textLine	= "";
}

void	SDRunoPlugin_cw::cw_addText			(char c)  {
	if (c < ' ')
	   c = ' ';
	textLine. append (1, c);
	if (textLine. length () > 60)
	   textLine. erase (0, 1);
	m_form. set_cwText (textLine);
}

void	SDRunoPlugin_cw::cw_setSquelchValue (int s) {
	SquelchValue. store (s);
}

void	SDRunoPlugin_cw::cw_switchTracking () {
	cwTracking = !cwTracking;
	m_form. cw_showTrackingMode (cwTracking ? "tracking on" : "tracking off");
}

void	SDRunoPlugin_cw::cw_setFilterDegree (int n) {
	cwFilterDegree	= n;
	locker. lock ();
	if (cw_BandPassFilter != NULL)
	   delete cw_BandPassFilter;
	cw_BandPassFilter	= new bandpassFilter (2 * cwFilterDegree + 1,
	                                              cw_IF - 30,
	                                              cw_IF + 30,
	                                              WORKING_RATE);
	locker. unlock ();
}

void	SDRunoPlugin_cw::cw_setWordsperMinute (int n) {
	cwDefaultSpeed		= n;
	cwSpeed			= cwDefaultSpeed;	/* init	*/
	cwRange			= cwSpeed / 2;
	lowerBoundWPM		= cwDefaultSpeed - cwRange;
	upperBoundWPM		= cwDefaultSpeed + cwRange;

	cwDefaultDotLength	= DOT_MAGIC / cwDefaultSpeed;
	cwDotLength		= cwDefaultDotLength;
	cw_adaptive_threshold	= 2 * cwDefaultDotLength;

	cwDotLengthMax		= DOT_MAGIC / lowerBoundWPM;
	cwDotLengthMin		= DOT_MAGIC / upperBoundWPM;

	cwDashLength		= 3 * cwDotLength;
	cwDashLengthMax		= 3 * cwDotLengthMax;
	cwDashLengthMin		= 3 * cwDotLengthMin;

	cwNoiseSpike		= cwDotLength / 3;
	cwStartTimestamp	= 0;
	cwEndTimestamp		= 0;

	cwState			= MODE_IDLE;
	cwCurrent		= 0;
	memset			(dataBuffer, 0, sizeof (dataBuffer));
//	thresholdFilter		-> clear (2 * cwDotLength);
	cw_clrText	();
	cw_showSymbol (" ");
}

#define	MAX_SIZE 40
int	SDRunoPlugin_cw::offset (std::complex<float> *v) {
float avg	= 0;
float max	= 0;
float	supermax	= 0;
int	superIndex	= 0;

	for (int i = 0 ; i < searchRange; i ++)
	   avg += abs (v [(CW_RATE - searchRange / 2 + i) % CW_RATE]);
	avg /= searchRange;
	int	index	= CW_RATE - searchRange / 2;
	for (int i = 0; i < MAX_SIZE; i ++)
	   max +=  abs (v [index + i]);

	supermax	= max;
	for (int i = MAX_SIZE; i < searchRange; i ++) {
	   max -=  abs (v [(index + i - MAX_SIZE) % CW_RATE]);
	   max +=  abs (v [(index + i) % CW_RATE]);
	   if (max > supermax) {
	      superIndex = (index + i - MAX_SIZE / 2);
	      supermax = max;
	   }
	}

	if (supermax / MAX_SIZE > 3 * avg)
	   return superIndex - CW_RATE;
	else
	   return NO_OFFSET_FOUND;
}

void	SDRunoPlugin_cw::updateFrequency (int d) {
	if (abs (d) <= 2) 
	   return;
	int currentFrequency =
	                m_controller -> GetVfoFrequency (0);
	m_controller -> SetVfoFrequency (0, currentFrequency + d);
}

void	SDRunoPlugin_cw::set_searchWidth(int w) {
	searchRange = w;
}

void	SDRunoPlugin_cw::trigger_tune	() {
	newFFT. reset ();
//	fftTeller	= 0;
	cw_IF		= 0;
}

