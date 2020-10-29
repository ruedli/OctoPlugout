/*******************************************************************
 *  OctoPlugout                                                    */

#define Version_major 1
#define Version_minor 1
 
 /*
 *  v1.0 - 27 oct 2020
 *    Initial release 
 *
 *  An octoprint Arduino (ESP8266) sketch, to transform 
 *  a SonOff plug into an "intelligent" socket that will safely remove power
 *  from your printer AND the Raspberry Pi which is running octoprint.
 *
 *  After a print has finished and after and the extruder temperature 
 *  is below 50ºC (configurable), a "shutdown" command is invoked to the Raspberry PI.
 *  Next, after 60 seconds (configurable), the power is removed. This
 *  Allows the Pi to safely shutdown.
 *
 *  For this, the ESP8266  connects wirelessly to Octoprint and monitors
 *  the printer status. 
 *
 *  The ESP8266 in the Sonoff and the Raspberry Pi running the
 *  OctoPrint server can speak to each other. 
 *
 *  The plug will read your OctoPrint Server, API and 3D printer 
 *  statistics from the OctoPrint API.
 *
 *  You will need the IP or hostname of your OctoPrint server, a
 *  port number (will be 80 unless you are reaching it from an
 *  external source) and your API key from the OctoPrint
 *  installation - http://docs.octoprint.org/en/master/api/general.html#authorization 
 *
 *  Allthough the used API needed CORS, I found that the functions I use in the plug
 *  also work fine without. I could only connect by specifying its IP address, not through its hostname.
 *  Consider allowing CORS if you have trouble.
 * 
 *  To enable CORS - http://docs.octoprint.org/en/master/api/general.html#cross-origin-requests
 * 
 *  OctoPlugout:  By Ruud Rademaker   ruud dot rademaker at gmail dot com
 *
 *  OctoPrintAPI: By Stephen Ludgate https://www.youtube.com/channel/UCVEEuAouZ6ua4oetLjjHAuw
 *
 *  Copyright (C) 2020  OctoPlugout by Ruud Rademaker
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *******************************************************************/
 
 /* vvvvvvvvvvvvvvv   COPY BELOW LINES INTO https://state-machine-cat.js.org/  vvvvvvvvvvvvvv

"unplugged",

# PlugOut: an electrical plug Made from a "sonoff" device by reflashing the firmware that is:
#- WiFi connected: not wired through eg. GPIO
#- OctoPrint aware, but not a plugin: a PlugOut!
#
# Triger evaluation order:
# - press: short (SP) and long (LP)
# - WiFi: Down (Wifi-) and up (Wifi+)
# - Pi:  Down (Pi-) / Alive (Pi+)
# - Print state: Printing (Print+) / Not Printing (Print-)
# - Extruder Temperature:  Hot (Temp+) / Cold (Temp-)
#
#use https://state-machine-cat.js.org/ to visualize these lines and see the states.

"plugged" 
{ "0: Relay OFF" [color="#ee2222"],  "Relay ON"  {

//======================== Switched based transitions (blue)

  "2: Waiting for print activity" => "3: waiting for printactivity (connected)" [color="blue"] :  Pi+;

  "3: waiting for printactivity (connected)" => "4: ready for shutting down" [color="blue"] :  print+;
  "3: waiting for printactivity (connected)" => "2: Waiting for print activity" [color="blue"] :   Pi- | WiFi-;

  "4: ready for shutting down" => "5: ready for shutting down HOT" [color="blue"] :  print-;
  "4: ready for shutting down" => "7: delayed powering relay off" [color="blue"] :  Pi-;

  "5: ready for shutting down HOT" => "4: ready for shutting down" : print+;
  "5: ready for shutting down HOT" => "6: shut down PI" [color="blue"] :  Temp-;
  "5: ready for shutting down HOT" => "7: delayed powering relay off" [color="blue"] :  Pi-;

  "6: shut down PI" => "7: delayed powering relay off" [color="blue"] :  Pi-; 

  "7: delayed powering relay off" => "3: waiting for printactivity (connected)" [color="blue"] :  Pi+  ;


  "8: Going down" => "7: delayed powering relay off" [color="blue"] : Pi-;
  "8: Going down" => "4: ready for shutting down"  [color="blue"] : Pi+;

//======================== Time based transitions (dark blue)

  "7: delayed powering relay off" => "0: Relay OFF" [color="#2222AA"] :  timeout;

//======================== Switched based transitions (red and green)

  "1: Switched ON" => "2: Waiting for print activity" [color="#008800"] : SP;
  "1: Switched ON" => "8: Going down" [color="#ee2222"] :  LP; 

  "2: Waiting for print activity" => "1: Switched ON"[color="#008800"]  : SP;
  "2: Waiting for print activity" => "8: Going down" [color="#ee2222"] : LP;

  "3: waiting for printactivity (connected)" => "1: Switched ON" [color="#008800"]  : SP;
  "3: waiting for printactivity (connected)" => "4: ready for shutting down" [color="#ee2222"] : LP;  

  "4: ready for shutting down" => "1: Switched ON" [color="#008800"]  :  SP;
  "4: ready for shutting down"=> "5: ready for shutting down HOT" [color="#ee2222"] : LP;

  "5: ready for shutting down HOT" => "1: Switched ON" [color="#008800"]  : SP;

  "6: shut down PI" => "1: Switched ON" [color="#008800"] : SP;

  "7: delayed powering relay off" => "1: Switched ON"  [color="#008800"]  : SP   ;
  "7: delayed powering relay off" => "0: Relay OFF" [color="#ee2222"] : LP;

  "0: Relay OFF"  => "1: Switched ON" [color="#008800"] : SP;

};

"0: Relay OFF"  => "unplugged" : proper unplug;

};

"Relay ON"  => "unplugged" [color="purple"] : unproper unplug;
"unplugged" => "1: Switched ON" [color="purple"] : plug in;

*/ //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ END OF LINES TO VISUALIZE THE STATE DIAGRAM
 

