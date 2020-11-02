# A Plugout for Octoprint
<img align="left" style="padding-right:10px;" src="https://octoprint.org/assets/img/logo.png">
Contrary to most contributions to Octoprint, this software is not a Plugin, but does talk to Octoprint through its API. It works outside of the (python) framework of [OctoPrint](https://octoprint.org/). Octoprint needs no further introduction: the well known the 3D printer web server designed and maintained by the brilliant Gina Häußge, aka @foosel.

## I called it.... "OctoPlugout"
Because it is not a plugin, but still communicates like plugins -outside of the framework-, I called it a "plugout". So... OctoPlugout was born.

In essense this "Arduino" software "lives" in the ESP8266 inside the wall plug. Here is where you plugin the powercord of your Printer.
<img align="left" style="padding-right:30px;" src="https://github.com/ruedli/OctoPlugout/blob/master/images/SONOF%20reflashed%20working.jpg">

## What does it do?
It uses this Sonoff plug, that you flash with the sketch provided here. Now the plug itself communicates wirelessly with the Octoprint server running on the Raspberry Pi.

**After a print** it will:
- Wait for the extruder to cool down
- Shutdown the Pi
- Wait for the Pi to finish shutting down
- Power off, both **printer** and **Pi**

## What is in it for you?

OctoPlugout delivers a "one touch" safe shutdown button for both your Pi and printer! No additional wiring to the Pi, just the printer's powercord!

The beauty of the solution, is that unlike Tasmota or Sonoff, there is no separate software or framework needed. Neither locally nor in the cloud. It is sufficient to directly communicate (wirelessly) between the plug and octoprint. Only the powercord is plugged in, so that the plug can control the power to your printer.

## What does it need?

For hardware you just need this programmable relay card. A Sonoff is most convenient, as its housing is designed for this. I used a S26, so there is no 220V rewiring necessary at all! Just plug in your printer, no further connection between plug and Pi. Your worries like which GPIO to use are gone!

A Sonoff can switch using its relay, has an indicator LED and a button. All three are used! The button is used to power on/off the printer manually and the LED indicates the state that the printer is in.

The software in the plug considers what to do if Wifi is lost, when your printer goes in "pause" mode and when it is a good moment to shutdown. It even initiates this shutdown of your Pi for you.

## Software

I used the Arduino IDE to compile everything. You need:

- The ESP8266 library 
- A library to communicate with the Octoprint API, called OctoprintAPI. You find it here: https://github.com/chunkysteveo/OctoPrintAPI 
- Prerequisits for OTA upgrades (over the air upgrade). Ensure you can compile to BasicOTA.ino sketch and see the listed IP address, which you need to update octoPlugout through the IP port. You see how to setup OTA here: https://randomnerdtutorials.com/esp8266-ota-updates-with-arduino-ide-over-the-air/

With OctoprintAPI you also find other preprequisites for installation and making the sketch work for your environment, follow these recommendations.

*hint*
*Currently (1-11-2020) you could also install the OctoPrintAPI through the Arduino IDE, but.... This one is not the latest version. If you get an error "‘class OctoprintAPI’ has no member named ‘octoPrintCoreShutdown" your version is not up-to-date, install the library OctoPrintAPI directly from Stephen's github, as in the url above.*


The "OctoPlugout" sketch has ALL its configuration parameters in one place: the file OctoPlugout.config.h. Copy it from OctoPlugout.config.h.RELEASED and adapt it to your Octoprint and WiFi. Adapt at least these parameters indicated in yellow, this depends on the IP address of Octoprint and Octiprints so-called API-key (accessible through the settings of Octoprint).

### OctoPlugout.config.h

![The critical configuration](https://github.com/ruedli/OctoPlugout/blob/master/images/config.jpg)  

Also the OctoprintAPI describes all these parameters in further details. The "green" ones should stay like they are. Consider the other parameters: you can adapt it to other Sonoff plugs (in case it uses different pins), as well as configure the 8 "blinking" patterns for the states the plug is in, as well as some timing parameters. The parameter file describes them.

If you do not change the OTA password in the config file, when you "flash" over the air, the password is "1234".
Note that this password only is in effect the NEXT time you flash OTA. If you forgotten the passowrd and lost the config file, you need to flash using the serial port, then no passowrd is required and you can (re)set the OTA password.

## States

The states look like this:

![All the states](https://github.com/ruedli/OctoPlugout/blob/master/images/All%20states.jpg) 

In this state diagram the action that trigger state changes are:
- SP: a short press of the button
- LP: a long press on the button
- WiFi-: Wifi goes down
- Pi-: Octoprint goes down
- Pi+: Octoprint is up, and the plug is connected.
- print+: A print is running
- print-: No print is running
- temp-: The extruder is cooled enough to power off the printer
- temp+: The extruder is heated up enough to trigger shutdown procedure after print
- timeout: a sufficiently long time between "shutdown" of the Pi and poweringoff the Pi.

## Flashing a Sonoff

I 3D-printed a convenient plug to flash the S26. You can find it here: https://www.thingiverse.com/thing:3193583

Here you see the first flashing using its serial port. Later flashes can be done "over the air".
![Flashing a sonoff S26](https://github.com/ruedli/OctoPlugout/blob/master/images/flash%20a%20S26.jpg)

Please "save" your original Sonoff software first before you flash OctoPlugout over it. Instructions are here: https://hobbytronics.com.pk/sonoff-original-firmware-backup-restore/#Step-by-Step-Procedure 

# Using it...

In practice it is easier to operate how it looks in written form.

In short: The red LED lights whenever the relay is on (so your printer/raspberry is powered). The green LED indicates interaction with Octoprint.

No green LED: The plug stays on "forever" until you press something (see below).

*Slooooooow blinking green* LED. It tries to find your printer.. When is is found: *Normal blinking green* LED.

### Now there are three possibilities:
- *Short press*: you go into "disconnected power on mode again", your printer will stasy on forever...
- *Long press*: As soon as no print is running AND the extruder is cool, it will shutdown the Pi. The LED will blink very rapidly... It will look whether Octoprint "comes back" (LEDs not blinking) If it does (very rarely!!!), it goes in "switched on forever mode". If not (the normal case) after 30 seconds, the power is removed.
- A print is *started... finishes.. the extruder cools down*: then the shutdown procedure is also triggered.

### When in "switched on forever" (Only red LED, NO green)
- A *short press* will go into "connected to octoprint mode".
- A *long press* will power off.

### When powered off:
- A *short press* will "power on".

During most states, you can always with a *short press* force "switch on mode forever", or a *long press* "power off immidiately". In the modes when it is connected where Octoprint is alive, it will shutdown octoprint first.

When your WiFi goes away, it will switch to "power on" and try to (re) connect mode. If it does, it will not shutdown Octoprint just like that when the extruder is cooled down.

As you can see, there is quite some "sense" in the states and how the button operates. The state diagram is a good place to better understand what goes on in the plug, and what a button press (long or short) triggers. The LEDS show you in which state it is. Some states are "quickly passed", so the LED does not have time to reflect it in the blinking pattern. You will understand when you see it happening.

You can configure "how the plug comes alive" when powering on the plug. Initially I configure "dumb mode - forever on", but in hind sight I liked "connected to octoprint" better. That is how the configuration template ships.

If you don't like the blinking patterns per state, you can reconfigure them faster / slower / more flashy... whatever you like.
If you do not like the timing: change it. But remember: interogating the API of octoprint takes some resources, so do not overdo it! By nature this plug only needs a "slow" update rate of information.

For technical reasons: while it checks for "Octoprint" to be alive (and it is not) the LEDS do not blink for 3 seconds. So the states that assume Octoprint not to be there, reduce their update rate. 

## I do not see anything of this in Octoprint
True, allthough the LEDS blink and reflect the state the plug is in, it is difficult to see what is going on. This is why I updated OctoPlugout so that it shows messages on the LCD of your printer. Your printer should be able to understand the standard Marlin M117 GCODE.

Here are some examples: when the print is finished and will shutdown Octoprint when it is cooled down.

<img align="left" style="padding-right:30px;" src="https://github.com/ruedli/OctoPlugout/blob/master/images/shutdown.jpg">























And when Octoprint has been shutdown, and the printer + Octoprint will be powered off.

<img align="left" style="padding-right:30px;" src="https://github.com/ruedli/OctoPlugout/blob/master/images/poweroff.jpg">






























## If you like OctoPlugout...

Consider buying me a coffee

<a href="https://www.buymeacoffee.com/ruedli" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 60px !important;width: 217px !important;" ></a>

### If you don't like OctoPlugout...

Let me know, and I will see what I can do to make you like it.

## Acknowledgments

* Gina Häußge (@foosel) for her amazing work on OctoPrint, which without her, none of this would be even possible.
* Stephen Ludgate [chunkysteveo](https://github.com/chunkysteveo) for his OctoPrintAPI work.
* Andreas Spiess https://www.youtube.com/channel/UCu7_D0o48KbfhpEohoP7YSQ for triggering discussion and the work necessary to complete the task.
* Sander Verweij For his state machine representation which you find here: https://state-machine-cat.js.org/

## Authors

* **Stephen Ludgate** - *Inspiration* - For his example sketch HelloWorldSerial that proved my concept and was my "start" for the sketch..
* **Ruud Rademaker**  - *Initial work* - 

## License

See the [LICENSE.md](LICENSE.md) file for details


## Release History
** 1.0 Initial Release - 30 oct 2020

** 1.1 Extra features  - 1 nov 2020
- Extra state: "going down"
- Parameters added for LED info extension (download and version blink pattern when booting up)
- Check that the version of the configuration has all required parameters

** 1.2 Improved state transitions

** 1.3 Not released

** 1.4 Messages - 2 nov 2020
-    State interpretation also when LED is configured to be off
-    Messages on the LCD of your printer, when 
     - plug is connected 
     - Print is monitored
     - Pi will be shutdwono
     - Power will be switched off.

** 2.1 Messages - 2 nov 2020
-    Main version 2.x because you need to configute the LED_ON / LED_OFF time for the new state (print_started)
     
	 This new state prevents very short / aborted print jobs (configurable time and reached extruder 
     temperature) to trigger a shutdown/power off
     Thnx to Tim for pointing this out!

## Requests / Future To Do List
- Avoid switching off if Octoprint is running and not shutdown: even if you try to force it, with a non-monitored "long press"
- DONE Show the "version" with a LED blinking pattern when stating up. 
- Who can make the programmable with platformio?
- A plugin interface that allows Octoprint to set a state with an embedded GCODE, so that Octoprint will trigger a "shutdown".

## Meta

Ruud Rademaker:

Email - ruud.rademaker@gmail.com

git - https://github.com/ruedli

## Contributing

1. Fork it (<https://github.com/ruedli/OctoPlugout/fork>)
2. Create your feature branch (`git checkout -b feature/fooBar`)
3. Commit your changes (`git commit -am 'Add some fooBar'`)
4. Push to the branch (`git push origin feature/fooBar`)
5. Create a new Pull Request
