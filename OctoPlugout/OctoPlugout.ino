 /*  OctoPlugout      
 */

#define Version_major 4
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
 * v3.2 - 7 sep 2021
 * - Added optional defines to invert working of relay and/or led.
 * - Simplified logic for switch, it is reset to '-' after interpretation
 *
 * v4.0 - 9 sep 2021
 * - Added functionality to configure parameters and WiFi through webpage http://192.168.4.1
 * 
 * - Long press (more than 6 seconds) will open a wp portal on Wifi access point "SetupOctoPLugout"
 *   (no credentials needed). When connected, browse to http://192.168.4.1 and set parameters for 
 *   IP address of your printer, its API string, mqtt server/topic root/mqtt user/mqtt password
 *   AND choose your wiFi and enter the password.
 * 
 v4.1 - 16 sep 2021
 * - To support Wifi / Web portal / mqtt / OTA /debugging the memory size became more important to manage.
 *
 *   - Arduino IDE users must select "1Mb / 128k FS OTA:~438KB" under "flash size" in the "tools" menu. 
 *   - platformio users should load the attached flash definitio in edge.flash.1m128.ld by
 *     entering a line with board_build.ldscript = eagle.flash.1m128.ld in their platformio.ini file.
 *	   The file "eagle.flash.1m128.ld" is added for your convenience as of this v4.0 of OctoPlugout.
 *
 * If this is FS 128k forgotten one OTA update will succeed, but subsequent updates might be required 
 * to use serial. Also the config portal will not work in a stable manner.
 * An indicator is that the update will not accept OTA Auth request and reply with 
 *
 * "[ERROR]: No Answer to our Authentication"
 *
 * Selecting 128k resolved this for my Sonoff S26.
 *=============================================================================================
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
 *  WiFimanager: By Tzapu https://github.com/tzapu/WiFiManager.git (or arduino libray manager)
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

#ifdef def_mqtt_server
// Includes for MQTT
#include <PubSubClient.h>
#endif

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include <EEPROM.h>

//WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
WiFiManager wm;

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

#ifdef def_mqtt_server
WiFiClient MQTT_client; 

PubSubClient MQTTclient(MQTT_client);

unsigned long TimeMQTTReported;

#define MAX_TOPIC_LEN	(60)
char Topic[MAX_TOPIC_LEN];
#define MSG_BUFFER_SIZE	(40)
char msg[MSG_BUFFER_SIZE];

bool mqtt_active = true;	//By setting the mqtt_server to "" (or "none") you can skip mqtt functionality.
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
  print_started,
  nothing
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

// Helper to force loop() to reconnect, even when WiFi.status() == WL_CONNECTED
bool Force_reconnect = false;

state State;
state LastState;
bool  State_transition_checking_ok;

OctoprintApi* api=0;

// Set the length in bytes
#define L_salt			 2
#define L_octopi_api	32
#define L_octopi_ip  	 4
#define L_mqtt_server	40
#define L_mqtt_topic	52
#define L_mqtt_user		16
#define L_mqtt_pass		16
#define L_mqtt_port		 2	

#define S_salt			0
#define S_octopi_api 	0 + S_salt   	  + L_salt
#define S_octopi_ip 	1 + S_octopi_api  + L_octopi_api
#define S_mqtt_server   0 + S_octopi_ip   + L_octopi_ip
#define S_mqtt_topic	1 + S_mqtt_server + L_mqtt_server
#define S_mqtt_user		1 + S_mqtt_topic  + L_mqtt_topic
#define S_mqtt_pass		1 + S_mqtt_user   + L_mqtt_user
#define S_mqtt_port		1 + S_mqtt_pass   + L_mqtt_pass
#define S_NEXT			0 + S_mqtt_port   + L_mqtt_port

IPAddress ip_octopi_ip;
char s_octopi_api[L_octopi_api+1];

#ifdef def_mqtt_server
char s_mqtt_server[L_mqtt_server+1];
char s_mqtt_topic[L_mqtt_topic+1];
char s_mqtt_user[L_mqtt_user+1];
char s_mqtt_pass[L_mqtt_pass+1];
short i_mqtt_port;

state StateMQTT = nothing;			// Callback will set this, loop() will use it.
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
unsigned long CrazyLED;

// The state of the info LED
#if LED_blink_flash==true
int LED_count;
#endif
bool LED_on;
bool CrazyLED_on;

