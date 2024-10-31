#include <TinyGPS++.h>
#include <vector>
#include "notification_utils.h"
#include "custom_characters.h"
#include "station_utils.h"
#include "configuration.h"
#include "APRSPacketLib.h"
#include "battery_utils.h"
#include "power_utils.h"
#include "menu_utils.h"
#include "msg_utils.h"
#include "gps_utils.h"
#include "wx_utils.h"
#include "display.h"
#include "utils.h"

extern int                  menuDisplay;
extern Beacon               *currentBeacon;
extern Configuration        Config;
extern TinyGPSPlus          gps;
extern std::vector<String>  loadedAPRSMessages;
extern std::vector<String>  loadedWLNKMails;
extern int                  messagesIterator;
extern uint8_t              loraIndex;
extern uint32_t             menuTime;
extern bool                 symbolAvailable;
extern bool                 keyDetected;
extern String               messageCallsign;
extern String               messageText;
extern bool                 flashlight;
extern bool                 digirepeaterActive;
extern bool                 sosActive;
extern bool                 bluetoothActive;
extern bool                 displayEcoMode;
extern bool                 screenBrightness;
extern bool                 disableGPS;
extern bool                 showHumanHeading;
extern APRSPacket           lastReceivedPacket;

extern uint8_t              winlinkStatus;
extern String               winlinkMailNumber;
extern String               winlinkAddressee;
extern String               winlinkSubject;
extern String               winlinkBody;
extern String               winlinkAlias;
extern String               winlinkAliasComplete;
extern bool                 winlinkCommentState;
extern int                  wxModuleType;
extern bool                 gpsIsActive;

String      freqChangeWarning;
uint8_t     lowBatteryPercent       = 21;


namespace MENU_Utils {

    const String MainMenuHeader =   "__MENU____";
    const String ConfigHeader =     "_CONFIG___";
    const String DisplayHeader =    "_DISPLAY__";
    const String StatusHeader =     "_STATUS___";
    const String CallsingHeader =   "_CALLSIGN_";
    const String MessagesHeader =   "_MESSAGES_";
    const String ExtrasHeader =     "__EXTRAS__";
    const String WriteMsgHeader =   "WRITE_MSG>";
    const String DeleteMsgHeader =  "DELETE_MSG";
    const String RebootHeader =     "_REBOOT?__";
    const String WlinkLoginHeader = "_WINLINK_>";
    const String WlinkMenuHeader =  "WLNK__MENU";
    const String WlinkMailHeader =  "WLNK__MAIL";
    const String WlinkAliasHeader = "WLNK_ALIAS";
    const String WlinkDelHeader =   "WLNK__DEL";
    const String WlinkReadHeader =  "WLNK__READ";
    const String WlinkForwHeader =  "WLNK__FORW";
    const String WlinkReplyHeader = "WLNK_REPLY";
    const String CommentHeader =    "_COMMENT_>";
    const String AprsThruHeader =   "APRS Thu._";
    const String StationsHeader =   "STATIONS>";
    const String PowerOffHeader =   "POWER_OFF?";
    const String BluetoothHeader =  "BLUETOOTH";
    const String NotificHeader =    "_NOTIFIC__";
    const String LoraFreqHeader =   "LORA__FREQ";

    const String MessagesMenuItem =                 "  1.Messages";
    const String MessagesMenuItemSelected =         "> 1.Messages";
    const String ConfigurationMenuItem =            "  2.Configuration";
    const String ConfigurationMenuItemSelected =    "> 2.Configuration";
    const String StationsMenuItem =                 "  3.Stations";
    const String StationsMenuItemSelected =         "> 3.Stations";
    const String WeatherMenuItem =                  "  4.Weather Report";
    const String WeatherMenuItemSelected =          "> 4.Weather Report";
    const String WinlinkMenuItem =                  "  5.Winlink/Mail";
    const String WinlinkMenuItemSelected =          "> 5.Winlink/Mail";
    const String ExtrasMenuItem =                   "  6.Extras";
    const String ExtraskMenuItemSelected =          ">  6.Extras";

    const String checkBTType() {
        switch (Config.bluetooth.type) {
            case 0:
                return "BLE iPhone";
            case 1:
                return "BT Android";
            case 2:
                return "BLE Text";
            default:
                return "no BT";
        }
    }

    const String checkProcessActive(const bool process) {
        if (process) {
            return "ON";
        } else {
            return "OFF";
        }
    }

    const String screenBrightnessAsString(const uint8_t bright) {
        if (bright == 255) {
            return "MAX";
        } else {
            return "min";
        }
    }

