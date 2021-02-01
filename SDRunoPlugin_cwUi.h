#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <iunoplugin.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>
#include "SDRunoPlugin_cwForm.h"

// Forward reference
class SDRunoPlugin_cw;

class SDRunoPlugin_cwUi {
public:

	SDRunoPlugin_cwUi		(SDRunoPlugin_cw& parent,
	                                 IUnoPluginController& controller);
	~SDRunoPlugin_cwUi		();

	void HandleEvent		(const UnoEvent& evt);
	void FormClosed			();

	void ShowUi			();

	int LoadX			();
	int LoadY			();

//	passing on upwards
	void    cw_setFilterDegree      (int);
	void    cw_setSquelchValue      (int);
	void    cw_setWordsperMinute    (int);
	int     cw_getFilterDegree      ();
	int     cw_getSquelchValue      ();
	int     cw_getWordsperMinute    ();
	void	cw_switchTracking	();

//
//	passing on downwards
	void	cw_showTrackingMode	(const std::string &);
	void	cw_showSymbol		(char *);
	void    cw_showdotLength        (int);
	void    cw_showspaceLength      (int);
	void    cw_showagcpeak          (int);
	void    cw_shownoiseLevel       (int);
	void	cw_audioRate		(int);
	void    cw_showspeed            (int);
	void	set_cwText		(const std::string &);
private:
	
	SDRunoPlugin_cw & m_parent;
	std::thread m_thread;
	std::shared_ptr<SDRunoPlugin_cwForm> m_form;

	bool m_started;
	std::mutex m_lock;
	IUnoPluginController & m_controller;
};
