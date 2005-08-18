/**
 * chanfix.cc
 * 
 * Copyright (C) 2003	Reed Loden <reed@reedloden.com>
 *			Matthias Crauwels <ultimate_@wol.be>
 *			Jimmy Lipham <music0m@alltel.net>
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

#include	"libpq++.h"

#include	"gnuworld_config.h"
#include	"client.h"
#include	"EConfig.h"
#include	"Network.h"
#include	"server.h"
#include	"StringTokenizer.h"

#include	"chanfix.h"
#include	"chanfix_misc.h"
#include	"chanfixCommands.h"
#include	"sqlChannel.h"
#include	"sqlChanOp.h"
#include	"sqlUser.h"

RCSTAG("$Id$");

namespace gnuworld
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

std::string Query = "host=" + sqlHost + " dbname=" + sqlDB + " port=" + sqlPort + " user=" + sqlUsername;

elog	<< "chanfix::chanfix> Attempting to connect to "
	<< sqlHost << " at port " << sqlPort
	<< " as User " << sqlUsername << " to database: "
	<< sqlDB
	<< std::endl;

SQLDb = new (std::nothrow) cmDatabase( Query.c_str() ) ;
assert( SQLDb != 0 ) ;
//-- Make sure we connected to the SQL database; if
// we didn't we exit entirely.
if (SQLDb->ConnectionBad ())
	{
	elog	<< "chanfix::chanfix> Unable to connect to SQL server."
		<< std::endl
		<< "chanfix::chanfix> PostgreSQL error message: "
		<< SQLDb->ErrorMessage()
		<< std::endl;

	::exit( 0 ) ;
	}
else
	{
	elog	<< "chanfix::chanfix> Connection established to SQL server"
		<< std::endl;
	}

/* Register the commands we want to use */
RegisterCommand(new ADDFLAGCommand(this, "ADDFLAG",
	"<username> <flag>",
	3,
	sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN
	));
RegisterCommand(new ADDNOTECommand(this, "ADDNOTE",
	"<#channel> <reason>",
	3,
	sqlUser::F_CHANNEL
	));
RegisterCommand(new ADDUSERCommand(this, "ADDUSER",
	"<username> [host]",
	2,
	sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN
	));
RegisterCommand(new ALERTCommand(this, "ALERT",
	"<#channel>",
	2,
	sqlUser::F_CHANNEL
	));
RegisterCommand(new BLOCKCommand(this, "BLOCK",
	"<#channel> <reason>",
	3,
	sqlUser::F_BLOCK
	));
RegisterCommand(new CHANFIXCommand(this, "CHANFIX",
	"<#channel> [override]",
	2,
	sqlUser::F_CHANFIX
	));
RegisterCommand(new CHECKCommand(this, "CHECK",
	"<#channel>",
	2,
	sqlUser::F_LOGGEDIN
	));
RegisterCommand(new DELFLAGCommand(this, "DELFLAG",
	"<username> <flag>",
	3,
	sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN
	));