//Button state variables.
bool ButtonPressed = false;
bool LongPress = false;
bool ShortPress = false;

char *OctoplugoutState(state OctoPO_State) {
	switch (OctoPO_State) {
	case Relay_off: 							return((char *)"Relay_off");
	case Switched_ON:							return((char *)"Switched_ON");
	case Waiting_for_print_activity: 			return((char *)"Waiting_for_print_activity");
	case Waiting_for_print_activity_connected: 	return((char *)"Waiting_for_print_activity_connected");
	case ready_for_shutting_down:				return((char *)"ready_for_shutting_down");
	case ready_for_shutting_down_extruder_HOT:	return((char *)"ready_for_shutting_down_extruder_HOT");  
	case shutting_down_PI:						return((char *)"shutting_down_PI");
	case delayed_powering_relay_off:			return((char *)"delayed_powering_relay_off");
	case going_down:							return((char *)"going_down");
	case print_started:							return((char *)"print_started");
	case nothing:								return((char *)"nothing");
	};
	
	return ((char *)"");
}


#ifdef def_mqtt_server
void MQTTcallback(char* topic, byte* payload, unsigned int length) {
	if (mqtt_active) {
		#ifdef debug_
		Serial.print(F("Message arrived ["));
		Serial.print(topic);
		Serial.print("] [");
		for (unsigned int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
		}
		Serial.println("]");
		#endif
		// Set State following the message received.
		std::string TopicSwitch = s_mqtt_topic;
		TopicSwitch += topic_OnOffSwitch;
		std::string TopicReceived = topic;

	/*
		strcpy(TopicSwitch,s_mqtt_topic);
		strcat(TopicSwitch,topic_OnOffSwitch);
	*/	
		if (TopicReceived == TopicSwitch) {
			payload[length] = '\0'; // NULL terminate the array
			if ((strcmp("ON", (char *)payload) == 0) or (strcmp("AFTERJOB", (char *)payload) == 0)) {
				StateMQTT = Waiting_for_print_activity;
			} else if (strcmp("PERMON", (char *)payload) == 0) {
				StateMQTT = Switched_ON;
			} else if (strcmp("OFF", (char *)payload) == 0) {
				StateMQTT = ready_for_shutting_down;
			} else if (strcmp("FORCEOFF", (char *)payload) == 0) {
				StateMQTT = Relay_off;
			} else {
				#ifdef debug_
				//Serial.println(F("No message detected, received: "));
				//Serial.println((char *)payload);
				#endif		
			}			
		} else {
			#ifdef debug_
			//Serial.println(F("No topic detected"));
			//Serial.print(TopicSwitch.c_str());
			//Serial.println("<<<");
			//Serial.print(TopicReceived.c_str());
			//Serial.println("<<<");
			#endif
		}
	}		
}
#endif

bool reconnect_mqtt() {
	#ifdef def_mqtt_server
	if (mqtt_active) {
		MQTTclient.setServer(s_mqtt_server, i_mqtt_port);
		MQTTclient.setCallback(MQTTcallback);

		// Try (just once) to reconnected if necessary
		if (MQTTclient.connected()) {
			#ifdef debug_
			//Serial.print(F("reconnect_mqtt: MQTT CONNECTED *****************"));
			#endif
			  return true;
		} else {
			#ifdef debug_
			//Serial.print(F("Attempting MQTT connection..."));
			#endif

			// Create a random MQTTclient ID
			String clientId = "OCTOPLUGOUT-";
			clientId += String(random(0xffff), HEX);
			WiFi.mode(WIFI_STA);
			WiFi.hostname(op_hostname);

			delay(100);
			// Attempt to connect
			if (MQTTclient.connect(clientId.c_str(),s_mqtt_user,s_mqtt_pass)) {
				#ifdef debug_
				Serial.print(clientId);
				Serial.print(F(" connected to MQTT server "));
				Serial.print(s_mqtt_user);
				#endif
				// Resubscribe
				
				if (SubscribeMQTT(s_mqtt_topic,(char *)topic_OnOffSwitch)) {
					#ifdef debug_
					Serial.print (F("Subscribed "));
					Serial.print (s_mqtt_topic);
					#endif
				} else {
					#ifdef debug_
					//Serial.println (F("Subscribe FAILED"));
					#endif
				}

				return true;

			} else {
				#ifdef debug_
				Serial.print(F("failed, rc="));
				Serial.println(MQTTclient.state());
				Serial.println(s_mqtt_server);
				//Serial.println(s_mqtt_user);
				//Serial.println(s_mqtt_pass);
				//Serial.println(s_mqtt_topic);
				//Serial.println(" try again in next loop");
				#endif

				return false;
			}
		}
		#endif
	} else return false;
}

