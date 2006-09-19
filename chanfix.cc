/**
 * chanfix.cc
 * 
 * Copyright (C) 2003	Reed Loden <reed@reedloden.com>
 *			Matthias Crauwels <ultimate_@wol.be>
 *			Jimmy Lipham <music0m@alltel.net>
 *			Neil Spierling <sirvulcan@gmail.com>
 *
 * Automatically and manually fix opless and taken over channels
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 *
 * $Id$
 */

#include	<csignal>
#include	<cstdarg>
#include	<ctime>
#include	<iomanip>
#include	<iostream>
#include	<map>
#include	<new>
#include	<sstream>
#include	<string>
#include	<utility>
#include	<vector>

#include	<sys/resource.h>

#include	"libpq++.h"

#include	"gnuworld_config.h"
#include	"client.h"
#include	"EConfig.h"
#include	"Network.h"
#include	"server.h"
#include	"StringTokenizer.h"

#include	"chanfix.h"
#include	"chanfix_defs.h"
#include	"chanfix_misc.h"
#include	"chanfixCommands.h"
#include	"responses.h"
#include	"sqlChanOp.h"
#include	"sqlChannel.h"
#include	"sqlUser.h"

#ifdef CHANFIX_HAVE_BOOST_THREAD
#include	<boost/thread/thread.hpp>
#endif /* CHANFIX_HAVE_BOOST_THREAD */

RCSTAG("$Id$");