RegisterCommand(new DELNOTECommand(this, "DELNOTE",
	"<#channel> <note_id>",
	3,
	sqlUser::F_CHANNEL
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
RegisterCommand(new OPLISTCommand(this, "OPLIST",
	"<#channel>",
	2,
	sqlUser::F_CHANFIX
	));
RegisterCommand(new OPNICKSCommand(this, "OPNICKS",
	"<#channel>",
	2,
	sqlUser::F_CHANFIX
	));
RegisterCommand(new QUOTECommand(this, "QUOTE",
	"<text>",
	2,
	sqlUser::F_OWNER
	));
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
RegisterCommand(new ROTATECommand(this, "ROTATE",
	"",
	1,
	sqlUser::F_OWNER
	));
RegisterCommand(new SCORECommand(this, "SCORE",
	"<#channel> [nick|*account]",
	2,
	0
	));
RegisterCommand(new SCORECommand(this, "CSCORE",
	"<#channel> [nick|*account]",
	2,
	0
	));
RegisterCommand(new SETCommand(this, "SET",
	"<option> <value>",
	3,
	sqlUser::F_OWNER
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
RegisterCommand(new UNALERTCommand(this, "UNALERT",
	"<#channel>",
	2,
	sqlUser::F_CHANNEL
	));
RegisterCommand(new UNBLOCKCommand(this, "UNBLOCK",
	"<#channel>",
	2,
	sqlUser::F_BLOCK
	));
RegisterCommand(new USETCommand(this, "USET",
	"<option> <value>",
	3,
	sqlUser::F_LOGGEDIN
	));
RegisterCommand(new WHOISCommand(this, "WHOIS",
	"<username>",
	2,
	sqlUser::F_LOGGEDIN
	));

/* Set our current day. */
setCurrentDay();

/* Preload the ChanOps cache */
preloadChanOpsCache();

/* Preload the Channels cache */
preloadChannelCache();

/* Preload the user access cache */
preloadUserCache();

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
operChan = chanfixConfig->Require("operChan")->second ;
operChanModes = chanfixConfig->Require("operChanModes")->second ;
supportChan = chanfixConfig->Require("supportChan")->second ;
supportChanModes = chanfixConfig->Require("supportChanModes")->second ;
enableAutoFix = atob(chanfixConfig->Require("enableAutoFix")->second) ;
enableChanFix = atob(chanfixConfig->Require("enableChanFix")->second) ;
enableChannelBlocking = atob(chanfixConfig->Require("enableChannelBlocking")->second) ;
version = atoi((chanfixConfig->Require("version")->second).c_str()) ;
numServers = atoi((chanfixConfig->Require("numServers")->second).c_str()) ;
minServersPresent = atoi((chanfixConfig->Require("minServersPresent")->second).c_str()) ;
numTopScores = atoi((chanfixConfig->Require("numTopScores")->second).c_str()) ;
minClients = atoi((chanfixConfig->Require("minClients")->second).c_str()) ;
clientNeedsIdent = atob(chanfixConfig->Require("clientNeedsIdent")->second) ;
clientNeedsReverse = atob(chanfixConfig->Require("clientNeedsReverse")->second) ;
connectCheckFreq = atoi((chanfixConfig->Require("connectCheckFreq")->second).c_str()) ;

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

/* OnAttach */
void chanfix::OnAttach()
{
/**
 * Don't send END_OF_BURST (EB) so we can stay in burst mode 
 * indefinitely in order to be able to do TS-1.
 */
if (version < 12) /* Not needed for u2.10.12+ */
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
MyUplink->RegisterEvent( EVT_KILL, this );
MyUplink->RegisterEvent( EVT_QUIT, this );
MyUplink->RegisterEvent( EVT_BURST_CMPLT, this );
MyUplink->RegisterEvent( EVT_NETJOIN, this );
MyUplink->RegisterEvent( EVT_NETBREAK, this );

/**
 * Register for all channel events
 */
MyUplink->RegisterChannelEvent( xServer::CHANNEL_ALL, this );

xClient::OnAttach() ;
}

/* OnTimer */
void chanfix::OnTimer(const gnuworld::xServer::timerID& theTimer, void*)
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
}

/* OnDetach */
void chanfix::OnDetach( const std::string& reason )
{
/* Delete our config */
delete chanfixConfig; chanfixConfig = 0;

/* Delete our connection to the database */
delete SQLDb; SQLDb = 0;

/* Delete our timer */
delete theTimer; theTimer = 0;

/* Delete our commands */
for (commandMapType::iterator ptr = commandMap.begin();
     ptr != commandMap.end(); ++ptr) {
  delete ptr->second;
}
commandMap.clear();

xClient::OnDetach( reason ) ;
}

/* OnConnect */
void chanfix::OnConnect()
{

/* If we have just reloaded, we won't be in BURST. */
if (currentState == INIT)
  changeState(RUN);

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
if (!theClient->isOper()) {
  return;
}

if (currentState == BURST) {
  SendTo(theClient, "Sorry, I do not accept commands during a burst.");
  return;
}

StringTokenizer st(Message) ;
if ( st.empty() )
  return;

const std::string Command = string_upper(st[0]);

commandMapType::iterator commHandler = commandMap.find(Command);
if (commHandler == commandMap.end()) {
  return;
}

if (st.size() < commHandler->second->getNumParams()) {
  commHandler->second->Usage(theClient);
  return;
}

sqlUser* theUser = isAuthed(theClient->getAccount());
sqlUser::flagType requiredFlags = commHandler->second->getRequiredFlags();
if (requiredFlags) {
  if (!theUser) {
    SendTo(theClient, "You need to authenticate to use this command.");
    return;
  }

  if (requiredFlags != sqlUser::F_LOGGEDIN &&
      !theUser->getFlag(requiredFlags)) {
    if (getFlagChar(requiredFlags) != ' ')
      SendTo(theClient, "This command requires flag '%c'.",
	     getFlagChar(requiredFlags));
    else
      SendTo(theClient, "This command requires one of these flags: \"%s\".",
	     getFlagsString(requiredFlags).c_str());
    return;
  }
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
  DoCTCP(theClient, CTCP, "evilnet development - GNUWorld chanfix v" CF_VERSION);
}

xClient::OnCTCP(theClient, CTCP, Message, Secure);
}

// Burst any channels.
void chanfix::BurstChannels()
{
xClient::BurstChannels();

Join(consoleChan, consoleChanModes, 0, true);
Join(operChan, operChanModes, 0, true);
Join(supportChan, supportChanModes, 0, true);

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
	case EVT_CREATE:
	case EVT_JOIN:
		{
		/* First we need to see if the channel is first = minClients */
		if (theChan->size() == minClients)
		  startScoringChan(theChan);

		/* If this is the operChan, op opers on join */
		theClient = static_cast< iClient* >( data1 );
		if (theClient->isOper() && theChan->getName() == operChan)
		  Op(theChan, theClient);

		break ;
		}
	case EVT_KICK:
	case EVT_PART:
		{
		theClient = static_cast< iClient* >( data1 );
		if (wasOpped(theChan, theClient))
		  lostOps(theChan, theClient);
		break ;
		}
	default:
		break ;
	}

xClient::OnChannelEvent( whichEvent, theChan,
	data1, data2, data3, data4 ) ;
}

void chanfix::OnChannelModeO( Channel* theChan, ChannelUser*,
			const xServer::opVectorType& theTargets)
{
/* if (currentState != RUN) return; */

if (theChan->size() < minClients) return;

for (xServer::opVectorType::const_iterator ptr = theTargets.begin();
     ptr != theTargets.end(); ++ptr) {
  ChannelUser* tmpUser = ptr->second;
  bool polarity = ptr->first;

  if (polarity) {
    // Someone is opped
    gotOpped(theChan, tmpUser->getClient());
  } else {
    // Someone is deopped
    if (wasOpped(theChan, tmpUser->getClient()))
      lostOps(theChan, tmpUser->getClient());
  } // if
} // for
}

/* OnEvent */
void chanfix::OnEvent( const eventType& whichEvent,
	void* data1, void* data2, void* data3, void* data4 )
{
switch(whichEvent)
	{
	case EVT_BURST_CMPLT:
		{
		changeState(RUN);
		break;
		}
	case EVT_NETJOIN:
	case EVT_NETBREAK:
		{
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
		  lostOps(ptr->second, theClient);
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
Channel* tmpChan = Network->findChannel(operChan);
if (!tmpChan)
	{
	return false;
	}

std::string message = std::string( "[" ) + nickName + "] " + buf ;
serverNotice(tmpChan, message);
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
}

void chanfix::preloadUserCache()
{

	std::stringstream theQuery;
	theQuery	<< "SELECT id, user_name, created, last_seen, last_updated, last_updated_by, flags, issuspended, usenotice "
			<< "FROM users"
			;

	ExecStatusType status = SQLDb->Exec(theQuery.str().c_str());

	if(PGRES_TUPLES_OK == status) {
		/* First we need to clear the current cache. */
		for(usersMapType::iterator itr = usersMap.begin() ;
		    itr != usersMap.end() ; ++itr) {
			delete itr->second;
		}
		usersMap.clear();

		for(int i = 0; i < SQLDb->Tuples(); ++i) {
			sqlUser *newUser = new sqlUser(SQLDb);
			assert(newUser != 0);

			newUser->setAllMembers(i);
			usersMap.insert(usersMapType::value_type(newUser->getUserName(), newUser));
		}
	} else {
		elog	<< "chanfix::preloadUserCache> "
			<< SQLDb->ErrorMessage();
	}

	elog	<< "chanfix::preloadUserCache> Loaded "
		<< usersMap.size()
		<< " users."
		<< std::endl ;
}

void chanfix::preloadChanOpsCache()
{
	std::stringstream theQuery;
	theQuery	<< "SELECT channel,account,last_seen_as,ts_firstopped,ts_lastopped,day0,day1,day2,day3,day4,day5,day6,day7,day8,day9,day10,day11,day12,day13 FROM chanOps"
			<< std::ends;

	elog		<< "*** [chanfix::preloadChanOpsCache]: Loading chanOps and their points ..." 
			<< std::endl;

	ExecStatusType status = SQLDb->Exec(theQuery.str().c_str()) ;

	if( PGRES_TUPLES_OK == status )
	{
		for (int i = 0 ; i < SQLDb->Tuples(); i++)
		{
			sqlChanOp* newOp = new (std::nothrow) sqlChanOp(SQLDb);
			assert( newOp != 0 ) ;

			newOp->setAllMembers(i);
			std::pair<std::string, std::string> thePair (newOp->getChannel(), newOp->getAccount());
			sqlChanOps.insert(sqlChanOpsType::value_type(thePair, newOp));
		}
	} else	{
		elog << "[chanfix::preloadChanOpsCache] Something went wrong: "
			<< SQLDb->ErrorMessage()
			<< std::endl;
		exit(0);
	}

	elog	<< "*** [chanfix::preloadChanOpsCache]: Done. Loaded "
			<< SQLDb->Tuples()
			<< " chanops."
			<< std::endl;
}

void chanfix::preloadChannelCache()
{
	std::stringstream theQuery;
	theQuery	<< "SELECT channel, flags FROM channels"
			<< std::ends;

	elog		<< "*** [chanfix::preloadChannelCache]: Loading channels ..."
			<< std::endl;

	ExecStatusType status = SQLDb->Exec(theQuery.str().c_str()) ;

	if( PGRES_TUPLES_OK == status )
	{
		for (int i = 0 ; i < SQLDb->Tuples(); i++)
		{
			sqlChannel* newChan = new (std::nothrow) sqlChannel(SQLDb);
			assert( newChan != 0 ) ;

			newChan->setAllMembers(i);
			sqlChanCache.insert(sqlChannelCacheType::value_type(newChan->getChannel(), newChan));
		}
	} else  {
		elog	<< "[chanfix::preloadChannelCache] Something went wrong: "
			<< SQLDb->ErrorMessage()
			<< std::endl;
		exit(0);
	}

	elog	<< "*** [chanfix::preloadChannelCache]: Done. Loaded "
			<< SQLDb->Tuples()
			<< " channels."
			<< std::endl;
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
	}
	case SPLIT:
	{
	elog	<< "chanfix::changeState> Exiting state SPLIT"
		<< std::endl;
	}
	case INIT:
	{
	elog	<< "chanfix::changeState> Exiting state INIT"
		<< std::endl;
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

sqlChanOpsType::iterator ptr = sqlChanOps.find(std::pair<std::string,std::string>(channel, account));
if (ptr != sqlChanOps.end()) {
  elog	<< "chanfix::findChanOp> DEBUG: We've got a winner: "
	<< ptr->second->getAccount() << " on " << ptr->second->getChannel() << "!!" << std::endl;
  return ptr->second ;
}

return 0;
}

sqlChanOp* chanfix::newChanOp(const std::string& channel, const std::string& account)
{
sqlChanOp* newOp = new (std::nothrow) sqlChanOp(SQLDb);
assert( newOp != 0 ) ;

sqlChanOps.insert(sqlChanOpsType::value_type(std::pair<std::string,std::string>(channel, account), newOp));

elog << "chanfix::newChanOp> DEBUG: Added new operator: " << account << " on " << channel << "!!" << std::endl;

newOp->setChannel(channel);
newOp->setAccount(account);
newOp->setTimeFirstOpped(currentTime());
newOp->setTimeLastOpped(currentTime());
newOp->Insert();

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

chanfix::chanOpsType chanfix::getMyOps(Channel* theChan)
{
chanOpsType myOps;
for (sqlChanOpsType::iterator ptr = sqlChanOps.begin();
     ptr != sqlChanOps.end(); ptr++) {
  if (ptr->second->getChannel() == theChan->getName())
    myOps.push_back(ptr->second);
}

myOps.sort(compare_points);

return myOps;
}

const std::string chanfix::getHostList( sqlUser* User)
{
static const char* queryHeader
	= "SELECT host FROM hosts WHERE user_id =  ";

std::stringstream theQuery;
theQuery	<< queryHeader 
		<< User->getID()
		<< std::ends;

ExecStatusType status = SQLDb->Exec( theQuery.str().c_str() ) ;

if( PGRES_TUPLES_OK != status )
	{
	elog	<< "getHostList> SQL Error: "
		<< SQLDb->ErrorMessage()
		<< std::endl ;
	return NULL ;
	}

// SQL Query succeeded
std::stringstream hostlist;
for (int i = 0 ; i < SQLDb->Tuples(); i++)
	{
	if (!i)
	  hostlist << SQLDb->GetValue(i, 0);
	else
	  hostlist << ", " << SQLDb->GetValue(i, 0);
	}
if (hostlist.str() == "") hostlist << "None.";
return hostlist.str();
}

const std::string gnuworld::escapeSQLChars(const std::string& theString)
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

bool gnuworld::atob( std::string str )
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
if(!thisOp) thisOp = newChanOp(theChan, theClient);

thisOp->addPoint();
thisOp->setTimeLastOpped(currentTime()); //Update the time they were last opped
thisOp->commit();

elog	<< "chanfix::givePoints> DEBUG: Gave " << thisOp->getAccount()
	<< " on " << thisOp->getChannel() << " a point."
	<< std::endl;
}

void chanfix::gotOpped(Channel* thisChan, iClient* thisClient)
{
//Not enough users, forget about it.
//if (thisChan->size() < minClients) return;

//No tracking for unidented clients
if (clientNeedsIdent && !hasIdent(thisClient))
  return;

if (thisClient->getAccount() != "" &&
    !thisClient->getMode(iClient::MODE_SERVICES) &&
    !thisChan->getMode(Channel::MODE_A)) {
  elog	<< "chanfix::gotOpped> DEBUG: " << thisClient->getAccount()
	<< " got opped on " << thisChan->getName()
	<< std::endl;

  sqlChanOp* thisOp = findChanOp(thisChan, thisClient);
  if (!thisOp) thisOp = newChanOp(thisChan, thisClient);

  thisOp->setLastSeenAs(thisClient->getRealNickUserHost());
  thisOp->setTimeLastOpped(currentTime());
  
  if (wasOpped(thisChan, thisClient))
    return;
  
  
  clientOpsType* myOps = findMyOps(thisClient);
  myOps->insert(clientOpsType::value_type(thisChan->getName(), thisChan));
  thisClient->setCustomData(this, static_cast< void*>(myOps));
} //if
return;
}

bool chanfix::hasIdent(iClient* theClient)
{
std::string userName = theClient->getUserName();
if (userName[0] == '~')
  return false;
return true;
}

bool chanfix::wasOpped(Channel* theChan, iClient* theClient)
{
clientOpsType* myOps = findMyOps(theClient);
if (!myOps || myOps->empty())
  return false;

theClient->setCustomData(this, static_cast< void*>(myOps));

clientOpsType::iterator ptr = myOps->find(theChan->getName());
if (ptr != myOps->end())
  return true;
return false;
}

void chanfix::lostOps(Channel* theChan, iClient* theClient)
{
clientOpsType* myOps = findMyOps(theClient);
if (!myOps || myOps->empty())
  return;

clientOpsType::iterator ptr = myOps->find(theChan->getName());
if (ptr != myOps->end())
  myOps->erase(theChan->getName());
theClient->setCustomData(this, static_cast< void*>(myOps));
}

void chanfix::checkNetwork()
{
if (100 * Network->serverList_size() < numServers * minServersPresent) {
  elog << "chanfix::checkNetwork> DEBUG: Not enough servers linked! Going to SPLIT-state" << std::endl;
  changeState(SPLIT);
  return;
}

if (currentState == SPLIT) {
  elog << "chanfix::checkNetwork> DEBUG: Enough servers linked! Going to BURST-state" << std::endl;
  changeState(BURST);
  return;
}
}

void chanfix::autoFix()
{
/* If autofixing has been disabled, well, forget it. */
if (!enableAutoFix) {
  elog << "chanfix::autoFix> DEBUG: AutoFix not enabled." << std::endl;
  return;
}

/* If there are too many servers split, don't autofix. */
if (currentState != RUN) {
  elog << "chanfix::autoFix> DEBUG: currentState != RUN" << std::endl;
  return;
}

/* Start our timer. */
Timer autoFixTimer;
autoFixTimer.Start();

/* Now walk through all channels to find the opless ones. */
Channel* thisChan;
ChannelUser* curUser;
int numOpLess = 0;
for (xNetwork::channelIterator ptr = Network->channels_begin(); ptr != Network->channels_end(); ptr++) {
   thisChan = ptr->second;
   bool opLess = true;
   bool hasService = false;
   if (thisChan->getMode(Channel::MODE_A))
     continue;
   if (thisChan->size() >= minClients && !isBeingFixed(thisChan)) {
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
       sqlChannel* sqlChan = getChannelRecord(thisChan);
       if (!sqlChan) sqlChan = newChannelRecord(thisChan);

       chanOpsType myOps = getMyOps(thisChan);
       if (myOps.begin() != myOps.end())
	 sqlChan->setMaxScore((*myOps.begin())->getPoints());

       if ((sqlChan->getMaxScore() > 
	   static_cast<int>(static_cast<float>(FIX_MIN_ABS_SCORE_END)
	   * MAX_SCORE)) && !sqlChan->getFlag(sqlChannel::F_BLOCKED)) {
	 elog << "chanfix::autoFix> DEBUG: " << thisChan->getName() << " is opless, fixing." << std::endl;
	 autoFixQ.push_back(fixQueueType::value_type(thisChan, currentTime()));
	 numOpLess++;
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
elog << "chanfix::manualFix> DEBUG: Manual fix " << thisChan->getName() << "!" << std::endl;

if (thisChan->getCreationTime() > 1) {
  if (version >= 12) /* temporary fix until GNUWorld is fixed */
    MyUplink->setBursting(true);

  xServer::modeVectorType modeVector;
  if (thisChan->getMode(Channel::MODE_I))
    modeVector.push_back(std::make_pair(false, Channel::MODE_I));
  if (thisChan->getMode(Channel::MODE_K))
    OnChannelModeK(thisChan, false, 0, std::string());
  if (thisChan->getMode(Channel::MODE_L))
    OnChannelModeL(thisChan, false, 0, 0);
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
    OnChannelMode(thisChan, 0, modeVector);

  MyUplink->BurstChannel(thisChan->getName(), thisChan->getModeString(),
			 thisChan->getCreationTime() - 1);

  if (version >= 12)
    MyUplink->setBursting(false);
} else {
  ClearMode(thisChan, "obiklrD", true);
}

Message(thisChan, "Channel fix in progress, please stand by.");

manFixQ.push_back(fixQueueType::value_type(thisChan, currentTime() + CHANFIX_DELAY));
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
		time_since_start / max_time *
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
		time_since_start / max_time *
		(maxScore * static_cast<float>(FIX_MIN_REL_SCORE_BEGIN)
		 - static_cast<float>(FIX_MIN_REL_SCORE_END) * maxScore));

/* The minimum score needed for ops is the HIGHER of these two
 * scores. */
int min_score = min_score_abs;
if (min_score_rel > min_score)
  min_score = min_score_rel;

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


chanfix::acctListType chanfix::findAccount(Channel* theChan, const std::string& Account)
{
acctListType chanAccts;
for (Channel::userIterator ptr = theChan->userList_begin();
     ptr != theChan->userList_end(); ptr++) {
  if (Account == ptr->second->getClient()->getAccount())
    chanAccts.push_back(ptr->second->getClient());
}

return chanAccts;
}

sqlChannel* chanfix::getChannelRecord(const std::string& Channel)
{
sqlChannelCacheType::iterator ptr = sqlChanCache.find(Channel);
if(ptr != sqlChanCache.end())
	{
	elog << "chanfix::getChannelRecord> DEBUG: cached channel " << Channel << " found" << std::endl;
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
sqlChannel* newChan = new (std::nothrow) sqlChannel(SQLDb);
assert( newChan != 0 ) ;

newChan->setChannel(Channel);
newChan->setFixStart(0);
newChan->setLastAttempt(0);
newChan->Insert();

sqlChanCache.insert(sqlChannelCacheType::value_type(Channel, newChan));
elog << "chanfix::getChannelRecord> DEBUG: Added new channel: " << Channel << std::endl;

return newChan;
}

sqlChannel* chanfix::newChannelRecord(Channel* theChan)
{
return newChannelRecord(theChan->getName());
}

bool chanfix::deleteChannelRecord(sqlChannel* sqlChan)
{
if (!sqlChan->Delete())
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

void chanfix::processQueue()
{
/* If there are too many servers split, don't process queue. */
if (currentState != RUN) {
  //elog << "chanfix::processQueue> DEBUG: currentState != RUN" << std::endl;
  return;
}

for (fixQueueType::iterator ptr = autoFixQ.begin(); ptr != autoFixQ.end(); ) {
   //elog << "chanfix::processQueue> DEBUG: Processing " << ptr->first->getName() << " in autoFixQ ..." << std::endl;
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
       ptr = autoFixQ.erase(ptr);
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
   //elog << "chanfix::processQueue> DEBUG: Processing " << ptr->first->getName() << " in manFixQ ..." << std::endl;
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
       ptr = manFixQ.erase(ptr);
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
for (fixQueueType::iterator ptr = autoFixQ.begin(); ptr != autoFixQ.end(); ptr++) {
  if (ptr->first->getName() == theChan->getName()) return true;
}

return false;
}

bool chanfix::isBeingChanFixed(Channel* theChan)
{
for (fixQueueType::iterator ptr = manFixQ.begin(); ptr != manFixQ.end(); ptr++) {
  if (ptr->first->getName() == theChan->getName()) return true;
}

return false;
}

bool chanfix::removeFromAutoQ(Channel* theChan)
{
for (fixQueueType::iterator ptr = autoFixQ.begin(); ptr != autoFixQ.end(); ptr++) {
  if (ptr->first->getName() == theChan->getName()) {
    ptr = autoFixQ.erase(ptr);
    return true;
  }
}

return false;
}

bool chanfix::removeFromManQ(Channel* theChan)
{
for (fixQueueType::iterator ptr = manFixQ.begin(); ptr != manFixQ.end(); ptr++) {
  if (ptr->first->getName() == theChan->getName()) {
    ptr = manFixQ.erase(ptr);
    return true;
  }
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
if(!tempUser)
	{
	usersMap.erase(usersMap.find(Name));
	}
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
time_t theTime = time(NULL) + connectCheckFreq;
tidCheckDB = MyUplink->RegisterTimer(theTime, this, NULL);
theTime = time(NULL) + CHECK_CHANS_TIME;
tidAutoFix = MyUplink->RegisterTimer(theTime, this, NULL);
theTime = time(NULL) + PROCESS_QUEUE_TIME;
tidFixQ = MyUplink->RegisterTimer(theTime, this, NULL);
theTime = time(NULL) + POINTS_UPDATE_TIME;
tidGivePoints = MyUplink->RegisterTimer(theTime, this, NULL);
theTime = time(NULL) + getSecsTilMidnight();
tidRotateDB = MyUplink->RegisterTimer(theTime, this, NULL);
elog	<< "chanfix::startTimers> Started all timers."
	<< std::endl;
}

void chanfix::rotateDB()
{
/* CODER NOTES:
 * go through everybody and remove the oldest day (set it to 0)
 * then loop back through and pick out the users with 0 points total and check their ts_firstopped
 * if it is older than 1 day, delete the user
 * cache: acct,chan
 */

logAdminMessage("Beginning database rotation.");
sqlChanOp* curOp = 0;
std::string removeKey;
short nextDay = currentDay;
setCurrentDay();
if (nextDay >= (DAYSAMPLES - 1))
  nextDay = 0;
else
  nextDay++;



time_t maxFirstOppedTS = currentTime() - 86400;
for (sqlChanOpsType::iterator ptr = sqlChanOps.begin();
     ptr != sqlChanOps.end(); ptr++) {

  curOp = ptr->second;
  curOp->setDay(nextDay, 0);
  curOp->calcTotalPoints();
  if (curOp->getPoints() <= 0 && maxFirstOppedTS > curOp->getTimeFirstOpped()) {
    sqlChanOps.erase(ptr++);
    if (!curOp->Delete())
      elog << "chanfix::rotateDB> Error: Could not delete op "<< curOp->getLastSeenAs() << " " << curOp->getChannel() << std::endl;
    else
      delete curOp; curOp = 0;
  }
}
#ifndef REMEMBER_CHANNELS_WITH_NOTES_OR_FLAGS
//TODO: Implement a loop/section here that removes channel records/notes with no ops left in them
#endif
return;
}

void chanfix::giveAllOpsPoints()
{
Channel* thisChan;
typedef std::map<std::string,bool> ScoredOpsMapType;
ScoredOpsMapType scoredOpsList;
ScoredOpsMapType::iterator scOpiter;
for (xNetwork::channelIterator ptr = Network->channels_begin();
     ptr != Network->channels_end(); ptr++) {
  thisChan = ptr->second;
  if (thisChan->getMode(Channel::MODE_A))
    continue; // Exit the loop and go to the next chan
  if (thisChan->size() >= minClients && !isBeingFixed(thisChan)) {
    scoredOpsList.clear();
    for (Channel::userIterator ptr = thisChan->userList_begin();
	 ptr != thisChan->userList_end(); ptr++) {
      ChannelUser* curUser = ptr->second;
      if (curUser->getClient()->getMode(iClient::MODE_SERVICES))
	break; // Exit the loop fully, and go to the next chan
      if (curUser->isModeO() && curUser->getClient()->getAccount() != "") {
	//Ok hes an op
	//Grab an iClient for curUser
	scOpiter = scoredOpsList.find(curUser->getClient()->getAccount());
	if (scOpiter == scoredOpsList.end()) {
	  givePoints(thisChan, curUser->getClient());
	  scoredOpsList.insert(make_pair(curUser->getClient()->getAccount(), true));
	}
      }
    }
  }
}
if (scoredOpsList.size() > 0)
  scoredOpsList.clear();
return;
} //giveAllOpsPoints

void chanfix::startScoringChan(Channel* theChan)
{
/* Ok, if a channel record exists, should we
 * still add points to ALL ops? -- Compy
 */
chanfix::chanOpsType myOps = getMyOps(theChan);
if (!myOps.empty()) return;

typedef std::map<std::string,bool> ScoredOpsMapType;
ScoredOpsMapType scoredOpsList;
ScoredOpsMapType::iterator scOpiter;
if (theChan->getMode(Channel::MODE_A)) return;
  for (Channel::userIterator ptr = theChan->userList_begin();
       ptr != theChan->userList_end(); ptr++) {
    ChannelUser* curUser = ptr->second;
    if (curUser->getClient()->getMode(iClient::MODE_SERVICES))
      break;
    if (curUser->isModeO() && curUser->getClient()->getAccount() != "") {
      scOpiter = scoredOpsList.find(curUser->getClient()->getAccount());
      if (scOpiter == scoredOpsList.end()) {
	gotOpped(theChan, curUser->getClient());
	scoredOpsList.insert(make_pair(curUser->getClient()->getAccount(), true));
      }
    }
  }
  scoredOpsList.clear();
return;
}

char chanfix::getFlagChar(const sqlUser::flagType& whichFlag)
{
 if (whichFlag == sqlUser::F_SERVERADMIN)
   return 'a';
 else if (whichFlag == sqlUser::F_BLOCK)
   return 'b';
 else if (whichFlag == sqlUser::F_CHANNEL)
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
 if (whichFlags & sqlUser::F_CHANNEL)
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
  case 'c': return sqlUser::F_CHANNEL;
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
if (SQLDb->Status() == CONNECTION_BAD) { //Check if the connection has died
  delete(SQLDb);
  dbConnected = false;
  updateSqldb(NULL);
  MsgChanLog("PANIC! - The Connection With The Db Was Lost\n");
  MsgChanLog("Attempting to reconnect, Attempt %d out of %d\n",
	     connectCount+1,connectRetry+1);
  std::string Query = "host=" + sqlHost + " dbname=" + sqlDb + " port=" + sqlPort;
  if (strcasecmp(sqlUser,"''"))
    Query += (" user=" + sqlUser);
  if (strcasecmp(sqlPass,"''"))
    Query += (" password=" + sqlPass);
  SQLDb = new (std::nothrow) cmDatabase(Query.c_str());
  assert(SQLDb != NULL);

  if (SQLDb->ConnectionBad()) {
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
    updateSqldb(SQLDb);
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
bot->SendTo( theClient, std::string( "SYNTAX: " ) + getInfo() ) ;
}

} // namespace gnuworld
