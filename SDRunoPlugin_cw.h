#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <iunoplugincontroller.h>
#include <iunoplugin.h>
#include <iunostreamobserver.h>
#include <iunoaudioobserver.h>
#include <iunoaudioprocessor.h>
#include <iunostreamobserver.h>
#include <iunoannotator.h>
#include "SDRunoPlugin_cwUi.h"

//
//	for the payload we have
#include	"ringbuffer.h"
class   cwAverage;
class	upFilter;

#include	"utilities.h"
#include	"cw-bandfilter.h"
#include	"decimator-25.h"
#include	"cw-shifter.h"

#define CW_RECEIVE_CAPACITY     030
#define WORKING_RATE    2000

class SDRunoPlugin_cw : public IUnoPlugin,	
	                       public IUnoStreamProcessor,
	                       public IUnoAudioProcessor {
public:
	
		SDRunoPlugin_cw		(IUnoPluginController& controller);
	virtual ~SDRunoPlugin_cw	();

	virtual const
	char*	GetPluginName() const override { return "cw decoder"; }

	virtual
	void	StreamProcessorProcess (channel_t channel,
	                                Complex *buffer, int length,
	                                    bool& modified) override;
	virtual
        void    AudioProcessorProcess (channel_t channel,
                                       float *buffer,
                                       int length, bool& modified) override;

	// IUnoPlugin
	virtual
	void HandleEvent	(const UnoEvent& ev) override;

//     passing on upwards
	void    cw_setFilterDegree      (int);
	void    cw_setSquelchValue      (int);
	void    cw_setWordsperMinute    (int);
	void	cw_switchTracking	();
private:
	std::atomic<bool>	running;
	IUnoPluginController *m_controller;
	void			WorkerFunction ();
	std::mutex		m_lock;
	std::mutex		locker;
	SDRunoPlugin_cwUi	m_form;
	std::thread*		m_worker;
	RingBuffer<Complex>	cwBuffer;
	RingBuffer<float>	cwAudioBuffer;
	bandpassFilter		passbandFilter;
	cwShifter		theMixer;
	decimator_25		theDecimator;
	std::vector<std::complex<float>> cwToneBuffer;
//	former signals, now handled locally
	void			cw_showSymbol		(char *);
	void			cw_showdotLength	(int);
	void			cw_showspaceLength	(int);
	void			cw_showspeed		(int);
	void			cw_showagcpeak		(int);
	void			cw_shownoiseLevel	(int);
	void			cw_clrText		();
	void			cw_addText		(char);

	int			resample		(std::complex<float>,
	                                                 std::complex<float> *);
	int			centerFrequency;
	int			selectedFrequency;
	int			cwTonePhase;
	std::vector<std::complex<float>> convBuffer;
	int			convIndex;
	int16_t			mapTable_int   [WORKING_RATE / 100];
	float			mapTable_float [WORKING_RATE / 100];

	void			processSample		(std::complex<float>);
	void			process			(std::complex<float>);
	int			newThreshold		(int, int);
	int			getMeanofDotDash	(int, int);
	void			lookupToken		(char *, char *);

	int32_t			rawRate;	
	bool			cwError;
	bandpassFilter	        *cw_BandPassFilter;
	upFilter	*audioFilter;
	int16_t		cwFilterDegree;
	cwAverage	*SmoothenSamples;
	cwAverage	*thresholdFilter;
	cwAverage	*spaceFilter;

	float		agc_peak;
	float		noiseLevel;
//	float		value;
//	float		metric;
	double		cw_IF;
	int32_t		cwDotLength;
	int32_t		cwSpaceLength;
	int32_t		cwDashLength;
	std::atomic<int>	SquelchValue;
	int16_t		cwState;
	int16_t		cwPreviousState;
	int32_t		cw_adaptive_threshold;
	int32_t		currentTime;
	int32_t		cwCurrent;
	int16_t		cwSpeed;
	int16_t		cwDefaultSpeed;
	char		dataBuffer [CW_RECEIVE_CAPACITY];
	int32_t		cwRange;
	int32_t		cwDefaultDotLength;
	int32_t		lowerBoundWPM;
	int32_t		upperBoundWPM;
	int32_t		cwDotLengthMin;
	int32_t		cwDotLengthMax;
	int32_t		cwDashLengthMin;
	int32_t		cwDashLengthMax;
	int32_t		lengthofPreviousTone;
	int32_t		cwNoiseSpike;
	int32_t		cwStartTimestamp;
	int32_t		cwEndTimestamp;
	bool		cwTracking;
	int		cwAudioRate;
};