    void showOnScreen() {
        String lastLine, firstLineDecoder, courseSpeedAltitude, speedPacketDec, coursePacketDec, pathDec;
        uint32_t lastMenuTime = millis() - menuTime;
        if (!(menuDisplay==0) && !(menuDisplay==300) && !(menuDisplay==310) && !(menuDisplay==40) && !(menuDisplay>=500 && menuDisplay<=5100) && lastMenuTime > 30*1000) {
            menuDisplay = 0;
            messageCallsign = emptyString;
            messageText = emptyString;
        }
        if (keyDetected) {
            lastLine = "<Back Up/Down Select>";
        } else {
            lastLine = "1P=Down 2P=Back LP=Go";
        }
        switch (menuDisplay) { // Graphic Menu is in here!!!!
            case 1:     // 1. Messages
                displayShow(MainMenuHeader, ExtrasMenuItem, MessagesMenuItemSelected, ConfigurationMenuItem, StationsMenuItem, lastLine);
                break;
            case 2:     // 2. Configuration
                displayShow(MainMenuHeader, MessagesMenuItem, ConfigurationMenuItemSelected, StationsMenuItem, WeatherMenuItem, lastLine);
                break;
            case 3:     //3. Stations
                displayShow(MainMenuHeader, ConfigurationMenuItem, StationsMenuItemSelected, WeatherMenuItem, WinlinkMenuItem, lastLine);
                break;
            case 4:     //4. Weather
                displayShow(MainMenuHeader, StationsMenuItem, WeatherMenuItemSelected, WinlinkMenuItem, ExtrasMenuItem, lastLine);
                break;
            case 5:     //5. Winlink
                displayShow(MainMenuHeader, WeatherMenuItem, WinlinkMenuItemSelected, ExtrasMenuItem, MessagesMenuItem, lastLine);
                break;
            case 6:     //6. Extras
                displayShow(MainMenuHeader, WinlinkMenuItem, ExtraskMenuItemSelected, MessagesMenuItem, ConfigurationMenuItem, lastLine);
                break;

//////////
            case 10:    // 1.Messages ---> Messages Read
                displayShow(MessagesHeader, "> Read (" + String(MSG_Utils::getNumAPRSMessages()) + ")", "  Write", "  Delete", "  APRSThursday", lastLine);
                break;
            case 100:   // 1.Messages ---> Messages Read ---> Display Received/Saved APRS Messages
                {
                    String msgSender    = loadedAPRSMessages[messagesIterator].substring(0, loadedAPRSMessages[messagesIterator].indexOf(","));
                    String msgText      = loadedAPRSMessages[messagesIterator].substring(loadedAPRSMessages[messagesIterator].indexOf(",") + 1);

                    #ifdef HAS_TFT
                        displayMessage(msgSender, msgText, 26, true);
                    #else
                        displayShow("MSG_APRS>", "From --> " + msgSender, msgText, emptyString, emptyString, "           Next=Down");
                    #endif                   
                }
                break;
            case 11:    // 1.Messages ---> Messages Write
                displayShow(MessagesHeader, "  Read (" + String(MSG_Utils::getNumAPRSMessages()) + ")", "> Write", "  Delete", "  APRSThursday", lastLine);
                break;
            case 110:   // 1.Messages ---> Messages Write ---> Write
                if (keyDetected) {
                    displayShow(WriteMsgHeader, emptyString, "CALLSIGN = " + String(messageCallsign), emptyString, emptyString, "<Back          Enter>");
                } else {
                    displayShow(WriteMsgHeader, emptyString, "No Keyboard Detected", "Can't write Message", emptyString, "1P = Back");           
                }     
                break;
            case 111:
                if (messageText.length() <= 67) {
                    if (messageText.length() < 10) {
                        displayShow(WriteMsgHeader, "CALLSIGN -> " + messageCallsign, "MSG -> " + messageText, emptyString, emptyString, "<Back   (0" + String(messageText.length()) + ")   Enter>");
                    } else {
                        displayShow(WriteMsgHeader, "CALLSIGN -> " + messageCallsign, "MSG -> " + messageText, emptyString, emptyString, "<Back   (" + String(messageText.length()) + ")   Enter>");
                    }     
                } else {
                    displayShow(WriteMsgHeader, "---  MSG TO LONG! ---", " -> " + messageText, emptyString, emptyString, "<Back   (" + String(messageText.length()) + ")");
                }
                break;
            case 12:    // 1.Messages ---> Messages Delete
                displayShow(MessagesHeader, "  Read (" + String(MSG_Utils::getNumAPRSMessages()) + ")", "  Write", "> Delete", "  APRSThursday", lastLine);
                break;
            case 120:   // 1.Messages ---> Messages Delete ---> Delete: ALL
                displayShow(DeleteMsgHeader, emptyString, "  DELETE APRS MSG?", emptyString, emptyString, " Confirm = LP or '>'");
                break;
            case 13:    // 1.Messages ---> APRSThursday
                displayShow(MessagesHeader, "  Read (" + String(MSG_Utils::getNumAPRSMessages()) + ")", "  Write", "  Delete", "> APRSThursday", lastLine);
                break;
            case 130:   // 1.Messages ---> APRSThursday ---> Delete: ALL
                displayShow(AprsThruHeader, "> Check In", "  Join", "  Unsubscribe", "  KeepSubscribed+12h", lastLine);
                break;
            case 1300:
                if (messageText.length() <= 67) {
                    if (messageText.length() < 10) {
                        displayShow(WriteMsgHeader, "  - APRSThursday -", "MSG -> " + messageText, emptyString, emptyString, "<Back   (0" + String(messageText.length()) + ")   Enter>");
                    } else {
                        displayShow(WriteMsgHeader, "  - APRSThursday -", "MSG -> " + messageText, emptyString, emptyString, "<Back   (" + String(messageText.length()) + ")   Enter>");
                    }     
                } else {
                    displayShow(WriteMsgHeader, "---  MSG TO LONG! ---", " -> " + messageText, emptyString, emptyString, "<Back   (" + String(messageText.length()) + ")");
                }
                break;
            case 131:   // 1.Messages ---> APRSThursday ---> Delete: ALL
                displayShow(AprsThruHeader, "  Check In", "> Join", "  Unsubscribe", "  KeepSubscribed+12h", lastLine);
                break;
            case 1310:
                if (messageText.length() <= 67) {
                    if (messageText.length() < 10) {
                        displayShow(WriteMsgHeader, "  - APRSThursday -", "MSG -> " + messageText, emptyString, emptyString, "<Back   (0" + String(messageText.length()) + ")   Enter>");
                    } else {
                        displayShow(WriteMsgHeader, "  - APRSThursday -", "MSG -> " + messageText, emptyString, emptyString, "<Back   (" + String(messageText.length()) + ")   Enter>");
                    }     
                } else {
                    displayShow(WriteMsgHeader, "---  MSG TO LONG! ---", " -> " + messageText, emptyString, emptyString, "<Back   (" + String(messageText.length()) + ")");
                }
                break;
            case 132:   // 1.Messages ---> APRSThursday ---> Delete: ALL
                displayShow(AprsThruHeader, "  Check In", "  Join", "> Unsubscribe", "  KeepSubscribed+12h", lastLine);
                break;
            case 133:   // 1.Messages ---> APRSThursday ---> Delete: ALL
                displayShow(AprsThruHeader, "  Check In", "  Join", "  Unsubscribe", "> KeepSubscribed+12h", lastLine);
                break;

//////////            
            case 20:    // 2.Configuration ---> Callsign
                displayShow(ConfigHeader, "  Power Off", "> Change Callsign ", "  Change Frequency", "  Display", lastLine);
                break;
            case 21:    // 2.Configuration ---> Change Freq
                displayShow(ConfigHeader, "  Change Callsign ", "> Change Frequency", "  Display", "  " + checkBTType() + " (" + checkProcessActive(bluetoothActive) + ")", lastLine);
                break;
            case 22:    // 2.Configuration ---> Display
                displayShow(ConfigHeader, "  Change Frequency", "> Display", "  " + checkBTType() + " (" + checkProcessActive(bluetoothActive) + ")", "  Status", lastLine);
                break;
            case 23:    // 2.Configuration ---> Bluetooth
                displayShow(ConfigHeader, "  Display",  "> " + checkBTType() + " (" + checkProcessActive(bluetoothActive) + ")", "  Status", "  Notifications", lastLine);
                break;
            case 24:    // 2.Configuration ---> Status
                displayShow(ConfigHeader, "  " + checkBTType() + " (" + checkProcessActive(bluetoothActive) + ")", "> Status","  Notifications", "  Reboot", lastLine);
                break;
            case 25:    // 2.Configuration ---> Notifications
                displayShow(ConfigHeader, "  Status", "> Notifications", "  Reboot", "  Power Off", lastLine);
                break;
            case 26:    // 2.Configuration ---> Reboot
                displayShow(ConfigHeader, "  Notifications", "> Reboot", "  Power Off", "  Change Callsign", lastLine);
                break;
            case 27:    // 2.Configuration ---> Power Off
                displayShow(ConfigHeader, "  Reboot", "> Power Off", "  Change Callsign", "  Change Frequency", lastLine);
                break;


            case 200:   // 2.Configuration ---> Change Callsign
                displayShow(CallsingHeader, emptyString,"  Confirm Change?", emptyString, emptyString,"<Back         Select>");
                break;

            case 210:   // 2.Configuration ---> Change Frequency
                switch (loraIndex) {
                    case 0: freqChangeWarning = "      EU --> PL"; break;
                    case 1: freqChangeWarning = "      PL --> UK"; break;
                    case 2: freqChangeWarning = "      UK --> EU"; break;
                }
                displayShow(LoraFreqHeader, emptyString,"   Confirm Change?", freqChangeWarning, emptyString, "<Back         Select>");
                break;

            case 220:   // 2.Configuration ---> Display ---> ECO Mode
                displayShow(DisplayHeader, emptyString, "> ECO Mode    (" + checkProcessActive(displayEcoMode) + ")","  Brightness  (" + screenBrightnessAsString(screenBrightness) + ")",emptyString, lastLine);
                break;
            case 221:   // 2.Configuration ---> Display ---> Brightness
                displayShow(DisplayHeader, emptyString, "  ECO Mode    (" + checkProcessActive(displayEcoMode) + ")","> Brightness  (" + screenBrightnessAsString(screenBrightness) + ")",emptyString, lastLine);
                break;

            case 230:
                if (bluetoothActive) {
                    bluetoothActive = false;
                    displayShow(BluetoothHeader, emptyString, " Bluetooth --> OFF", 1000);
                } else {
                    bluetoothActive = true;
                    displayShow(BluetoothHeader, emptyString, " Bluetooth --> ON", 1000);
                }
                menuDisplay = 23;
                break;

            case 240:    // 2.Configuration ---> Status
                displayShow(StatusHeader, emptyString, "> Write","  Select", emptyString, lastLine);
                break;
            case 241:    // 2.Configuration ---> Status
                displayShow(StatusHeader, emptyString, "  Write","> Select", emptyString, lastLine);
                break;

            case 250:    // 2.Configuration ---> Notifications
                displayShow(NotificHeader, "> Turn Off Sound/Led", emptyString, emptyString, emptyString, lastLine);
                break;

            case 260:   // 2.Configuration ---> Reboot
                if (keyDetected) {
                    displayShow(RebootHeader, emptyString,"Confirm Reboot...", emptyString, emptyString,"<Back   Enter=Confirm");
                } else {
                    displayShow(RebootHeader, "no Keyboard Detected"," Use RST Button to","Reboot Tracker", emptyString, lastLine);
                }
                break;
            case 270:   // 2.Configuration ---> Power Off
                if (keyDetected) {
                    displayShow(PowerOffHeader, emptyString,"Confirm Power Off...", emptyString, emptyString,"<Back   Enter=Confirm");
                } else {
                    displayShow(PowerOffHeader, "no Keyboard Detected"," Use PWR Button to","Power Off Tracker", emptyString, lastLine);
                }
                break;

//////////
            case 30:    //3.Stations ---> Packet Decoder
                displayShow(StationsHeader, emptyString, "> Packet Decoder", "  Near By Stations", emptyString, "<Back");
                break;
            case 31:    //3.Stations ---> Near By Stations
                displayShow(StationsHeader, emptyString, "  Packet Decoder", "> Near By Stations", emptyString, "<Back");
                break;

            case 300:   //3.Stations ---> Packet Decoder
                if (lastReceivedPacket.sender != currentBeacon->callsign) {
                    firstLineDecoder = lastReceivedPacket.sender;
                    for(int i = firstLineDecoder.length(); i < 9; i++) {
                        firstLineDecoder += ' ';
                    }
                    firstLineDecoder += lastReceivedPacket.symbol;
                    if (lastReceivedPacket.type == 0 || lastReceivedPacket.type == 4) {      // gps and Mic-E gps
                        courseSpeedAltitude = String(lastReceivedPacket.altitude);
                        for(int j = courseSpeedAltitude.length(); j < 4; j++) {
                            courseSpeedAltitude = '0' + courseSpeedAltitude;
                        }
                        courseSpeedAltitude = "A=" + courseSpeedAltitude + "m ";
                        speedPacketDec = String(lastReceivedPacket.speed);
                        for (int k = speedPacketDec.length(); k < 3; k++) {
                            speedPacketDec = ' ' + speedPacketDec;
                        }
                        courseSpeedAltitude += speedPacketDec + "km/h ";
                        for(int l = courseSpeedAltitude.length(); l < 17; l++) {
                            courseSpeedAltitude += ' ';
                        }
                        coursePacketDec = String(lastReceivedPacket.course);
                        for(int m = coursePacketDec.length(); m < 3; m++) {
                            coursePacketDec = ' ' + coursePacketDec;
                        }
                        courseSpeedAltitude += coursePacketDec;
                        
                        double distanceKm = TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), lastReceivedPacket.latitude, lastReceivedPacket.longitude) / 1000.0;
                        double courseTo   = TinyGPSPlus::courseTo(gps.location.lat(), gps.location.lng(), lastReceivedPacket.latitude, lastReceivedPacket.longitude);
                        
                        if (lastReceivedPacket.path.length()>14) {
                            pathDec = "P:";
                        } else {
                            pathDec = "PATH:  ";
                        }
                        pathDec += lastReceivedPacket.path;

                        displayShow(firstLineDecoder, "GPS " + String(lastReceivedPacket.latitude,3) + " " + String(lastReceivedPacket.longitude,3), courseSpeedAltitude, "D:" + String(distanceKm) + "km    " + String(courseTo,0), pathDec, "< RSSI:" + String(lastReceivedPacket.rssi) + " SNR:" + String(lastReceivedPacket.snr));
                    } else if (lastReceivedPacket.type == 1) {    // message
                        displayShow(firstLineDecoder, "ADDRESSEE: " + lastReceivedPacket.addressee, "MSG:  " + lastReceivedPacket.message, emptyString, emptyString, "< RSSI:" + String(lastReceivedPacket.rssi) + " SNR:" + String(lastReceivedPacket.snr));
                    } else if (lastReceivedPacket.type == 2) {    // status
                        displayShow(firstLineDecoder, "-------STATUS-------", lastReceivedPacket.message, emptyString, emptyString, "< RSSI:" + String(lastReceivedPacket.rssi) + " SNR:" + String(lastReceivedPacket.snr));
                    } else if (lastReceivedPacket.type == 3) {    // telemetry
                        displayShow(firstLineDecoder, "------TELEMETRY------", emptyString, emptyString, emptyString, "< RSSI:" + String(lastReceivedPacket.rssi) + " SNR:" + String(lastReceivedPacket.snr));
                    } else if (lastReceivedPacket.type == 5) {    // object
                        displayShow(firstLineDecoder, "-------OBJECT-------", emptyString, emptyString, emptyString, "< RSSI:" + String(lastReceivedPacket.rssi) + " SNR:" + String(lastReceivedPacket.snr));
                    }
                }
                break;
            case 310:    //3.Stations ---> Near By Stations
                displayShow("NEAR BY >", STATION_Utils::getNearTracker(0), STATION_Utils::getNearTracker(1), STATION_Utils::getNearTracker(2), STATION_Utils::getNearTracker(3), "<Back");
                break;

//////////
            case 40:
                // waiting for Weather Report
                break;

//////////
            case 50:    // 5.Winlink MENU
                if (winlinkStatus == 5) {
                    menuDisplay = 5000;
                } else {
                    displayShow(WlinkLoginHeader, "> Login" , "  Read SavedMails(" + String(MSG_Utils::getNumWLNKMails()) + ")", "  Delete SavedMails", "  Wnlk Comment (" + checkProcessActive(winlinkCommentState) + ")", lastLine);
                }
                break;
            case 51:    // 5.Winlink
                displayShow(WlinkLoginHeader, "  Login" , "> Read SavedMails(" + String(MSG_Utils::getNumWLNKMails()) + ")", "  Delete SavedMails", "  Wnlk Comment (" + checkProcessActive(winlinkCommentState) + ")", lastLine);
                break;
            case 52:    // 5.Winlink
                displayShow(WlinkLoginHeader, "  Login" , "  Read SavedMails(" + String(MSG_Utils::getNumWLNKMails()) + ")", "> Delete SavedMails", "  Wnlk Comment (" + checkProcessActive(winlinkCommentState) + ")", lastLine);
                break;
            case 53:    // 5.Winlink
                displayShow(WlinkLoginHeader, "  Login" , "  Read SavedMails(" + String(MSG_Utils::getNumWLNKMails()) + ")", "  Delete SavedMails", "> Wnlk Comment (" + checkProcessActive(winlinkCommentState) + ")", lastLine);
                break;

