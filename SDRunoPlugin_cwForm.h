#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/widgets/spinbox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/dragger.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>

// Shouldn't need to change these
#define topBarHeight (27)
#define bottomBarHeight (8)
#define sideBorderWidth (8)

// TODO: Change these numbers to the height and width of your form
//#define formWidth (297)
#define formWidth (500)
#define formHeight (240)

class SDRunoPlugin_cwUi;

class SDRunoPlugin_cwForm : public nana::form {
public:

	SDRunoPlugin_cwForm (SDRunoPlugin_cwUi& parent,
	                     IUnoPluginController& controller);		
	~SDRunoPlugin_cwForm();
void    cw_setFilterDegree      (int);
void    cw_setSquelchValue      (int);
void    cw_setWordsperMinute    (int);
int	cw_getFilterDegree	();
int	cw_getSquelchValue	();
int	cw_getWordsperMinute    ();
void	cw_switchTracking	();
void	cw_showTrackingMode	(const std::string &);
//
void    cw_showSymbol		(char *);
void    cw_showdotLength        (int);
void    cw_showspaceLength      (int);
void    cw_showagcpeak          (int);
void    cw_shownoiseLevel       (int);
void    cw_showspeed            (int);
void	cw_audioRate		(int);
void    set_cwText              (const std::string &);
void	set_searchWidth		(int);
void	trigger_tune		(void);


	void Run();
	
private:

	void Setup();

	// The following is to set up the panel graphic to look like a standard SDRuno panel
	nana::picture bg_border{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::picture bg_inner{ bg_border, nana::rectangle(sideBorderWidth, topBarHeight, formWidth - (2 * sideBorderWidth), formHeight - topBarHeight - bottomBarHeight) };
	nana::picture header_bar{ *this, true };
	nana::label title_bar_label{ *this, true };
	nana::dragger form_dragger;
	nana::label form_drag_label{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::paint::image img_min_normal;
	nana::paint::image img_min_down;
	nana::paint::image img_close_normal;
	nana::paint::image img_close_down;
	nana::paint::image img_header;
	nana::picture close_button{ *this, nana::rectangle(0, 0, 20, 15) };
	nana::picture min_button{ *this, nana::rectangle(0, 0, 20, 15) };

	// Uncomment the following 4 lines if you want a SETT button and panel
	nana::paint::image img_sett_normal;
	nana::paint::image img_sett_down;
	nana::picture sett_button{ *this, nana::rectangle(0, 0, 40, 15) };
	void SettingsButton_Click();

	// TODO: Now add your UI controls here

//	first the test line
	nana::label  cwText		{*this, nana::rectangle ( 30,  40, 400, 30) };
	nana::label  cwSymbol		{*this, nana::rectangle ( 30,  80,  40, 20) };
	nana::label  cwLetter		{*this, nana::rectangle ( 80,  80, 100, 20) };
	nana::label  dotLengthdisplay 	{*this, nana::rectangle ( 30, 110,  40, 20) };
	nana::label  spaceLengthdisplay {*this, nana::rectangle ( 80, 110,  40, 20) };
	nana::label  agcPeakdisplay	{*this, nana::rectangle (130, 110,  60, 20) };
	nana::label  noiseLeveldisplay 	{*this, nana::rectangle (200, 110,  60, 20) };
	nana::label  rateDisplay	{*this, nana::rectangle (270, 110,  40, 20) };
	nana::label  actualWPM		{*this, nana::rectangle (340, 110,  40, 20) };
	nana::spinbox WPM		{*this, nana::rectangle ( 30, 170,  80, 20) };
	nana::button trackingButton     {*this, nana::rectangle (200, 170,  80, 20) };
	nana::spinbox filterDegree	{*this, nana::rectangle ( 30, 200, 100, 20) };
	nana::spinbox squelchLevel 	{*this, nana::rectangle (150, 200, 100, 20) };

	nana::spinbox searchWidth	{*this, nana::rectangle (260, 200, 100, 20) };
	nana::button tuneIn		{*this, nana::rectangle (380, 200, 40, 20)};
	SDRunoPlugin_cwUi & m_parent;
	IUnoPluginController & m_controller;
};
