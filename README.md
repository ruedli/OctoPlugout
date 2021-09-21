# A Plugout for Octoprint

<img align="left" src="https://octoprint.org/assets/img/logo.png" width=200>

Contrary to most contributions to Octoprint, this software is not a Plugin, but does talk to Octoprint through its API. It works outside of the (python) framework of [OctoPrint](https://octoprint.org/). Octoprint needs no further introduction: the well known the 3D printer web server designed and maintained by the brilliant Gina Häußge, aka @foosel.

## I called it.... "OctoPlugout"
Because it is not a plugin, but still communicates like plugins -outside of the framework-, I called it a "plugout". So... OctoPlugout was born.
<p> 
<p>
<p>
<p>
In essense this "Arduino" software "lives" in the ESP8266 inside the wall plug. Here is where you also plugin the powercord of your Printer/Raspberry Pi.

<img align="right" style="padding-left:60px;" src="https://github.com/ruedli/OctoPlugout/blob/master/images/SONOF%20reflashed%20working.jpg">

## What does it do?
It uses this Sonoff plug, that you flash with the sketch provided here. Now the plug itself communicates wirelessly with the Octoprint server running on the Raspberry Pi.

**After a print** OctoPlugout running in the plug will:
- Wait for the extruder to cool down
- Shutdown the Pi
- Wait for the Pi to finish shutting down
- Power off, both **printer** and **Pi**

In essence  this move the shutdown power-off functionality outside of the Octoprint server running on the Pi. This OctoPlugout software can therefore continue to control things, even after the Pi server itself is shutdown. Plugins running in the server cannot do this.

## What is in it for you?

OctoPlugout delivers a "one touch" **safe shutdown button** for both your Pi and printer! No additional wiring is needed into the Pi, just the printer's powercord!

The beauty of the solution, is that unlike Tasmota or Sonoff, there is no separate software or framework needed to make this plug "work". Neither locally nor in the cloud. It is sufficient to directly communicate (wirelessly) between the plug and octoprint. Only the powercord is plugged in, so that the plug can control the power to your printer.

## What does it need?

For hardware you just need this programmable relay card. A Sonoff is most convenient, as its housing is designed for this. I used a S26, so there is no 220V rewiring necessary at all! Just plug in your printer, no further connection between plug and Pi. Your worries like which GPIO to use are gone!

A Sonoff can switch using its relay, has an indicator LED and a button. All three are used! The button is used to power on/off the printer manually and the LED indicates the state that the printer is in.

The software in the plug considers what to do if Wifi is lost, when your printer goes in "pause" mode and when it is a good moment to shutdown. It even initiates this shutdown of your Pi for you.

## Software

I used the Arduino IDE to compile everything. You need:

- The ESP8266 library 
- A library to communicate with the Octoprint API, called OctoprintAPI. You can download it through the Arduino library manager: search for "Octoprint: .
- Prerequisits for OTA upgrades (over the air upgrade). Ensure you can compile to BasicOTA.ino sketch and see the listed IP address, which you need to update octoPlugout through the IP port. You see how to setup OTA here: https://randomnerdtutorials.com/esp8266-ota-updates-with-arduino-ide-over-the-air/
- A library for the configuration portal (WiFiManager)
- A library for mqtt access (Pubsubclient)

With OctoprintAPI you also find other preprequisites for installation and making the sketch work for your environment, follow these recommendations.

The "OctoPlugout" sketch has ALL its configuration parameters in one place: the file OctoPlugout.config.h. Copy it from OctoPlugout.config.h.RELEASED and adapt it to your Octoprint and WiFi. Adapt at least these parameters indicated in yellow, this depends on the IP address of Octoprint and Octiprints so-called API-key (accessible through the settings of Octoprint).

### OctoPlugout.config.h

![The critical configuration](https://github.com/ruedli/OctoPlugout/blob/master/images/config.jpg) 

**New in version 4.0** is the "configuration portal". If you press the button of your sonoff more then 5 seconds, the LED will start "fast blinking" and when you release it, a configuration website will be available at http://octoplugout Here you can also configure all other parameters, or change your wifi. Proceed as follows:
- Press and hold the button at least 5 seconds (or more) until the LED starts blinking.
- Browse http://octoplugout (do this within one minute) or browse to the IP address that octoplugout received in your wifi network.
- Here you see a menu like below, with a page for Wifi setup and another page (setup) for all parameters.
- After you redefine parameters, the ESP will reboot.

* Hidden in the "info" page is an upgrade button.
This button allows you to select a "bin" file, which then can be uploaded "over the air". Future releases might be provided as "bin" file (mail me if you need one). These then can be uploaded without requiring any tools (when 4.4 or newer is installed, or a sketch supporting OTA).

Parameters define through the menu:

<img src="https://user-images.githubusercontent.com/5008440/133644736-1f268113-ec5d-4009-ade6-1c7d3070684f.png" alt="drawing" width="200"/>

If you _did not set the define_ "def_mqtt_server", no mqtt setting will be requested.
If you set the mqtt_server to "none" or leave it empty, all mqtt_functions are suppressed.

The octoprint address MUST be an IP-adress, not a name! The API string can be obtained from octoprint. Since typically people do not plugin their printer "remotely", I assumed this address to be a local IP address. Note however that with mqtt you can switch your printer on and off from anywhere!

The OctoprintAPI describes all the API parameters for connecting to your printer through WiFi in further details. The "green" ones should stay like they are. Consider the other parameters: you can adapt it to other Sonoff plugs (in case it uses different pins), as well as configure the 8 "blinking" patterns for the states the plug is in, as well as some timing parameters. The parameter file describes them.

If you do not change the OTA password in the config file, when you want to "flash" over the air, the password is "1234".
Note that if you change the password, it only is in effect the NEXT time you flash OTA. If you forgotten the password and lost the config file (to read back what you flashed), you need to flash using the serial port. For a flash over the serial port, no password is required and you can (re)set the OTA password for your next flash(es).

## MQTT **NEW** in version 3.x!

New since version 3.0 is the possibility to operate the plug from your phone and thus remotely switch your printer on, or (safely) off.
This requires access to a MQTT server. The plug will publish topics to this server and look at a configurable topic whether to power the printer, switch it off, or ensure it stays on also after your printjob finishes. No need to touch the button on your Sonoff anymore!

See the updated released configuration file (it is now version 3) to learn which #defines to add. They are all in one section for "mqtt", or you can as off version 4 configure them from the web portal.
If you do not set the #define mqtt_server, everything works without mqtt, but then you can not operate it remotely. It is not different from the verion 2.x in this way.

You can use one of the many mqtt clients on your phone. I tried 6 different ones, on Android phone and iPad. They all worked.

For the required mqtt server, I installed Mosquitto on my NAS. You can also install Mosquitto on a Raspberry Pi, but installing it on your Octoprint server is not recommended, as it needs to be on, even to shutdown the Pi. Secure your mqtt server prefereably through through a VPN. I did not try free public domain mqtt servers, but there is not reason this should not work. You can probably integrate the topics with your home automation or Node Red, but I did not try that and operated it straight from a standard mqtt client. Topics are configurable from the portal or the defines in the settings file.

The picture below shows topics send to and from the plug, you can see the plug reacting to ON OFF PERMON and FORCEOFF messages and go through its states.

<img src="https://user-images.githubusercontent.com/5008440/133646228-97b307a5-f8b8-407b-b9d3-e8cd3846401e.png" alt="drawing" width="300"/>

If you install mqtt plugins in octoprint, you can also operate your printer through mqtt or see what it is doing. In this way I was able to have a pulldown in octoprint to safely power off.
Not "on", because the webpage is not available until the Pi is running.

## States

The states look like this:

![All the states](https://github.com/ruedli/OctoPlugout/blob/master/images/All%20states.jpg) 

In this state diagram the action that trigger state changes are:
- SP: a short press of the button
- LP: a long press on the button
- WiFi-: Wifi is down
- WiFi+: Wifi is up
- Pi-: Octoprint is down
- Pi+: Octoprint is up, and the plug is connected.
- print+: A print is running
- print-: No print is running
- temp-: The extruder is cool enough to power off the printer
- temp+: The extruder is heated up enough to trigger shutdown procedure after print
- timeout: a sufficiently long time between "shutdown" of the Pi and powering off the Pi.

## Flashing a Sonoff

I 3D-printed a convenient plug to flash the S26. You can find it here: https://www.thingiverse.com/thing:3193583

Here you see the first flashing using its serial port. Later flashes can be done "over the air".
![Flashing a sonoff S26](https://github.com/ruedli/OctoPlugout/blob/master/images/flash%20a%20S26.jpg)

Please "save" your original Sonoff software first before you flash OctoPlugout over it. Instructions are here: https://hobbytronics.com.pk/sonoff-original-firmware-backup-restore/#Step-by-Step-Procedure

To preserve stability in combination with OTA, WiFimanager, mqtt and the octoprint API it is important to avoid reserving space for a filesystem. In the arduino IDE select the option under tools like this:

<img src="https://user-images.githubusercontent.com/5008440/133854299-6ad2c094-599e-42ff-b308-c8156e0d41d0.png" alt="drawing" width="400"/>

In platformio, you add a script for uploading, by adding a line like this:
	
<img src="https://user-images.githubusercontent.com/5008440/133854213-8d89ac36-7904-40db-8446-58735b84c301.png" alt="drawing" width="300"/>

This script can be downloaded from platformio, but is also provided.

# Using it...

In practice it is easier to operate compared to how it looks in written form.

In short: The red LED lights shows whenever the relay is on (so your printer/raspberry is powered). The green LED indicates *interaction with Octoprint*.

No green LED: The plug stays on "forever" until you press something (see below).

*Slooooooow blinking green* LED. It tries to find your printer.. When it is found: *Normal blinking green* LED.

This is the "normal" state after startup.

### Now there are three possibilities:
- *Short press*: you go into "disconnected power on mode again", your printer will stay on forever...
- *Long press*: switch off: As soon as no print is running AND the extruder is cool, it will shutdown the Pi. The LED will blink very rapidly... and after 30 seconds the power is removed. If in these 30 seconds Octoprint "comes back", it goes into the initial configured mode. 
- A print is *started... finishes.. the extruder cools down*. This also triggers the shutdown / power off procedure.

### When in "switched on forever" (Only red LED, NO green)
It will stay on forever, no activities of the printer will influence this. Only you can influence this with the button on the plug as follows:
- A *short press* will go into "connected to octoprint mode".
- A *long press* will initiate a safe shutdown / power off.

### When powered off:
- A *short press* will "power on".

During most states, you can always with a *short press* force "switch on mode forever", or a *long press* "power off immediately". In the modes when it is connected where Octoprint is alive, it will shutdown octoprint first.

When your WiFi goes away, it will switch to "power on" and try to (re)connect mode. If it does, it will not shutdown Octoprint! Instead it will wait for a (long) press or wait for a print to be initiated and finished.

As you can see, there is some "sense" in the states and how the button operates. The state diagram is a good place to better understand what goes on in the plug, and what a button press (long or short) triggers. The LEDS show you in which state it is. Some states are "quickly passed", so the LED does not have time to reflect it in the blinking pattern.You will understand when you see it happening. You also see messages on the printers LCD, coming from the plug.

You can configure "how the plug comes alive" when powering on the plug. Initially I configure "dumb mode - forever on", but after some experience, I liked "connected to octoprint" better, because I always found myself switching to that mode. This initial mode (connected to Octoprint) is how the configuration template ships. Reconfigure if you have a different preference.

If you don't like the blinking patterns per state, you can reconfigure them faster / slower / more flashy... whatever you like.
If you do not like the timing for checking things: change it. But remember: interrogating the API of octoprint takes some resources, so do not overdo it! By nature this plug only needs a "slow" update rate of information.

For technical reasons: while it checks for "Octoprint" to be alive (and it is not) the LEDS do not blink for over 3 seconds. So the states that assume Octoprint not to be there, reduce their update rate. 

## I do not see anything of this in Octoprint
True, allthough the LEDS blink and reflect the state the plug is in, it is difficult to see what is going on. This is why I updated OctoPlugout so that it shows messages on the LCD of your printer. Your printer should be able to understand the standard Marlin M117 GCODE.

Here are some examples: when the print is finished and will shutdown Octoprint when it is cooled down.
	
<img src="https://github.com/ruedli/OctoPlugout/blob/master/images/shutdown.jpg" alt="drawing" width="250"/>

And when Octoprint has been shutdown, and the printer + Octoprint will be powered off.

<img src="https://github.com/ruedli/OctoPlugout/blob/master/images/poweroff.jpg" alt="drawing" width="250"/>

## Arduino IDE is nice, but how about platformIO?
Yes, compiling and deploying using platformIO was a "wish" on my todo-list as well. I quite like the platformIO way of organizing embedded platform firmware! As of release 2.3, platformIO is supported. I did not abandon the Arduino IDE way of preparing and deploying firmware, but you can now do this as well using platformIO. If you have not yet installed platformIO: Go through the setup of Visual studio Code and install the platformIO plugin in VCcode. Now you can open the project folder as an platformIO project. Inside you will still find OctoPlugout.ino in a OctoPlugout folder, so that can be used by the Arduino IDE straight away. If you are coming from version 2.2, simply copy your OctoPlugout.config.h file in that folder and all will be fine. You can remove the old OctoPlugout.ino file (the 2.2 version, in the root), should the git update not have removed this file. Thanks to Bob Green for paving the road here!

If you find your OTA failing, like the picture below, it might be due to the state the plug is in after updating its parameters or uploading a sketch. Power it off and retry.

<img align="left" style="padding-right:30px;" src="https://user-images.githubusercontent.com/5008440/133645235-57577728-dbd1-46ab-ae46-07ee9dd9c450.png">


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
* Bob Green for helping getting platformio behave the way I wanted
* Jim Neill (@jneilliii) for sparring on the approach for integration with a (new) octoprint plugin
* Nick O'Leary for providing a library that allows to connect to an mqtt server.
* Tzapu for his WiFimanager that now allows to configure Octoplugout from a webportal, so from your phone.

## Authors

* **Stephen Ludgate** - *Inspiration* - For his example sketch HelloWorldSerial that proved my concept and was my "start" for the sketch..
* **Ruud Rademaker**  - *Initial work / all releases* - 

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
     - Pi will be shutdown
     - Power will be switched off.

** 2.1 Messages - 2 nov 2020
-    Main version 2.x because you need to configute the LED_ON / LED_OFF time for the new state (print_started)
     
	 This new state prevents very short / aborted print jobs (configurable time and reached extruder 
     temperature) to trigger a shutdown/power off
     Thnx to Tim for pointing this out!
	 
** 2.2 Change configuation / timing parameters - 2 nov 202
- Better state transitions
- Recommended a "short job" (not triggering a shutdown) being (less than) 5 minutes. This include the "heating up time", as well as doing any bed level routine you have setup. Increase / decrease to your own taste. 	 

** 2.3 Support building and uploading OTA through platformIO

Uptil 2.2, I used the Arduino IDE for compiling and deploying firmware for the plug. I modified the directory structure to also allow management of the firmware in your plug using platformIO. Arduino IDE can stil be used: simply open the Octoplugout.ino file with the Arduino IDE. Open the [Octoplugout] folder (the one that has paltformio.ini in it) in Visual Studio Code (with the platformio plugin installed) and enjoy compiling and deploying in this environment. It is now easy to support multiple platforms, in case you want to flash something different from a Sonoff. 

** 2.4 State printing extended

Also statues liking "resuming" "pausing" (in addition to the existing "paused") "Error" are now considered as "job in progress". This prevents unexpected sutdowns,e.g. when Octoprint is busy when uploading files.

** 2.5 State printing more resilient

Some states that indicate a print in progress, will NOT respond to a Pi-DOWN message. This is to prevent premature power-off, when the Pi is too busy to respond.

** 2.6 2021-02-14: Add message for printer

Message on printer indicating that the plug is on (but no longer monitoring). 
(just because it looks so nice)

** v3.0 2021-08-31: MQTT awareness
- Switch the printer on using MQTT, also printer and printjob information can be published

** v3.1 2021-09-04
- Ensure the plug also works without MQTT 
- Document README

** v3.2 2021-09-07
- Added optional defines to invert working of relay and/or led.
- Simplified logic for switch, it is reset to '-' after interpretation

** v4.0 2021-09-09
- Added functionality to configure parameters and WiFi through webpage http://192.168.4.1
- Long press (more than 6 seconds) will open a wp portal on Wifi access point "SetupOctoPLugout"
    (no credentials needed). When connected, browse to http://192.168.4.1 and set parameters for 
    IP address of your printer, its API string, mqtt server/topic root/mqtt user/mqtt password
    AND choose your wiFi and enter the password.

** v4.1 2021-09-16
- To support Wifi / Web portal / mqtt / OTA /debugging the memory size became more important to manage.
- Arduino IDE users must select "1Mb / FS none:~502KB" under "flash size" in the "tools" menu. 
- platformio users should load the attached flash definitio in edge.flash.1m.ld by
      entering a line with board_build.ldscript = eagle.flash.1m.ld in their platformio.ini file.
 	  The file "eagle.flash.1m.ld" is added for your convenience as of this v4.2 of OctoPlugout.
	   
** v4.2 2021-09-17
- More stable build without FS: load script adapted for loading without filesystem.

** v4.3 - not released

** v4.4 21 sep 2021
- Enabled config portal
- When Wifi is NOT configured (or not working) connect to:
    AP (wifi SSID) "SetupOctoplugout" 
		 website 		"192.168.4.1 and configure wifi.
- Long pressing the button (>6s) will result in a fast blinking LED
	- When LED is fast blinking:
    - Release
	- Open http://octoplugout 
	- Click "setup" and all parameters can be configured.
	- After "save" the plug will reboot and use the new parameters.
	
** v4.5 21 sep 2021
- Ensured that the build would succeed when no mqtt was used (def_mqtt_server undefined)

## Requests / Future To Do List
- DONE ~~Avoid switching off if Octoprint is running and not shutdown: even if you try to force it, with a non-monitored "long press"~~
- DONE ~~Show the "version" with a LED blinking pattern when stating up.~~
- DONE - thank you Bob Green, for helping me here! ~~Who can make the programmable with platformio?~~
- DONE ~~A plugin interface that allows Octoprint to set a state with an embedded GCODE, so that Octoprint will trigger a "shutdown".~~
Now that Octoplugout supports mqtt, you can install a plugin in octoprint and send commands from the printer to the plug from gcode.



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



