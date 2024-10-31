#include "winlink_utils.h"
#include "configuration.h"
#include "msg_utils.h"
#include "display.h"
#include "logger.h"


extern      Configuration           Config;
extern      int                     menuDisplay;
extern      logging::Logger         logger;

uint8_t     winlinkStatus           = 0;
String      winlinkMailNumber       = "_?";
String      winlinkAddressee        = emptyString;
String      winlinkSubject          = emptyString;
String      winlinkBody             = emptyString;
String      winlinkAlias            = emptyString;
String      winlinkAliasComplete    = emptyString;
bool        winlinkCommentState     = false;

uint32_t    lastChallengeTime       = 0;
String      challengeAnswer;



namespace WINLINK_Utils {

    void processWinlinkChallenge(const String& winlinkInteger) {
        if (lastChallengeTime == 0 || (millis() - lastChallengeTime) > 10 * 60 * 10000) {
            challengeAnswer = emptyString;
            for (int i = 0; i < winlinkInteger.length(); i++) {
                String number = String(winlinkInteger[i]);
                int digit = number.toInt();
                if (digit > Config.winlink.password.length()) {
                    displayShow("__WINLINK_", emptyString , "PASS Length<REQUIRED", emptyString, emptyString , emptyString, 2000);
                    challengeAnswer += Config.winlink.password[0];
                } else {
                    challengeAnswer += Config.winlink.password[digit - 1];
                }
            }
            challengeAnswer += char(random(65,90));
            challengeAnswer += char(random(48,57));
            challengeAnswer += char(random(65,90));
            MSG_Utils::addToOutputBuffer(1, "WLNK-1", challengeAnswer);
            lastChallengeTime = millis();
        } else {
            MSG_Utils::addToOutputBuffer(1, "WLNK-1", challengeAnswer);
        }
    }

    void login() {
        logger.log(logging::LoggerLevel::LOGGER_LEVEL_INFO, "Winlink","---> Start Login");
        displayShow("__WINLINK_", emptyString , "Login Initiation ...", emptyString, emptyString , "<Back");
        if (winlinkStatus == 5) {
            menuDisplay = 5000;
        } else {
            winlinkStatus = 1;
            MSG_Utils::addToOutputBuffer(1, "WLNK-1", "Start");
            menuDisplay = 500;
        }
    }

}