// SALT is save to eeprom, if it is wrong, it assumed to be corrupted and reinitialized
#ifdef def_mqtt_server		
	#define SALT 5930
	#define L_EEPROM  7 + 2 + L_octopi_api + L_octopi_ip + L_mqtt_server + L_mqtt_user + L_mqtt_pass + L_mqtt_topic + L_mqtt_port
#else
	#define SALT 5929	// By making SALTs different, EEPROM is reinitialized when mqtt_server mode is selected.
	#define L_EEPROM  2 + 2 + L_octopi_api + L_octopi_ip
#endif

void EEPROMputs(int strEEPROM, const char *save) {
	byte i = 0;
	while (byte(save[i])!=0) {
		EEPROM.put(strEEPROM + i,(byte)(save[i]));
		i++;
	}
	EEPROM.put(strEEPROM + i,(byte)(0));
}
void EEPROMputIP(int strEEPROM, IPAddress ipsave, byte size) {
	for (byte i=0;i<size;i++) EEPROM.put(strEEPROM + i,ipsave[i]);
}

void eeprom_saveconfig()
{
	short salt = SALT;
	EEPROM.begin(L_EEPROM);
	EEPROM.put(0, salt);
	EEPROMputs(S_octopi_api, s_octopi_api);
	EEPROMputIP(S_octopi_ip, ip_octopi_ip, L_octopi_ip);  
	#ifdef def_mqtt_server		  
	EEPROMputs(S_mqtt_server, s_mqtt_server);
	EEPROMputs(S_mqtt_topic, s_mqtt_topic);
	EEPROMputs(S_mqtt_user, s_mqtt_user);
	EEPROMputs(S_mqtt_pass, s_mqtt_pass);
	EEPROM.put(S_mqtt_port, i_mqtt_port);
	#endif
	EEPROM.commit();
	EEPROM.end();
	if (((byte)s_mqtt_server[0]==0) or (strcmp(s_mqtt_server,"none")==0)) {
		mqtt_active = false;
	} else {
		mqtt_active = true;
	}	
}

void eeprom_read() 
{
	short salt;
	EEPROM.begin(L_EEPROM);
	EEPROM.get(0, salt);
	//Serial.print(F("Comparing "));
	//Serial.print(salt);
	//Serial.print(" with ");
	//Serial.println(SALT);
	
	if (salt == SALT) {
		//Serial.println(F("Reading EEPROM")); 
		EEPROM.get(S_octopi_api, s_octopi_api);
		for (byte i=0;i<4;i++) EEPROM.get(S_octopi_ip + i,ip_octopi_ip[i]);
		#ifdef def_mqtt_server		
		EEPROM.get(S_mqtt_server, s_mqtt_server);
		EEPROM.get(S_mqtt_topic, s_mqtt_topic);
		EEPROM.get(S_mqtt_user, s_mqtt_user);
		EEPROM.get(S_mqtt_pass, s_mqtt_pass);
		EEPROM.get(S_mqtt_port, i_mqtt_port);
		if (((byte)s_mqtt_server[0]==0) or (strcmp(s_mqtt_server,"none")==0)) {
			mqtt_active = false;
		} else {
			mqtt_active = true;
		}		
		#endif
		
		#ifdef debug_
		Serial.print(F("Retrieved from EEPROM"));
		#endif
		
		EEPROM.end();
	} else {
		
		EEPROM.end();
		//Serial.println(F("Initializing EEPROM from defaults"));
		strcpy (s_octopi_api, def_octoprint_apikey);
		if (not ip_octopi_ip.fromString(def_octoprint_ip)) {
			#ifdef debug_
			//Serial.print(F("ip.Fromstring failed: "));
			//Serial.println(F(def_octoprint_ip));
			#endif
		} else {
			#ifdef debug_
			//Serial.print(F("IP addres: "));
			//Serial.println(ip_octopi_ip);
			#endif
		}
		#ifdef def_mqtt_server		
		strcpy (s_mqtt_server, def_mqtt_server);
		strcpy (s_mqtt_topic, def_mqtt_topic);
		strcpy (s_mqtt_user, def_mqtt_user);
		strcpy (s_mqtt_pass, def_mqtt_pass);
		i_mqtt_port = def_mqtt_port;
		#endif
		eeprom_saveconfig();
		
		if (((byte)s_mqtt_server[0]==0) or (strcmp(s_mqtt_server,"none")==0)) {
			mqtt_active = false;
		} else {
			mqtt_active = true;
		}		
		
	};	  	  
}