            case 500:    // 5.Winlink ---> Login
                displayShow(WlinkLoginHeader, emptyString , "Login Initiation ...", "Challenge -> waiting", emptyString , emptyString);
                break;
            case 501:    // 5.Winlink ---> Login
                displayShow(WlinkLoginHeader, emptyString , "Login Initiation ...", "Challenge -> sended", emptyString , emptyString);
                break;
            case 502:    // 5.Winlink ---> Login
                displayShow(WlinkLoginHeader, emptyString , "Login Initiation ...", "Challenge -> ack ...", emptyString , emptyString);
                break;

            case 5000:   // WINLINK: List Pend. Mail //
                displayShow(WlinkMenuHeader, "  Write Mail" , "> List Pend. Mails", "  Downloaded Mails", "  Read Mail    (R#)", lastLine);
                break;

            case 5010:    // WINLINK: Downloaded Mails //
                displayShow(WlinkMenuHeader, "  List Pend. Mails", "> Downloaded Mails", "  Read Mail    (R#)", "  Reply Mail   (Y#)", lastLine);
                break;
            case 50100:    // WINLINK: Downloaded Mails //
                displayShow(WlinkLoginHeader, emptyString , "> Read SavedMails(" + String(MSG_Utils::getNumWLNKMails()) + ")", "  Delete SavedMails", emptyString, lastLine);
                break;
            case 50101:    // WINLINK: Downloaded Mails //
                {
                    String mailText = loadedWLNKMails[messagesIterator];
                    displayShow(WlinkMailHeader, emptyString, mailText, emptyString, emptyString, "           Next=Down");
                }
                break;
            case 50110:    // WINLINK: Downloaded Mails //
                displayShow(WlinkLoginHeader, emptyString , "  Read SavedMails(" + String(MSG_Utils::getNumWLNKMails()) + ")", "> Delete SavedMails", emptyString, lastLine);
                break;
            case 50111:    // WINLINK: Downloaded Mails //
                displayShow(WlinkDelHeader, emptyString, "  DELETE ALL MAILS?", emptyString, emptyString, " Confirm = LP or '>'");
                break;

