
-----------------------------------------------------------------
Simple CW decoder plugin
-----------------------------------------------------------------

![overview](/cw-example.png?raw=true)

The decoder plugin requires an input rate 2000000 / 32, and an audiorate
of 48000.

Operation is simple:
the input is filtered, shifted in frequency using the difference of 
centerFrequency and the VFO, then decimated to 2.5 KHz.
This can be accomplished by simple decimating, after all, the filtering
was done already.

A final decimation step is made 2 2000 Hz samplerate, just by interpolation
with 25 samples in being converted to 20 samples out.

The actual decoding starts then:

	first, the sample passes through a smallbanc filter,
	it then is modulated with a 801 Hz tone. That tone
	is made audible and is used to help tuning.

	after that, we only need the amplitude of the sample.
	To decode, we just count samples! Based on the average
	amplitude of the signal we distinuish three states

	we are in mode IDLE until we receive data with an amplitude
	that is high enough, we then switch to mode MODE_IN_TONE.
	As long as we receive amplitudes with a high amplitude we stay
	in this mode.
	The end of the mode is reached as soon as samples with a low amplitude
	are arriving or, if we get more high amplitude samples that a dash
	can reasonably have

	In the "AFTER_TONE" mode we received a dash or a dot and essentially
	wait until we receive an end of letter space.

	One of the issues is that whenever the dotlength is not right,
	we seem to receive only dots. 
	A count is there to limit the number of dashes and dots that 
	might constitute a letter.

	If everything looks reasonable, we try to map the dots/dashes
	onto a letter or digit.



