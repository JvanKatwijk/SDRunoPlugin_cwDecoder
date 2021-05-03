#include <sstream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <unoevent.h>

#include "SDRunoPlugin_cw.h"
#include "SDRunoPlugin_cwUi.h"
#include "SDRunoPlugin_cwForm.h"

// Ui constructor - load the Ui control into a thread
SDRunoPlugin_cwUi::
              SDRunoPlugin_cwUi (SDRunoPlugin_cw& parent,
	                               IUnoPluginController& controller) : 
	m_parent(parent),
	m_form(nullptr),
	m_controller(controller) {
	m_thread = std::thread(&SDRunoPlugin_cwUi::ShowUi, this);
}

// Ui destructor (the nana::API::exit_all();) is required if using Nana UI library
	SDRunoPlugin_cwUi::~SDRunoPlugin_cwUi () {	
	nana::API::exit_all();
	m_thread.join();	
}

// Show and execute the form
void	SDRunoPlugin_cwUi::ShowUi () {	
	m_lock.lock();
	m_form = std::make_shared<SDRunoPlugin_cwForm>(*this, m_controller);
	m_lock.unlock();
	m_form->Run();
}

// Load X from the ini file (if exists)
// TODO: Change Template to plugin name
int	SDRunoPlugin_cwUi::LoadX () {
	std::string tmp;
	m_controller.GetConfigurationKey ("cw.X", tmp);
	if (tmp.empty ()) {
	   return -1;
	}
	return stoi (tmp);
}

// Load Y from the ini file (if exists)
// TODO: Change Template to plugin name
int	SDRunoPlugin_cwUi::LoadY () {
	std::string tmp;
	m_controller.GetConfigurationKey ("cw.Y", tmp);
	if (tmp.empty()) {
	   return -1;
	}
	return stoi(tmp);
}

// Handle events from SDRuno
// TODO: code what to do when receiving relevant events
void SDRunoPlugin_cwUi::HandleEvent(const UnoEvent& ev) {
	switch (ev. GetType ()) {
	   case UnoEvent::StreamingStarted:
	   break;

	case UnoEvent::StreamingStopped:
	   break;

	case UnoEvent::SavingWorkspace:
	   break;

	case UnoEvent::ClosingDown:
	   FormClosed();
	   break;

	default:
	   break;
	}
}

// Required to make sure the plugin is correctly unloaded when closed
void	SDRunoPlugin_cwUi::FormClosed () {
	m_controller.RequestUnload(&m_parent);
}

//
//      passing on upwards
void	SDRunoPlugin_cwUi::cw_setFilterDegree	(int d) {
	m_parent. cw_setFilterDegree (d);
}
//
void	SDRunoPlugin_cwUi::cw_setSquelchValue	(int v) {
	m_parent. cw_setSquelchValue (v);
}
//
void	SDRunoPlugin_cwUi::cw_setWordsperMinute	(int w) {
	m_parent. cw_setWordsperMinute (w);
}

int	SDRunoPlugin_cwUi::cw_getFilterDegree	() {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   return m_form -> cw_getFilterDegree ();
	else
	   return -1;
}
	
int	SDRunoPlugin_cwUi::cw_getSquelchValue	() {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   return m_form -> cw_getSquelchValue ();
	else
	   return -1;
}

int	SDRunoPlugin_cwUi::cw_getWordsperMinute	() {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   return m_form -> cw_getWordsperMinute ();
	else
	   return -1;
}

void	SDRunoPlugin_cwUi::cw_switchTracking	() {
	m_parent. cw_switchTracking ();
}

void	SDRunoPlugin_cwUi::set_searchWidth	(int w) {
	m_parent. set_searchWidth (w);
}

void	SDRunoPlugin_cwUi::trigger_tune		() {
	m_parent. trigger_tune ();
}

//
void	SDRunoPlugin_cwUi::cw_showTrackingMode	(const std::string &s) {
	std::lock_guard<std::mutex> l (m_lock);
        if (m_form != nullptr)
           return m_form -> cw_showTrackingMode (s);
}

//
//      passing on downwards
void	SDRunoPlugin_cwUi::cw_showSymbol	(char *v) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> cw_showSymbol (v);
}
//
void	SDRunoPlugin_cwUi::cw_showdotLength	(int v) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> cw_showdotLength (v);
}
//
void	SDRunoPlugin_cwUi::cw_showspaceLength	(int v) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> cw_showspaceLength (v);
}
//
void	SDRunoPlugin_cwUi::cw_showagcpeak	(int p) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> cw_showagcpeak (p);
}
//
void	SDRunoPlugin_cwUi::cw_shownoiseLevel	(int n) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> cw_shownoiseLevel (n);
}
//
void	SDRunoPlugin_cwUi::cw_audioRate		(int n) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> cw_audioRate (n);
}

//
void	SDRunoPlugin_cwUi::cw_showspeed		(int n) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> cw_showspeed (n);
}
//
void	SDRunoPlugin_cwUi::set_cwText	(const std::string &s) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> set_cwText (s);
}
//

