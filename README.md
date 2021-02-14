
-----------------------------------------------------------------
Simple CW decoder plugin
-----------------------------------------------------------------

![overview](/cw-example.png?raw=true)

CW (continuous Wave) is probably the oldest communication form on radio.
This plugin implements a - pretty straightforward - decoder.

The decoder plugin assumes an input rate 2000000 / 32, and an audiorate
of 48000.

The spectrumdisplay shows a width of 62.5 KHz, CW signals are easily
recognizable. Tuning to a CW signal is not always easy: the transmissions
in the amateur bands are usually short.

A key issue in decoding is the WPM (words per minute): they decide
what the length of a dot, a space and a dash are.
The default setting in the plugin is 30 WPM, the catch range is then roughly 
20 .. 40.

The implementation of the algorithm is taken from the swradio-8 
implementation.