            case 5020:    // WINLINK: Read Mail //
                displayShow(WlinkMenuHeader, "  Downloaded Mails", "> Read Mail    (R#)", "  Reply Mail   (Y#)", "  Forward Mail (F#)", lastLine);
                break;
            case 5021:
                displayShow(WlinkReadHeader, emptyString, "    READ MAIL N." + winlinkMailNumber, emptyString, emptyString, "<Back          Enter>");
                break;

            case 5030:    // WINLINK: Reply Mail //
                displayShow(WlinkMenuHeader, "  Read Mail    (R#)", "> Reply Mail   (Y#)", "  Forward Mail (F#)", "  Delete Mail  (K#)", lastLine);
                break;
            case 5031:
                displayShow(WlinkReplyHeader, emptyString, "   REPLY MAIL N." + winlinkMailNumber , emptyString, emptyString, "<Back          Enter>");
                break;

            case 5040:    // WINLINK: Foward Mail //
                displayShow(WlinkMenuHeader, "  Reply Mail   (Y#)", "> Forward Mail (F#)", "  Delete Mail  (K#)", "  Alias Menu", lastLine);
                break;
            case 5041:    // WINLINK: Forward Mail //
                displayShow(WlinkForwHeader, emptyString, "  FORWARD MAIL N." + winlinkMailNumber , emptyString, emptyString, "<Back          Enter>");
                break;
            case 5042:    // WINLINK: Forward Mail //
                displayShow(WlinkForwHeader, "  FORWARD MAIL N." + winlinkMailNumber , "To = " + winlinkAddressee, emptyString, emptyString, "<Back          Enter>");
                break;