bool PublishMQTT(const char* topicRoot, const char* topic, const char* msg) {
	char Topic[60];
	strcpy(Topic,topicRoot);
	strcat(Topic,topic);
	return MQTTclient.publish(Topic,msg);
}

bool SubscribeMQTT(const char* topicRoot, const char* topic) {
	char Topic[60];
	strcpy(Topic,topicRoot);
	strcat(Topic,topic);
	return MQTTclient.subscribe(Topic);
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
			//Serial.print("Blink: ");
			//Serial.println(j);
			#endif
			delay(300);
			digitalWrite(PinLED, LOW);  // The LED is on...
			delay(50);
			digitalWrite(PinLED, HIGH);  // The LED is off...
		}
	}
	delay(2000);
	#endif



	/* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
	 would try to act as both a client and an access-point and could cause
	 network-issues with your other WiFi-devices on your WiFi-network. */
	WiFi.mode(WIFI_STA);
	

	//Set the hostname
	ArduinoOTA.setHostname(op_hostname);
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


  });
  
  ArduinoOTA.onEnd([]() {
	Serial.println("\nEnd");
	#ifdef PinLED
	LED_on = false;
	digitalWrite(PinLED, LED_on ? LOW: HIGH);  // The LED is off...
	#endif
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
	#ifdef PinLED
	// switch off all the PWMs during upgrade
	analogWrite(PinLED, 0);
	#endif
  
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	#ifdef PinLED
		if ((LED_count++ % 2) == 0) LED_on = not LED_on;
		digitalWrite(PinLED, LED_on ? LOW: HIGH);  // The LED is blinks during uploading...
	#endif
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
	Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println(F("Auth Failed"));
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println(F("Begin Failed"));
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println(F("Connect Failed"));
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println(F("Receive Failed"));
    } else if (error == OTA_END_ERROR) {
      Serial.println(F("End Failed"));
    }
  });
  ArduinoOTA.begin();  

//Initial state and timers for OctoPrintPlugout


	// This is the initial state
	State = InitialState;
	LastState = delayed_powering_relay_off;    //Ensures InitialState is ALWAYS considered in the "loop".
	State_transition_checking_ok = true;	   // Is set "after" the LED switches "off", to avoid that the LED is on, while timing out for Pi checking.


    OctoprintTimer = LastPressed = PowerOffRequested = TimerLED = 0;
	WifiBeginDone = millis() - CheckWifiState - 1; // This ensures the wifi is initialized immediately.
	OctoprintInterval = OctoprintInterval_running;
	IntervalLED = 1000;  // Ensures that ofr one second, the LED stays off;
	LED_on = false;

    //reset settings - wipe credentials for testing
    //wm.resetSettings();
	
	//read from EEPROM, these are the defaults (for the non-wifi parameters)
	eeprom_read();
	
	#ifdef debug_
	Serial.println(F("[EEPROM] settings retrieved:"));
	Serial.print(F("O_API          = "));
	Serial.println(s_octopi_api);
	Serial.print(F("PARAM O_IP     = "));
	Serial.println(ip_octopi_ip);
	Serial.print(F("PARAM M_server = "));
	Serial.println(s_mqtt_server);
	Serial.print(F("PARAM M_topic  = "));
	Serial.println(s_mqtt_topic);
	Serial.print(F("PARAM M_user   = "));
	Serial.println(s_mqtt_user);
	Serial.print(F("PARAM M_pass   = "));
	Serial.println(s_mqtt_pass);
	Serial.print(F("PARAM M_port   = "));
	Serial.println(i_mqtt_port);
	#endif

	// wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails
	
	
	// Force setting up cnnection in loop(), as WiFi.Status will shown WL_CONNECTED due to persistent login
	Force_reconnect = true;
	
	// Show information on the Serial port
	#ifdef debug_
	wm.setDebugOutput(true);

	//Serial.print(F("Ready: version v"));
	//Serial.print(Version_major);
	//Serial.print(".");
	//Serial.println(Version_minor);
	#else
	wm.setDebugOutput(false);

	#endif
}

