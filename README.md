# Monster Lab
### We sequence monsters.

In 2017, we built a Lego DNA sequencer to teach kids about DNA, Sequencing and Phenotypes. 
[Our imgur album](http://imgur.com/gallery/4O8r4) gives a reasonable construction tutorial. For more information on the **Monster Lab** activity itself, you can read [this blog post](https://samnicholls.net/2017/03/15/lego-sequencer/).

### Can I use this for public engagement?
Yes! Our activity is open-source and we'd love you to use it. This repository contains (almost) everything you would need to construct your own Lego sequencer and run our Monster Lab activity.  **Monster Lab** was hard work that we aren't paid for, so we ask that you kindly attempt to credit us as the original authors of the idea where you can; such as a link to us in a blog post of your event, or some shout-outs to `@samstudio8` and `@monsterdnalab` during your event on Twitter.

* `laser\`: Contains DXF formatted files for the laser cut parts, including the base plate, Lego tray and gear to attach to the stepper. The 3mm and 6mm refers to the thickness of the acrylic used to cut those parts for our model.
* `wiring\`: Contains a Fritzing diagram for wiring the Arduino Uno, EasyDriver and RGB Colour Sensor. Note that we had to flip the inner pair of wires from our stepper motor but this may differ for yours. [This hook-up guide](https://learn.sparkfun.com/tutorials/easy-driver-hook-up-guide) gives more information on checking the coils of your stepper and how to wire it up.
* `software\`: The current version of the `ino` to compile and upload to the Uno.
* `paperwork\`: Contains the monster record sheet and gene list for our iteration of the activity, you can change these as you wish.

## Housekeeping
### Licenses

* Monster Lab software (`software\`) is made available under the MIT License.
* Our designs for use with a laser cutter or 3D printer are licensed under a Creative Commons Attribution 4.0 International License.
* Blank forms (`paperwork\`) are licensed under a Creative Commons Attribution 4.0 International License.