            case 5050:    // WINLINK: Delete Mail //
                displayShow(WlinkMenuHeader, "  Forward Mail (F#)", "> Delete Mail  (K#)", "  Alias Menu", "  Log Out", lastLine);
                break;
            case 5051:    // WINLINK: Delete Mail //
                displayShow(WlinkDelHeader, emptyString, "   DELETE MAIL N."  + winlinkMailNumber, emptyString, emptyString, "<Back          Enter>");
                break;
            
            case 5060:    // WINLINK: Alias Menu //
                displayShow(WlinkMenuHeader, "  Delete Mail  (K#)", "> Alias Menu", "  Log Out", "  Write Mail", lastLine);
                break;
            case 5061:    // WINLINK: Alias Menu : Create Alias //
                displayShow(WlinkAliasHeader, "> Create Alias" , "  Delete Alias ", "  List All Alias", emptyString, lastLine);
                break;
            case 50610:   // WINLINK: Alias Menu : Create Alias //
                displayShow(WlinkAliasHeader, emptyString, "Write Alias to Create", "     -> " + winlinkAlias, emptyString, "<Back          Enter>");
                break;
            case 50611:   // WINLINK: Alias Menu : Create Alias //
                displayShow(WlinkAliasHeader, emptyString, "      " + winlinkAlias + " =", winlinkAliasComplete, emptyString, "<Back          Enter>");
                break;
            case 5062:    // WINLINK: Alias Menu : Delete Alias //
                displayShow(WlinkAliasHeader, "  Create Alias" , "> Delete Alias ", "  List All Alias", emptyString, lastLine);
                break;
            case 50620:   // WINLINK: Alias Menu : Delete Alias //
                displayShow(WlinkAliasHeader, "Write Alias to Delete", emptyString, "     -> " + winlinkAlias, emptyString, "<Back          Enter>");
                break;
            case 5063:    // WINLINK: Alias Menu : List Alias//
                displayShow(WlinkAliasHeader, "  Create Alias" , "  Delete Alias ", "> List All Alias", emptyString, lastLine);
                break;

