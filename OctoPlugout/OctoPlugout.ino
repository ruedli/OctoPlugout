 /*  OctoPlugout      
 */

#define Version_major 3
#define Version_minor 1
 
 /*
 *  v1.0 - 27 oct 2020
 *    Initial release 
 *
 *  v1.1 - 28 oct 2020
 *    Additional state
 *
 *  v1.2 -  1 nov 2020
 *    Improved state transitions
 *    Avoid pi checking when LED is on
 * 
 *  v1.3 - Equal to V1.2
 *
 *  v1.4 - 2 nov 2020
 *    State interpretation also when LED is configured to be off
 *    Messages on the LCD of your printer, when 
 *    - plug is connected 
 *    - Print is monitored
 *    - Pi will be shutdwono
 *    - Power will be switched off.
 *
 * v2.1 - 2 nov 2020
 *  - Added an additional state "print started"
 *    This state prevents very short / aborted print jobs (configurable time and reached extruder 
 *    temperature) to trigger a shutdown/power off
 *    Thnx to Tim for pointing this out!
 *
 * v2.2 - 3 nov 2020
 *  - improved state transitions and timing
 *
 * v2.3 - 4 nov 2020
 * - Support building and uploading OTA through platformIO
 *
 * v2.4 - 10 nov 2020
 * State printing extended
 * - Also statues liking "resuming" "pausing" (in addition to the existing "paused") "Error" are now considered as "job in progress". 
 *   This prevents unexpected sutdowns,e.g. when Octoprint is busy when uploading files.
 *
 * v2.5 - 26 dec 2020
 * State printing more resilient
 * - some states that indicate a print in progress, will NOT respond to a Pi-DOWN message. 
 *   This is to prevent premature power-off, when the Pi is too busy to respond.
 *
 * v2.6 - 14 feb 2021
 * Show message on display when going "offline"
 * - This makes it easier to see that you are not any longer using Octoplug to monitor a print
 *
 * v3.0 - 26 aug 2021
 * MQTT awareness
 * - Switch the printer on using MQTT, also printer and printjob information can be published
 *
 * v3.1 - 4 sep 2021
 * - Ensure the plug also works without MQTT 
 * - Document README
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
 *  pubsubclient: By Nick O'Leary https://github.com/knolleary/pubsubclient.git (or arduino library manager)
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

  "3: waiting for printactivity (connected)" => "9: print started" [color="blue"] :  print+;
  "3: waiting for printactivity (connected)" => "2: Waiting for print activity" [color="blue"] :   Pi- | WiFi-;

  "9: print started" => "4: ready for shutting down"   [color="blue"] : Temp+ & Time+;
  "9: print started" => "3: waiting for printactivity (connected)"   [color="blue"] : print-;

  "4: ready for shutting down" => "5: ready for shutting down HOT" [color="blue"] :  print-;
//"4: ready for shutting down" => "7: delayed powering relay off" [color="blue"] :  Pi-;

  "5: ready for shutting down HOT" => "4: ready for shutting down" : print+;
  "5: ready for shutting down HOT" => "6: shut down PI" [color="blue"] :  Temp-;
//"5: ready for shutting down HOT" => "7: delayed powering relay off" [color="blue"] :  Pi-;

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

  "9: print started" => "1: Switched ON" [color="#008800"]  : SP;
  "9: print started" => "4: ready for shutting down" [color="#ee2222"] : LP;

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

//Not needed! although the OTA example sketch contained them
//#ifndef platformio_build
//#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>
//#endif

#include <ArduinoOTA.h>

// Set these defines to match your environment, copy initial file from OctoPlugout.config.h.RELEASE ========
#include "OctoPlugout.config.h"

#ifdef mqtt_server
// Includes for MQTT
#include <PubSubClient.h>
#endif

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

// Both sockets must be different, or API calls will disconnect the MQTT client.
WiFiClient API_client; 

#ifdef mqtt_server
WiFiClient MQTT_client; 

PubSubClient MQTTclient(MQTT_client);

unsigned long TimeMQTTReported;

#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

int Status_counter=0;		//For counting the keep-alive-messages
#endif


// You only need to set one of the of following, but note: I COULD NOT GET THE HOSTNAME TO WORK, use IP address!
#ifdef UseIP
	IPAddress ip(ip1, ip2, ip3, ip4);        // Your IP address of your OctoPrint server (inernal or external)
	OctoprintApi api(API_client, ip, octoprint_httpPort, octoprint_apikey);				  // When using IP Address
#else
	char* octoprint_host = octoprintHost;    				// Or your hostname. Comment out one or the other.
	OctoprintApi api(API_client, octoprint_host, octoprint_httpPort, octoprint_apikey);   // When using hostname 
#endif


//======================================================================Define messages on printer  =========
// Messages are new as off 1.4 and I did not want to "force" users in
// defining the messages in the OctoPllugout.config.h file.
// So... If they are defined I keep them, otherwise I define them here

#ifndef Message_startup
#define Message_startup "M117 OctoPlugout %i.%i"
#endif

#ifndef Message_announce_print
#define Message_announce_print "M117 PRINT started"
#endif

#ifndef Message_announce_power_off
#define Message_announce_power_off "M117 Poweroff after PRINT"
#endif

#ifndef message_temperature
#define message_temperature "M117 Shutdown at T=%4.1f C"
#endif

#ifndef message_poweroff
#define message_poweroff "M117 Poweroff in %ds"
#endif

#ifndef message_NoMonitor
#define message_NoMonitor "M117 Plug NOT monitoring"
#endif

//====================================================================== Type & Functions ==================
// OctoPlugout types and functions
//====================================================================== State names =======================
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
  going_down,
  print_started
};

//declare the functions defined and used furtheron
bool OctoprintRunning   (bool Default = false);
bool OctoprintNotRunning(bool Default = false);

bool OctoprintPrinting   (bool Default = false);
bool OctoprintNotPrinting(bool Default = false);

bool OctoprintTemperatureTest(float Temperature, bool Default = false);
bool OctoprintCool(bool Default = false);
bool OctoprintHot (bool Default = false);

bool WifiAvailable    (bool Default = false);
bool WifiNotAvailable (bool Default = false);

bool OctoprintShutdown(void);


state State;
state LastState;
bool  State_transition_checking_ok;

#ifdef mqtt_server
state StateMQTT;			// Callback will set this, loop() will use it.
bool  ConsiderMQTT = false;
#endif

// Timer stuff
unsigned long OctoprintTimer;
unsigned long LastPressed;
unsigned long WifiBeginDone;
unsigned long PowerOffRequested;
unsigned long TimerLED;
unsigned long IntervalLED;
unsigned long OctoprintInterval;
unsigned long Time_print_started;

// The state of the info LED
#if LED_blink_flash==true
int LED_count;
#endif
bool LED_on;

//Button state variables.
bool ButtonPressed = false;
bool LongPress = false;
bool ShortPress = false;

String OctoplugoutState(state OctoPO_State) {
	switch (OctoPO_State) {
	case Relay_off: 							return("Relay_off");
	case Switched_ON:							return("Switched_ON");
	case Waiting_for_print_activity: 			return("Waiting_for_print_activity");
	case Waiting_for_print_activity_connected: 	return("Waiting_for_print_activity_connected");
	case ready_for_shutting_down:				return("ready_for_shutting_down");
	case ready_for_shutting_down_extruder_HOT:	return("ready_for_shutting_down_extruder_HOT");  
	case shutting_down_PI:						return("shutting_down_PI");
	case delayed_powering_relay_off:			return("delayed_powering_relay_off");
	case going_down:							return("going_down");
	case print_started:							return("print_started");
	};
	
	return "";
}

#ifdef mqtt_server
void callback(char* topic, byte* payload, unsigned int length) {
	#ifdef debug_
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] <");
	for (int i = 0; i < length; i++) {
	Serial.print((char)payload[i]);
	}
	Serial.println(">");
	#endif
	// Set State following the message received.
	if (strcmp(topic, topic_OnOffSwitch) == 0) {
		payload[length] = '\0'; // NULL terminate the array
		ConsiderMQTT = true;
		if ((strcmp("ON", (char *)payload) == 0) or (strcmp("AFTERJOB", (char *)payload) == 0))
			StateMQTT = Waiting_for_print_activity;
		else if (strcmp("PERMON", (char *)payload) == 0)
			StateMQTT = Switched_ON; 
		else if (strcmp("OFF", (char *)payload) == 0)
			StateMQTT = ready_for_shutting_down;
		else if (strcmp("FORCEOFF", (char *)payload) == 0)
			StateMQTT = Relay_off; 
		else
			ConsiderMQTT = false;
	}
}
#endif

void reconnect() {
  #ifdef mqtt_server
  // Loop until we're reconnected
  while (!MQTTclient.connected()) {
    #ifdef debug_
	Serial.print("Attempting MQTT connection...");
	#endif
	
	// Create a random MQTTclient ID
    String clientId = "OCTOPLUGOUT-";
    clientId += String(random(0xffff), HEX);
	
    // Attempt to connect
    if (MQTTclient.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
		#ifdef debug_
		Serial.println(clientId + " connected");
		#endif
		// Resubscribe
		if (MQTTclient.subscribe(topic_OnOffSwitch,1)) {
			#ifdef debug_
			Serial.print ("Subscribed ");
			Serial.println (topic_OnOffSwitch);
			#endif
		} else {
			#ifdef debug_
			Serial.println ("Subscribe FAILED");
			#endif
		}		

    } else {
	  #ifdef debug_
      Serial.print("failed, rc=");
      Serial.print(MQTTclient.state());
      Serial.println(" try again in 5 seconds");
	  #endif
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  #endif
}

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
  
  //Set the hostname
  ArduinoOTA.setHostname(hostname);
  #ifdef OTApass
  ArduinoOTA.setPassword(OTApass);
  #endif

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
		digitalWrite(PinLED, LED_on ? LOW: HIGH);  // The LED blinks during uploading...
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
	delay(2000);
	#endif

	// This is the initial state
	State = InitialState;
	LastState = delayed_powering_relay_off;    //Ensures InitialState is ALWAYS considered in the "loop".
	State_transition_checking_ok = true;	   // Is set "after" the LED switches "off", to avoid that the LED is on, while timing out for Pi checking.


    OctoprintTimer = LastPressed = PowerOffRequested = TimerLED = 0;
	WifiBeginDone = millis() - CheckWifiState - 1; // This ensures the wifi is initialized immediately.
	OctoprintInterval = OctoprintInterval_running;
	IntervalLED = 1000;  // Ensures that ofr one second, the LED stays off;
	LED_on = false;

	while ( WiFi.status()!= WL_CONNECTED) {
		delay(500);
	}
	
	#ifdef mqtt_server	
	MQTTclient.setServer(mqtt_server, mqtt_port);
	MQTTclient.setCallback(callback);
	
	if (!MQTTclient.connected()) {
		reconnect();
	}

	// If for some reason we reconnect, we always want to be mqtt state to reflect ON, since obviously the relay is on...
	if (MQTTclient.publish(topic_OnOffSwitch,"ON")) {
		#ifdef debug_
		Serial.print ("Published: [ON] to ");
		Serial.println (topic_OnOffSwitch);
	} else {
		Serial.println ("Publish ON: FAILED");
		#endif 
	}			
	
	// ... and subscribe
	if (MQTTclient.subscribe(topic_OnOffSwitch)) {
		#ifdef debug_
		Serial.print ("Subscribed ");
		Serial.println (topic_OnOffSwitch);
	} else {
		Serial.println ("Subscribe FAILED");
		#endif
	}		

	snprintf (msg, MSG_BUFFER_SIZE, "%s v%i.%i #%i",hostname,Version_major,Version_minor, ++Status_counter);
	if (MQTTclient.publish(topic_OctoPlugout,msg)) {
		#ifdef debug_		
		Serial.println ("I am alive published");
	} else {
		Serial.println ("I am alive publishing FAILED");
		#endif
	}
	#endif

	// Show information on the Serial port
	#ifdef debug_
	Serial.print("Ready: version v");
	Serial.print(Version_major);
	Serial.print(".");
	Serial.println(Version_minor);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	#endif
}


void loop() {
	
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

	#ifdef mqtt_server
	if (!MQTTclient.connected()) {
		reconnect();
	}

	if (!MQTTclient.loop()) {
		#ifdef debug_
		Serial.print("Calling MQTT Loop FAILED");
		#endif
	}
	#endif

	ArduinoOTA.handle();
		
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
				State_transition_checking_ok = true;	// Ensures the LED is OFF while checking the state, only necessary 
														// for people who configure a state without LED on that should interpret Pi state.
			}
		} else {				//Set the time the LED should be OFF
			IntervalLED=LED_OFF_TIME[State];
			State_transition_checking_ok = true;	// Ensures the LED is OFF while checking the state.
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

	// Initialize message on LCD
	char Message[40];
	Message[0] = '\0';
	
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
			} else {
				State = Switched_ON;
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
		case print_started : // 9			
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
		case delayed_powering_relay_off : // 7
			if (ShortPress) {
				State = Switched_ON;
			} else if (LongPress) {
				State = Relay_off;
			}
			break;
		default:				// Do nothing
			break;
		};

		
	// Now interpret every 15 or 5 (configurable) seconds whether the states need to change because of Octoprint statistics
	} else if (((Now - OctoprintTimer) > OctoprintInterval) and State_transition_checking_ok) {
		#ifdef debug_
		Serial.println("Checking State transitions");
		#endif
		OctoprintTimer = Now;
		State_transition_checking_ok = false;
		switch(State) {
		case Waiting_for_print_activity: // 2
			if (OctoprintRunning()) State = Waiting_for_print_activity_connected;
			break;		
		case Waiting_for_print_activity_connected: // 3
			if (OctoprintNotRunning(true)) State = Waiting_for_print_activity;
			else if (OctoprintPrinting()) {
				State = print_started;
				Time_print_started = Now;
			}				
			break;
		case print_started : // 9			
			if (OctoprintNotPrinting(true)) State = Waiting_for_print_activity_connected;							// 9->3
			else if (((Now - Time_print_started) > MinJobTime) and OctoprintHot()) State = ready_for_shutting_down;	// 9->4
			break;
		case ready_for_shutting_down: //4
//			if (OctoprintNotRunning()) State = delayed_powering_relay_off;											// 4->7
			if (OctoprintNotPrinting()) State = ready_for_shutting_down_extruder_HOT;  							// 4->5
			break;				
		case ready_for_shutting_down_extruder_HOT: // 5
//			if (OctoprintNotRunning()) State = delayed_powering_relay_off;											// 5->7
			if (OctoprintPrinting()) State = ready_for_shutting_down;											// 5->4
			else if (OctoprintCool()) State = shutting_down_PI;														// 5->6
			else {
				snprintf(Message,40,message_temperature,MaxExtruderTemperature);
				api.octoPrintPrinterCommand(Message);
			}
			break;				
		case shutting_down_PI : // 6
			if (OctoprintRunning()) {
				if (OctoprintShutdown()) {																			// 6->7 (a)
					State = delayed_powering_relay_off;
					PowerOffRequested = Now;
				}
			} else {																								// 6->7 (b)
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
		default:				// Do nothing
			break;
		}
	}
	// Now interpret states that need to change independently of button presses, wifi or Octoprint statistics
	
	switch(State) {
	case delayed_powering_relay_off:
		if ((Now - PowerOffRequested) > WaitPeriodForShutdown ) {

			// One "final check" whether Octoprint has come alive again...
			if (OctoprintRunning(false)) {State = Switched_ON;
			} else State = Relay_off;  // If not... poweroff the printer and Pi!
		}
		break;
	default:				// Do nothing
		break;
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
	default:				// Do nothing
		break;
	}

	#ifdef mqtt_server
	if (ConsiderMQTT) {
		ConsiderMQTT=false;
		State=StateMQTT;
	}
	#endif
	
	//Take action for State changes
	if (State != LastState ) {
		// Only do an action "ONCE"
		LastState = State;

		//Set the relay
		digitalWrite(PinRelay, (State == Relay_off) ? LOW : HIGH);

		#ifdef mqtt_server
		// Publish new state
		MQTTclient.publish(topic_OctoPlugout,OctoplugoutState(State).c_str());
		#endif
			
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

		//Set the next OctoprintInterval, SHORTEN the state change and display messages.
		switch(State) {
		case Switched_ON: // 1
			snprintf(Message,30,message_NoMonitor);
			api.octoPrintPrinterCommand(Message);
			break;
		case Waiting_for_print_activity_connected: // 3
			snprintf(Message,30,Message_startup,Version_major,Version_minor);
			api.octoPrintPrinterCommand(Message);
			//delay(1000);  // Wait at least 1000 ms, before putting another state
			OctoprintInterval = 200; // Poll faster (only ONCE)
			break;
		case ready_for_shutting_down: // 4
			snprintf(Message,30,Message_announce_power_off);
			api.octoPrintPrinterCommand(Message);
			OctoprintInterval = 200; // Poll faster (only ONCE)
			break;
		case shutting_down_PI: // 6
			snprintf(Message,30,message_poweroff,WaitPeriodForShutdown/1000);
			api.octoPrintPrinterCommand(Message);
			OctoprintInterval = 200; // Poll faster (only ONCE)
			break;
		case print_started: // 9
			snprintf(Message,30,Message_announce_print);
			api.octoPrintPrinterCommand(Message);
			break;
		case delayed_powering_relay_off : // 7		
		case going_down: // 8		
			OctoprintInterval = 200; // Poll faster (only ONCE)
			break;		
		default:				// Do nothing
			break;
		}
	}
	
	// Report the position through mqtt
	#ifdef mqtt_server
	switch (State) {
	case Relay_off:
	case Switched_ON:
	case shutting_down_PI:
	case delayed_powering_relay_off:
	case going_down:
		break;
	case Waiting_for_print_activity:
	case Waiting_for_print_activity_connected:
	case ready_for_shutting_down:
	case ready_for_shutting_down_extruder_HOT:  
	case print_started:
		if (WiFi.status() == WL_CONNECTED) {
			if (Now - TimeMQTTReported > mqtt_UpdateInterval) {
				TimeMQTTReported = Now;
				if(api.getPrinterStatistics()){
					#ifdef topic_PrintState
					MQTTclient.publish(topic_PrintState, 	api.printerStats.printerState.c_str());
					#endif
					#ifdef topic_ExtruderTemp
					snprintf (msg, MSG_BUFFER_SIZE, "%.1f",api.printerStats.printerTool0TempActual);
					MQTTclient.publish(topic_ExtruderTemp, msg);
					#endif
					#ifdef topic_BedTemp
					snprintf (msg, MSG_BUFFER_SIZE, "%.1f",api.printerStats.printerBedTempActual);
					MQTTclient.publish(topic_BedTemp, msg);
					#endif
				}	
				if(api.getPrintJob())	{  //Get the print job API endpoint
					#ifdef topic_JobState
					MQTTclient.publish(topic_JobState,api.printJob.printerState.c_str());
					#endif
					#ifdef topic_JobProgress
					snprintf (msg, MSG_BUFFER_SIZE, "%.1f",api.printJob.progressCompletion);
					MQTTclient.publish(topic_JobProgress, msg);
					#endif
				}
			}
		}
		break;
	}
	#endif
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
	if (State == Relay_off) return false;
	
	if (WiFi.status() == WL_CONNECTED) {
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
			} else if (api.printerStats.printerStatepausing == 1) {
				#ifdef debug_
				Serial.println("Octoprint is PAUSING");
				#endif
				return true;			
			} else if (api.printerStats.printerStateerror == 1) {
				#ifdef debug_
				Serial.println("Octoprint is Error");
				#endif
				return true;
			} else if (api.printerStats.printerStateclosedOrError == 1) {
				#ifdef debug_
				Serial.println("Octoprint is closedOrError");
				#endif
				return true;			
			} else if (api.printerStats.printerStatefinishing == 1) {
				#ifdef debug_
				Serial.println("Octoprint is finishing");
				#endif
				return true;
			} else if (api.printerStats.printerStateresuming == 1) {
				#ifdef debug_
				Serial.println("Octoprint is resuming");
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
		if (api.getPrinterStatistics()) {
			return (true);
		} else {
			#ifdef debug_
			Serial.println("Octoprint not running");
			#endif
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
	return not OctoprintTemperatureTest(MinExtruderTemperature, not Default);
}

bool OctoprintCool(bool Default) {
	return OctoprintTemperatureTest(MaxExtruderTemperature, Default);
}
	
bool OctoprintTemperatureTest(float ExtruderTemp, bool Default)
{
	#ifdef debug_
	Serial.println("TEST whether extruder is cold");
	#endif
	
	if (State == Relay_off) return true;
	
	if (WiFi.status() == WL_CONNECTED) {
		if (api.getPrinterStatistics()) {
			#ifdef debug_
			Serial.print("Printer Temp - Tool0 (°C):  ");
			Serial.println(api.printerStats.printerTool0TempActual);
			#endif
			return (api.printerStats.printerTool0TempActual < ExtruderTemp);
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
