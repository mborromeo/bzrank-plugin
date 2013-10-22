#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <fstream>
#include <sstream>
using namespace std;

const char kPathSeparator =
#ifdef _WIN32
  '\\';
#else
  '/';
#endif

ofstream logFile;
string partLogName;
string finalLogName;
double lastFlush;

std::string getTeamName(bz_eTeamType team);
std::string getScoreType(bz_eScoreElement element);

class bzrank : public bz_Plugin
{
  virtual const char* Name (){return "bzrank";}
  virtual void Init ( const char* config);

  virtual void Event ( bz_EventData *);
  virtual void Cleanup();
};

BZ_PLUGIN(bzrank)

void bzrank::Init ( const char* commandLine )
{
  bz_debugMessage(4,"bzrank plugin loaded");

  string logBasePath = commandLine;
  if (logBasePath.empty()) {
    char const *tempFolder = getenv("TMPDIR");
      if (tempFolder == 0) {tempFolder = "/tmp";}
    logBasePath = tempFolder;
  }

  logBasePath = logBasePath + kPathSeparator;

  string partLogExt = ".bzrankpart";
  string finalLogExt = ".bzrankdata";

  unsigned long int sec= time(NULL);
  std::ostringstream s;
  s << sec;

  partLogName = logBasePath + s.str() + partLogExt;
  finalLogName = logBasePath + s.str() + finalLogExt;

  logFile.open(partLogName.c_str(), ios::out | ios::trunc);

  if (!logFile.is_open()) {bz_debugMessage(4,"bzrank can't open log file");}

  Register(bz_eTeamScoreChanged);
  Register(bz_eGetWorldEvent);
  Register(bz_eGetAutoTeamEvent);
  Register(bz_ePlayerPartEvent);
  Register(bz_ePlayerJoinEvent);
  Register(bz_ePlayerScoreChanged);
  Register(bz_eGameStartEvent);
  Register(bz_eGameEndEvent);
  Register(bz_eCaptureEvent);
  Register(bz_eFlagGrabbedEvent);
  Register(bz_eFlagDroppedEvent);
  Register(bz_eFlagTransferredEvent);
  Register(bz_ePlayerDieEvent);
  Register(bz_eShotFiredEvent);
  Register(bz_eTickEvent);
}

void bzrank::Cleanup() {
  
  if (!logFile.is_open()) {
    bz_debugMessage(4,"bzrank can't write to file");
    return;
  }

  logFile.flush();
  logFile.close();

  rename(partLogName.c_str(), finalLogName.c_str());
}