bool reconnect_handle_autoconnect() {
	
	//wm.setCountry("NL"); 
	
	// set Hostname
	//wm.setHostname("OctoPlugoutConfig");

	// show password publicly in form
	//wm.setShowPassword(true);

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result
	
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
	
	//Serial.println(F("going to autoconnect"));
	WiFiManagerParameter octopi_api("O_API", "octopi API", s_octopi_api, L_octopi_api);
	WiFiManagerParameter octopi_ip("O_IP", "octopi ip", ip_octopi_ip.toString().c_str(), L_octopi_ip*4);

	#ifdef def_mqtt_server			
	WiFiManagerParameter mqtt_server("M_server", "mqtt server", s_mqtt_server, L_mqtt_server);
	WiFiManagerParameter mqtt_topic("M_topic", "mqtt topic", s_mqtt_topic, L_mqtt_topic);
	WiFiManagerParameter mqtt_user("M_user", "mqtt user", s_mqtt_user, L_mqtt_user);
	WiFiManagerParameter mqtt_pass("M_pass", "mqtt password", s_mqtt_pass, L_mqtt_pass);
	sprintf(msg, "%d", i_mqtt_port);
	WiFiManagerParameter mqtt_port("M_port", "mqtt port", msg, MSG_BUFFER_SIZE);
	#endif
	
	wm.addParameter(&octopi_ip);	
	wm.addParameter(&octopi_api);	

	#ifdef def_mqtt_server		
	wm.addParameter(&mqtt_server);
	wm.addParameter(&mqtt_port);
	wm.addParameter(&mqtt_user);
	wm.addParameter(&mqtt_pass);
	wm.addParameter(&mqtt_topic);
	#endif
	
    //wm.setConfigPortalBlocking(true);
    wm.setSaveParamsCallback(saveParamCallback);
	
	std::vector<const char *> menu = {"wifi","info","sep","restart","exit"};
//	std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
	wm.setMenu(menu);
	
	// set dark theme
	wm.setClass("invert");	

	wm.setConfigPortalTimeout(180); // auto close configportal after n seconds
	//wm.setCaptivePortalEnable(false); // disable captive portal redirection
	wm.setTimeout(60); // Wait in config portal, before trying the original wifi again.
	//wm.setCaptivePortalEnable(false); // disable captive portal redirection
	wm.setAPClientCheck(true); // avoid timeout if client connected to softap	
	
    bool res = wm.autoConnect("SetupOctoPlugout"); // password protected ap
	//WiFi.begin("user","pass"); When the autoconnect fails....
	//bool res = true;
	
	//if not connected: reboot
    if (!res) {
        Serial.println(F("Failed to connect, restarting"));
        ESP.restart();
		while(true); //Do not continue....
    } 
    else {
        //if you get here you have connected to the WiFi    
        #ifdef debug_
		Serial.println(F("connected...yeey"));
		#endif
    }
	
	// You only need to set one of the of following, but note: I COULD NOT GET THE HOSTNAME TO WORK, use IP address!
	#ifdef UseIP
		api = new OctoprintApi(API_client, ip_octopi_ip, octoprint_httpPort, s_octopi_api);				  // When using IP Address
	#else
		char* octoprint_host = octoprintHost;    				// Or your hostname. Comment out one or the other.
		api = new OctoprintApi(API_client, octoprint_host, octoprint_httpPort, s_octopi_api);   // When using hostname 
	#endif		
	return true;
}

