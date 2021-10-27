
-----------------------------------------------------------------
Simple CW decoder plugin
-----------------------------------------------------------------

The "cw" plugin for SDRuno is - as the name suggests - a plugin for
decoding CW (continuous waves) signals.

![overview](/cw-example.png?raw=true)

-----------------------------------------------------------------------------
  READ THIS FIRST installing the plugin
-----------------------------------------------------------------------------

Since cw is a small band signal (a few dozens of Hz in this plugin),
the samplerate used as input for the plugin is *62500* samples/second.

**On the main widget select samplerate 2000000, and decimation factor 32**.

![overview](/drm-main-widget.png?raw=true)

The plugin itself can be stored in the folder for community plugins

The plugin is - as other plugins - developed under MSVC. Its functioning
depends on lots of other "dll's" (Dynamic Load Libraries);

If a 0x000012f error is encountered on trying to load the plugin,
it means that dll's from the Visual C++ Redistributable(VS 2015) are
not found.

--------------------------------------------------------------------------
Using the plugin
-------------------------------------------------------------------------

CW (continuous Wave) is probably the oldest communication form on radio.
This plugin implements a - pretty straightforward - decoder.

The spectrumdisplay shows a width of 62.5 KHz, CW signals are easily
recognizable. Tuning to a CW signal is howver not always easy:
the transmissions in the amateur bands are usually short.
Happily, the main spectrum display on SDRuno has an excellent zoom
facility, furthermore, there is an auxiliary (small) spectrum window,
on which further zooming in is possible.

A key issue in decoding is the WPM (words per minute): they decide
what the length of a dot, a space and a dash are.
The default setting in the plugin is 30 WPM, the catch range is then roughly 
20 .. 40.

The current implementation has an additional button that - when enabled -
is able to shift the signal frequency a few Hz towards the nearest strong
signal.

The implementation of the algorithm is taken from the swradio-8 
implementation.