#include <OctoPrintAPI.h> //This is where the magic happens... shazam!

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

//Necessary for OTA
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Set these defines to match your environment, copy initial file from OctoPlugout.config.h.RELEASE ========
#include "OctoPlugout.config.h"

#ifndef LED_blink_flash 
#define LED_blink_flash false
#endif

#ifndef LED_blink_initial 
#define LED_blink_initial false
#endif

#ifndef OctoPlugout_config_version
#error The version of the config file cannot be determined, ensure you copy OctoPlugout.config.h from the latest OctoPlugout.config.h.RELEASE
#endif

#if OctoPlugout_config_version != Version_major
#error "There are new configuration parameters determined, ensure you copy OctoPlugout.config.h from the latest OctoPlugout.config.h.RELEASE"
#endif

WiFiClient client; 

// You only need to set one of the of following, but note: I COULD NOT GET THE HOSTNAME TO WORK, use IP address!
#ifdef UseIP
	IPAddress ip(ip1, ip2, ip3, ip4);        // Your IP address of your OctoPrint server (inernal or external)
	OctoprintApi api(client, ip, octoprint_httpPort, octoprint_apikey);				  // When using IP Address
#else
	char* octoprint_host = octoprintHost;    				// Or your hostname. Comment out one or the other.
	OctoprintApi api(client, octoprint_host, octoprint_httpPort, octoprint_apikey);   // When using hostname 
#endif

// DO NOT UPDATE the state names! 
// Make sure your "blinking definition" in the config has 9 entries (0..8): one for each state.

enum state {
  Relay_off,
  Switched_ON,
  Waiting_for_print_activity,
  Waiting_for_print_activity_connected,
  ready_for_shutting_down,
  ready_for_shutting_down_extruder_HOT,  
  shutting_down_PI,
  delayed_powering_relay_off,
  going_down
};


state State;
state LastState;

