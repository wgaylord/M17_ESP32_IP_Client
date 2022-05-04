# M17_ESP32_IP_Client
M17 IP only client based on  https://github.com/nakhonthai/M17AnalogGateway  

Pin 26 is the speaker output, attach a headphone bewteen it and ground to hear audio. (Definetly need to improve this.)  
Pin 36 is the mic input, needs to have an audio signal between 0 and 3.3v on it.  
Pin 32 is PTT ground it to transmit M17 to the reflector.  

The system is configured over Serial.  
The commands are as follows.  

"WifiSSID->SSID" where SSID is your SSID of your wifi  
"WifiPasss->PASS" where PASS is your Wifi Password  
"SetCall->CALL" where CALL is your callsign  
"SetRefName->NAME" where NAME is the reflector name (example M17-M17)  
"SetRefIp->0.0.0.0" where 0.0.0.0 is the IP of the reflector. Currently do not support IPv6  
"SetRefPort->17000" where 7000 is the port.   
"SetModule->C" where C is the Module letter.  
"Save" Saves current settings into EEPROM, rest the board after this to allow it to read the config and connect.  
