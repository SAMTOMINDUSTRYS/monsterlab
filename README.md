# A Lego DNA Sequencer
**Legogen Sequenceer 9001**

We built a Lego DNA sequencer to teach kids about DNA, Sequencing and Phenotypes. This repository contains (almost) everything you would need to construct your own Lego sequencer and run our activity.

* `laser\`: Contains DXF formatted files for the laser cut parts, including the base plate, Lego tray and gear to attach to the stepper. The 3mm and 6mm refers to the thickness of the acrylic used to cut those parts for our model.
* `wiring\`: Contains a Fritzing diagram for wiring the Arduino Uno, EasyDriver and RGB Colour Sensor. Note that we had to flip the inner pair of wires from our stepper motor but this may differ for yours. [This hook-up guide](https://learn.sparkfun.com/tutorials/easy-driver-hook-up-guide) gives more information on checking the coils of your stepper and how to wire it up.
* `software\`: The current version of the `ino` to compile and upload to the Uno.
* `paperwork\`: Contains the monster record sheet and gene list for our iteration of the activity, you can change these as you wish.

For more information on the **Monster Lab** activity itself, you can read [this blog post](https://samnicholls.net/2017/03/15/lego-sequencer/).
