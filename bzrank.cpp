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
  switch (eventData->eventType) {
    case bz_eCaptureEvent: {
      bz_CTFCaptureEventData_V1* cpe = (bz_CTFCaptureEventData_V1*)eventData;

      std::string playerName = bz_getPlayerCallsign(cpe->playerCapping);

      logFile << "flagcaptured" << "\t" << playerName << std::endl;
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

      logFile << eventType << "\t" << playerName << "\t" << flagName << std::endl;
    }
    break;
    case bz_eFlagTransferredEvent: {
      bz_FlagTransferredEventData_V1* fge = (bz_FlagTransferredEventData_V1*)eventData;

      std::string fromPlayerName = bz_getPlayerCallsign(fge->fromPlayerID);
      std::string toPlayerName = bz_getPlayerCallsign(fge->toPlayerID);

      logFile << "flagstolen\t" << fromPlayerName << "\t" << toPlayerName << "\t" << fge->flagType << std::endl;
    }
    break;
    case bz_ePlayerDieEvent: {
      bz_PlayerDieEventData_V1* pki = (bz_PlayerDieEventData_V1*)eventData;

      std::string playerName = bz_getPlayerCallsign(pki->playerID);
      std::string killerName = bz_getPlayerCallsign(pki->killerID);

      logFile << "playerkilled\t" << playerName << "\t" << killerName << "\t" << pki->flagKilledWith.c_str() << std::endl;
    }
    break;
    case bz_eShotFiredEvent: {
      bz_ShotFiredEventData_V1* pki = (bz_ShotFiredEventData_V1*)eventData;

      std::string playerName = bz_getPlayerCallsign(pki->playerID);

      logFile << "shotfired\t" << playerName << "\t" << pki->type.c_str() << std::endl;
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