String getParam(String name){
  //read parameter from server, for customhmtl input
  String value;
  if(wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void saveParamCallback(){
	#ifdef debug_
	Serial.println(F("[CALLBACK] saveParamCallback fired"));
	#endif

	strcpy(s_octopi_api,getParam("O_API").c_str());
	ip_octopi_ip.fromString(getParam("O_IP").c_str());
	#ifdef def_mqtt_server
	/*
	strcpy(s_mqtt_server,mqtt_server.getValue().c_str());
	strcpy(s_mqtt_topic,mqtt_topic.getValue().c_str());
	strcpy(s_mqtt_user,mqtt_user.getValue().c_str());
	strcpy(s_mqtt_pass,mqtt_pass.getValue().c_str());
	*/
	strcpy(s_mqtt_server,getParam("M_server").c_str());
	strcpy(s_mqtt_topic,getParam("M_topic").c_str());
	strcpy(s_mqtt_user,getParam("M_user").c_str());
	strcpy(s_mqtt_pass,getParam("M_pass").c_str());
	i_mqtt_port = getParam("M_port").toInt();
	#endif

	#ifdef debug_
	Serial.print(F("octopi server @: "));
	Serial.println(ip_octopi_ip);
	#endif
	
	eeprom_saveconfig();
}

void loop() {
	
	unsigned long Now = millis();
	
	bool mqtt_connected = false;
	
	//Always try Wifi state and if necessary connect to wifi..	
	if (WiFi.status() == WL_CONNECTED and !Force_reconnect) {
		#ifdef debug_
		//Serial.println(F("In loop: Wifi was connected"));
		#endif		//
	} else {
		// Now that it is not connected:
		//    Attempt to initialize WIFI... ONLY once every minute....
		//    This is useful if the Wifi is "lost" and reappears.	
		if ((Now - WifiBeginDone) >= CheckWifiState) {
			#ifdef debug_
			//Serial.println(F("Autoconnecting to WIFI"));
			#endif

			Force_reconnect = false; //Only force once...
			
			if (reconnect_handle_autoconnect()) {
				//Serial.println(F("Now connected to WIFI"));
			} else {
				//Serial.println(F("Connect FAILED restarting..."));
				ESP.restart();
				while(true); //Do not continue....
			}
	
			WifiBeginDone = Now;

			#ifdef debug_
			Serial.print(F("Wifi connected, IP address: "));
			Serial.println(WiFi.localIP());
			#endif

		} else {
			#ifdef debug_
			//Serial.println(F("Wifi disconnected: Wifi connecting was skipped"));
			#endif		
		}
	}

	#ifdef def_mqtt_server
	if (mqtt_active) {
		if (MQTTclient.connected()) {
			//Serial.println(F("In loop: mqtt is CONNECTED"));
			mqtt_connected = true;
		} else {
			#ifdef debug_
			//Serial.print(F("In loop: mqtt is DISCONNECTED"));
			#endif
			mqtt_connected = reconnect_mqtt();
		}

		if (mqtt_connected) {
			if (!MQTTclient.loop()) {
				#ifdef debug_
				//Serial.print(F("Calling MQTT Loop FAILED"));
				#endif
			}
		}
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
		#ifdef INVERT_LED
		digitalWrite(PinLED, LED_on ? HIGH: LOW);	// Actually switch the LED on or Off, depending on LED_on.
		#else
		digitalWrite(PinLED, LED_on ? LOW: HIGH);	// Actually switch the LED on or Off, depending on LED_on.
		#endif
	}
	
	// Evaluate button: debouncing after it is pressed
	
	#define CrazyLED_int 200
	ShortPress = LongPress = false; // Reset
	if ((Now - LastPressed) > debounce_interval) {
		if (ButtonPressed) {
			if (digitalRead(PinButton) == HIGH) {            //This means the button is (now) released
				digitalWrite(PinLED, HIGH);					// Switch led off, in case we were "Crazy Blinking"
				if ((Now - LastPressed) > reset_press_time) { // Checking whether it is a short, or RESET button press
					wm.resetSettings();
					delay(1000);
					ESP.restart();
					while(true); //Do not continue....
					
				} else if ((Now - LastPressed) > long_press_time) { // Checking whether it is a short, or long button press
					LongPress = true;
				} else {
					ShortPress = true;
				}
				ButtonPressed = false;
				LastPressed = Now; 				// For debouncing
			} else {							// Button pressed, give sign that we passed the resettime
				if ((Now - LastPressed) > reset_press_time) {
					// Do some crazy blinking while you keep it pressed
					if ((Now - CrazyLED) >= CrazyLED_int) {
						CrazyLED = Now;
						CrazyLED_on = !CrazyLED_on;
						digitalWrite(PinLED, CrazyLED_on ? LOW: HIGH);
					}					
				}
			}				
			
		} else {
			if (digitalRead(PinButton) == LOW) { 			//This means the button is (now) pressed
				ButtonPressed = true;
				LastPressed = Now; 				 			// For debouncing
			}
		}
	}

	// Initialize message on LCD of the printer
	char Message[40];
	Message[0] = '\0';
	
	// Interpret buttons immediately when detected (after release), actions involving transitions and printer job every 5 seconds
	if (ShortPress or LongPress) {
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
		Serial.println(F("Checking State transitions"));
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
				api->octoPrintPrinterCommand(Message);
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

	#ifdef def_mqtt_server
	if (mqtt_active) {
		if (mqtt_connected) {
			if (StateMQTT != nothing) {
				// reset, so the On/Off switch is not taken into account a second time for whatever reason
				PublishMQTT(s_mqtt_topic,topic_OnOffSwitch,"-");
				State=StateMQTT;
				StateMQTT=nothing;
			}
		}
	}
	#endif
	
	//Take action for State changes
	if (State != LastState ) {
		
		#ifdef debug_
		Serial.print(F("State changed from "));
		Serial.print(OctoplugoutState(LastState));
		Serial.print(" to ");
		Serial.println(OctoplugoutState(State));
		//Serial.print(F(" LED on/off: "));
		//Serial.print(LED_ON_TIME[State]);
		//Serial.print("/");
		//Serial.println(LED_OFF_TIME[State]);

	#endif
		
		// Only do an action "ONCE"
		LastState = State;

		//Set the relay
		#ifdef INVERT_RELAY
		digitalWrite(PinRelay, (State == Relay_off) ? HIGH : LOW);
		#else
		digitalWrite(PinRelay, (State == Relay_off) ? LOW : HIGH);
		#endif
		
		#ifdef def_mqtt_server
		// Publish new state
		if (mqtt_connected and mqtt_active) PublishMQTT(s_mqtt_topic,topic_State,OctoplugoutState(State));
		#endif
			
		// Force LED to reflect a new pattern.
		TimerLED = 0;  

		// Show debug_ information
		#ifdef debug_
		if (OctoprintInterval < OctoprintInterval_NOT_running) {
			if(api->getPrinterStatistics()){
				/*
				Serial.println(F("---------States---------"));
				Serial.print(F("Printer Current State: "));
				Serial.println(api->printerStats.printerState);
				Serial.print(F("Printer State - operational:  "));
				Serial.println(api->printerStats.printerStateoperational);
				Serial.print(F("Printer State - paused:  "));
				Serial.println(api->printerStats.printerStatepaused);
				Serial.print(F("Printer State - printing:  "));
				Serial.println(api->printerStats.printerStatePrinting);
				Serial.print(F("Printer State - ready:  "));
				Serial.println(api->printerStats.printerStateready);
				Serial.println(F("------------------------"));
				Serial.println();
				Serial.println(F("------Temperatures-----"));
				Serial.print(F("Printer Temp - Tool0 (°C):  "));
				Serial.println(api->printerStats.printerTool0TempActual);
				Serial.print(F("Printer State - Bed (°C):  "));
				Serial.println(api->printerStats.printerBedTempActual);
				Serial.println(F("------------------------"));
				*/
			}
		}
		#endif

		//Set the next OctoprintInterval, SHORTEN the state change and display messages.
		switch(State) {
		case Switched_ON: // 1
			snprintf(Message,30,message_NoMonitor);
			api->octoPrintPrinterCommand(Message);
			break;
		case Waiting_for_print_activity_connected: // 3
			snprintf(Message,30,Message_startup,Version_major,Version_minor);
			api->octoPrintPrinterCommand(Message);
			//delay(1000);  // Wait at least 1000 ms, before putting another state
			OctoprintInterval = 200; // Poll faster (only ONCE)
			break;
		case ready_for_shutting_down: // 4
			snprintf(Message,30,Message_announce_power_off);
			api->octoPrintPrinterCommand(Message);
			OctoprintInterval = 200; // Poll faster (only ONCE)
			break;
		case shutting_down_PI: // 6
			snprintf(Message,30,message_poweroff,WaitPeriodForShutdown/1000);
			api->octoPrintPrinterCommand(Message);
			OctoprintInterval = 200; // Poll faster (only ONCE)
			break;
		case print_started: // 9
			snprintf(Message,30,Message_announce_print);
			api->octoPrintPrinterCommand(Message);
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
	#ifdef def_mqtt_server
	if (mqtt_connected and mqtt_active) {
		if (WiFi.status() == WL_CONNECTED) {
			if (Now - TimeMQTTReported > mqtt_UpdateInterval) {
				TimeMQTTReported = Now;			
				switch (State) {
				case Relay_off:
				case Switched_ON:
				case shutting_down_PI:
				case delayed_powering_relay_off:
				case going_down:
				case Waiting_for_print_activity:
				case nothing:
					break;
				case Waiting_for_print_activity_connected:
				case ready_for_shutting_down:
				case ready_for_shutting_down_extruder_HOT:  
				case print_started:
					if(api->getPrinterStatistics()){
						#ifdef topic_PrintState
						PublishMQTT(s_mqtt_topic,topic_PrintState,api->printerStats.printerState.c_str());
						#endif
						#ifdef topic_ExtruderTemp
						snprintf (msg, MSG_BUFFER_SIZE, "%.1f",api->printerStats.printerTool0TempActual);
						PublishMQTT(s_mqtt_topic,topic_ExtruderTemp,msg);
						#endif
						#ifdef topic_BedTemp
						snprintf (msg, MSG_BUFFER_SIZE, "%.1f",api->printerStats.printerBedTempActual);
						PublishMQTT(s_mqtt_topic,topic_BedTemp,msg);
						#endif
					}	
					if(api->getPrintJob())	{  //Get the print job API endpoint
						#ifdef topic_JobState
						PublishMQTT(s_mqtt_topic,topic_JobState,api->printJob.printerState.c_str());				
						#endif
						#ifdef topic_JobProgress
						snprintf (msg, MSG_BUFFER_SIZE, "%.1f",api->printJob.progressCompletion);
						PublishMQTT(s_mqtt_topic,topic_JobProgress,msg);
						#endif
					}
				}
			}
		}
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
	if (WiFi.status() == WL_CONNECTED) {
		if(api->getPrinterStatistics()) {
			return api->octoPrintCoreShutdown();
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
		if(api->getPrinterStatistics()) {
			if (api->printerStats.printerStatePrinting == 1) {
				#ifdef debug_
				Serial.println(F("Octoprint is PRINTING"));
				#endif
				return true;
			} else if (api->printerStats.printerStatepaused == 1) {
				#ifdef debug_
				//Serial.println(F("Octoprint is PAUSED"));
				#endif
				return true;
			} else if (api->printerStats.printerStatepausing == 1) {
				#ifdef debug_
				//Serial.println(F("Octoprint is PAUSING"));
				#endif
				return true;			
			} else if (api->printerStats.printerStateerror == 1) {
				#ifdef debug_
				//Serial.println(F("Octoprint is Error"));
				#endif
				return true;
			} else if (api->printerStats.printerStateclosedOrError == 1) {
				#ifdef debug_
				//Serial.println(F("Octoprint is closedOrError"));
				#endif
				return true;			
			} else if (api->printerStats.printerStatefinishing == 1) {
				#ifdef debug_
				//Serial.println(F("Octoprint is finishing"));
				#endif
				return true;
			} else if (api->printerStats.printerStateresuming == 1) {
				#ifdef debug_
				//Serial.println(F("Octoprint is resuming"));
				#endif
				return true;				
			} else { 	
				Serial.println(F("Octoprint is not printing"));
				return false;
			}
		} else {
			#ifdef debug_
			Serial.println(F("(Octoprint is NOT running"));
			#endif
			return Default;
		}
	} else {
		#ifdef debug_
		//Serial.println(F("No Wifi (printer printing)"));
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
	//Serial.println(F("TEST whether Octoprint is running"));
	#endif
	if (WiFi.status() == WL_CONNECTED) {
		if (api->getPrinterStatistics()) {
			return (true);
		} else {
			#ifdef debug_
			Serial.println(F("Octoprint not running"));
			#endif
			return (false);
		}
	} else {
		#ifdef debug_
		//Serial.println(F("No Wifi (octoprint running"));
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
	Serial.println(F("TEST whether extruder is cold"));
	#endif
	
	if (State == Relay_off) return true;
	
	if (WiFi.status() == WL_CONNECTED) {
		if (api->getPrinterStatistics()) {
			#ifdef debug_
			Serial.print(F("Printer Temp - Tool0 (°C):  "));
			Serial.println(api->printerStats.printerTool0TempActual);
			#endif
			return (api->printerStats.printerTool0TempActual < ExtruderTemp);
		} else {
			return Default;
		}
	} else {
		#ifdef debug_
		Serial.println(F("No Wifi (extruder temp test"));
		#endif
		return Default;
	}
}