// Timer stuff
unsigned long OctoprintTimer;
unsigned long LastPressed;
unsigned long WifiBeginDone;
unsigned long PowerOffRequested;
unsigned long TimerLED;
unsigned long IntervalLED;
unsigned long OctoprintInterval;

// The state of the info LED
#if LED_blink_flash==true
int LED_count;
#endif
bool LED_on;

//Button state variables.
bool ButtonPressed = false;
bool LongPress = false;
bool ShortPress = false;

//declare the functions defined and used furtheron
bool OctoprintRunning   (bool Default = false);
bool OctoprintNotRunning(bool Default = false);
bool OctoPrintDown; // Indicates that there is no octoprint server, usiing a much longer delay to check that is is running.

bool OctoprintPrinting   (bool Default = false);
bool OctoprintNotPrinting(bool Default = false);

bool OctoprintCool(bool Default = false);
bool OctoprintHot (bool Default = false);

bool WifiAvailable    (bool Default = false);
bool WifiNotAvailable (bool Default = false);

bool OctoprintShutdown(void);

// The various states:


void setup () {
  // initialize digital pins for Sonoff
  pinMode(PinRelay, OUTPUT);
  // Set the relay ON....
  digitalWrite(PinRelay, (InitialState == Relay_off) ? LOW : HIGH);

  pinMode(PinLED, OUTPUT);
  pinMode(PinButton, INPUT);

  digitalWrite(PinLED, HIGH);  // The LED is off...
  
  Serial.begin(115200);
  delay(1000);
  Serial.flush();
  

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  //Set the hostname
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.setPassword(OTApass);
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
	
	#if LED_blink_flash==true
	// switch off all the PWMs during upgrade
	analogWrite(PinLED, 0);
	LED_count = 0;
	LED_on = false;
	digitalWrite(PinLED, LED_on ? LOW: HIGH);  // The LED is off...
	#endif
	
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
	#if LED_blink_flash==true
	LED_on = false;
	digitalWrite(PinLED, LED_on ? LOW: HIGH);  // The LED is off...
	#endif
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	#if LED_blink_flash==true
		if ((LED_count++ % 2) == 0) LED_on = not LED_on;
		digitalWrite(PinLED, LED_on ? LOW: HIGH);  // The LED is blinks during uploading...
	#endif
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();  

//Initial state and timers for OctoPrintPlugout

	#if LED_blink_initial==true
	// do a fancy thing with our board led when starting up
	for (int i = 0; i < 30; i++) {
	  analogWrite(PinLED, (i * 100) % 1001);
	  delay(50);
	}
	digitalWrite(PinLED, HIGH);  // The LED is off...

	// Indicate the manor and minor version
	const int delays[] = { Version_major , Version_minor };
	for (int i = 0; i <= 1; i++) {
		delay(500);
		for (int j = 1; j <= delays[i]; j++) {
			#ifdef debug_
			Serial.print("Blink: ");
			Serial.println(j);
			#endif
			delay(300);
			digitalWrite(PinLED, LOW);  // The LED is on...
			delay(50);
			digitalWrite(PinLED, HIGH);  // The LED is off...
		}
	}
	#endif

	// This is the initial state
	State = InitialState;
	LastState = delayed_powering_relay_off;    //Ensures InitialState is ALWAYS considered in the "loop".

    OctoprintTimer = LastPressed = PowerOffRequested = TimerLED = 0;
	WifiBeginDone = ULONG_MAX - CheckWifiState; // This ensures the wifi is initialized immediately.
	OctoprintInterval = OctoprintInterval_running;
	IntervalLED = 1000;  // Ensures that ofr one second, the LED stays off;
	LED_on = false;
	
	
// Show information on the Serial port
  Serial.print("Ready: version v");
  Serial.print(Version_major);
  Serial.print(".");
  Serial.println(Version_minor);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {
	
	ArduinoOTA.handle();
	
	unsigned long Now = millis();
	
	//Always try Wifi state and if necessary connect to wifi..	
	if ((WiFi.status()) != WL_CONNECTED) {
		//
		// Now that it is not connected:
		//    Attempt to initialize WIFI... ONLY once every minute....
		//    This is useful if the Wifi is "lost" and reappears.	
		if ((Now - WifiBeginDone) >= CheckWifiState) {
			#ifdef debug_
			Serial.println("WiFi initialized");
			#endif
			
			WiFi.begin(ssid, password);
			WifiBeginDone = Now;
		}
	}

	//Evaluate LED stuff: All switching on and off (for "blinking") takes place here...
	//First determine whether it is time to toggle the LED:
	if ((Now - TimerLED) > IntervalLED) {
		LED_on = ! LED_on;		// toggle
		TimerLED = Now;
		if (LED_on) {			//Set the time the LED should be ON
			IntervalLED=LED_ON_TIME[State];
			if ( IntervalLED == 0) {  // This led should NEVER be on...
				LED_on = false;
				IntervalLED = 1000;
			}
		} else {				//Set the time the LED should be OFF
			IntervalLED=LED_OFF_TIME[State];
		}
		digitalWrite(PinLED, LED_on ? LOW: HIGH);	// Actually switch the LED on or Off, depending on LED_on.
	}
	
	// Evaluate button: debouncing after it is pressed
	ShortPress = LongPress = false; // Reset
	if ((Now - LastPressed) > debounce_interval) {
		if (ButtonPressed) {
			if (digitalRead(PinButton) == HIGH) {            //This means the button is (now) released
				if ((Now - LastPressed) > long_press_time) { // Checking whether it is a short, or long button press
					LongPress = true;
				} else {
					ShortPress = true;
				}
				ButtonPressed = false;
				LastPressed = Now; 							// For debouncing
			}
		} else {
			if (digitalRead(PinButton) == LOW) { 			//This means the button is (now) pressed
				ButtonPressed = true;
				LastPressed = Now; 				 			// For debouncing
			}
		}
	}
	
	// Interpret buttons immediately when detected (after release), actions involving transitions and printer job every 5 seconds
	if (ShortPress or LongPress) {
		#ifdef debug_
		Serial.println("Checking Button transitions");
		#endif		
		switch(State) {
		case Relay_off: // 0
			if (ShortPress) {
				State = InitialState;
				if (State == Relay_off) State = Switched_ON;
			}
			break;
		case ready_for_shutting_down_extruder_HOT: // 5
		case shutting_down_PI: // 6
			if (ShortPress) {
				State = Switched_ON;
			}
			break;
		case Switched_ON: // 1
			if (ShortPress) {
				State = Waiting_for_print_activity;
			} else if (LongPress) {
				State = going_down;
			}
			break;
		case Waiting_for_print_activity: // 2
			if (ShortPress) {
				State = Switched_ON;
			} else if (LongPress) {
				State = going_down;
			}
			break;		
		case Waiting_for_print_activity_connected: // 3
		case delayed_powering_relay_off : // 7			
			if (ShortPress) {
				State = Switched_ON;
			} else if (LongPress) {
				State = ready_for_shutting_down;
			}
			break;
		case ready_for_shutting_down: // 4
			if (ShortPress) {
				State = Switched_ON;
			} else if (LongPress) {
				State = ready_for_shutting_down_extruder_HOT;
			}
			break;			
		}

		
	// Now interpret every 15 or 5 (configurable) seconds whether the states need to change because of Octoprint statistics
	} else if ((Now - OctoprintTimer) > OctoprintInterval) {
		#ifdef debug_
		Serial.println("Checking State transitions");
		#endif
		OctoprintTimer = Now;
		switch(State) {
		case Waiting_for_print_activity: // 2
			if (OctoprintRunning()) State = Waiting_for_print_activity_connected;
			break;		
		case Waiting_for_print_activity_connected: // 3
			if (OctoprintNotRunning()) State = Waiting_for_print_activity;
			else if (OctoprintPrinting()) State = ready_for_shutting_down;		
			break;
		case ready_for_shutting_down: //4
			if (OctoprintNotRunning(true)) State = delayed_powering_relay_off;
			else if (OctoprintNotPrinting()) State = ready_for_shutting_down_extruder_HOT;
			break;				
		case ready_for_shutting_down_extruder_HOT: // 5
			if (OctoprintCool()) State = shutting_down_PI;
			break;				
		case shutting_down_PI : // 6
			if (OctoprintNotRunning() or OctoprintShutdown()) {
				State = delayed_powering_relay_off;
				PowerOffRequested = Now;
			}
			break;
		case delayed_powering_relay_off :
			// Execute this JUST once, before poweroff ~15 lines down in this sourcecode....
			//if (OctoprintRunning()) State = Switched_ON;
			break;
		case going_down :
			if (OctoprintRunning()) State = ready_for_shutting_down;
			else {
				State = delayed_powering_relay_off;
				PowerOffRequested = Now;
			}
			break;			
		}

	// Now interpret states that need to change independently of button presses, wifi or Octoprint statistics
	} else {
		switch(State) {
		case delayed_powering_relay_off:
			if ((Now - PowerOffRequested) > WaitPeriodForShutdown ) {

				// One "final check" whether Octoprint has come alive again...
				if (OctoprintRunning()) {State = Switched_ON;
				} else State = Relay_off;  // If not... poweroff the printer and Pi!
			}
			break;
		}		
		
	}
	
	//Set the next OctoprintInterval, may be shorted in the state change.
	switch(State) {
	case Relay_off: // 0
	case Switched_ON: // 1
	case Waiting_for_print_activity: // 2 
	case delayed_powering_relay_off : // 7
	case going_down: // 8	
		OctoprintInterval = OctoprintInterval_NOT_running; // Poll slower (every minute)
		break;	
	case Waiting_for_print_activity_connected: // 3
	case ready_for_shutting_down: // 4
	case ready_for_shutting_down_extruder_HOT: // 5
	case shutting_down_PI: // 6
		OctoprintInterval = OctoprintInterval_running; // Poll faster (every 5 seconds)
		break;		
	}		

	//Take action for State changes
	if (State != LastState ) {

		//Set the relay
		digitalWrite(PinRelay, (State == Relay_off) ? LOW : HIGH);

		// Force LED to reflect a new pattern.
		TimerLED = 0;  

		// Show debug_ information
		#ifdef debug_
		if (OctoprintInterval < OctoprintInterval_NOT_running) {
			if(api.getPrinterStatistics()){
				Serial.println("---------States---------");
				Serial.print("Printer Current State: ");
				Serial.println(api.printerStats.printerState);
				Serial.print("Printer State - operational:  ");
				Serial.println(api.printerStats.printerStateoperational);
				Serial.print("Printer State - paused:  ");
				Serial.println(api.printerStats.printerStatepaused);
				Serial.print("Printer State - printing:  ");
				Serial.println(api.printerStats.printerStatePrinting);
				Serial.print("Printer State - ready:  ");
				Serial.println(api.printerStats.printerStateready);
				Serial.println("------------------------");
				Serial.println();
				Serial.println("------Termperatures-----");
				Serial.print("Printer Temp - Tool0 (°C):  ");
				Serial.println(api.printerStats.printerTool0TempActual);
				Serial.print("Printer State - Bed (°C):  ");
				Serial.println(api.printerStats.printerBedTempActual);
				Serial.println("------------------------");
			}
		}
		Serial.print("State changed from ");
		Serial.print(LastState);
		Serial.print(" to ");
		Serial.print(State);
		Serial.print(" LED on/off: ");
		Serial.print(LED_ON_TIME[State]);
		Serial.print("/");
		Serial.println(LED_OFF_TIME[State]);
		#endif

		// Only do an action "ONCE"
		LastState = State; 

		//Set the next OctoprintInterval, SHORTEN the state change.
		switch(State) {
		case Waiting_for_print_activity_connected: // 3
		case ready_for_shutting_down: // 4
		case ready_for_shutting_down_extruder_HOT: // 5
		case shutting_down_PI: // 6
		case delayed_powering_relay_off : // 7		
		case going_down: // 8		
			OctoprintInterval = 200; // Poll faster (only ONCE)
			break;		
		}
	}

}

bool WifiAvailable(void)
{
	return (WiFi.status() == WL_CONNECTED); 
}

bool WifiNotAvailable(void)
{
	return (WiFi.status() != WL_CONNECTED); 
}
	
bool OctoprintShutdown(void)
{
	#ifdef debug_
	Serial.println("SHUTDOWN attempt");
	#endif
	if (WiFi.status() == WL_CONNECTED) {
		// Switch the LED off, it might stay on for 3s otherwise...
		digitalWrite(PinLED,HIGH); //switches it off...
		if(api.getPrinterStatistics()) {
			return api.octoPrintCoreShutdown();
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool OctoprintNotPrinting(bool Default) {
	return not OctoprintPrinting(not Default);
}

bool OctoprintPrinting(bool Default)
{
	if (WiFi.status() == WL_CONNECTED) {
		// Switch the LED off, it might stay on for 3s otherwise...
		digitalWrite(PinLED,HIGH); //switches it off...		
		if(api.getPrinterStatistics()) {
			#ifdef debug_
			Serial.print("Printer State - paused:  ");
			Serial.println(api.printerStats.printerStatepaused);
			Serial.print("Printer State - printing:  ");
			Serial.println(api.printerStats.printerStatePrinting);
			Serial.print("Printer Temp - Tool0 (°C):  ");
			Serial.println(api.printerStats.printerTool0TempActual);
			#endif
			if (api.printerStats.printerStatePrinting == 1) {
				#ifdef debug_
				Serial.println("Octoprint is PRINTING");
				#endif
				return true;
			} else if (api.printerStats.printerStatepaused == 1) {
				#ifdef debug_
				Serial.println("Octoprint is PAUSED");
				#endif
				return true;
			} else { 	
				return false;
			}
		} else {
			#ifdef debug_
			Serial.println("(is Printing? Octoprint NOT running");
			#endif
			return Default;
		}
	} else {
		#ifdef debug_
		Serial.println("No Wifi (printer printing)");
		#endif
		return Default;
	}
}

bool OctoprintNotRunning(bool Default) {
	return not OctoprintRunning(not Default);
}

bool OctoprintRunning(bool Default)
{
	#ifdef debug_
	Serial.println("TEST whether Octoprint is running");
	#endif
	if (WiFi.status() == WL_CONNECTED) {
		// Switch the LED off, it might stay on for 3s otherwise...
		digitalWrite(PinLED,HIGH); //switches it off...
		if (api.getPrinterStatistics()) {
			OctoPrintDown = false;
			return (true);
		} else {
			#ifdef debug_
			Serial.println("Octoprint not running");
			#endif
			OctoPrintDown = true;
			return (false);
		}
	} else {
		#ifdef debug_
		Serial.println("No Wifi (octoprint running");
		#endif
		return Default;
	}
}

bool OctoprintHot(bool Default) {
	return not OctoprintCool(not Default);
}

bool OctoprintCool(bool Default)
{
	#ifdef debug_
	Serial.println("TEST whether extruder is cold");
	#endif
	if (WiFi.status() == WL_CONNECTED) {
		// Switch the LED off, it might stay on for 3s otherwise...
		digitalWrite(PinLED,HIGH); //switches it off...
		if (api.getPrinterStatistics()) {
			#ifdef debug_
			Serial.print("Printer Temp - Tool0 (°C):  ");
			Serial.println(api.printerStats.printerTool0TempActual);
			#endif
			return (api.printerStats.printerTool0TempActual < MaxExtruderTemperature);
		} else {
			#ifdef debug_
			Serial.println("Octoprint not running");
			#endif
			return Default;
		}
	} else {
		#ifdef debug_
		Serial.println("No Wifi (extruder temp test");
		#endif
		return Default;
	}
}