            case 5070:    // WINLINK: Log Out MAIL //
                displayShow(WlinkMenuHeader, "  Alias Menu", "> Log Out", "  Write Mail", "  List Pend. Mails", lastLine);
                break;

            case 5080:    // WINLINK: WRITE MAIL //
                displayShow(WlinkMenuHeader, "  Log Out", "> Write Mail", "  List Pend. Mails", "  Downloaded Mails", lastLine);
                break;
            case 5081:    // WINLINK: WRITE MAIL: Addressee //
                displayShow(WlinkMailHeader, "--- Send Mail to ---", emptyString, "-> " + winlinkAddressee, emptyString, "<Back          Enter>");
                break;
            case 5082:    // WINLINK: WRITE MAIL: Subject //
                displayShow(WlinkMailHeader, "--- Write Subject ---", emptyString, "-> " + winlinkSubject, emptyString, "<Back          Enter>");
                break;
            case 5083:    // WINLINK: WRITE MAIL: Body //
                if (winlinkBody.length() <= 67) {
                displayShow(WlinkMailHeader, "-- Body (lenght=" + String(winlinkBody.length()) + ")", "-> " + winlinkBody, emptyString, emptyString, "<Clear Body    Enter>");
                } else {
                displayShow(WlinkMailHeader, "-- Body To Long = " + String(winlinkBody.length()) + "!", "-> " + winlinkBody, emptyString, emptyString, "<Clear Body");
                }
                break;
            case 5084:    // WINLINK: WRITE MAIL: End Mail? //
                displayShow(WlinkMailHeader, emptyString, "> End Mail", "  1 More Line", emptyString, "      Up/Down Select>");
                break;
            case 5085:    // WINLINK: WRITE MAIL: One More Line(Body) //
                displayShow(WlinkMailHeader, emptyString, "  End Mail", "> 1 More Line", emptyString, "      Up/Down Select>");
                break;

