# A Plugout for Octoprint
<img align="left" style="padding-right:10px;" src="https://octoprint.org/assets/img/logo.png">
Contrary to most contributions to Octoprint, this software is not a Plugin, but a Sonoff Plug, that works outside of the framework of [OctoPrint](https://octoprint.org/) the 3D printer web server by the brilliant Gina Häußge, aka @foosel.

### It is called "OctoPlugout"
Because it is not a plugin, but it is a socket in which you plug in your printer/Raspberry Pi powercord, I called it a "plugout", so OctoPlugout was born.

In its essense this "Arduino" software just "lives" in the ESP8266 in a plug in the wall, in which you plug your Printer.
<img align="left" style="padding-right:10px;" src="https://github.com/ruedli/OctoPlugout/blob/master/images/SONOF%20reflashed%20working.jpg" =250x>

### What does it do?
It uses a Sonoff plug, that you reflash with the sketch provided here. This plug communicates wirelessly with the Octoprint server running on the Raspberry Pi.
After a print, it will wait for the extruder to cooldown, shutdown the Pi server and wait for the Pi to safely come to a stop. Then it will power off your Pi and the Printer.

If no print is running and the extruder temperature is low, it will just initiate the shutdown, wait, and power off.

### What is in it for you?

OctoPlugout delivers a "one touch" safe shutdown button for both your Pi and printer!

The beauty of the solution, is that unlike Tasmota or Sonoff, there is no separate software or framework locally or in the cloud. It is based upon direct (wireless) communication between plug and octoprint.

### What does it need?

For hardware you just need a programmable relay card. A Sonoff is most convenient. I used a S26, so there is no 220V rewiring necessary at all! Just plug in your printer, there is no connection between plug and Pi, to communicate.

A Sonoff can switch a relay, has an indicator LED and a button. All three are used! The button is used to power on/off the printer manually and the LED indicates the state the printer is in.

It considers what to do if Wifi is lost, when your printer goes in "pause" mode and when it is a good moment to shutdown. It even initiates the shutdown of the Pi for you.

### Software

I used the Arduino IDE to compile everything. You need:

- The ESP8266 library 
- A library to communicate with the Octoprint API, called OctoprintAPI. You find it here: https://github.com/chunkysteveo/OctoPrintAPI 
- Prerequisits for OTA upgrades (over the air upgrade). Ensure you can compile to BasicOTA.ino sketch and see the listed IP address, which you need to update octoPlugout through the IP port.

With OctoprintAPI you also find other preprequisites for installation and making the sketch work for your environment.

The sketch has ALL configuration parameter in one place: OctoPlugout.config.h. Copy it from OctoPlugout.config.h.RELEASED and adapt it to your Octoprint and WiFi. Adapt at least these three parameters, as well as the IP address of Octoprint. Consider the other values: you can adapt it to other Sonoff plugs, as well as configure the 8 "blinking" patterns for the states the plug is in.

These states look like this:
![All the states](https://github.com/ruedli/OctoPlugout/blob/master/images/All%20states.jpg) 

I printed a convenient plug, to flash the S26, you can find it here: https://www.thingiverse.com/thing:3193583

Here you see the first flashing by its serial port. Later flashes can be done "over the air".
![Flashing a sonoff S26](https://github.com/ruedli/OctoPlugout/blob/master/images/flash%20a%20S26.jpg)

PLease "save" your Sonoff software first, before you flash OctoPlugout over it. Instructions are here: https://hobbytronics.com.pk/sonoff-original-firmware-backup-restore/#Step-by-Step-Procedure 


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
