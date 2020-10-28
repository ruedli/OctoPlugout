# A Plugout for Octoprint
<img align="left" style="padding-right:10px;" src="https://octoprint.org/assets/img/logo.png">
Contrary to most contributions to Octoprint, this software is not a Plugin, but does talk to Octoprint through its API. It works outside of the (python) framework of [OctoPrint](https://octoprint.org/). Octoprint needs no further introduction: the well known the 3D printer web server designed and maintained by the brilliant Gina Häußge, aka @foosel.

### I called it.... "OctoPlugout"
Because it is not a plugin, but still communicates like plugins -outside of the framework-, I called it a "plugout". So... OctoPlugout was born.

In essense this "Arduino" software "lives" in the ESP8266 inside the wall plug. Here is where you plugin the powercord of your Printer.
<img align="left" style="padding-right:30px;" src="https://github.com/ruedli/OctoPlugout/blob/master/images/SONOF%20reflashed%20working.jpg">

### What does it do?
It uses this Sonoff plug, that you flash with the sketch provided here. Now the plug itself communicates wirelessly with the Octoprint server running on the Raspberry Pi.

**After a print** it will:
- Wait for the extruder to cool down
- Shutdown the Pi
- Wait for the Pi to finish shutting down
- Power off, both **printer** and **Pi**

### What is in it for you?

OctoPlugout delivers a "one touch" safe shutdown button for both your Pi and printer! No additional wiring to the Pi, just the printer's powercord!

The beauty of the solution, is that unlike Tasmota or Sonoff, there is no separate software or framework needed. Neither locally nor in the cloud. It is sufficient to directly communicate (wirelessly) between the plug and octoprint. Only the powercord is plugged in, so that the plug can control the power to your printer.

### What does it need?

For hardware you just need this programmable relay card. A Sonoff is most convenient, as its housing is designed for this. I used a S26, so there is no 220V rewiring necessary at all! Just plug in your printer, no further connection between plug and Pi. Your worries like which GPIO to use are gone!

A Sonoff can switch using its relay, has an indicator LED and a button. All three are used! The button is used to power on/off the printer manually and the LED indicates the state that the printer is in.

The software in the plug considers what to do if Wifi is lost, when your printer goes in "pause" mode and when it is a good moment to shutdown. It even initiates this shutdown of your Pi for you.

### Software

I used the Arduino IDE to compile everything. You need:

- The ESP8266 library 
- A library to communicate with the Octoprint API, called OctoprintAPI. You find it here: https://github.com/chunkysteveo/OctoPrintAPI 
- Prerequisits for OTA upgrades (over the air upgrade). Ensure you can compile to BasicOTA.ino sketch and see the listed IP address, which you need to update octoPlugout through the IP port. You see how to setup OTA here: https://randomnerdtutorials.com/esp8266-ota-updates-with-arduino-ide-over-the-air/

With OctoprintAPI you also find other preprequisites for installation and making the sketch work for your environment, follow these recommendations.

The "OctoPlugout" sketch has ALL its configuration parameters in one place: the file OctoPlugout.config.h. Copy it from OctoPlugout.config.h.RELEASED and adapt it to your Octoprint and WiFi. Adapt at least these parameters indicated in yellow, this depends on the IP address of Octoprint and Octiprints so-called API-key (accessible through the settings of Octoprint).
![The critical configuration](https://github.com/ruedli/OctoPlugout/blob/master/images/config.jpg)  


Also the OctoprintAPI describes all these parameters in further details. The "green" ones should stay like they are. Consider the other parameters: you can adapt it to other Sonoff plugs (in case it uses different pins), as well as configure the 8 "blinking" patterns for the states the plug is in, as well as some timing parameters. The parameter file describes them.

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
- temp-: The extruder is cool enough to power off the printer
- timeout: a sufficiently long time between "shutdown" of the Pi and poweringoff the Pi.

I 3D-printed a convenient plug to flash the S26. You can find it here: https://www.thingiverse.com/thing:3193583

Here you see the first flashing using its serial port. Later flashes can be done "over the air".
![Flashing a sonoff S26](https://github.com/ruedli/OctoPlugout/blob/master/images/flash%20a%20S26.jpg)

Please "save" your original Sonoff software first before you flash OctoPlugout over it. Instructions are here: https://hobbytronics.com.pk/sonoff-original-firmware-backup-restore/#Step-by-Step-Procedure 

### If you like it...

Consider buying me a coffee

<a href="https://www.buymeacoffee.com/ruedli" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 60px !important;width: 217px !important;" ></a>

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
* 1.0 Initial Release

## Requests / Future To Do List
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
