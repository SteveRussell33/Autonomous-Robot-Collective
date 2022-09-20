Here are some ideas for things you can do with TRACK-4:

1) Create a plain "normal" mix, ignoring the various green SEND outputs on the
right hand side.  Adjust the various Level and Pan knobs to taste.  The final
mix will come out of the blue mono outputs on the Mix Track.

2) Use the polyphonic Level and Pan CV inputs of Track 1 thru Track 4 to modify
the channels on each Track individually, before they are sent to the Mix Track.

3) Ignore the Mix Track and Send Mix, and use Track 1 thru Track 4 as four
independent Level and Pan controls. Use Send 1 thru Send 4 as four independent
per-Track outputs.

4) Observe that the Left and Right stereo inputs feeding into the Mix Track will
always have exactly 4 channels, corresponding to each of the four Tracks.  Use
the polyphonic Level and Pan CV inputs of the Mix Track to modify the per-Track
channels individually, before they are summed and sent to the blue mono outputs.

5) Set up a mix on Track 1 thru Track 4.  Ignore the Mix Track, and use Send Mix
to do arbitrary processing and routing on its four per-Track stereo channels.

6) Do some loop-back routing via Send Mix. Here's an [example patch](???):

(a) Use Tracks 1 and 2 to set the Level and Pan of some incoming audio.  

(b) Use Send Mix to send Tracks 1 and 2 for some extra processing.  Let's add
some reverb, by summing the two tracks together and sending that to an instance
of a reverb module.  Turn the Dry/Wet knob on the reverb module all the
way to Wet.

(c) Take the outputs from the reverb module and plug them into the inputs of
Track 3.

(d) Adjust the Level on Track 3 to control how much reverb goes into your final
mix.

NOTE: Its important in step (b) above that when we process the output from Send
Mix, we DON'T include Track 3 in any extra processing that we do with Tracks 1
and 2.  If we include Track 3, we've created a feedback loop that will overwhelm
your patch.  The output from the Mix Track will be hard-clipped at [-10V,10V] to
protect your speakers and your ears, but obviously we should take care to avoid
this scenario.

7) Do some fancy DAW-style bussing and auxiliary routing. Here's an [example patch](???):

(a) In example (6) above , Track 1 and Track 2 both end up with the same amout
of reverb.  Lets modify our patch to make it a bit more sophisticated, so that
Track 1 has more reverb than Track 2.

(b) It would be annoying to have to set up a whole entire sub-mix and/or
sequence of modules just to modify the individual per-track channels coming out
of Send Mix before we give them to the reverb module.  Let's use BUS-8 instead,
since it was designed to suit this very purpose. 