namespace gnuworld
{

namespace cf
{

short currentDay;

/*
 *  Exported function used by moduleLoader to gain an
 *  instance of this module.
 */

extern "C"
{
  xClient* _gnuwinit(const std::string& args)
  { 
    return new chanfix( args );
  }

} 

/**
 * This constructor calls the base class constructor.  The xClient
 * constructor will open the configuration file given and retrieve
 * basic client info (nick/user/host/etc).
 * Any additional processing must be done here.
 */
chanfix::chanfix( const std::string& configFileName )
 : xClient( configFileName )
{
/* Load the config file */
readConfigFile(configFileName);

/* Initial state */
currentState = INIT;

/* Initial finding of the channel service */
chanServLinked = false;

/* Initial update status */
updateInProgress = false;

std::string dbString = "host=" + sqlHost + " dbname=" + sqlDB
  + " port=" + sqlPort + " user=" + sqlUsername + " password=" + sqlPass;

theManager = sqlManager::getInstance(dbString);

/* Open our logfiles for writing */
adminLog.open(adminLogFile.c_str(), std::ios::out | std::ios::app);
debugLog.open(debugLogFile.c_str(), std::ios::out | std::ios::app);

/* Register the commands we want to use */
RegisterCommand(new ADDFLAGCommand(this, "ADDFLAG",
	"<username> <flag>",
	3,
	sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN
	));
RegisterCommand(new ADDHOSTCommand(this, "ADDHOST",
	"<username> <nick!user@host>",
	3,
	sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN
	));
RegisterCommand(new ADDNOTECommand(this, "ADDNOTE",
	"<#channel> <reason>",
	3,
	sqlUser::F_COMMENT
	));
RegisterCommand(new ADDUSERCommand(this, "ADDUSER",
	"<username> [host]",
	2,
	sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN
	));
RegisterCommand(new ALERTCommand(this, "ALERT",
	"<#channel> [reason]",
	2,
	sqlUser::F_COMMENT
	));
RegisterCommand(new BLOCKCommand(this, "BLOCK",
	"<#channel> <reason>",
	3,
	sqlUser::F_BLOCK
	));
RegisterCommand(new CANFIXCommand(this, "CANFIX",
	"<#channel>",
	2,
	0
	));
RegisterCommand(new CHANFIXCommand(this, "CHANFIX",
	"<#channel> [override] [contact]",
	2,
	sqlUser::F_CHANFIX
	));
RegisterCommand(new CHECKCommand(this, "CHECK",
	"<#channel>",
	2,
	0
	));
#ifdef CHANFIX_DEBUG
RegisterCommand(new DEBUGCommand(this, "DEBUG",
	"<ROTATE|UPDATE>",
	2,
	sqlUser::F_OWNER
	));
#endif /* CHANFIX_DEBUG */
RegisterCommand(new DELFLAGCommand(this, "DELFLAG",
	"<username> <flag>",
	3,
	sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN
	));
RegisterCommand(new DELHOSTCommand(this, "DELHOST",
	"<username> <nick!user@host>",
	3,
	sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN
	));
RegisterCommand(new DELNOTECommand(this, "DELNOTE",
	"<#channel> <note_id>",
	3,
	sqlUser::F_COMMENT
	));
RegisterCommand(new DELUSERCommand(this, "DELUSER",
	"<username>",
	2,
	sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN
	));
RegisterCommand(new HELPCommand(this, "HELP",
	"[command]",
	1,
	0
	));
RegisterCommand(new HISTORYCommand(this, "HISTORY",
	"<#channel>",
	2,
	0
	));
RegisterCommand(new INFOCommand(this, "INFO",
	"<#channel>",
	2,
	0
	));
RegisterCommand(new INVITECommand(this, "INVITE",
	"",
	1,
	sqlUser::F_OWNER
	));
RegisterCommand(new LASTCOMCommand(this, "LASTCOM",
	"[amount of commands] [days from]",
	1,
	sqlUser::F_OWNER
	));
RegisterCommand(new LISTBLOCKEDCommand(this, "LISTBLOCKED",
	"",
	1,
	sqlUser::F_BLOCK
	));
RegisterCommand(new LISTHOSTSCommand(this, "LISTHOSTS",
	"[username]",
	1,
	sqlUser::F_LOGGEDIN
	));
RegisterCommand(new OPLISTCommand(this, "OPLIST",
	"<#channel> [-all] [-days]",
	2,
	0
	));
RegisterCommand(new OPNICKSCommand(this, "OPNICKS",
	"<#channel>",
	2,
	0
	));
#ifdef ENABLE_QUOTE
RegisterCommand(new QUOTECommand(this, "QUOTE",
	"<text>",
	2,
	sqlUser::F_OWNER
	));
#endif /* ENABLE_QUOTE */
RegisterCommand(new REHASHCommand(this, "REHASH",
	"",
	1,
	sqlUser::F_OWNER
	));
RegisterCommand(new RELOADCommand(this, "RELOAD",
	"[reason]",
	1,
	sqlUser::F_OWNER
	));
RegisterCommand(new REQUESTOPCommand(this, "REQUESTOP",
	isAllowingTopOpAlert() ? "<#channel> [contact]" : "<#channel>",
	2,
	0
	));
RegisterCommand(new SCORECommand(this, "SCORE",
	"<#channel> [account|=nick]",
	2,
	0
	));
RegisterCommand(new SCORECommand(this, "CSCORE",
	"<#channel> [account|=nick]",
	2,
	0
	));
RegisterCommand(new SAYCommand(this, "SAY",
	"<#channel> <text>",
	3,
	sqlUser::F_OWNER
	));
RegisterCommand(new SETCommand(this, "SET",
	"<option> <value>",
	3,
	sqlUser::F_OWNER
	));
RegisterCommand(new SETGROUPCommand(this, "SETGROUP",
	"<username> <group>",
	3,
	sqlUser::F_USERMANAGER
	));
RegisterCommand(new SHUTDOWNCommand(this, "SHUTDOWN",
	"[reason]",
	1,
	sqlUser::F_OWNER
	));
RegisterCommand(new STATUSCommand(this, "STATUS",
	"",
	1,
	0
	));
RegisterCommand(new SUSPENDCommand(this, "SUSPEND",
	"<username>",
	2,
	sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN
	));
RegisterCommand(new UNALERTCommand(this, "UNALERT",
	"<#channel>",
	2,
	sqlUser::F_COMMENT
	));
RegisterCommand(new UNBLOCKCommand(this, "UNBLOCK",
	"<#channel>",
	2,
	sqlUser::F_BLOCK
	));
RegisterCommand(new UNSUSPENDCommand(this, "UNSUSPEND",
	"<username>",
	2,
	sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN
	));
RegisterCommand(new USERSCORESCommand(this, "USERSCORES",
	"<account>",
	2,
	sqlUser::F_LOGGEDIN
	));
RegisterCommand(new USETCommand(this, "USET",
	"[username] <option> <value>",
	3,
	sqlUser::F_LOGGEDIN
	));
RegisterCommand(new WHOGROUPCommand(this, "WHOGROUP",
	"[group]",
	1,
	sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN
	));
RegisterCommand(new WHOISCommand(this, "WHOIS",
	"<username|=nick|*> [-modif]",
	2,
	sqlUser::F_LOGGEDIN
	));

/* Set our current day. */
setCurrentDay();

/* Precache our chanOps. */
precacheChanOps();

/* Precache our channels. */
precacheChannels();

/* Precache our staff. */
precacheUsers();

/* Load help messages. */
loadHelpTable();

/* Load our translation tables. */
loadTranslationTable();

/* Set up our timer. */
theTimer = new Timer();

}

chanfix::~chanfix()
{
}

/* Load (or reload) the configuration file */
void chanfix::readConfigFile(const std::string& configFileName)
{
chanfixConfig = new (std::nothrow) EConfig(configFileName);
assert(chanfixConfig != 0);

/* Config file processing */
consoleChan = chanfixConfig->Require("consoleChan")->second ;
consoleChanModes = chanfixConfig->Require("consoleChanModes")->second ;
sendConsoleNotices = atob(chanfixConfig->Require("sendConsoleNotices")->second) ;
joinChanModes = chanfixConfig->Require("joinChanModes")->second ;
enableAutoFix = atob(chanfixConfig->Require("enableAutoFix")->second) ;
enableChanFix = atob(chanfixConfig->Require("enableChanFix")->second) ;
enableChannelBlocking = atob(chanfixConfig->Require("enableChannelBlocking")->second) ;
autoFixNotice = atob(chanfixConfig->Require("autoFixNotice")->second) ;
manualFixNotice = atob(chanfixConfig->Require("manualFixNotice")->second) ; 
joinChannels = atob(chanfixConfig->Require("joinChannels")->second) ;
stopAutoFixOnOp = atob(chanfixConfig->Require("stopAutoFixOnOp")->second) ;
stopChanFixOnOp = atob(chanfixConfig->Require("stopChanFixOnOp")->second) ;
allowTopOpFix = atob(chanfixConfig->Require("allowTopOpFix")->second) ;
allowTopOpAlert = atob(chanfixConfig->Require("allowTopOpAlert")->second) ;
topOpPercent = atoi((chanfixConfig->Require("topOpPercent")->second).c_str()) ;
minFixScore = atoi((chanfixConfig->Require("minFixScore")->second).c_str()) ;
minCanFixScore = atoi((chanfixConfig->Require("minCanFixScore")->second).c_str()) ;
minRequestOpTime = atoi((chanfixConfig->Require("minRequestOpTime")->second).c_str()) ;
version = atoi((chanfixConfig->Require("version")->second).c_str()) ;
useBurstToFix = atob(chanfixConfig->Require("useBurstToFix")->second) ;
numServers = atoi((chanfixConfig->Require("numServers")->second).c_str()) ;
minServersPresent = atoi((chanfixConfig->Require("minServersPresent")->second).c_str()) ;
chanServName = chanfixConfig->Require("chanServName")->second ;
numTopScores = atoi((chanfixConfig->Require("numTopScores")->second).c_str()) ;
minClients = atoi((chanfixConfig->Require("minClients")->second).c_str()) ;
clientNeedsIdent = atob(chanfixConfig->Require("clientNeedsIdent")->second) ;
connectCheckFreq = atoi((chanfixConfig->Require("connectCheckFreq")->second).c_str()) ;
adminLogFile = chanfixConfig->Require("adminLogFile")->second ;
debugLogFile = chanfixConfig->Require("debugLogFile")->second ;

/* Set up the channels that chanfix should join */
EConfig::const_iterator ptr = chanfixConfig->Find("joinChan");
while (ptr != chanfixConfig->end() && ptr->first == "joinChan") {
  chansToJoin.push_back(ptr->second);
  ++ptr;
}

/* Make sure that the consoleChan is not a joinChan */
for (joinChansType::iterator ptr = chansToJoin.begin();
     ptr != chansToJoin.end(); ++ptr) {
  if (!strcasecmp((*ptr).c_str(), consoleChan.c_str())) {
    /* Found, remove it from the list of chansToJoin */
    chansToJoin.erase(ptr);
    break;
  }
}

/* Database processing */
sqlHost = chanfixConfig->Require("sqlHost")->second;
sqlPort = chanfixConfig->Require("sqlPort")->second;
sqlDB = chanfixConfig->Require("sqlDB")->second;
sqlUsername = chanfixConfig->Require("sqlUser")->second;
sqlPass = chanfixConfig->Require("sqlPass")->second;

elog	<< "chanfix::readConfigFile> Configuration loaded!"
	<< std::endl;
}


/* Register a new command */
bool chanfix::RegisterCommand( Command *theCommand )
{
UnRegisterCommand( theCommand->getName() ) ;
return commandMap.insert( commandMapType::value_type(theCommand->getName(), theCommand)).second;
}

/* UnRegister a command */
bool chanfix::UnRegisterCommand( const std::string& commName )
{
commandMapType::iterator ptr = commandMap.find( commName ) ;
if ( ptr == commandMap.end() ) {
  return false ;
}
delete ptr->second ;
commandMap.erase( ptr ) ;
return true ;
}

/* OnShutdown */
void chanfix::OnShutdown(const std::string& reason)
{
MyUplink->UnloadClient(this, reason);
}

/* OnSignal */
void chanfix::OnSignal(int sig)
{
if (sig == SIGHUP) {
  /* Close/reopen log files */
  logAdminMessage("---------- End of log file. Rotating... ----------");
  if (adminLog.is_open())
    adminLog.close();

  adminLog.open(adminLogFile.c_str(), std::ios::out | std::ios::app);
}

xClient::OnSignal(sig);
}

/* OnAttach */
void chanfix::OnAttach()
{
/**
 * Don't send END_OF_BURST (EB) so we can stay in burst mode 
 * indefinitely in order to be able to do TS-1.
 */
if (useBurstToFix && version < 12) /* Not needed for u2.10.12+ */
  MyUplink->setSendEB(false);

/**
 * Set our uplink as our main server for our commands.
 */
for (commandMapType::iterator ptr = commandMap.begin(); ptr != commandMap.end(); ++ptr) {
  ptr->second->setServer(MyUplink);
}

/**
 * Register for global network events
 */
MyUplink->RegisterEvent( EVT_ACCOUNT, this );
MyUplink->RegisterEvent( EVT_KILL, this );
MyUplink->RegisterEvent( EVT_QUIT, this );
MyUplink->RegisterEvent( EVT_BURST_CMPLT, this );
MyUplink->RegisterEvent( EVT_NETJOIN, this );
MyUplink->RegisterEvent( EVT_NETBREAK, this );
MyUplink->RegisterEvent( EVT_NICK, this );
MyUplink->RegisterEvent( EVT_SERVERMODE, this );

/**
 * Register for all channel events
 */
MyUplink->RegisterChannelEvent( xServer::CHANNEL_ALL, this );

xClient::OnAttach() ;
}

/**
 * Thread class only used for score updates, updates on reload and shutdown are not threaded
 */
#ifdef CHANFIX_HAVE_BOOST_THREAD
class ClassUpdateDB {
  public:
    ClassUpdateDB(chanfix& cf) : cf_(cf) {}
    void operator()() {
      cf_.updateDB();
      return;
    }

private:
        chanfix& cf_;
};
#endif /* CHANFIX_HAVE_BOOST_THREAD */

/* OnTimer */
void chanfix::OnTimer(const xServer::timerID& theTimer, void*)
{
time_t theTime;
if (theTimer == tidGivePoints) {
  /* 5 min timer, loop through channels and give all ops a point! */
  giveAllOpsPoints();

  /* Refresh Timer */
  theTime = time(NULL) + POINTS_UPDATE_TIME;
  tidGivePoints = MyUplink->RegisterTimer(theTime, this, NULL);
  }
else if (theTimer == tidAutoFix) {
  autoFix();

  /* Refresh Timer */
  theTime = time(NULL) + CHECK_CHANS_TIME;
  tidAutoFix = MyUplink->RegisterTimer(theTime, this, NULL);
  }
else if (theTimer == tidCheckDB) {
  /*checkDBConnection();*/

  /* Refresh Timer */
  theTime = time(NULL) + connectCheckFreq;
  tidCheckDB = MyUplink->RegisterTimer(theTime, this, NULL);
  }
else if (theTimer == tidFixQ) {
  processQueue();

  /* Refresh Timer */
  theTime = time(NULL) + PROCESS_QUEUE_TIME;
  tidFixQ = MyUplink->RegisterTimer(theTime, this, NULL);
}
else if (theTimer == tidRotateDB) {
  /* Clean-up the database if its 00 GMT */
  rotateDB();

  /* Refresh Timer */
  theTime = time(NULL) + getSecsTilMidnight();
  tidRotateDB = MyUplink->RegisterTimer(theTime, this, NULL);
}
else if (theTimer == tidUpdateDB) {
  /* Prepare to synchronize the database in a thread */
  prepareUpdate(true);

  /* Refresh Timer */
  theTime = time(NULL) + SQL_UPDATE_TIME;
  tidUpdateDB = MyUplink->RegisterTimer(theTime, this, NULL);
}
else if (theTimer == tidTempBlocks) {
  /* Remove expired temporary blocks */
  expireTempBlocks();
	
  /* Refresh Timer */
  theTime = time(NULL) + TEMPBLOCKS_CHECK_TIME;
  tidTempBlocks = MyUplink->RegisterTimer(theTime, this, NULL);
}
}

/* OnDetach */
void chanfix::OnDetach( const std::string& reason )
{
/* Save our database */
prepareUpdate(false);

/* Delete our config */
delete chanfixConfig; chanfixConfig = 0;

/* Delete our timer */
delete theTimer; theTimer = 0;

/* Delete our commands */
for (commandMapType::iterator ptr = commandMap.begin();
     ptr != commandMap.end(); ++ptr) {
  delete ptr->second;
}
commandMap.clear();

/* Delete our sqlManager */
theManager->removeManager();

/* Finally, close our logs */
if (adminLog.is_open())
  adminLog.close();
if (debugLog.is_open())
  debugLog.close();

xClient::OnDetach( reason ) ;
}

/* OnConnect */
void chanfix::OnConnect()
{
/* If we have just reloaded, we won't be in BURST. */
if (currentState == INIT) {
  findChannelService();
  changeState(RUN);
}

xClient::OnConnect() ;
}

/* OnDisconnect */
void chanfix::OnDisconnect()
{
xClient::OnDisconnect() ;
}

void chanfix::OnPrivateMessage( iClient* theClient,
	const std::string& Message, bool)
{
sqlUser* theUser = isAuthed(theClient->getAccount());

if (currentState == BURST) {
  if (theUser)
    SendTo(theClient,
           getResponse(theUser,
                       language::no_commands_during_burst,
                       std::string("Sorry, I do not accept commands during a burst.")).c_str());
  else
    SendTo(theClient,
           getResponse(theUser,
                       language::no_commands_during_burst_noper,
                       std::string("Sorry, I'm too busy at the moment. Please try again soon.")).c_str());
  return;
}

StringTokenizer st(Message);
if (st.empty())
  return;

const std::string Command = string_upper(st[0]);

if (!theClient->isOper()) {
  if (Command != "CANFIX" && Command != "HELP" && Command != "REQUESTOP") {
    if (!theUser || theUser->getNeedOper())
      return;
  }
}

commandMapType::iterator commHandler = commandMap.find(Command);
if (commHandler == commandMap.end()) {
  SendTo(theClient,
         getResponse(theUser,
                     language::unknown_command,
                     std::string("Unknown command.")).c_str());
  return;
}

if (st.size() < commHandler->second->getNumParams()) {
  commHandler->second->Usage(theClient);
  return;
}

/* If you change this code, remember to change it in HELPCommand.cc */
sqlUser::flagType requiredFlags = commHandler->second->getRequiredFlags();
if (requiredFlags) {
  if (!theUser) {
    SendTo(theClient,
           getResponse(theUser,
                       language::need_to_auth,
                       std::string("You need to authenticate to use this command.")).c_str());
    return;
  }

  if (theUser->getIsSuspended()) {
    SendTo(theClient,
           getResponse(theUser,
                       language::access_suspended,
                       std::string("Your access to this service is suspended.")).c_str());
    return;
  }

  if (!theUser->matchHost(theClient->getRealNickUserHost().c_str())) {
    SendTo(theClient,
           getResponse(theUser,
                       language::host_not_matching,
                       std::string("Your current host does not match any registered hosts for your username.")).c_str());
    return;
  }

  if (requiredFlags != sqlUser::F_LOGGEDIN &&
      !theUser->getFlag(requiredFlags)) {
    if (getFlagChar(requiredFlags) != ' ')
      SendTo(theClient,
             getResponse(theUser,
                         language::requires_flag,
                         std::string("This command requires flag '%c'.")).c_str(),
                                     getFlagChar(requiredFlags));
    else
      SendTo(theClient,
             getResponse(theUser,
                         language::requires_flags,
                         std::string("This command requires one of these flags: \"%s\".")).c_str(),
                                     getFlagsString(requiredFlags).c_str());
    return;
  }
}

if (theUser) {
  theUser->setLastSeen(currentTime());
  theUser->commit();
}

commHandler->second->Exec(theClient, theUser ? theUser : NULL, Message);

xClient::OnPrivateMessage(theClient, Message);
}

void chanfix::OnCTCP( iClient* theClient, const std::string& CTCP,
		const std::string& Message, bool Secure )
{
StringTokenizer st(CTCP);

if (st.empty()) return;

const std::string Command = string_upper(st[0]);

if (Command == "DCC") {
  DoCTCP(theClient, CTCP, "REJECT");
} else if (Command == "PING" || Command == "ECHO") {
  DoCTCP(theClient, CTCP, Message);
} else if (Command == "VERSION") {
  DoCTCP(theClient, CTCP, "evilnet development - GNUWorld " CHANFIX_PACKAGE_STRING " [compiled "__DATE__" "__TIME__"]");
} else if (Command == "WHODUNIT?") {
  DoCTCP(theClient, CTCP, "reed, ULtimaTe_, Compy, SiRVulcaN");
} else if (Command == "SUBVERSION") {
  DoCTCP(theClient, CTCP, rcsId);
} else if (Command == "GENDER") {
  DoCTCP(theClient, CTCP, "Gender pending vote, but for now I'll be whatever you want!");
}

xClient::OnCTCP(theClient, CTCP, Message, Secure);
}

// Burst any channels.
void chanfix::BurstChannels()
{
xClient::BurstChannels();

MyUplink->JoinChannel(this, consoleChan, consoleChanModes);

for (joinChansType::size_type i = 0; i < chansToJoin.size(); i++) {
  /* Burst our channels */
  MyUplink->JoinChannel(this, chansToJoin[i], joinChanModes);
}

/* Start our timers */
startTimers();
}

/* OnChannelEvent */
void chanfix::OnChannelEvent( const channelEventType& whichEvent,
	Channel* theChan,
	void* data1, void* data2, void* data3, void* data4 )
{
iClient* theClient = 0;

/* If we are not running, we don't want to be giving points. */
if (currentState != RUN) return;

/* If this channel is too small, don't worry about it. */
if (theChan->size() < minClients)  return;

switch( whichEvent )
	{
	case EVT_JOIN:
		{
		/* If this is the operChan or supportChan, op opers on join */
		theClient = static_cast< iClient* >( data1 );
		if (theClient->isOper()) {
		  for (joinChansType::iterator ptr = chansToJoin.begin();
		       ptr != chansToJoin.end(); ++ptr) {
		    if (!strcasecmp((*ptr).c_str(), theChan->getName().c_str())) {
		      /* Found, op the user */
		      Op(theChan, theClient);
		      break;
		    }
		  }
		}
		break ;
		}
	case EVT_KICK:
	case EVT_PART:
		{
		theClient = static_cast< iClient* >( data1 );
		lostOp(theChan->getName(), theClient, NULL);
		break ;
		}
	case EVT_SERVERMODE:
		{
		elog << "chanfix: GOT SERVER MODE EVENT!" << std::endl;
		break ;
		}
	default:
		break ;
	}

xClient::OnChannelEvent( whichEvent, theChan,
	data1, data2, data3, data4 ) ;
}

void chanfix::OnChannelModeO( Channel* theChan, ChannelUser* theUser,
			const xServer::opVectorType& theTargets)
{
/* if (currentState != RUN) return; */

if (theUser) {
  iServer* theServer = Network->findServer(theUser->getClient()->getIntYY());
  if (theServer && theServer != MyUplink->getUplink() && theServer->isService()) {
    if (!isTempBlocked(theChan->getName()))
      tempBlockList.insert(tempBlockType::value_type(theChan->getName(), currentTime()));
  }
}

	
if (theChan->size() < minClients)
  return;

if (!canScoreChan(theChan))
  return;

for (xServer::opVectorType::const_iterator ptr = theTargets.begin();
     ptr != theTargets.end(); ++ptr) {
  ChannelUser* tmpUser = ptr->second;

  if (tmpUser->getClient()->getAccount() == "")
    continue;

  bool polarity = ptr->first;
  if (polarity) {
    // Someone is opped
    gotOpped(theChan, tmpUser->getClient());

    // If the channel is being fixed and the op is done by a user,
    // cancel the fix, as there is an awake op
    if (theUser)
      stopFixingChan(theChan, false);
  } else {
    // Someone is deopped
    lostOp(theChan->getName(), tmpUser->getClient(), NULL);
  } // if
} // for
}

bool chanfix::logLastComMessage(iClient* theClient, const std::string& Message)
{
  StringTokenizer st(Message);
  if (st.empty())
    return false;

  const std::string Command = string_upper(st[0]);

  std::string log;
  PgDatabase* cacheCon = theManager->getConnection();

  static const char *Main = "INSERT into comlog (ts,user_name,command) VALUES (now()::abstime::int4,'";

  std::stringstream insertString;
  insertString	<< Main
		<< theClient->getAccount()
		<< "','"
		<< Command
		<< " ";

  unsigned int pos = 1;
  while (pos < st.size()) {
    insertString << st[pos]
		 << " ";
    pos++;
  }

  insertString	<< "')"
		<< std::ends;

  if (!cacheCon->ExecCommandOk(insertString.str().c_str())) {
    elog << "sqlChannel::Insert> Something went wrong: "
	 << cacheCon->ErrorMessage()
	 << std::endl;
  }

  theManager->removeConnection(cacheCon);

  return true;
}

/* msgTopOps */
bool chanfix::msgTopOps(Channel* netChan) {

  int opCount = 0;
  sqlChanOp* curOp = 0;
  bool inChan = true;

  chanOpsType myOps = getMyOps(netChan);

  for (chanOpsType::iterator opPtr = myOps.begin();
     opPtr != myOps.end() && (opCount < OPCOUNT); opPtr++) {
 
    curOp = *opPtr;
    opCount++;

    inChan = accountIsOnChan(netChan->getName(), curOp->getAccount());
    if (!inChan) {
      typedef std::list < iClient* > accountListType;
      const iClient* curAccount;
      accountListType accountList;
      authMapType::iterator cAuth = authMap.find(curOp->getAccount());
      if (cAuth == authMap.end())
	break;
      
      accountList = cAuth->second;

      if (accountList.empty())
        break;

      for (accountListType::iterator cptr = accountList.begin(); (cptr != accountList.end()); cptr++)
      {
        curAccount = *cptr;

        MyUplink->Write("%s P %s :%s channel modes have been removed to allow you to return. Please return so that I can op you during the channel fixing process\r\n",
                  getCharYYXXX().c_str(), curAccount->getCharYYXXX().c_str(), netChan->getName().c_str());

        MyUplink->Write("%s P %s :\002DO NOT REPLY TO THIS MESSAGE\002\r\n",
                  getCharYYXXX().c_str(), curAccount->getCharYYXXX().c_str());

      }
    }
  }

  return true;
}

/* OnEvent */
void chanfix::OnEvent( const eventType& whichEvent,
	void* data1, void* data2, void* data3, void* data4 )
{
switch(whichEvent)
	{
	case EVT_ACCOUNT:
		{
		iClient* tmpUser = static_cast< iClient* >( data1 ) ;
		authMapType::iterator ptr = authMap.find(tmpUser->getAccount());
		if (ptr != authMap.end()) {
			/* This user is already logged in, just add the iClient
			 * (if they aren't in the list already)
			 */
			authMapType::mapped_type::iterator listPtr = std::find(ptr->second.begin(),
					ptr->second.end(),
					tmpUser);
			if (listPtr == ptr->second.end())
			  ptr->second.push_back(tmpUser);
		} else {
			/* Add the map entry AND the initial list entry */
			authMapType::mapped_type theList;
			theList.push_back(tmpUser);
			authMap.insert(authMapType::value_type(tmpUser->getAccount(), theList));
		}
		break;
		}
	case EVT_BURST_CMPLT:
		{
		if (currentState != SPLIT)
		  changeState(RUN);
		break;
		}
	case EVT_NETJOIN:
	case EVT_NETBREAK:
		{
		iServer* theServer = static_cast< iServer* >(data1);
		checkChannelServiceLink(theServer, whichEvent);
		checkNetwork();
		break;
		}
	case EVT_KILL:
	case EVT_QUIT:
		{
		iClient* theClient = static_cast< iClient* >
			((whichEvent == EVT_QUIT) ?
				 data1 :
				 data2 );

		clientOpsType* myOps = findMyOps(theClient);
		for (clientOpsType::iterator ptr = myOps->begin();
		     ptr != myOps->end(); ptr++)
		  lostOp(*ptr, theClient, myOps);
		
		/* Now we need to remove this iClient from the auth map */
		authMapType::iterator ptr = authMap.find(theClient->getAccount());
		
		if (ptr != authMap.end()) {
		  ptr->second.erase(std::find(ptr->second.begin(), ptr->second.end(), theClient));
			
		  /* If the list is empty, remove the map entry */
		  if (ptr->second.empty())
		    authMap.erase(theClient->getAccount());
		}
		break;
		}
	case EVT_NICK:
		{
		iClient* tmpUser = static_cast< iClient* >( data1 );
		if (tmpUser->isModeR()) {
		  /* Check to see if this current account is already mapped,
		   * Then check to see if this nick is already there
		   */
		  authMapType::iterator ptr = authMap.find(tmpUser->getAccount());
		  if (ptr != authMap.end()) {
		    /* Already have an entry, just add the iClient if not there already */
		    authMapType::mapped_type::iterator listPtr = std::find(ptr->second.begin(),
					ptr->second.end(),
					tmpUser);
		    if (listPtr == ptr->second.end())
		      ptr->second.push_back(tmpUser);
		    
		  } else {
		    authMapType::mapped_type theList;
		    theList.push_back(tmpUser);
		    authMap.insert(authMapType::value_type(tmpUser->getAccount(), theList));
		  }
		}
		break;
		}
	}

xClient::OnEvent( whichEvent, data1, data2, data3, data4 ) ;
}

bool chanfix::serverNotice( Channel* theChannel, const char* format, ... )
{
char buf[ 1024 ] = { 0 } ;
va_list _list ;
va_start( _list, format ) ;
vsnprintf( buf, 1024, format, _list ) ;
va_end( _list ) ;

std::stringstream s;
s	<< MyUplink->getCharYY()
	<< " O "
	<< theChannel->getName()
	<< " :"
	<< buf
	<< std::ends;

Write( s );

return false;
}

/**
 * Send a notice to a channel from the server.
 * TODO: Move this method to xServer.
 */
bool chanfix::serverNotice( Channel* theChannel, const std::string& Message)
{
std::stringstream s;
s	<< MyUplink->getCharYY()
	<< " O "
	<< theChannel->getName()
	<< " :"
	<< Message
	<< std::ends;

Write( s );
return false;
}

bool chanfix::logAdminMessage(const char* format, ... )
{
char buf[ 1024 ] = { 0 } ;
va_list _list ;
va_start( _list, format ) ;
vsnprintf( buf, 1024, format, _list ) ;
va_end( _list ) ;

// Try and locate the relay channel.
if (sendConsoleNotices) {
  Channel* tmpChan = Network->findChannel(consoleChan);
  if (!tmpChan)
    return false;

  std::string message = std::string( "[" ) + nickName + "] " + buf ;
  serverNotice(tmpChan, message);
}

/* Everything sent here is also logged to a file on disk */
if (adminLog.is_open()) {
  std::string theLog = std::string( "[" ) + tsToDateTime(currentTime(), true) + "] " + buf ;
  adminLog << theLog << std::endl;
}

return true;
}

bool chanfix::logDebugMessage(const char* format, ... )
{
char buf[ 1024 ] = { 0 } ;
va_list _list ;
va_start( _list, format ) ;
vsnprintf( buf, 1024, format, _list ) ;
va_end( _list ) ;

// Try and locate the relay channel.
Channel* tmpChan = Network->findChannel(consoleChan);
if (!tmpChan)
  return false;

std::string message = std::string( "[" ) + nickName + "] " + buf ;
serverNotice(tmpChan, message);

/* Everything sent here is also logged to a file on disk */
if (debugLog.is_open()) {
  std::string theLog = std::string( "[" ) + tsToDateTime(currentTime(), true) + "] " + buf ;
  debugLog << theLog << std::endl;
}

return true;
}

void chanfix::SendTo(iClient* theClient, const std::string& theMessage)
{
sqlUser* theUser = isAuthed(theClient->getAccount());

if (theUser && !theUser->getUseNotice())
  Message(theClient, theMessage);
else
  Notice(theClient, theMessage);
}

char *chanfix::convertToAscTime(time_t NOW)
{
  time_t *tNow = &NOW;
  struct tm* Now = gmtime(tNow);
  char *ATime = asctime(Now);
  ATime[strlen(ATime)-1] = '\0';
  return ATime;
}

void chanfix::SendFmtTo(iClient* theClient, const std::string& theMessage)
{
char buffer[512] = { 0 };
char *b = buffer ;
const char *m = 0 ;

sqlUser* theUser = isAuthed(theClient->getAccount());

for (m = theMessage.c_str(); *m != 0; m++) {
  if (*m == '\n' || *m == '\r') {
    *b='\0';
    if (theUser) {
      if (!theUser->getUseNotice())
	MyUplink->Write("%s P %s :%s\r\n",
		getCharYYXXX().c_str(),
		theClient->getCharYYXXX().c_str(),
		buffer);
      else
	MyUplink->Write("%s O %s :%s\r\n",
		getCharYYXXX().c_str(),
		theClient->getCharYYXXX().c_str(),
		buffer);
    }
    b=buffer;
  }
  else if (b < buffer + 509)
    *(b++)=*m;
}

*b='\0'; // What's this for? 'b' isn't used anymore.

if (theUser) {
  if (!theUser->getUseNotice())
    MyUplink->Write("%s P %s :%s\r\n",
		    getCharYYXXX().c_str(),
		    theClient->getCharYYXXX().c_str(),
		    buffer);
  else
    MyUplink->Write("%s O %s :%s\r\n",
		    getCharYYXXX().c_str(),
		    theClient->getCharYYXXX().c_str(),
		    buffer);
}

return;
}

void chanfix::SendTo(iClient* theClient, const char *Msg, ...)
{
char buffer[ 1024 ] = { 0 } ;
va_list list;

va_start( list, Msg ) ;
vsprintf( buffer, Msg, list ) ;
va_end( list ) ;

sqlUser* theUser = isAuthed(theClient->getAccount());

if (theUser && !theUser->getUseNotice())
  Message(theClient, "%s", buffer);
else
  Notice(theClient, "%s", buffer);
}

void chanfix::doSqlError(const std::string& theQuery, const std::string& theError)
{
/* First, log it to error out */
elog	<< "SQL> Whilst executing: "
	<< theQuery
	<< std::endl;
elog	<< "SQL> "
	<< theError
	<< std::endl;
return;
}

void chanfix::precacheChanOps()
{
elog << "*** [chanfix::precacheChanOps] Precaching chanops." << std::endl;

/* Get a connection instance to our backend */
PgDatabase* cacheCon = theManager->getConnection();

/* Check for the backup table. If it exists, something went wrong. */
/* SELECT count(*) FROM pg_tables WHERE tablename = 'chanOpsBackup' */
/* SELECT chanOpsBackup FROM information_schema.tables */
if (!cacheCon->ExecTuplesOk("SELECT count(*) FROM pg_tables WHERE tablename = 'chanOpsBackup'")) {
  elog	<< "*** [chanfix::precacheChanOps]: Error checking for backup table presence: " 
		<< cacheCon->ErrorMessage()
		<< std::endl;
  return;
}

if (cacheCon->Tuples() && atoi(cacheCon->GetValue(0, 0))) {
  elog	<< "*** [chanfix::precacheChanOps]: Backup table still exists! "
	<< "Something must have gone wrong on the last update. Exiting..."
	<< std::endl;
  ::exit(0);
}

/* Retrieve the list of chanops */
std::stringstream theQuery;
theQuery	<< "SELECT channel,account,last_seen_as,ts_firstopped,ts_lastopped,day0,day1,day2,day3,day4,day5,day6,day7,day8,day9,day10,day11,day12,day13 FROM chanOps"
		;

elog		<< "*** [chanfix::precacheChanOps]: Loading chanOps and their points ..." 
		<< std::endl;

if (cacheCon->ExecTuplesOk(theQuery.str().c_str())) {
  for (int i = 0 ; i < cacheCon->Tuples(); i++) {
     sqlChanOp* newOp = new (std::nothrow) sqlChanOp(theManager);
     assert( newOp != 0 ) ;

     newOp->setAllMembers(cacheCon, i);
     sqlChanOpsType::iterator ptr = sqlChanOps.find(newOp->getChannel());
     if (ptr != sqlChanOps.end()) {
       ptr->second.insert(sqlChanOpsType::mapped_type::value_type(newOp->getAccount(),newOp));
     } else {
       sqlChanOpsType::mapped_type theMap;
       theMap.insert(sqlChanOpsType::mapped_type::value_type(newOp->getAccount(), newOp));
       sqlChanOps.insert(sqlChanOpsType::value_type(newOp->getChannel(), theMap));
     }
  }
} else {
  elog	<< "*** [chanfix::precacheChanOps] Error executing query: "
	<< cacheCon->ErrorMessage()
	<< std::endl;
  ::exit(0);
}


elog	<< "*** [chanfix::precacheChanOps]: Done. Loaded "
	<< sqlChanOps.size()
	<< " chanops."
	<< std::endl;

/* Dispose of our connection instance */
theManager->removeConnection(cacheCon);

return;
}

void chanfix::precacheChannels()
{
elog << "*** [chanfix::precacheChannels] Precaching channels." << std::endl;

/* Get a connection instance to our backend */
PgDatabase* cacheCon = theManager->getConnection();

/* Retrieve the list of channels */
std::stringstream theQuery;
theQuery	<< "SELECT id, channel, flags FROM channels"
		;

elog		<< "*** [chanfix::precacheChannels]: Loading channels ..."
		<< std::endl;

if (cacheCon->ExecTuplesOk(theQuery.str().c_str())) {
  for (int i = 0; i < cacheCon->Tuples(); i++) {
     sqlChannel* newChan = new (std::nothrow) sqlChannel(theManager);
     assert( newChan != 0 ) ;

     newChan->setAllMembers(cacheCon, i);
     sqlChanCache.insert(sqlChannelCacheType::value_type(newChan->getChannel(), newChan));
  }
} else {
  elog << "*** [chanfix::precacheChannels] Error executing query: "
    << cacheCon->ErrorMessage()
    << std::endl;
  ::exit(0);
}

elog	<< "*** [chanfix::precacheChannels]: Done. Loaded "
	<< sqlChanCache.size()
	<< " channels."
	<< std::endl;

/* Dispose of our connection instance */
theManager->removeConnection(cacheCon);

return;
}

void chanfix::precacheUsers()
{
elog << "*** [chanfix::precacheUsers] Precaching users." << std::endl;

/* Get a connection instance to our backend */
PgDatabase* cacheCon = theManager->getConnection();

/* Retrieve the list of chanops */
std::stringstream theQuery;
theQuery	<< "SELECT id, user_name, created, last_seen, last_updated, last_updated_by, language_id, faction, flags, issuspended, usenotice, needoper "
		<< "FROM users"
		;

if (cacheCon->ExecTuplesOk(theQuery.str().c_str())) {
  /* First we need to clear the current cache. */
  for (usersMapType::iterator itr = usersMap.begin();
       itr != usersMap.end(); ++itr) {
    delete itr->second;
  }
  usersMap.clear();

  for (int i = 0; i < cacheCon->Tuples(); ++i) {
    sqlUser *newUser = new sqlUser(theManager);
    assert(newUser != 0);

    newUser->setAllMembers(cacheCon, i);
    usersMap.insert(usersMapType::value_type(newUser->getUserName(), newUser));
  }
} else {
  elog << "*** [chanfix::precacheUsers] Error executing query: "
    << cacheCon->ErrorMessage()
    << std::endl;
  ::exit(0);
}

/* Load up the host cache */
for (usersMapType::iterator itr = usersMap.begin();
     itr != usersMap.end(); ++itr) {
  itr->second->loadHostList();
}
	
elog	<< "chanfix::precacheUsers> Loaded "
	<< usersMap.size()
	<< " users."
	<< std::endl;

/* Dispose of our connection instance */
theManager->removeConnection(cacheCon);

return;
}

void chanfix::changeState(STATE newState)
{
if (currentState == newState) return;

/* Start our timer. */
Timer stateTimer;
stateTimer.Start();

/* First, do what we need to exit our current state */
switch( currentState ) {
	case BURST:
	{
	elog	<< "chanfix::changeState> Exiting state BURST"
		<< std::endl;
	break;
	}
	case RUN:
	{
	elog	<< "chanfix::changeState> Exiting state RUN"
		<< std::endl;
	break;
	}
	case SPLIT:
	{
	elog	<< "chanfix::changeState> Exiting state SPLIT"
		<< std::endl;
	break;
	}
	case INIT:
	{
	elog	<< "chanfix::changeState> Exiting state INIT"
		<< std::endl;
	break;
	}
}

currentState = newState;

switch( currentState ) {
	case BURST:
	{
	elog	<< "chanfix::changeState> Entering state BURST"
		<< std::endl;
	break;
	}
	case RUN:
	{
	elog	<< "chanfix::changeState> Entering state RUN"
		<< std::endl;
	break;
	}
	case SPLIT:
	{
	elog	<< "chanfix::changeState> Entering state SPLIT"
		<< std::endl;
	break;
	}
	case INIT:
	{
	elog	<< "chanfix::changeState> Entering state INIT"
		<< std::endl;
	break;
	}

}

elog	<< "Changed state in: "
	<< stateTimer.stopTimeMS()
	<< "ms"
	<< std::endl;
}

sqlChanOp* chanfix::findChanOp(const std::string& channel, const std::string& account)
{
sqlChanOpsType::iterator ptr = sqlChanOps.find(channel);
if (ptr != sqlChanOps.end()) {
  sqlChanOpsType::mapped_type::iterator chanOp = ptr->second.find(account);
  if (chanOp != ptr->second.end())
    /* elog	<< "chanfix::findChanOp> DEBUG: We've got a winner: "
     *	<< chanOp->second->getAccount() << " on " << chanOp->second->getChannel() << "!!" << std::endl;
     */
    return chanOp->second ;
}

return 0;
}

sqlChanOp* chanfix::newChanOp(const std::string& channel, const std::string& account)
{
sqlChanOp* newOp = new (std::nothrow) sqlChanOp(theManager);
assert( newOp != 0 ) ;

/* elog << "chanfix::newChanOp> DEBUG: Added new operator: " << account << " on " << channel << "!!" << std::endl; */

newOp->setChannel(channel);
newOp->setAccount(account);
newOp->setTimeFirstOpped(currentTime());
newOp->setTimeLastOpped(currentTime());

sqlChanOpsType::iterator ptr = sqlChanOps.find(channel);
if (ptr != sqlChanOps.end()) {
  ptr->second.insert(sqlChanOpsType::mapped_type::value_type(account, newOp));
} else {
  sqlChanOpsType::mapped_type theMap;
  theMap.insert(sqlChanOpsType::mapped_type::value_type(account, newOp));
  sqlChanOps.insert(sqlChanOpsType::value_type(channel, theMap));
}

return newOp;
}

sqlChanOp* chanfix::findChanOp(Channel* theChan, iClient* theClient) 
{
return findChanOp(theChan->getName(), theClient->getAccount());
}

sqlChanOp* chanfix::newChanOp(Channel* theChan, iClient* theClient)
{
return newChanOp(theChan->getName(), theClient->getAccount());
}

chanfix::chanOpsType chanfix::getMyOps(const std::string &channel)
{
chanOpsType myOps;

sqlChanOpsType::iterator ptr = sqlChanOps.find(channel);
if (ptr != sqlChanOps.end()) {
  for (sqlChanOpsType::mapped_type::iterator chanOp = ptr->second.begin();
       chanOp != ptr->second.end(); chanOp++) {
    /* elog	<< "chanfix::findChanOp> DEBUG: We've got a winner: "
     *	<< chanOp->second->getAccount() << " on " << chanOp->second->getChannel() << "!!" << std::endl;
     */
    myOps.push_back(chanOp->second);
  }
}

myOps.sort(compare_points);

return myOps;
}

chanfix::chanOpsType chanfix::getMyOps(Channel* theChan)
{
return getMyOps(theChan->getName());
}

size_t chanfix::countMyOps(const std::string& channel)
{
sqlChanOpsType::iterator ptr = sqlChanOps.find(channel);
if (ptr != sqlChanOps.end())
  return ptr->second.size();

return 0;
}

size_t chanfix::countMyOps(Channel* theChan)
{
return countMyOps(theChan->getName());
}

const std::string chanfix::getHostList( sqlUser* User)
{
/* Get a connection instance to our backend */
PgDatabase* cacheCon = theManager->getConnection();

/* Grab the user's host list */
static const char* queryHeader
	= "SELECT host FROM hosts WHERE user_id = ";

std::stringstream theQuery;
theQuery	<< queryHeader 
		<< User->getID()
		;

if (!cacheCon->ExecTuplesOk(theQuery.str().c_str())) {
  elog	<< "chanfix::getHostList> SQL Error: "
	<< cacheCon->ErrorMessage()
	<< std::endl;
  return 0;
}

// SQL Query succeeded
std::stringstream hostlist;
for (int i = 0 ; i < cacheCon->Tuples(); i++)
	{
	if (!i)
	  hostlist << cacheCon->GetValue(i, 0);
	else
	  hostlist << ", " << cacheCon->GetValue(i, 0);
	}
if (hostlist.str() == "") hostlist << "None.";

/* Dispose of our connection instance */
theManager->removeConnection(cacheCon);

return hostlist.str();
}

const std::string escapeSQLChars(const std::string& theString)
{
std::string retMe ;

for( std::string::const_iterator ptr = theString.begin() ;
	ptr != theString.end() ; ++ptr )
	{
	if( *ptr == '\'' )
		{
		retMe += "\\\047" ;
		}
	else if ( *ptr == '\\' )
		{
		retMe += "\\\134" ;
		}
	else
		{
		retMe += *ptr ;
		}
	}
return retMe ;
}

bool atob( std::string str )
{
str = string_lower(str);
if (str == "y" || str == "true" || str == "yes")
  return true;
return false;
}

void chanfix::givePoints(Channel* theChan, iClient* theClient)
{
//No points for unidented clients
if (clientNeedsIdent && !hasIdent(theClient))
  return;

sqlChanOp* thisOp = findChanOp(theChan, theClient);
if (!thisOp) {
  size_t numMyOps = countMyOps(theChan);
  if ((numMyOps >= MAXOPCOUNT) || (!numMyOps && !chanServLinked))
    return; /* No room for new ops or channel service not linked */
  thisOp = newChanOp(theChan, theClient);
  gotOpped(theChan, theClient);
}

thisOp->addPoint();
thisOp->setTimeLastOpped(currentTime()); //Update the time they were last opped

/* elog	<< "chanfix::givePoints> DEBUG: Gave " << thisOp->getAccount()
 *	<< " on " << thisOp->getChannel() << " a point."
 *	<< std::endl;
 */
}

void chanfix::gotOpped(Channel* thisChan, iClient* thisClient)
{
//No tracking for unidented clients
if (clientNeedsIdent && !hasIdent(thisClient))
  return;

/* elog	<< "chanfix::gotOpped> DEBUG: " << thisClient->getAccount()
 *	<< " got opped on " << thisChan->getName()
 *	<< std::endl;
 */
sqlChanOp* thisOp = findChanOp(thisChan, thisClient);
if (!thisOp) {
  size_t numMyOps = countMyOps(thisChan);
  if ((numMyOps >= MAXOPCOUNT) || (!numMyOps && !chanServLinked))
    return; /* No room for new ops or channel service not linked */
  thisOp = newChanOp(thisChan, thisClient);
} else {
  thisOp->setTimeLastOpped(currentTime());
}

thisOp->setLastSeenAs(thisClient->getRealNickUserHost());

clientOpsType* myOps = findMyOps(thisClient);
if (myOps && !myOps->empty()) {
  clientOpsType::iterator ptr = myOps->find(thisChan->getName());
  if (ptr != myOps->end())
    return;
}

myOps->insert(clientOpsType::value_type(thisChan->getName()));
thisClient->setCustomData(this, static_cast< void*>(myOps));

return;
}

void chanfix::lostOp(const std::string& channel, iClient* theClient, clientOpsType* myOps)
{
if (myOps == NULL)
  myOps = findMyOps(theClient);

if (!myOps || myOps->empty())
  return;

clientOpsType::iterator ptr = myOps->find(channel);
if (ptr != myOps->end()) {
  myOps->erase(ptr);
  theClient->setCustomData(this, static_cast< void*>(myOps));
}
}

bool chanfix::hasIdent(iClient* theClient)
{
std::string userName = theClient->getUserName();
if (userName[0] == '~')
  return false;
return true;
}

void chanfix::checkNetwork()
{
if (100 * Network->serverList_size() < numServers * minServersPresent) {
  if (currentState != SPLIT) {
    elog << "chanfix::checkNetwork> DEBUG: Not enough servers linked! Going to SPLIT-state" << std::endl;
    changeState(SPLIT);
  }
  return;
}

if (currentState == SPLIT) {
  elog << "chanfix::checkNetwork> DEBUG: Enough servers linked! Going to BURST-state" << std::endl;
  changeState(BURST);
  return;
}
}

void chanfix::checkChannelServiceLink(iServer* theServer, const eventType& theEvent)
{
if (string_lower(theServer->getName()) == string_lower(chanServName)) {
  if (theEvent == EVT_NETJOIN)
    chanServLinked = true;
  else if (theEvent == EVT_NETBREAK)
    chanServLinked = false;
}
return;
}

void chanfix::JoinChan(Channel* theChan)
{
MyUplink->JoinChannel(this, theChan->getName());
}

void chanfix::PartChan(Channel* theChan)
{
MyUplink->PartChannel(this, theChan->getName(), "");
}

/* Check for the channel service server to see if it is linked. */
void chanfix::findChannelService()
{
iServer* curServer = 0;
for (xNetwork::serverIterator ptr = Network->servers_begin();
     ptr != Network->servers_end(); ptr++) {
   curServer = ptr->second;
   if ((curServer == NULL) || (Network->findFakeServer(curServer) != 0))
     continue;
   if (string_lower(curServer->getName()) == string_lower(chanServName)) {
     chanServLinked = true;
     break;
   }
}
return;
}

const int chanfix::getLastFix(sqlChannel* theChan)
{
/* Get a connection instance to our backend */
PgDatabase* cacheCon = theManager->getConnection();

/* Grab the user's host list */
static const char* queryHeader
	= "SELECT ts,event FROM notes WHERE channelID = ";

std::stringstream theQuery;
theQuery	<< queryHeader 
		<< theChan->getID()
		;

if (!cacheCon->ExecTuplesOk(theQuery.str().c_str())) {
  elog	<< "chanfix::getHostList> SQL Error: "
	<< cacheCon->ErrorMessage()
	<< std::endl;
  return 0;
}

int max_ts = 0;
int ts = 0;
int event;

// SQL Query succeeded
std::stringstream notes;
for (int i = 0 ; i < cacheCon->Tuples(); i++)
{
	ts = atoi(cacheCon->GetValue(i, 0));
	event = atoi(cacheCon->GetValue(i, 1));

	if ((event == sqlChannel::EV_CHANFIX) || (event == sqlChannel::EV_REQUESTOP)) {
		if (ts > max_ts)
			max_ts = ts;
	}
}

/* Dispose of our connection instance */
theManager->removeConnection(cacheCon);

return max_ts;
}

void chanfix::autoFix()
{
/* If autofixing has been disabled, well, forget it. */
if (!enableAutoFix) {
  //elog << "chanfix::autoFix> DEBUG: AutoFix not enabled." << std::endl;
  return;
}

/* If there are too many servers split, don't autofix. */
if (currentState != RUN) {
  //elog << "chanfix::autoFix> DEBUG: currentState != RUN" << std::endl;
  return;
}

/* Start our timer. */
Timer autoFixTimer;
autoFixTimer.Start();

/* Now walk through all channels to find the opless ones. */
Channel* thisChan;
ChannelUser* curUser;
int numOpLess = 0;
tempBlockType::iterator tbPtr;
for (xNetwork::channelIterator ptr = Network->channels_begin(); ptr != Network->channels_end(); ptr++) {
   thisChan = ptr->second;
   bool opLess = true;
   bool hasService = false;
   if (thisChan->size() >= minClients && !isBeingFixed(thisChan)) {
     /* Don't autofix if the chan is temp blocked */
     tbPtr = tempBlockList.find(thisChan->getName());
     if (tbPtr != tempBlockList.end()) continue;

     /* Loop through the channel list to check users for op and umode +k */
     for (Channel::userIterator ptr = thisChan->userList_begin(); ptr != thisChan->userList_end(); ptr++) {
	curUser = ptr->second;
	if (curUser->getClient()->getMode(iClient::MODE_SERVICES)) {
	  hasService = true;
	  break;
	}
	if (curUser->isModeO())
	  opLess = false;
     }
     if (opLess && !hasService) {
       chanOpsType myOps = getMyOps(thisChan);
       if (myOps.empty())
	 continue;

       sqlChannel* sqlChan = getChannelRecord(thisChan);
       if (!sqlChan) sqlChan = newChannelRecord(thisChan);

       if (myOps.begin() != myOps.end())
	 sqlChan->setMaxScore((*myOps.begin())->getPoints());

       if ((sqlChan->getMaxScore() > 
	   static_cast<int>(static_cast<float>(FIX_MIN_ABS_SCORE_END)
	   * MAX_SCORE)) && !sqlChan->getFlag(sqlChannel::F_BLOCKED)) {
	 elog << "chanfix::autoFix> DEBUG: " << thisChan->getName() << " is opless, fixing." << std::endl;
	 autoFixQ.insert(fixQueueType::value_type(thisChan->getName(), currentTime()));
	 numOpLess++;

	 if (doJoinChannels())
	   JoinChan(thisChan);
	 if (doAutoFixNotice())
	   Message(thisChan, "Automatic channel fix in progress, please stand by.");
       }
     }
   }
}

//elog << "chanfix::autoFix> DEBUG: Found " << numOpLess << " of "
//	<< Network->channelList_size() << " channels in "
//	<< autoFixTimer.stopTimeMS() << " ms." << std::endl;
}

void chanfix::manualFix(Channel* thisChan)
{
/* If the channel doesn't exist (anymore), don't try to fix it. */
if (!thisChan) return;

elog << "chanfix::manualFix> DEBUG: Manual fix " << thisChan->getName() << "!" << std::endl;

if (useBurstToFix && thisChan->getCreationTime() > 1) {
  if (version >= 12) /* temporary fix until GNUWorld is fixed */
    MyUplink->setBursting(true);

  xServer::modeVectorType modeVector;
  if (thisChan->getMode(Channel::MODE_I))
    modeVector.push_back(std::make_pair(false, Channel::MODE_I));
  if (thisChan->getMode(Channel::MODE_K))
    MyUplink->OnChannelModeK(thisChan, false, 0, std::string());
  if (thisChan->getMode(Channel::MODE_L))
    MyUplink->OnChannelModeL(thisChan, false, 0, 0);
  if (thisChan->getMode(Channel::MODE_R))
    modeVector.push_back(std::make_pair(false, Channel::MODE_R));
  if (thisChan->getMode(Channel::MODE_D))
    modeVector.push_back(std::make_pair(false, Channel::MODE_D));
  /* Due to a bug in .11, we need to set at least one mode. */
  if (version < 12) {
    if (!thisChan->getMode(Channel::MODE_N))
      modeVector.push_back(std::make_pair(true, Channel::MODE_N));
    if (!thisChan->getMode(Channel::MODE_T))
      modeVector.push_back(std::make_pair(true, Channel::MODE_T));
  }
  if (!modeVector.empty())
    MyUplink->OnChannelMode(thisChan, 0, modeVector);

  MyUplink->BurstChannel(thisChan->getName(), thisChan->getModeString(),
			 thisChan->getCreationTime() - 1);

  if (version >= 12)
    MyUplink->setBursting(false);
} else {
  ClearMode(thisChan, "obiklrD", true);
}

if (doJoinChannels())
  JoinChan(thisChan);
if (doManualFixNotice())
  Message(thisChan, "Channel fix in progress, please stand by.");

manFixQ.insert(fixQueueType::value_type(thisChan->getName(), currentTime() + CHANFIX_DELAY));
}

bool chanfix::fixChan(sqlChannel* sqlChan, bool autofix)
{
/* First update the time of the previous attempt to now. */
if (sqlChan->getFixStart() == 0) sqlChan->setFixStart(currentTime());
sqlChan->setLastAttempt(currentTime());

/* If the channel doesn't exist (anymore), the fix is successful. */
Channel* netChan = Network->findChannel(sqlChan->getChannel());
if (!netChan) return true;

chanOpsType myOps = getMyOps(netChan);

if (myOps.begin() != myOps.end())
  sqlChan->setMaxScore((*myOps.begin())->getPoints());

int maxScore = sqlChan->getMaxScore();

/* If the max score of the channel is lower than the absolute minimum
 * score required, don't even bother trying. */
if (maxScore <= FIX_MIN_ABS_SCORE_END * MAX_SCORE)
  return false;

/* Get the number of clients that should have ops */
unsigned int maxOpped = (autofix ? AUTOFIX_NUM_OPPED : CHANFIX_NUM_OPPED);

/* If the channel has enough ops, abort & return. */
unsigned int currentOps = countChanOps(netChan);
if (currentOps >= maxOpped) {
  elog << "chanfix::fixChan> DEBUG: Enough clients opped on " << netChan->getName() << std::endl;
  return true;
} 

int time_since_start;
if (autofix)
  time_since_start = currentTime() - sqlChan->getFixStart();
else
  time_since_start = currentTime() - (sqlChan->getFixStart() + CHANFIX_DELAY);

int max_time = (autofix ? AUTOFIX_MAXIMUM : CHANFIX_MAXIMUM);

/* Determine minimum score required for this time. */

/* Linear interpolation of (0, fraction_abs_max * max_score) ->
 * (max_time, fraction_abs_min * max_score)
 * at time t between 0 and max_time. */
int min_score_abs = static_cast<int>((MAX_SCORE *
		static_cast<float>(FIX_MIN_ABS_SCORE_BEGIN)) -
		static_cast<float>(time_since_start) /
		static_cast<float>(max_time) *
		(MAX_SCORE * static_cast<float>(FIX_MIN_ABS_SCORE_BEGIN)
		 - static_cast<float>(FIX_MIN_ABS_SCORE_END) * MAX_SCORE));

elog << "chanfix::fixChan> [" << netChan->getName() << "] max "
	<< MAX_SCORE << ", begin " << FIX_MIN_ABS_SCORE_BEGIN
	<< ", end " << FIX_MIN_ABS_SCORE_END << ", time "
	<< time_since_start << ", maxtime " << max_time << "."
	<< std::endl;

/* Linear interpolation of (0, fraction_rel_max * max_score_channel) ->
 * (max_time, fraction_rel_min * max_score_channel)
 * at time t between 0 and max_time. */
int min_score_rel = static_cast<int>((maxScore *
		static_cast<float>(FIX_MIN_REL_SCORE_BEGIN)) -
		static_cast<float>(time_since_start) /
		static_cast<float>(max_time) *
		(maxScore * static_cast<float>(FIX_MIN_REL_SCORE_BEGIN)
		 - static_cast<float>(FIX_MIN_REL_SCORE_END) * maxScore));

/* The minimum score needed for ops is the HIGHER of these two
 * scores. */
int min_score = min_score_abs;
if (min_score_rel > min_score)
  min_score = min_score_rel;

/* We need to check to see if the highest scoring ops score for the chan
 * is less than min_score, if so, adjust min_score so we wont have
 * to wait forever for the first op! */

if (myOps.begin() != myOps.end())
  sqlChan->setMaxScore((*myOps.begin())->getPoints());

if (sqlChan->getMaxScore() < min_score)
  min_score = sqlChan->getMaxScore();

elog << "chanfix::fixChan> [" << netChan->getName() << "] start "
	<< sqlChan->getFixStart() << ", delta " << time_since_start
	<< ", max " << maxScore << ", minabs " << min_score_abs
	<< ", minrel " << min_score_rel << "." << std::endl;

/**
 * Get the scores of the accounts of the non-opped clients.
 * Find out which clients need to be opped.
 */
iClient* curClient = 0;
sqlChanOp* curOp = 0;
acctListType acctToOp;
std::string modes = "+";
std::string args;
unsigned int numClientsToOp = 0;
bool cntMaxedOut = false;
for (chanOpsType::iterator opPtr = myOps.begin(); opPtr != myOps.end();
     opPtr++) {
  curOp = *opPtr;
// elog	<< "chanfix::fixChan> DEBUG: "
//	<< curOp->getPoints() << " >= " << min_score << std::endl;
  if (curOp->getPoints() >= min_score) {
    acctToOp = findAccount(netChan, curOp->getAccount());
    std::vector< iClient* >::const_iterator acctPtr = acctToOp.begin(),
	end = acctToOp.end();
    while (acctPtr != end) {
      curClient = *acctPtr;
      if (curClient && !netChan->findUser(curClient)->isModeO()) {
	elog	<< "chanfix::fixChan> DEBUG: Decided to op: "
		<< curClient->getNickName() << " on "
		<< netChan->getName() << ". Client has "
		<< curOp->getPoints() << " points. ABS_MIN = "
		<< min_score_abs << " and REL_MIN = " << min_score_rel
		<< std::endl;
	modes += "o";
	if (!args.empty())
	  args += " ";
	args += curClient->getNickName();
	if ((++numClientsToOp + currentOps) >= maxOpped) {
	  elog	<< "chanfix::fixChan> DEBUG: Enough clients are to be "
		<< "opped (" << numClientsToOp << "); breaking the loop."
		<< std::endl;
	  cntMaxedOut = true;
	  break;
	}
      }
      ++acctPtr;
    }
    acctToOp.clear();
    if (cntMaxedOut)
      break;
  }
}

/* If no scores are high enough, return. */
if ((!numClientsToOp || maxScore < min_score) &&
    (!autofix || !(numClientsToOp + currentOps))) {
  if (autofix && !sqlChan->getModesRemoved() &&
      needsModesRemoved(netChan)) {
    ClearMode(netChan, "biklrD", true);
    sqlChan->setModesRemoved(true);
    Message(netChan, "Channel modes have been removed.");
  }
return false;
}

/* If we need to op at least one client, op him/her. */
if (numClientsToOp) {
  Mode(netChan, modes, args, true);

  if (numClientsToOp == 1)
    Message(netChan, "1 client should have been opped.");
  else
    Message(netChan, "%d clients should have been opped.",
	    numClientsToOp);
}

/* Now see if there are enough ops; if so, the fix is complete. */
if (numClientsToOp + currentOps >= netChan->size() ||
    numClientsToOp + currentOps >= maxOpped)
  return true;

return false;
}

void chanfix::stopFixingChan(Channel* theChan, bool force)
{
/* If the channel doesn't exist (anymore), don't try to end the fix. */
if (!theChan) return;

bool inFix = false;
int type = 0;

if ((stopAutoFixOnOp || force) && isBeingAutoFixed(theChan)) {
  type = 1;
  inFix = true;
  removeFromAutoQ(theChan);
}
if ((stopChanFixOnOp || force) && isBeingChanFixed(theChan)) {
  type = 2;
  inFix = true;
  removeFromManQ(theChan);
}

if (inFix) {
  sqlChannel* sqlChan = getChannelRecord(theChan);
  if (sqlChan) {
    sqlChan->setFixStart(0);
    sqlChan->setLastAttempt(0);
  }
}

if ((type == 1) && (doAutoFixNotice()))
  Message(theChan, "Channel has been automatically fixed.");
else if ((type == 2) && (doManualFixNotice()))
  Message(theChan, "Channel has been fixed.");

if (doJoinChannels())
  PartChan(theChan);

return;
}

chanfix::acctListType chanfix::findAccount(Channel* theChan, const std::string& account)
{
acctListType chanAccts;
for (Channel::userIterator ptr = theChan->userList_begin();
     ptr != theChan->userList_end(); ptr++) {
  if (account == ptr->second->getClient()->getAccount())
    chanAccts.push_back(ptr->second->getClient());
}

return chanAccts;
}

bool chanfix::accountIsOnChan(const std::string& channel, const std::string& account)
{
Channel* tmpChan = Network->findChannel(channel);
if (!tmpChan) return false;

for (Channel::userIterator ptr = tmpChan->userList_begin();
     ptr != tmpChan->userList_end(); ptr++) {
  if (account == ptr->second->getClient()->getAccount())
    return true;
}
return false;
}

const std::string chanfix::getChanNickName(const std::string& channel, const std::string& account)
{
std::string theNick = "";
Channel* tmpChan = Network->findChannel(channel);
if (!tmpChan) return "";

for (Channel::userIterator ptr = tmpChan->userList_begin();
     ptr != tmpChan->userList_end(); ptr++) {
  if (account == ptr->second->getClient()->getAccount()) {
    if (ptr->second->isModeO()) {
      theNick = "@";
      theNick += ptr->second->getClient()->getNickName();
      return theNick;
    }
    else if (ptr->second->isModeV()) {
      theNick = "+";
      theNick += ptr->second->getClient()->getNickName();
      return theNick;
    }
    else {
      theNick = ptr->second->getClient()->getNickName();
      return theNick;
    }
  }
}
return theNick;
}

sqlChannel* chanfix::getChannelRecord(const std::string& Channel)
{
sqlChannelCacheType::iterator ptr = sqlChanCache.find(Channel);
if (ptr != sqlChanCache.end()) {
  //elog << "chanfix::getChannelRecord> DEBUG: cached channel " << Channel << " found" << std::endl;
  return ptr->second;
}
return 0;
}

sqlChannel* chanfix::getChannelRecord(Channel* theChan)
{
return getChannelRecord(theChan->getName());
}

sqlChannel* chanfix::newChannelRecord(const std::string& Channel)
{
sqlChannel* newChan = new (std::nothrow) sqlChannel(theManager);
assert( newChan != 0 ) ;

newChan->setChannel(Channel);
newChan->setFixStart(0);
newChan->setLastAttempt(0);

sqlChanCache.insert(sqlChannelCacheType::value_type(Channel, newChan));
elog << "chanfix::newChannelRecord> DEBUG: Added new channel: " << Channel << std::endl;

return newChan;
}

sqlChannel* chanfix::newChannelRecord(Channel* theChan)
{
return newChannelRecord(theChan->getName());
}

bool chanfix::deleteChannelRecord(sqlChannel* sqlChan)
{
if (sqlChan->useSQL() && sqlChan->Delete())
  return false;
sqlChanCache.erase(sqlChan->getChannel());
delete sqlChan; sqlChan = 0;

return !sqlChan;
}

size_t chanfix::countChanOps(const Channel* theChan)
{
if (!theChan) {
  /* Don't try this on a null channel. */
  return 0;
}

size_t chanOps = 0;

for (Channel::const_userIterator ptr = theChan->userList_begin();
     ptr != theChan->userList_end(); ++ptr)
   if (ptr->second->isModeO())
     chanOps++;

return chanOps;
}

bool chanfix::needsModesRemoved(Channel* theChan)
{
/* Modes need to be removed if +b/i/k/l/r/D is set.
 * This check should actually be more specific (checking each ban
 * to see if it matches a high scored hostmask) but this will do
 * for now.
 */
if (theChan->banList_size() ||
    theChan->getMode(Channel::MODE_I) ||
    theChan->getMode(Channel::MODE_K) ||
    theChan->getMode(Channel::MODE_L) ||
    theChan->getMode(Channel::MODE_R) ||
    theChan->getMode(Channel::MODE_D))
  return true;

return false;
}

bool chanfix::canScoreChan(Channel* theChan)
{
for (Channel::const_userIterator ptr = theChan->userList_begin();
     ptr != theChan->userList_end(); ++ptr)
   if (ptr->second->getClient()->getMode(iClient::MODE_SERVICES))
     return false;

return true;
}

void chanfix::processQueue()
{
/* If there are too many servers split, don't process queue. */
if (currentState != RUN) {
  //elog << "chanfix::processQueue> DEBUG: currentState != RUN" << std::endl;
  return;
}

for (fixQueueType::iterator ptr = autoFixQ.begin(); ptr != autoFixQ.end(); ) {
   //elog << "chanfix::processQueue> DEBUG: Processing " << ptr->first << " in autoFixQ ..." << std::endl;
   if (ptr->second <= currentTime()) {
     sqlChannel* sqlChan = getChannelRecord(ptr->first);
     if (!sqlChan) sqlChan = newChannelRecord(ptr->first);
     bool isFixed = false;

     if (currentTime() - sqlChan->getLastAttempt() >= AUTOFIX_INTERVAL)
       isFixed = fixChan(sqlChan, true);

     /**
      * If the channel has been fixed, or the fixing time window
      * has passed, remove it from the list
      */
     if (isFixed || currentTime() - sqlChan->getFixStart() > AUTOFIX_MAXIMUM) {
       Channel* theChan = Network->findChannel(sqlChan->getChannel());
       if (doAutoFixNotice())
         Message(theChan, "Channel has been automatically fixed.");
       if (doJoinChannels())
         PartChan(theChan);
       autoFixQ.erase(ptr++);
       sqlChan->setFixStart(0);
       sqlChan->setLastAttempt(0);
       elog << "chanfix::processQueue> DEBUG: Channel " << sqlChan->getChannel() << " done!" << std::endl;
     } else {
       ptr->second = currentTime() + AUTOFIX_INTERVAL;
       ptr++;
       elog << "chanfix::processQueue> DEBUG: Channel " << sqlChan->getChannel() << " not done yet ..." << std::endl;
     }
   } else ptr++;
}

for (fixQueueType::iterator ptr = manFixQ.begin(); ptr != manFixQ.end(); ) {
   //elog << "chanfix::processQueue> DEBUG: Processing " << ptr->first << " in manFixQ ..." << std::endl;
   if (ptr->second <= currentTime()) {
     sqlChannel* sqlChan = getChannelRecord(ptr->first);
     if (!sqlChan) sqlChan = newChannelRecord(ptr->first);
     bool isFixed = false;

     if (currentTime() - sqlChan->getLastAttempt() >= CHANFIX_INTERVAL)
       isFixed = fixChan(sqlChan, false);

     /**
      * If the channel has been fixed, or the fixing time window
      * has passed, remove it from the list
      */
     if (isFixed || currentTime() - sqlChan->getFixStart() > CHANFIX_MAXIMUM + CHANFIX_DELAY) {
       Channel* theChan = Network->findChannel(sqlChan->getChannel());
       if (doManualFixNotice())
         Message(theChan, "Channel has been fixed.");
       if (doJoinChannels())
         PartChan(theChan);
       manFixQ.erase(ptr++);
       sqlChan->setFixStart(0);
       sqlChan->setLastAttempt(0);
       elog << "chanfix::processQueue> DEBUG: Channel " << sqlChan->getChannel() << " done!" << std::endl;
     } else {
       ptr->second = currentTime() + CHANFIX_INTERVAL;
       ptr++;
       elog << "chanfix::processQueue> DEBUG: Channel " << sqlChan->getChannel() << " not done yet ..." << std::endl;
     }
   } else ptr++;
}

return;
}

bool chanfix::isBeingFixed(Channel* theChan)
{
return (isBeingAutoFixed(theChan) || isBeingChanFixed(theChan));
}

bool chanfix::isBeingAutoFixed(Channel* theChan)
{
fixQueueType::iterator ptr = autoFixQ.find(theChan->getName());
if (ptr != autoFixQ.end())
  return true;

return false;
}

bool chanfix::isBeingChanFixed(Channel* theChan)
{
fixQueueType::iterator ptr = manFixQ.find(theChan->getName());
if (ptr != manFixQ.end())
  return true;

return false;
}

bool chanfix::removeFromAutoQ(Channel* theChan)
{
fixQueueType::iterator ptr = autoFixQ.find(theChan->getName());
if (ptr != autoFixQ.end()) {
  autoFixQ.erase(ptr);
  return true;
}

return false;
}

bool chanfix::removeFromManQ(Channel* theChan)
{
fixQueueType::iterator ptr = manFixQ.find(theChan->getName());
if (ptr != manFixQ.end()) {
  manFixQ.erase(ptr);
  return true;
}

return false;
}

const std::string chanfix::prettyDuration( int duration )
{

// Pretty format a 'duration' in seconds to
// x day(s), xx:xx:xx.

char tmpBuf[ 64 ] = {0};

int	res = currentTime() - duration,
	secs = res % 60,
	mins = (res / 60) % 60,
	hours = (res / 3600) % 24,
	days = (res / 86400) ;

sprintf(tmpBuf, "%i day%s, %02d:%02d:%02d",
	days,
	(days == 1 ? "" : "s"),
	hours,
	mins,
	secs );

return std::string( tmpBuf ) ;
}

const std::string chanfix::tsToDateTime(time_t timestamp, bool time)
{
char datetimestring[ 20 ] = {0};
struct tm *stm;

stm = localtime(&timestamp);
memset(datetimestring, 0, sizeof(datetimestring));

if (time)
  strftime(datetimestring, sizeof(datetimestring), "%Y-%m-%d %H:%M:%S", stm);
else
  strftime(datetimestring, sizeof(datetimestring), "%Y-%m-%d", stm);

return std::string(datetimestring);
}

const int chanfix::getCurrentGMTHour()
{
time_t rawtime;
tm * ptm;

time ( &rawtime );
ptm = gmtime ( &rawtime );

return ptm->tm_hour;
}

sqlUser* chanfix::isAuthed(const std::string Name)
{
//Name = escapeSQLChars(Name);
sqlUser* tempUser = usersMap[Name];
if (!tempUser)
  usersMap.erase(usersMap.find(Name));
return tempUser;
}

chanfix::clientOpsType* chanfix::findMyOps(iClient* theClient)
{
clientOpsType* myOps = static_cast< clientOpsType* >(theClient->getCustomData(this) );

if (!myOps) myOps = new clientOpsType;

return myOps;
}

/*
 * Start timers
 */
void chanfix::startTimers()
{
time_t theTime;
theTime = time(NULL) + connectCheckFreq;
tidCheckDB = MyUplink->RegisterTimer(theTime, this, NULL);
theTime = time(NULL) + CHECK_CHANS_TIME;
tidAutoFix = MyUplink->RegisterTimer(theTime, this, NULL);
theTime = time(NULL) + PROCESS_QUEUE_TIME;
tidFixQ = MyUplink->RegisterTimer(theTime, this, NULL);
theTime = time(NULL) + POINTS_UPDATE_TIME;
tidGivePoints = MyUplink->RegisterTimer(theTime, this, NULL);
theTime = time(NULL) + getSecsTilMidnight();
tidRotateDB = MyUplink->RegisterTimer(theTime, this, NULL);
theTime = time(NULL) + SQL_UPDATE_TIME;
tidUpdateDB = MyUplink->RegisterTimer(theTime, this, NULL);
theTime = time(NULL) + TEMPBLOCKS_CHECK_TIME;
tidTempBlocks = MyUplink->RegisterTimer(theTime, this, NULL);
elog	<< "chanfix::startTimers> Started all timers."
	<< std::endl;
}

/**
 * prepareUpdate - Copies the sqlChanOp map to a temporary multimap
 */
#ifdef CHANFIX_HAVE_BOOST_THREAD
void chanfix::prepareUpdate(bool threaded)
#else
void chanfix::prepareUpdate(bool)
#endif /* CHANFIX_HAVE_BOOST_THREAD */ 
{
  if (updateInProgress) {
    elog	<< "*** [chanfix::prepareUpdate] Update already in progress; not starting."
		<< std::endl;
    return;	  
  }

  elog	<< "*** [chanfix::prepareUpdate] Updating the SQL database "
#ifdef CHANFIX_HAVE_BOOST_THREAD
	<< (threaded ? "(threaded)." : "(unthreaded).")
#else
	<< "(unthreaded [no boost])."
#endif /* CHANFIX_HAVE_BOOST_THREAD */
	<< std::endl;
  logDebugMessage("Starting to update the SQL database.");

  /**
   * Set updateInProgress boolean to true so that any other updates 
   * and/or requests for shutdown/reload will be denied.
   */
  updateInProgress = true;

  /* Start our timer */
  Timer snapShotTimer;
  snapShotTimer.Start();
  
  /* Clear the snapShot map */
  snapShot.clear();

  //snapShotStruct* curStruct = new (std::nothrow) snapShotStruct;
  snapShotStruct curStruct;

  sqlChanOp* curOp;
  std::string curChan;
  int i = 0;

  for (sqlChanOpsType::iterator ptr = sqlChanOps.begin();
       ptr != sqlChanOps.end(); ptr++) {
    curChan = ptr->first;
    for (sqlChanOpsType::mapped_type::iterator chanOp = ptr->second.begin();
	 chanOp != ptr->second.end(); chanOp++) {
      curOp = chanOp->second;
		 
      /* Fill the structures and maps */
      curStruct.account = curOp->getAccount();
      curStruct.lastSeenAs = curOp->getLastSeenAs();
      curStruct.firstOpped = curOp->getTimeFirstOpped();
      curStruct.lastOpped = curOp->getTimeLastOpped();
      
      for (i = 0; i < DAYSAMPLES; i++) {
        curStruct.day[i] = curOp->getDay(i);
      }
      
      snapShot.insert(DBMapType::value_type(curChan, curStruct));
    }
  }

  logDebugMessage("Created snapshot map in %u ms.",
		  snapShotTimer.stopTimeMS());

#ifdef CHANFIX_HAVE_BOOST_THREAD
  if (threaded) {
    ClassUpdateDB updateDB(*this);
    boost::thread pthrd(updateDB);
  } else
#endif /* CHANFIX_HAVE_BOOST_THREAD */
    updateDB();
    printResourceStats();
  return;
}

/**
 * updateDB - Copies the contents of sqlChanOps to the SQL database
 * Note: Only threaded if called via the ClassUpdateDB function with
 * boost::thread
 */
void chanfix::updateDB()
{  
  /* Start our timer */
  Timer updateDBTimer;
  updateDBTimer.Start();

  /* Get a connection instance to our backend */
  PgDatabase* cacheCon = theManager->getConnection();

  /* Check for the backup table. If it exists, something went wrong. */
  /* SELECT count(*) FROM pg_tables WHERE tablename = 'chanOpsBackup' */
  /* SELECT chanOpsBackup FROM information_schema.tables */
  if (!cacheCon->ExecTuplesOk("SELECT count(*) FROM pg_tables WHERE tablename = 'chanOpsBackup'")) {
    elog	<< "*** [chanfix::updateDB]: Error checking for backup table presence: " 
		<< cacheCon->ErrorMessage()
		<< std::endl;
    return;
  }

  if (cacheCon->Tuples() && atoi(cacheCon->GetValue(0, 0))) {
    /* Drop the backup table. */
    if (!cacheCon->ExecCommandOk("DROP TABLE chanOpsBackup")) {
      elog	<< "*** [chanfix::updateDB]: Error dropping backup table: " 
		<< cacheCon->ErrorMessage()
		<< std::endl;
      return;
    }
  }

  /* Copy all data from the main table to the backup table. */
  if (!cacheCon->ExecCommandOk("CREATE TABLE chanOpsBackup AS SELECT * FROM chanOps")) {
    elog	<< "*** [chanfix::updateDB]: Error creating backup table: " 
		<< cacheCon->ErrorMessage()
		<< std::endl;
    return;
  }

  /* Truncate the current chanOps table. */
  if (!cacheCon->ExecCommandOk("TRUNCATE TABLE chanOps")) {
    elog	<< "*** [chanfix::updateDB]: Error truncating current chanOps table: " 
		<< cacheCon->ErrorMessage()
		<< std::endl;
    return;
  }

  /* Copy the current chanOps to SQL. */
  ExecStatusType status = cacheCon->Exec("COPY chanOps FROM stdin");
  if (PGRES_COPY_IN != status) {
    elog	<< "*** [chanfix::updateDB]: Error starting copy of chanOps table."
		<< std::endl;
    return;
  }

  std::stringstream theLine;
  int chanOpsProcessed = 0;
  int i = 0;
  
  for (DBMapType::iterator ptr = snapShot.begin();
       ptr != snapShot.end(); ptr++) {
    theLine.str("");
    theLine	<< escapeSQLChars(ptr->first) << "\t"
		<< escapeSQLChars(ptr->second.account) << "\t"
		<< escapeSQLChars(ptr->second.lastSeenAs) << "\t"
		<< ptr->second.firstOpped << "\t"
		<< ptr->second.lastOpped
		;

    for (i = 0; i < DAYSAMPLES; i++) {
      theLine	<< "\t" << ptr->second.day[i]
		;
    }

    theLine	<< "\n"
		;

    cacheCon->PutLine(theLine.str().c_str());
    chanOpsProcessed++;
  }
  
  /* Send completion string for the end of the data. */
  cacheCon->PutLine("\\.\n");

  /**
   * Synchronize with the backend.
   * Returns 0 on success, 1 on failure
   */
  int copyStatus = cacheCon->EndCopy();
  if (copyStatus != 0) {
    elog	<< "*** [chanfix::updateDB] Error ending copy! Returned "
		<< copyStatus << " instead (should be 0 for success)."
		<< std::endl;
    return;
  }

  /* Count the rows to see if the cache == number of rows in table. */
  if (!cacheCon->ExecTuplesOk("SELECT count(*) FROM chanOps")) {
    elog	<< "*** [chanfix::updateDB]: Error counting rows in chanOps table: " 
		<< cacheCon->ErrorMessage()
		<< std::endl;
    return;
  }

  int actualChanOpsProcessed = atoi(cacheCon->GetValue(0, 0));
  if (actualChanOpsProcessed != chanOpsProcessed) {
    elog	<< "*** [chanfix::updateDB] Error updating chanOps! "
		<< "Only " << actualChanOpsProcessed << " of "
		<< chanOpsProcessed << " chanops were copied to the SQL database."
		<< std::endl;
    logDebugMessage("ERROR: Only %d of %d chanops were updated in %u ms.",
		    actualChanOpsProcessed, chanOpsProcessed,
		    updateDBTimer.stopTimeMS());
  } else {
    elog	<< "*** [chanfix::updateDB]: Done. Copied "
		<< actualChanOpsProcessed
		<< " chanops to the SQL database."
		<< std::endl;
    logDebugMessage("Synched %d members to the SQL database in %u ms.",
		    actualChanOpsProcessed, updateDBTimer.stopTimeMS());
  }

  /* Drop the backup table. */
  if (!cacheCon->ExecCommandOk("DROP TABLE chanOpsBackup")) {
    elog	<< "*** [chanfix::updateDB]: Error dropping backup table (after completion): " 
		<< cacheCon->ErrorMessage()
		<< std::endl;
    return;
  }

  /* Dispose of our connection instance */
  theManager->removeConnection(cacheCon);

  /* Clean-up after ourselves and allow new updates to be started */
  snapShot.clear();
  updateInProgress = false;

  return;
}

void chanfix::printResourceStats()
{
  int who = RUSAGE_SELF;
  struct rusage usage;
  int ret;
  ret = getrusage(who, &usage);
  logDebugMessage("Max. resident size used by chanfix (kB): %ld", usage.ru_maxrss);
}

bool chanfix::isTempBlocked(const std::string& theChan)
{
  tempBlockType::iterator ptr = tempBlockList.find(theChan);
  if (ptr == tempBlockList.end())
    return false;
  
  return true;
}

void chanfix::expireTempBlocks()
{
  time_t unixTime = currentTime();
  std::list<std::string> remList;
  std::list<std::string>::iterator remIterator;

  for (tempBlockType::iterator ptr = tempBlockList.begin();
       ptr != tempBlockList.end(); ptr++) {
    if ((unixTime - ptr->second) >= TEMPBLOCK_DURATION_TIME)
      remList.push_back(ptr->first);
  }
  
  for(remIterator = remList.begin();remIterator != remList.end();)
  {
    tempBlockList.erase(*remIterator);
    remIterator = remList.erase(remIterator);
  }
}

void chanfix::rotateDB()
{
/* 
 * CODER NOTES:
 * go through everybody and remove the oldest day (set it to 0)
 * then loop back through and pick out the users with 0 points total and check their ts_firstopped
 * if it is older than 1 day, delete the user
 * cache: acct,chan
 */

logDebugMessage("Beginning database rotation.");

/* Start our timer */
Timer rotateDBTimer;
rotateDBTimer.Start();

short nextDay = currentDay;
setCurrentDay();
if (nextDay >= (DAYSAMPLES - 1))
  nextDay = 0;
else
  nextDay++;

/**
 * Basically, we need to make sure we do not delete ops that were
 * just added. To do this, we take the currentTime(), subtract
 * POINTS_UPDATE_TIME (plus 10 seconds to make sure), and then
 * check it against the time that he/she was first opped.
 */
time_t maxFirstOppedTS = currentTime() - (POINTS_UPDATE_TIME + 10);
time_t maxLastOppedTS = currentTime() - (DAYSAMPLES * 86400);
sqlChanOp* curOp;
std::string curChan;

for (sqlChanOpsType::iterator ptr = sqlChanOps.begin();
     ptr != sqlChanOps.end(); ptr++) {
  for (sqlChanOpsType::mapped_type::iterator chanOp = ptr->second.begin();
       chanOp != ptr->second.end(); chanOp++) {
    curOp = chanOp->second;
    curOp->setDay(nextDay, 0);
    curOp->calcTotalPoints();
    if (((curOp->getPoints() <= 0) &&
	 (maxFirstOppedTS > curOp->getTimeFirstOpped()))
	|| (maxLastOppedTS > curOp->getTimeLastOpped())) {
      ptr->second.erase(chanOp);
      delete curOp; curOp = 0;
    }
  }
  if (!ptr->second.size()) {
    /* Empty channel, start deleting info */
    curChan = ptr->first;
    sqlChannel* sqlChan = getChannelRecord(curChan);
    if (!sqlChan)
      continue;

#ifndef REMEMBER_CHANNELS_WITH_NOTES_OR_FLAGS
    if (!sqlChan->deleteAllNotes()) {
      elog	<< "chanfix::rotateDB> Error: could not delete all the notes of channel "
		<< curChan.c_str()
		<< std::endl;
    }

#else
    if (!sqlChan->getFlags()) {
#endif
      if (!deleteChannelRecord(sqlChan)) {
	elog	<< "chanfix::rotateDB> Error: could not delete channel "
		<< curChan.c_str()
		<< std::endl;
      }
#ifdef REMEMBER_CHANNELS_WITH_NOTES_OR_FLAGS
    }
#endif
  }
}

logDebugMessage("Completed database rotation in %u ms.",
		rotateDBTimer.stopTimeMS());

return;
}

void chanfix::giveAllOpsPoints()
{
typedef std::list <std::string> ScoredOpsListType;

Channel* thisChan;
ScoredOpsListType scoredOpsList;
ScoredOpsListType::iterator scOpIter;
for (xNetwork::channelIterator ptr = Network->channels_begin();
     ptr != Network->channels_end(); ptr++) {
  thisChan = ptr->second;
  if (!canScoreChan(thisChan))
    continue; // Exit the loop and go to the next chan
  if (thisChan->size() >= minClients && !isBeingFixed(thisChan)) {
    scoredOpsList.clear();
    for (Channel::userIterator ptr = thisChan->userList_begin();
	 ptr != thisChan->userList_end(); ptr++) {
      ChannelUser* curUser = ptr->second;
      if (curUser->isModeO() && curUser->getClient()->getAccount() != "") {
	//Ok hes an op
	//Grab an iClient for curUser
	scOpIter = std::find(scoredOpsList.begin(), scoredOpsList.end(), curUser->getClient()->getAccount());
	if (scOpIter == scoredOpsList.end()) {
	  givePoints(thisChan, curUser->getClient());
	  scoredOpsList.push_back(ScoredOpsListType::value_type(curUser->getClient()->getAccount()));
	}
      }
    }
  }
}

return;
} //giveAllOpsPoints

char chanfix::getFlagChar(const sqlUser::flagType& whichFlag)
{
 if (whichFlag == sqlUser::F_SERVERADMIN)
   return 'a';
 else if (whichFlag == sqlUser::F_BLOCK)
   return 'b';
 else if (whichFlag == sqlUser::F_COMMENT)
   return 'c';
 else if (whichFlag == sqlUser::F_CHANFIX)
   return 'f';
 else if (whichFlag == sqlUser::F_OWNER)
   return 'o';
 else if (whichFlag == sqlUser::F_USERMANAGER)
   return 'u';
 else
   return ' ';
}

const std::string chanfix::getFlagsString(const sqlUser::flagType& whichFlags)
{
 std::string flagstr;
 if (whichFlags & sqlUser::F_SERVERADMIN)
   flagstr += "a";
 if (whichFlags & sqlUser::F_BLOCK)
   flagstr += "b";
 if (whichFlags & sqlUser::F_COMMENT)
   flagstr += "c";
 if (whichFlags & sqlUser::F_CHANFIX)
   flagstr += "f";
 if (whichFlags & sqlUser::F_OWNER)
   flagstr += "o";
 if (whichFlags & sqlUser::F_USERMANAGER)
   flagstr += "u";
return flagstr;
}

sqlUser::flagType chanfix::getFlagType(const char whichChar)
{
switch (whichChar) {
  case 'a': return sqlUser::F_SERVERADMIN;
  case 'b': return sqlUser::F_BLOCK;
  case 'c': return sqlUser::F_COMMENT;
  case 'f': return sqlUser::F_CHANFIX;
  case 'o': return sqlUser::F_OWNER;
  case 'u': return sqlUser::F_USERMANAGER;
}
return 0;
}

const std::string chanfix::getEventName(const int whichEvent)
{
if (whichEvent == sqlChannel::EV_MISC)
  return "MISC";
else if (whichEvent == sqlChannel::EV_NOTE)
  return "NOTE";
else if (whichEvent == sqlChannel::EV_CHANFIX)
  return "CHANFIX";
else if (whichEvent == sqlChannel::EV_REQUESTOP)
  return "REQUESTOP";
else if (whichEvent == sqlChannel::EV_BLOCK)
  return "BLOCK";
else if (whichEvent == sqlChannel::EV_UNBLOCK)
  return "UNBLOCK";
else if (whichEvent == sqlChannel::EV_ALERT)
  return "ALERT";
else if (whichEvent == sqlChannel::EV_UNALERT)
  return "UNALERT";
else
  return "";
}

const std::string chanfix::getHelpMessage(sqlUser* theUser, std::string topic)
{
int lang_id = 1;

if (theUser)
  lang_id = theUser->getLanguageId();

std::pair <int, std::string> thePair(lang_id, topic);
helpTableType::iterator ptr = helpTable.find(thePair);
if (ptr != helpTable.end())
  return ptr->second;

if (lang_id != 1)
  return getHelpMessage(theUser, topic);

return std::string("");
}

void chanfix::loadHelpTable()
{
/* Get a connection instance to our backend */
PgDatabase* cacheCon = theManager->getConnection();

/* Grab the help table */
std::stringstream theQuery;
theQuery	<< "SELECT language_id,topic,contents FROM help"
		;

if (cacheCon->ExecTuplesOk(theQuery.str().c_str()))
  for (int i = 0; i < cacheCon->Tuples(); i++)
    helpTable.insert(helpTableType::value_type(
		     std::make_pair(atoi(cacheCon->GetValue(i, 0)),
				    cacheCon->GetValue(i, 1)),
		     cacheCon->GetValue(i, 2)));

elog	<< "*** [chanfix::loadHelpTable]: Loaded "
	<< helpTable.size()
	<< " help messages."
	<< std::endl;

/* Dispose of our connection instance */
theManager->removeConnection(cacheCon);

return;
}

const std::string chanfix::getResponse( sqlUser* theUser,
	int response_id, std::string msg )
{

// Language defaults to English
int lang_id = 1;

if (theUser)
  lang_id = theUser->getLanguageId();

std::pair<int, int> thePair( lang_id, response_id );

translationTableType::iterator ptr = translationTable.find(thePair);
if (ptr != translationTable.end()) {
  /* Found something! */
  return ptr->second ;
}

/*
 * Can't find this response Id within a valid language.
 * Realistically we should bomb here, however it might be wise
 * to 'fallback' to a lower language ID and try again, only bombing if we
 * can't find an english variant. (Carrying on here could corrupt
 * numerous varg lists, and will most likely segfault anyway).
 */
if (lang_id != 1) {
  std::pair<int, int> thePair( 1, response_id );
  translationTableType::iterator ptr = translationTable.find(thePair);
  if (ptr != translationTable.end())
    return ptr->second ;
}

if (!msg.empty())
  return msg;

return std::string( "Unable to retrieve response. Please contact a chanfix "
        "administrator." ) ;
}

void chanfix::loadTranslationTable()
{
/* Get a connection instance to our backend */
PgDatabase* cacheCon = theManager->getConnection();

/* Grab the languages table */
std::stringstream langQuery;
langQuery	<< "SELECT id,code,name FROM languages"
		;

if (cacheCon->ExecTuplesOk(langQuery.str().c_str()))
  for (int i = 0; i < cacheCon->Tuples(); i++)
    languageTable.insert(languageTableType::value_type(cacheCon->GetValue(i, 1),
			 std::make_pair(atoi(cacheCon->GetValue(i, 0)),
					cacheCon->GetValue(i, 2))));

elog	<< "*** [chanfix::loadTranslationTable]: Loaded "
	<< languageTable.size()
	<< " language" << ((languageTable.size() != 1) ? "s." : ".")
	<< std::endl;

/* Grab the translations table */
std::stringstream transQuery;
transQuery	<< "SELECT language_id,response_id,text FROM translations"
		;

if (cacheCon->ExecTuplesOk(transQuery.str().c_str())) {
  for (int i = 0 ; i < cacheCon->Tuples(); i++) {
    /*
     *  Add to our translations table.
     */

    int lang_id = atoi(cacheCon->GetValue( i, 0 ));
    int resp_id = atoi(cacheCon->GetValue( i, 1 ));

    std::pair<int, int> thePair( lang_id, resp_id ) ;

    translationTable.insert(
			translationTableType::value_type(
			thePair, cacheCon->GetValue( i, 2 )) );
  }
}

elog	<< "*** [chanfix::loadTranslationTable]: Loaded "
	<< translationTable.size()
	<< " translations."
	<< std::endl;

/* Dispose of our connection instance */
theManager->removeConnection(cacheCon);

return;
}

void chanfix::updatePoints()
{
/*
sqlChanOp* curOp;

for(chanOpsType::iterator ptr = opList.begin(); ptr != opList.end(); ptr++) {
  curOp = *ptr;
  givePoints(curOp);
}
*/
}

/*void chanfix::checkDBConnection()
{
if (cacheCon->Status() == CONNECTION_BAD) { //Check if the connection has died
  delete(cacheCon);
  dbConnected = false;
  updateSQLDb(NULL);
  MsgChanLog("PANIC! - The Connection With The Db Was Lost\n");
  MsgChanLog("Attempting to reconnect, Attempt %d out of %d\n",
	     connectCount+1,connectRetry+1);
  std::string Query = "host=" + sqlHost + " dbname=" + sqlDB + " port=" + sqlPort;
  if (strcasecmp(sqlUser,"''"))
    Query += (" user=" + sqlUser);
  if (strcasecmp(sqlPass,"''"))
    Query += (" password=" + sqlPass);
  theManager = new (std::nothrow) cmDatabase(Query.c_str());
  assert(theManager != NULL);

  if (theManager->ConnectionBad()) {
    ++connectCount;
    if (connectCount > connectRetry) {
      MsgChanLog("Cant connect to the database, quiting\n");
      ::exit(1);
    } else {
      MsgChanLog("Attempt failed\n");
    }
  } else {
    dbConnected = true;
    MsgChanLog("The PANIC is over, db connection restored\n");
    updateSqldb(theManager);
    connectCount = 0;
  }
}

}

void chanfix::updateSQLDb(PgDatabase* _SQLDb)
{

for(glineIterator ptr = glineList.begin();ptr != glineList.end();++ptr)
	{
	(ptr->second)->setSqldb(_SQLDb);
	}

for(glineIterator ptr = rnGlineList.begin();ptr != rnGlineList.end();++ptr)
	{
	(ptr->second)->setSqldb(_SQLDb);
	}

for(exceptionIterator ptr = exception_begin();ptr != exception_end();++ptr)
	{
	(*ptr)->setSqldb(_SQLDb);
	}

for(usersIterator ptr = usersMap.begin();ptr != usersMap.end();++ptr)
	{
	ptr->second->setSqldb(_SQLDb);
	}

for(serversIterator ptr = serversMap.begin();ptr != serversMap.end();++ptr)
	{
	ptr->second->setSqldb(_SQLDb);
	}
}*/

void Command::Usage( iClient* theClient )
{
sqlUser* theUser = bot->isAuthed(theClient->getAccount());
bot->SendTo(theClient,
            bot->getResponse(theUser,
                             language::syntax,
                             std::string("SYNTAX: ")).c_str() + getInfo() );
}

} // namespace cf

} // namespace gnuworld