                // validar winlinkStatus = 0
                // check si no esta logeado o si

//////////
            case 60:    // 6. Extras ---> Flashlight
                displayShow(ExtrasHeader, "> Flashlight    (" + checkProcessActive(flashlight) + ")", "  DigiRepeater  (" + checkProcessActive(digirepeaterActive) + ")", "  S.O.S.        (" + checkProcessActive(sosActive) + ")","  Send GPS + Comment", lastLine);
                break;
            case 61:    // 6. Extras ---> Digirepeater
                displayShow(ExtrasHeader, "  Flashlight    (" + checkProcessActive(flashlight) + ")", "> DigiRepeater  (" + checkProcessActive(digirepeaterActive) + ")", "  S.O.S.        (" + checkProcessActive(sosActive) + ")","  Send GPS + Comment", lastLine);
                break;
            case 62:    // 6. Extras ---> S.O.S.
                displayShow(ExtrasHeader, "  Flashlight    (" + checkProcessActive(flashlight) + ")", "  DigiRepeater  (" + checkProcessActive(digirepeaterActive) + ")", "> S.O.S.        (" + checkProcessActive(sosActive) + ")","  Send GPS + Comment", lastLine);
                break;
            case 63:    // 6. Extras ---> Extra Comment.
                displayShow(ExtrasHeader, "  Flashlight    (" + checkProcessActive(flashlight) + ")", "  DigiRepeater  (" + checkProcessActive(digirepeaterActive) + ")", "  S.O.S.        (" + checkProcessActive(sosActive) + ")","> Send GPS + Comment", lastLine);
                break;
            case 630:
                if (messageText.length() <= 67) {
                    if (messageText.length() < 10) {
                        displayShow(CommentHeader, "Send this Comment in","the next GPS Beacon :", messageText, emptyString, "<Back   (0" + String(messageText.length()) + ")   Enter>");
                    } else {
                        displayShow(CommentHeader, "Send this Comment in","the next GPS Beacon :", messageText, emptyString, "<Back   (" + String(messageText.length()) + ")   Enter>");
                    }     
                } else {
                    displayShow(CommentHeader, " Comment is to long! ", " -> " + messageText, emptyString, emptyString, "<Back   (" + String(messageText.length()) + ")");
                }
                break;

//////////
            case 9000:  //  9. multiPress Menu ---> Turn ON WiFi AP
                displayShow(ConfigHeader, "> Turn Tracker Off","  Config. WiFi AP",  emptyString,emptyString, lastLine);
                break;
            case 9001:  //  9. multiPress Menu
                displayShow(ConfigHeader, "  Turn Tracker Off","> Config. WiFi AP",  emptyString,emptyString, lastLine);
                break;


//////////
            case 0:       ///////////// MAIN MENU //////////////
                String hdopState, firstRowMainMenu, secondRowMainMenu, thirdRowMainMenu, fourthRowMainMenu, fifthRowMainMenu, sixthRowMainMenu;

                firstRowMainMenu = currentBeacon->callsign;
                if (Config.display.showSymbol) {
                    for (int j = firstRowMainMenu.length(); j < 9; j++) {
                        firstRowMainMenu += " ";
                    }
                    if (!symbolAvailable) {
                        firstRowMainMenu += currentBeacon->symbol;
                    }
                }

                if (disableGPS) {
                    secondRowMainMenu = emptyString;
                    thirdRowMainMenu = "    LoRa APRS TNC";
                    fourthRowMainMenu = emptyString;
                } else {
                    const auto time_now = now();
                    secondRowMainMenu = Utils::createDateString(time_now) + "   " + Utils::createTimeString(time_now);
                    if (time_now % 10 < 5) {
                        thirdRowMainMenu = String(gps.location.lat(), 4);
                        thirdRowMainMenu += " ";
                        thirdRowMainMenu += String(gps.location.lng(), 4);
                    } else {
                        thirdRowMainMenu = String(Utils::getMaidenheadLocator(gps.location.lat(), gps.location.lng(), 8));
                        thirdRowMainMenu += " LoRa[";
                        switch (loraIndex) {
                            case 0: thirdRowMainMenu += "EU]"; break;
                            case 1: thirdRowMainMenu += "PL]"; break;
                            case 2: thirdRowMainMenu += "UK]"; break;
                        }
                    }
                    
                    for(int i = thirdRowMainMenu.length(); i < 18; i++) {
                        thirdRowMainMenu += " ";
                    }

                    if (gps.hdop.hdop() > 5) {
                        hdopState = "X";
                    } else if (gps.hdop.hdop() > 2 && gps.hdop.hdop() < 5) {
                        hdopState = "-";
                    } else if (gps.hdop.hdop() <= 2) {
                        hdopState = "+";
                    }

                    if (gps.satellites.value() <= 9) thirdRowMainMenu += " ";
                    if (gpsIsActive) {
                        thirdRowMainMenu += String(gps.satellites.value());
                        thirdRowMainMenu += hdopState;
                    } else {
                        thirdRowMainMenu += "--";
                    }

                    String fourthRowAlt = String(gps.altitude.meters(),0);
                    fourthRowAlt.trim();
                    for (int a = fourthRowAlt.length(); a < 4; a++) {
                        fourthRowAlt = "0" + fourthRowAlt;
                    }
                    String fourthRowSpeed = String(gps.speed.kmph(),0);
                    fourthRowSpeed.trim();
                    for (int b = fourthRowSpeed.length(); b < 3; b++) {
                        fourthRowSpeed = " " + fourthRowSpeed;
                    }
                    String fourthRowCourse = String(gps.course.deg(),0);
                    if (fourthRowSpeed == "  0") {
                        fourthRowCourse = "---";
                    } else {
                        fourthRowCourse.trim();
                        for(int c = fourthRowCourse.length(); c < 3; c++) {
                            fourthRowCourse = "0" + fourthRowCourse;
                        }
                    }
                    fourthRowMainMenu = "A=";
                    fourthRowMainMenu += fourthRowAlt;
                    fourthRowMainMenu += "m  ";
                    fourthRowMainMenu += fourthRowSpeed;
                    fourthRowMainMenu += "km/h  ";
                    fourthRowMainMenu += fourthRowCourse;
                    if (Config.wxsensor.active && (time_now % 10 < 5) && wxModuleType != 0) {
                        fourthRowMainMenu = WX_Utils::readDataSensor(1);
                    }
                    if (MSG_Utils::getNumWLNKMails() > 0) {
                        fourthRowMainMenu = "** WLNK MAIL: ";
                        fourthRowMainMenu += String(MSG_Utils::getNumWLNKMails());
                        fourthRowMainMenu += " **";
                    }
                    if (MSG_Utils::getNumAPRSMessages() > 0) {
                        fourthRowMainMenu = "*** MESSAGES: ";
                        fourthRowMainMenu += String(MSG_Utils::getNumAPRSMessages());
                        fourthRowMainMenu += " ***";
                    }
                    if (!gpsIsActive) {
                        fourthRowMainMenu = "*** GPS  SLEEPING ***";
                    }
                }