void bzrank::Event ( bz_EventData *eventData ) {

  std::ostringstream s;
  s << time(NULL);
  std::string secs = s.str();

  switch (eventData->eventType) {
    case bz_eTeamScoreChanged: {
      bz_TeamScoreChangeEventData_V1* tsc = (bz_TeamScoreChangeEventData_V1*)eventData;

      std::string teamName = getTeamName(tsc->team);
      std::string teamScore = getScoreType(tsc->element);

      logFile << secs << "\t" << "teamscore" << "\t" << teamName << "\t" << tsc->thisValue << "\t" << tsc->lastValue << std::endl;
    }
    break;
    case bz_eGetWorldEvent: {
      bz_GetWorldEventData_V1* wdt = (bz_GetWorldEventData_V1*)eventData;

      std::string worldtype;
      if (wdt->ctf) {worldtype = "ctt";}
      else if (wdt->rabbit) {worldtype = "rabbit";}
      else if (wdt->openFFA) {worldtype = "openffa";}

      logFile << secs << "\t" << "worldtype" << "\t" << worldtype << std::endl;
    }
    break;
    case bz_eGetAutoTeamEvent: {
      bz_GetAutoTeamEventData_V1* pjt = (bz_GetAutoTeamEventData_V1*)eventData;

      std::string teamName = getTeamName(pjt->team);

      if (teamName != "") {
        logFile << secs << "\t" << "playerjointeam" << "\t" << pjt->callsign.c_str() << "\t" << teamName << std::endl;
      } 
    }
    break;
    case bz_ePlayerPartEvent:
    case bz_ePlayerJoinEvent: {
      bz_PlayerJoinPartEventData_V1* pjo = (bz_PlayerJoinPartEventData_V1*)eventData;

      string eventType;
      if (pjo->eventType == bz_ePlayerPartEvent) {
        eventType = "playerpart";
      } else {
        eventType = "playerjoin";
      }

      std::string playerName = bz_getPlayerCallsign(pjo->playerID);

      logFile << secs << "\t" << eventType << "\t" << playerName << std::endl;
    }
    break;
    case bz_ePlayerScoreChanged: {
      bz_PlayerScoreChangeEventData_V1* psc = (bz_PlayerScoreChangeEventData_V1*)eventData;

      std::string playerName = bz_getPlayerCallsign(psc->playerID);
      std::string playerScore = getScoreType(psc->element);

      logFile << secs << "\t" << "playerscore" << "\t" << playerName << "\t" << playerScore << "\t" << psc->thisValue << "\t" << psc->lastValue << std::endl;
    }
    break;
    case bz_eGameEndEvent: {
      logFile << secs << "\t" << "gameend" << std::endl;
    }
    break;
    case bz_eGameStartEvent: {
      bz_GameStartEndEventData_V1* gam = (bz_GameStartEndEventData_V1*)eventData;

      logFile << secs << "\t" << "gamestart" << "\t" << gam->duration << "\t" << std::endl;
    }
    break;
    case bz_eCaptureEvent: {
      bz_CTFCaptureEventData_V1* cpe = (bz_CTFCaptureEventData_V1*)eventData;

      std::string playerName = bz_getPlayerCallsign(cpe->playerCapping);

      logFile << secs << "\t" << "flagcaptured" << "\t" << playerName << std::endl;
    }
    break;
    case bz_eFlagDroppedEvent:
    case bz_eFlagGrabbedEvent: {
      bz_FlagGrabbedEventData_V1* fge = (bz_FlagGrabbedEventData_V1*)eventData;

      string eventType;
      if (fge->eventType == bz_eFlagGrabbedEvent) {
        eventType = "flaggrabbed";
      } else {
        eventType = "flagdropped";
      }

      std::string flagName = bz_getName(fge->flagID).c_str(); // bz_getFlagName?
      std::string playerName = bz_getPlayerCallsign(fge->playerID);

      logFile << secs << "\t" << eventType << "\t" << playerName << "\t" << flagName << std::endl;
    }
    break;
    case bz_eFlagTransferredEvent: {
      bz_FlagTransferredEventData_V1* fge = (bz_FlagTransferredEventData_V1*)eventData;

      std::string fromPlayerName = bz_getPlayerCallsign(fge->fromPlayerID);
      std::string toPlayerName = bz_getPlayerCallsign(fge->toPlayerID);

      logFile << secs << "\t" << "flagstolen\t" << fromPlayerName << "\t" << toPlayerName << "\t" << fge->flagType << std::endl;
    }
    break;
    case bz_ePlayerDieEvent: {
      bz_PlayerDieEventData_V1* pki = (bz_PlayerDieEventData_V1*)eventData;

      std::string playerName = bz_getPlayerCallsign(pki->playerID);
      std::string killerName = bz_getPlayerCallsign(pki->killerID);

      logFile << secs << "\t" << "playerkilled\t" << playerName << "\t" << killerName << "\t" << pki->flagKilledWith.c_str() << std::endl;
    }
    break;
    case bz_eShotFiredEvent: {
      bz_ShotFiredEventData_V1* pki = (bz_ShotFiredEventData_V1*)eventData;

      std::string playerName = bz_getPlayerCallsign(pki->playerID);

      logFile << secs << "\t" << "shotfired\t" << playerName << "\t" << pki->type.c_str() << std::endl;
    }
    break;
    case bz_eTickEvent: {
      bz_TickEventData_V1* tke = (bz_TickEventData_V1*)eventData;

      if ((tke->eventTime - lastFlush) > 5) {
        
        if (!logFile.is_open()) {
          bz_debugMessage(4,"bzrank can't write to file");
          return;
        }

        logFile.flush();
        lastFlush = tke->eventTime;
      }
    }
    break;
  }
}

std::string getScoreType(bz_eScoreElement element) {
  switch (element) {
    case bz_eWins: {return "w";}
    case bz_eLosses: {return "l";}
    case bz_eTKs: {return "tk";}
  }
}

std::string getTeamName(bz_eTeamType team) {
  switch (team) {
    case eRedTeam: {return "red";}
    case eGreenTeam: {return "green";}
    case eBlueTeam: {return "blue";}
    case ePurpleTeam: {return "purple";}
    case eRabbitTeam: {return "rabbit";}
    case eHunterTeam: {return "hunter";}
    case eObservers: {return "observers";}
    case eAdministrators: {return "administrators";}
    case eRogueTeam: {return "rogue";}
    default: {return "";}
  }
}