                if (showHumanHeading) {
                    fifthRowMainMenu = GPS_Utils::getCardinalDirection(gps.course.deg());
                } else {
                    fifthRowMainMenu = "LAST Rx = ";
                    fifthRowMainMenu += MSG_Utils::getLastHeardTracker();
                }

                if (POWER_Utils::getBatteryInfoIsConnected()) {
                    String batteryVoltage = POWER_Utils::getBatteryInfoVoltage();
                    String batteryCharge = POWER_Utils::getBatteryInfoCurrent();
                    #if defined(TTGO_T_Beam_V0_7) || defined(TTGO_T_LORA32_V2_1_GPS) || defined(TTGO_T_LORA32_V2_1_GPS_915) || defined(TTGO_T_LORA32_V2_1_TNC) || defined(TTGO_T_LORA32_V2_1_TNC_915) || defined(HELTEC_V3_GPS) || defined(HELTEC_V3_TNC) || defined(HELTEC_WIRELESS_TRACKER) || defined(TTGO_T_DECK_GPS) || defined(TTGO_T_DECK_PLUS)
					    sixthRowMainMenu = "Battery: ";
                        sixthRowMainMenu += batteryVoltage;
                        sixthRowMainMenu += "V   ";
                        sixthRowMainMenu += BATTERY_Utils::getPercentVoltageBattery(batteryVoltage.toFloat());
                        sixthRowMainMenu += "%";
                    #endif
                    #ifdef HAS_AXP192
                        if (batteryCharge.toInt() == 0) {
                            sixthRowMainMenu = "Battery Charged ";
                            sixthRowMainMenu += batteryVoltage;
                            sixthRowMainMenu += "V";
                        } else if (batteryCharge.toInt() > 0) {
                            sixthRowMainMenu = "Bat: ";
                            sixthRowMainMenu += batteryVoltage;
                            sixthRowMainMenu += "V (charging)";
                        } else {
                            sixthRowMainMenu = "Battery ";
                            sixthRowMainMenu += batteryVoltage;
                            sixthRowMainMenu += "V ";
                            sixthRowMainMenu += batteryCharge;
                            sixthRowMainMenu += "mA";
                        }
                    #endif
                    #ifdef HAS_AXP2101
                        if (Config.notification.lowBatteryBeep && !POWER_Utils::isCharging() && batteryCharge.toInt() < lowBatteryPercent) {
                            lowBatteryPercent = batteryCharge.toInt();
                            NOTIFICATION_Utils::lowBatteryBeep();
                            if (batteryCharge.toInt() < 6) {
                                NOTIFICATION_Utils::lowBatteryBeep();
                            }
                        } 
                        if (POWER_Utils::isCharging()) {
                            lowBatteryPercent = 21;
                        }
                        if (POWER_Utils::isCharging() && batteryCharge != "100") {
                            sixthRowMainMenu = "Bat: ";
                            sixthRowMainMenu += String(batteryVoltage);
                            sixthRowMainMenu += "V (charging)";
                        } else if (!POWER_Utils::isCharging() && batteryCharge == "100") {
                            sixthRowMainMenu = "Battery Charged ";
                            sixthRowMainMenu += String(batteryVoltage);
                            sixthRowMainMenu += "V";
                        } else {
                            sixthRowMainMenu = "Battery  ";
                            sixthRowMainMenu += String(batteryVoltage);
                            sixthRowMainMenu += "V   ";
                            sixthRowMainMenu += batteryCharge;
                            sixthRowMainMenu += "%";
                        }
                    #endif
                } else {
                    sixthRowMainMenu = "No Battery Connected" ;
                }
                displayShow(firstRowMainMenu,
                            secondRowMainMenu,
                            thirdRowMainMenu,
                            fourthRowMainMenu,
                            fifthRowMainMenu,
                            sixthRowMainMenu);
                break;
        }
    }

}