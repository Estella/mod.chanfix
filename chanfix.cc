/**
 * chanfix.cc
 * 
 * Copyright (C) 2003	Reed Loden <reed@reedloden.com>
 *			Matthias Crauwels <ultimate_@wol.be>
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

#include	<new>
#include	<map>
#include	<vector>
#include	<iostream>
#include	<sstream>
#include	<string>
#include	<iomanip>
#include	<utility>

#include	<ctime>
#include	<cstdlib>
#include	<cstring>
#include	<cstdarg>

#include	"libpq++.h"

#include	"gnuworld_config.h"
#include	"client.h"
#include	"EConfig.h"
#include	"Network.h"
#include	"server.h"
#include	"StringTokenizer.h"

#include	"chanfix.h"
#include	"chanfixCommands.h"
#include	"chanfix_misc.h"
#include	"levels.h"
#include	"sqlChannel.h"
#include	"sqlChanOp.h"
#include	"sqlUser.h"
//#include	"Timer.h"

RCSTAG("$Id$");

namespace gnuworld
{

using std::endl;
using std::stringstream;
using std::ends;


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
defaultChannelModes = chanfixConfig->Require("defaultChannelModes")->second ;
version = atoi((chanfixConfig->Require("version")->second).c_str()) ;
numServers = atoi((chanfixConfig->Require("numServers")->second).c_str()) ;
minServersPresent = atoi((chanfixConfig->Require("minServersPresent")->second).c_str()) ;
numTopScores = atoi((chanfixConfig->Require("numTopScores")->second).c_str()) ;
minClients = atoi((chanfixConfig->Require("minClients")->second).c_str()) ;
clientNeedsIdent = atob(chanfixConfig->Require("clientNeedsIdent")->second) ;
clientNeedsReverse = atob(chanfixConfig->Require("clientNeedsReverse")->second) ;
connectCheckFreq = atoi((chanfixConfig->Require("connectCheckFreq")->second).c_str()) ;

/* Initial state */
currentState = INIT;

/* Database processing */
sqlHost = chanfixConfig->Require("sqlHost")->second;
sqlPort = chanfixConfig->Require("sqlPort")->second;
sqlDB = chanfixConfig->Require("sqlDB")->second;
sqlUser = chanfixConfig->Require("sqlUser")->second;
sqlPass = chanfixConfig->Require("sqlPass")->second;

elog    << "chanfix::chanfix::> Configuration loaded!"
        << endl;

std::string Query = "host=" + sqlHost + " dbname=" + sqlDB + " port=" + sqlPort + " user=" + sqlUser;

elog    << "chanfix::chanfix> Attempting to connect to "
        << sqlHost << " at port " << sqlPort
        << " as User " << sqlUser << " to database: "
        << sqlDB
        << endl;

SQLDb = new (std::nothrow) cmDatabase( Query.c_str() ) ;
assert( SQLDb != 0 ) ;
//-- Make sure we connected to the SQL database; if
// we didn't we exit entirely.
if (SQLDb->ConnectionBad ())
        {
        elog    << "chanfix::chanfix> Unable to connect to SQL server."
                << endl
                << "chanfix::chanfix> PostgreSQL error message: "
                << SQLDb->ErrorMessage()
                << endl ;

        ::exit( 0 ) ;
        }
else
        {
        elog    << "chanfix::chanfix> Connection established to SQL server"
                << endl ;
        }

/* Register the commands we want to use */
RegisterCommand(new ALERTCommand(this, "ALERT", "<#channel>"));
RegisterCommand(new BLOCKCommand(this, "BLOCK", "<#channel> <reason>"));
RegisterCommand(new CHANFIXCommand(this, "CHANFIX", "<#channel> [override]"));
RegisterCommand(new CHECKCommand(this, "CHECK", "<#channel>"));
RegisterCommand(new INFOCommand(this, "INFO", "<#channel>"));
RegisterCommand(new INVITECommand(this, "INVITE", ""));
RegisterCommand(new OPLISTCommand(this, "OPLIST", "<#channel>"));
RegisterCommand(new OPNICKSCommand(this, "OPNICKS", "<#channel>"));
RegisterCommand(new QUOTECommand(this, "QUOTE", "<text>"));
RegisterCommand(new RELOADCommand(this, "RELOAD", ""));
RegisterCommand(new SCORECommand(this, "SCORE", "<#channel> [nick|*account]"));
RegisterCommand(new SCORECommand(this, "CSCORE", "<#channel> [nick|*account]"));
RegisterCommand(new SHUTDOWNCommand(this, "SHUTDOWN", "[reason]"));
RegisterCommand(new STATUSCommand(this, "STATUS", ""));
RegisterCommand(new UNALERTCommand(this, "UNALERT", "<#channel>"));
RegisterCommand(new UNBLOCKCommand(this, "UNBLOCK", "<#channel>"));

/* Preload the ChanOps cache */
preloadChanOpsCache();

/* Preload the Channels cache */
preloadChannelCache();

/* Set up our timer. */
theTimer = new Timer();

}

chanfix::~chanfix()
{
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

/* Old timer space */

xClient::OnAttach() ;
}

/* onTimer */
void chanfix::OnTimer(const gnuworld::xServer::timerID& theTimer, void*)
{
time_t theTime;
Channel* tmpChan = Network->findChannel(operChan);
if (theTimer == tidGivePoints) {
  /* 5 min timer, loop through channels and give all ops a point! */
  theTime = time(NULL) + POINTS_UPDATE_TIME;
  tidGivePoints = MyUplink->RegisterTimer(theTime, this, NULL);
  giveAllOpsPoints();
  logAdminMessage("Giving all channel ops a point.");
}
if (theTimer == tidAutoFix) {
  autoFix();
  /* Refresh Timer */
  theTime = time(NULL) + CHECK_CHANS_TIME;
  tidAutoFix = MyUplink->RegisterTimer(theTime, this, NULL);
  if (tmpChan)
	{
	logAdminMessage("Autofixing channels...");
	elog    << "chanfix::chanfix::> Autofixing channels."
            << endl;
	}
  }
else if (theTimer == tidUpdateDB) {
  updatePoints();

  /* Refresh Timer */
  theTime = time(NULL) + SQL_UPDATE_TIME;
  tidUpdateDB = MyUplink->RegisterTimer(theTime, this, NULL);
  if (tmpChan)
	{
	logAdminMessage("Updating databases...");
	}
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
  if (tmpChan)
	{
	logAdminMessage("Processing fix queue...");
	elog    << "chanfix::chanfix::> Processing fix queue."
            << endl;
	}
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
for(commandMapType::iterator ptr = commandMap.begin() ;
    ptr != commandMap.end() ; ++ptr) {
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

bool chanfix::serverNotice( Channel* theChannel, const char* format, ... )
{
char buf[ 1024 ] = { 0 } ;
va_list _list ;
va_start( _list, format ) ;
vsnprintf( buf, 1024, format, _list ) ;
va_end( _list ) ;

stringstream s;
s	<< MyUplink->getCharYY()
	<< " O "
	<< theChannel->getName()
	<< " :"
	<< buf
	<< ends;

Write( s );

return false;
}

/**
 * Send a notice to a channel from the server.
 * TODO: Move this method to xServer.
 */
bool chanfix::serverNotice( Channel* theChannel, const string& Message)
{
stringstream s;
s	<< MyUplink->getCharYY()
	<< " O "
	<< theChannel->getName()
	<< " :"
	<< Message
	<< ends;

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
//Channel* tmpChan = Network->findChannel(getConfigVar("CMASTER.RELAY_CHAN")->asString());
Channel* tmpChan = Network->findChannel(operChan);
if (!tmpChan)
	{
	return false;
	}

string message = string( "[" ) + nickName + "] " + buf ;
serverNotice(tmpChan, message);
return true;
}

void chanfix::OnPrivateMessage( iClient* theClient,
	const std::string& Message, bool secure)
{
if (!theClient->isOper()) {
  return;
}

if (currentState == BURST) {
  Notice(theClient, "Sorry, I do not accept commands during a burst.");
  return;
}

StringTokenizer st(Message) ;
if ( st.empty() )
  return;

const std::string Command = string_upper(st[0]);

/**
 * Just quickly, abort if someone tries to LOGIN or NEWPASS
 * unsecurely.
 */
if (!secure && ((Command == "LOGIN") || (Command == "NEWPASS"))) {
  Notice(theClient, "To use %s, you must /msg %s@%s",
	 Command.c_str(), nickName.c_str(), getUplinkName().c_str());
  return;
}

commandMapType::iterator commHandler = commandMap.find(Command);
if (commHandler == commandMap.end()) {
  return;
}

commHandler->second->Exec(theClient, Message);

xClient::OnPrivateMessage(theClient, Message);
}

void chanfix::OnCTCP( iClient* theClient, const std::string& CTCP,
		const std::string& Message, bool Secure )
{
StringTokenizer st(CTCP);

if(st.empty()) return;

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
                theClient = static_cast< iClient* >( data1 ) ;

                if( theClient->isOper() && theChan->getName() == operChan)
                        {
                        Op( theChan, theClient ) ;
                        }
                 
                break ;
		}
	case EVT_KICK:
	case EVT_PART:
		{
                theClient = static_cast< iClient* >( data1 ) ;
		if (wasOpped(theClient, theChan)) {
		  givePoints(theClient, theChan);
		  lostOps(theClient, theChan);
		}
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

for( xServer::opVectorType::const_iterator ptr = theTargets.begin() ;
        ptr != theTargets.end() ; ++ptr )
        {
        ChannelUser* tmpUser = ptr->second;
        bool polarity = ptr->first;

        if (polarity) {
	  // Someone is opped
	  gotOpped(tmpUser->getClient(), theChan);
	} else {
	  // Someone is deopped
          if(wasOpped(tmpUser->getClient(), theChan)) {
	    givePoints(tmpUser->getClient(), theChan);
	    lostOps(tmpUser->getClient(), theChan);
	  }
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
        /* Start our timers */
        iServer* burstedServer = static_cast< iServer* >( data1);
        if (burstedServer == MyUplink->getUplink()) startTimers();
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
		for(clientOpsType::iterator ptr = myOps->begin(); ptr != myOps->end(); ptr++) {
		  givePoints(theClient, ptr->second);
		  lostOps(theClient, ptr->second);
		}
                break;
		}
	}

xClient::OnEvent( whichEvent, data1, data2, data3, data4 ) ;
}
void chanfix::doSqlError(const string& theQuery, const string& theError)
{
	/* First, log it to error out */
	elog	<< "SQL> Whilst executing: "
		<< theQuery
		<< std::endl;
	elog	<< "SQL> "
		<< theError
		<< std::endl;
}
void chanfix::preloadChanOpsCache()
{
        std::stringstream theQuery;
        theQuery        << "SELECT channel,userhost,last_seen_as,points,account,ts_firstopped,ts_lastopped FROM chanOps"
                                << ends;

        elog            << "*** [chanfix::preloadChanOpsCache]: Loading chanOps and their points ..." 
                                << endl;

        ExecStatusType status = SQLDb->Exec(theQuery.str().c_str()) ;

        if( PGRES_TUPLES_OK == status )
        {
                for (int i = 0 ; i < SQLDb->Tuples(); i++)
                        {
                                sqlChanOp* newOp = new (std::nothrow) sqlChanOp(SQLDb);
                                assert( newOp != 0 ) ;

                                newOp->setAllMembers(i);
				std::pair<string, string> thePair (newOp->getAccount(), newOp->getChannel());
                                sqlChanOps.insert(sqlChanOpsType::value_type(thePair, newOp));
                        }
        } else	{
		elog << "[chanfix::preloadChanOpsCache] Something went wrong: "
        	        << SQLDb->ErrorMessage()
                	<< endl;
		exit(0);
	}

        elog    << "*** [chanfix::preloadChanOpsCache]: Done. Loaded "
                        << SQLDb->Tuples()
                        << " chanops."
                        << endl;
}

void chanfix::preloadChannelCache()
{
        std::stringstream theQuery;
        theQuery        << "SELECT channel, fixed, lastfix, flags FROM channels"
                                << ends;

        elog            << "*** [chanfix::preloadChannelCache]: Loading channels ..."
                                << endl;

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
                elog << "[chanfix::preloadChannelCache] Something went wrong: "
                        << SQLDb->ErrorMessage()
                        << endl;
		exit(0);
        }

        elog    << "*** [chanfix::preloadChannelCache]: Done. Loaded "
                        << SQLDb->Tuples()
                        << " channels."
                        << endl;
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
		<< endl;
	break;
	}
	case RUN:
	{
	elog	<< "chanfix::changeState> Exiting state RUN"
		<< endl;
	}
        case SPLIT:
        {
        elog    << "chanfix::changeState> Exiting state SPLIT"
                << endl;
        }
        case INIT:
        {
        elog    << "chanfix::changeState> Exiting state INIT"
                << endl;
        }

}

currentState = newState;

switch( currentState ) {
	case BURST:
	{
	elog	<< "chanfix::changeState> Entering state BURST"
		<< endl;
	break;
	}
	case RUN:
	{
	elog	<< "chanfix::changeState> Entering state RUN"
		<< endl;
	break;
	}
        case SPLIT:
        {
        elog    << "chanfix::changeState> Entering state SPLIT"
                << endl;
        break;
        }
        case INIT:
        {
        elog    << "chanfix::changeState> Entering state INIT"
                << endl;
        break;
        }

}

elog	<< "Changed state in: "
	<< stateTimer.stopTimeMS()
	<< "ms"
	<< endl;
}

sqlChanOp* chanfix::findChanOp(const std::string& account, const std::string& channel)
{

sqlChanOpsType::iterator ptr = sqlChanOps.find(std::pair<string,string>(account, channel));
if(ptr != sqlChanOps.end())
        {
        elog << "chanfix::findChanOp> DEBUG: We've got a winner: "
                << ptr->second->getAccount() << " on " << ptr->second->getChannel() << "!!" << endl;
        return ptr->second ;
        }

return 0;
}

/*
 * sqlChanOp* chanfix::findChanOp(const std::string& account, const std::string& channel)
 * {
 * std::pair<string,string> curPair;
 *
 * for(sqlChanOpsType::iterator ptr = sqlChanOps.begin(); ptr != sqlChanOps.end(); ptr++) {
 *   curPair = ptr->first;
 *   if(!strcasecmp(curPair.first, account) && !strcasecmp(curPair.second, channel)) {
 * //    elog << "chanfix::findChanOp> DEBUG: We've got a winner: " 
 * //         << ptr->second->getAccount() << " on " << ptr->second->getChannel() << "!!" << endl;
 *    return ptr->second ;
 *  }
 * }
 *
 * return 0;
 * }
 */

sqlChanOp* chanfix::newChanOp(const std::string& account, const std::string& channel)
{
sqlChanOp* newOp = new (std::nothrow) sqlChanOp(SQLDb);
assert( newOp != 0 ) ;


sqlChanOps.insert(sqlChanOpsType::value_type(std::pair<string,string>(account, channel), newOp));

elog << "chanfix::newChanOp> DEBUG: Added new operator: " << account << " on " << channel << "!!" << endl;

newOp->setAccount(account);
newOp->setChannel(channel);
newOp->Insert();

return newOp;
}

sqlChanOp* chanfix::findChanOp(iClient* theClient, Channel* theChan) 
{
return findChanOp(theClient->getAccount(), theChan->getName());
}

sqlChanOp* chanfix::newChanOp(iClient* theClient, Channel* theChan)
{
return newChanOp(theClient->getAccount(), theChan->getName());
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

const std::string gnuworld::escapeSQLChars(const std::string& theString)
{
string retMe ;

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

void chanfix::givePoints(iClient* theClient, Channel* theChan)
{
//No points for unidented clients
if (clientNeedsIdent && !hasIdent(theClient)) return;
sqlChanOp* thisOp = findChanOp(theClient, theChan);
if(!thisOp) thisOp = newChanOp(theClient, theChan);

//int points = (currentTime() - thisOp->getTimeOpped()) / POINTS_UPDATE_TIME;

//points += thisOp->getPoints();

int points = thisOp->getPoints() + 1;

thisOp->setPoints(points); 
thisOp->commit();

elog << "chanfix::givePoints> DEBUG: Gave " << thisOp->getAccount()
        << " on " << thisOp->getChannel() << " "
        << (currentTime() - thisOp->getTimeOpped()) / POINTS_UPDATE_TIME << " points"
        << endl;
}

void chanfix::gotOpped(iClient* thisClient, Channel* thisChan)
{
//Not enough users, forget about it.
//if (thisChan->size() < minClients) return;
//No tracking for unidented clients
if (clientNeedsIdent && !hasIdent(thisClient)) return;
if (thisClient->getAccount() != "" &&
    !thisClient->getMode(iClient::MODE_SERVICES) && 
    !thisChan->getMode(Channel::MODE_A)) {
    elog << "chanfix::gotOpped> DEBUG: " << thisClient->getAccount()
          << " got opped on " << thisChan->getName()
          << endl;

  sqlChanOp* thisOp = findChanOp(thisClient, thisChan);
  if(!thisOp) thisOp = newChanOp(thisClient, thisChan);

  if(wasOpped(thisClient, thisChan)) return;
  
  clientOpsType* myOps = findMyOps(thisClient);
  thisOp->setLastSeenAs(thisClient->getNickUserHost());
  thisOp->setTimeOpped(time(NULL));
  if (thisOp->getTimeFirstOpped() < 1) thisOp->setTimeFirstOpped(time(NULL));
  myOps->insert(clientOpsType::value_type(thisChan->getName(), thisChan));
  thisClient->setCustomData(this, static_cast< void*>(myOps));
} //if

}
bool chanfix::hasIdent(iClient* theClient)
{
     string cHost;
     cHost = theClient->getNickUserHost();
     /* Search for the first of ~, if found,
      * They dont have ident
      */
     string::size_type pos = cHost.find_first_of('~');
     if( string::npos != pos )
         return false;
     else
         return true;    
}
bool chanfix::wasOpped(iClient* theClient, Channel* theChan)
{
clientOpsType* myOps = findMyOps(theClient);
if(!myOps || myOps->empty()) return false;
clientOpsType::iterator ptr = myOps->find(theChan->getName());
theClient->setCustomData(this, static_cast< void*>(myOps));
if(ptr != myOps->end()) return true;
return false;
}

void chanfix::lostOps(iClient* theClient, Channel* theChan)
{
clientOpsType* myOps = findMyOps(theClient);
if(!myOps || myOps->empty()) return;
clientOpsType::iterator ptr = myOps->find(theChan->getName());

if(ptr != myOps->end()) myOps->erase(theChan->getName());
theClient->setCustomData(this, static_cast< void*>(myOps));
}

void chanfix::checkNetwork()
{
if(100 * Network->serverList_size() < numServers * minServersPresent)
	{
	elog << "chanfix::checkNetwork> DEBUG: Not enough servers linked! Going to SPLIT-state" << endl;
	changeState(SPLIT);
	return;
	}

if(currentState == SPLIT)
	{
        elog << "chanfix::checkNetwork> DEBUG: Enough servers linked! Going to BURST-state" << endl;
        changeState(BURST);
        return;
	}
}

void chanfix::autoFix()
{
/* If autofixing has been disabled, well, forget it. */
if (!enableAutoFix) {
  elog << "chanfix::autoFix> DEBUG: AutoFix not enabled." << endl;
  return;
}

/* If there are too many servers split, don't autofix. */
if (currentState != RUN) {
  elog << "chanfix::autoFix> DEBUG: currentState != RUN" << endl;
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
   bool hasApass = thisChan->getMode(Channel::MODE_A);
   if (thisChan->size() >= minClients && !isBeingFixed(thisChan)) {
     for (Channel::userIterator ptr = thisChan->userList_begin(); ptr != thisChan->userList_end(); ptr++) {
	curUser = ptr->second;
	if (curUser->isModeO())
	  opLess = false;
	if (curUser->getClient()->getMode(iClient::MODE_SERVICES))
	  hasService = true;
     }
     if (opLess && !hasService && !hasApass) {
       sqlChannel* sqlChan = getChannelRecord(thisChan);
       if (!sqlChan) sqlChan = newChannelRecord(thisChan);

       chanOpsType myOps = getMyOps(thisChan);
       if (myOps.begin() != myOps.end())
	 sqlChan->setMaxScore((*myOps.begin())->getPoints());

       if ((sqlChan->getMaxScore() > 
	   static_cast<int>(static_cast<float>(FIX_MIN_ABS_SCORE_END)
	   * MAX_SCORE)) && !sqlChan->getFlag(sqlChannel::F_BLOCKED)) {
	 elog << "chanfix::autoFix> DEBUG: " << thisChan->getName() << " is opless, fixing." << endl;
	 autoFixQ.push_back(fixQueueType::value_type(thisChan, currentTime()));
	 numOpLess++;
       }
     }
   }
}

elog << "chanfix::autoFix> DEBUG: Found " << numOpLess << " of " \
	<< Network->channelList_size() << " channels in " \
	<< autoFixTimer.stopTimeMS() << " ms." << endl;
}

void chanfix::manualFix(Channel* thisChan)
{
elog << "chanfix::manualFix> DEBUG: Manual fix " << thisChan->getName() << "!" << endl;

if (thisChan->getCreationTime() > 1) {
  if (version >= 12) /* temporary fix until GNUWorld is fixed */
    MyUplink->setBursting(true);
  MyUplink->BurstChannel(thisChan->getName(), defaultChannelModes,
			 thisChan->getCreationTime() - 1);
  if (version >= 12)
    MyUplink->setBursting(false);
} else {
  ClearMode(thisChan, "ovpsmikblr", true);
}

Message(thisChan, "Channel fix in progress, please stand by.");

manFixQ.push_back(fixQueueType::value_type(thisChan, currentTime() + CHANFIX_DELAY));
}

bool chanfix::fixChan(Channel* theChan, bool autofix)
{
sqlChannel* sqlChan = getChannelRecord(theChan);
if (!sqlChan) sqlChan = newChannelRecord(theChan);

/* First update the time of the previous attempt to now. */
if (sqlChan->getFixStart() == 0) sqlChan->setFixStart(currentTime());
sqlChan->setLastAttempt(currentTime());

/* If the channel doesn't exist (anymore), the fix is successful. */
Channel* netChan = Network->findChannel(theChan->getName());
if (!netChan) return true;

chanOpsType myOps = getMyOps(theChan);

if (myOps.begin() != myOps.end())
  sqlChan->setMaxScore((*myOps.begin())->getPoints());

int maxScore = sqlChan->getMaxScore();

/* If the max score of the channel is lower than the absolute minimum
 * score required, don't even bother trying. */
if (maxScore <= FIX_MIN_ABS_SCORE_END * MAX_SCORE)
  return false;

/* If the channel has enough ops, abort & return. */
unsigned int currentOps = countChanOps(netChan);
if (currentOps >= (autofix ? AUTOFIX_NUM_OPPED : CHANFIX_NUM_OPPED)) {
  elog << "chanfix::fixChan> DEBUG: Enough clients opped on " << theChan->getName() << endl;
  return true;
} 

int time_passed;
if (autofix)
  time_passed = currentTime() - sqlChan->getFixStart();
else
  time_passed = currentTime() - (sqlChan->getFixStart() + CHANFIX_DELAY);

int max_time = (autofix ? AUTOFIX_MAXIMUM : CHANFIX_MAXIMUM);

/* Determine minimum score required for this time. */

/* Linear interpolation of (0, fraction_abs_max * max_score) ->
 * (max_time, fraction_abs_min * max_score)
 * at time t between 0 and max_time. */
int min_score_abs = static_cast<int>((MAX_SCORE *
		static_cast<float>(FIX_MIN_ABS_SCORE_BEGIN)) -
		time_passed / max_time *
		(MAX_SCORE * static_cast<float>(FIX_MIN_ABS_SCORE_BEGIN)
		 - static_cast<float>(FIX_MIN_ABS_SCORE_END) * MAX_SCORE));

elog << "chanfix::fixChan> [" << theChan->getName() << "] max "
	<< MAX_SCORE << ", begin " << FIX_MIN_ABS_SCORE_BEGIN
	<< ", end " << FIX_MIN_ABS_SCORE_END << ", time "
	<< time_passed << ", maxtime " << max_time << "." << endl;

/* Linear interpolation of (0, fraction_rel_max * max_score_channel) ->
 * (max_time, fraction_rel_min * max_score_channel)
 * at time t between 0 and max_time. */
int min_score_rel = static_cast<int>((maxScore *
		static_cast<float>(FIX_MIN_REL_SCORE_BEGIN)) -
		time_passed / max_time *
		(maxScore * static_cast<float>(FIX_MIN_REL_SCORE_BEGIN)
		 - static_cast<float>(FIX_MIN_REL_SCORE_END) * maxScore));

/* The minimum score needed for ops is the HIGHER of these two
 * scores. */
int min_score = min_score_abs;
if (min_score_rel > min_score)
  min_score = min_score_rel;

elog << "chanfix::fixChan> [" << theChan->getName() << "] start "
	<< sqlChan->getFixStart() << ", delta " << time_passed
	<< ", max " << maxScore << ", minabs " << min_score_abs
	<< ", minrel " << min_score_rel << "." << endl;

/**
 * Get the scores of the accounts of the non-opped clients.
 * Find out which clients need to be opped.
 */
iClient* curClient = 0;
sqlChanOp* curOp = 0;
string modes = "+";
string args;
for (chanOpsType::iterator opPtr = myOps.begin(); opPtr != myOps.end();
     opPtr++) {
   curOp = *opPtr;
   if (curOp->getPoints() > min_score) {
     curClient = findAccount(curOp->getAccount(), theChan);
     if (curClient && !theChan->findUser(curClient)->isModeO()) {
       elog << "chanfix::fixChan> DEBUG: Decided to op: " \
	    << curClient->getNickName() << " on " \
	    << theChan->getName() << ". Client has " \
	    << curOp->getPoints() << " points. ABS_MIN = " \
	    << min_score_abs << " and REL_MIN = " << min_score_rel \
	    << endl;
       modes += "o";
       if (!args.empty())
	 args += " ";
       args += curClient->getNickName();
     }
   }
}

/* If no scores are high enough, return. */
/* This code is wrong. TODO: fix and put in correct spot */
/* if (args.empty() || maxScore < min_score) {
  if (autofix && !sqlChan->getModesRemoved()) {
    ClearMode(theChan, "ovpsmikbl", true);
    sqlChan->setModesRemoved(true);
    Message(theChan, "Channel modes have been removed.");
  }
return false;
} */

/* If we need to op at least one client, op him/her. */
if (!args.empty()) {
  Mode(theChan, modes, args, true);

  if (args.size() == 1)
    Message(theChan, "1 client should have been opped.");
  else
    Message(theChan, "%d clients should have been opped.", args.size());
}

sqlChan->commit();

/* Now see if there are enough ops; if so, the fix is complete. */
if (args.size() + currentOps >= netChan->size() ||
    args.size() + currentOps >= (autofix ? AUTOFIX_NUM_OPPED : CHANFIX_NUM_OPPED))
  return true;

return false;
}


iClient* chanfix::findAccount(const std::string& Account, Channel* theChan)
{
// TODO: Accounts are not unique! Make this return a vector in case the 
//       same account occurs more then once on this chan (N/A on Undernet)

for(Channel::userIterator ptr = theChan->userList_begin(); ptr != theChan->userList_end(); ptr++)
	{
	if(Account == ptr->second->getClient()->getAccount())
		{
		return ptr->second->getClient();
		}
	}
return 0;
}

sqlChannel* chanfix::getChannelRecord(const std::string& Channel)
{
sqlChannelCacheType::iterator ptr = sqlChanCache.find(Channel);
if(ptr != sqlChanCache.end())
	{
	elog << "chanfix::getChannelRecord> DEBUG: cached channel " << Channel << " found" << endl;
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
newChan->setSuccessFixes(0);
newChan->setFixStart(0);
newChan->setLastAttempt(0);
newChan->Insert();

sqlChanCache.insert(sqlChannelCacheType::value_type(Channel, newChan));
elog << "chanfix::getChannelRecord> DEBUG: Added new channel: " << Channel << endl;

return newChan;
}

sqlChannel* chanfix::newChannelRecord(Channel* theChan)
{
return newChannelRecord(theChan->getName());
}

size_t chanfix::countChanOps(const Channel* theChan)
{
if (!theChan) {
  /* Don't try this on a null channel. */
  return 0;
}

size_t chanOps = 0;

for (Channel::const_userIterator ptr = theChan->userList_begin();
     ptr != theChan->userList_end(); ++ptr) {
   if (ptr->second->isModeO()) {
     chanOps++;
   } // If opped.
}

return chanOps;
}

void chanfix::processQueue()
{

for (fixQueueType::iterator ptr = autoFixQ.begin(); ptr != autoFixQ.end(); ) {
   elog << "chanfix::processQueue> DEBUG: Processing " << ptr->first->getName() << " in autoFixQ ..." << endl;
   if (ptr->second <= currentTime()) {
     sqlChannel* sqlChan = getChannelRecord(ptr->first);
     if (!sqlChan) sqlChan = newChannelRecord(ptr->first);
     bool isFixed = false;

     if (currentTime() - sqlChan->getLastAttempt() < AUTOFIX_INTERVAL) {
       /* do nothing */
     } else {
       isFixed = fixChan(ptr->first, true);
     }

     /**
      * If the channel has been fixed, or the fixing time window
      * has passed, remove it from the list
      */
     if (isFixed || currentTime() - sqlChan->getFixStart() > AUTOFIX_MAXIMUM) { 
       ptr = autoFixQ.erase(ptr);
       sqlChan->addSuccessFix();
       sqlChan->setFixStart(0);
       elog << "chanfix::processQueue> DEBUG: Channel " << sqlChan->getChannel() << " done!" << endl;
     } else {
       ptr->second = currentTime() + AUTOFIX_INTERVAL;
       ptr++;
       elog << "chanfix::processQueue> DEBUG: Channel " << sqlChan->getChannel() << " not done yet ..." << endl;
     }
   } else ptr++;
}

for (fixQueueType::iterator ptr = manFixQ.begin(); ptr != manFixQ.end(); ) {
   elog << "chanfix::processQueue> DEBUG: Processing " << ptr->first->getName() << " in manFixQ ..." << endl;
   if (ptr->second <= currentTime()) {
     sqlChannel* sqlChan = getChannelRecord(ptr->first);
     if (!sqlChan) sqlChan = newChannelRecord(ptr->first);
     bool isFixed = false;

     if (currentTime() - sqlChan->getLastAttempt() >= CHANFIX_INTERVAL) {
       isFixed = fixChan(ptr->first, false);
     }

     /**
      * If the channel has been fixed, or the fixing time window
      * has passed, remove it from the list
      */
     if (isFixed || currentTime() - sqlChan->getFixStart() > CHANFIX_MAXIMUM + CHANFIX_DELAY) {
       /* TODO: send notice to oper saying "Manual chanfix of %s complete." */
       ptr = manFixQ.erase(ptr);
       sqlChan->addSuccessFix();
       sqlChan->setFixStart(0);
       elog << "chanfix::processQueue> DEBUG: Channel " << sqlChan->getChannel() << " done!" << endl;
     } else {
       ptr->second = currentTime() + CHANFIX_INTERVAL;
       ptr++;
       elog << "chanfix::processQueue> DEBUG: Channel " << sqlChan->getChannel() << " not done yet ..." << endl;
     }
   } else ptr++;
}

return;
}
char *chanfix::GetSmallTime(time_t tmVar)
{
	static char datetimestring[24];
	struct tm *stm;
	stm = localtime(&tmVar);
	strftime(datetimestring, 24, "%Y-%m-%d %H:%M:%S", stm);
	return datetimestring;
}

bool chanfix::isBeingFixed(Channel* theChan)
{
return (isBeingAutoFixed(theChan) || isBeingChanFixed(theChan));
}

bool chanfix::isBeingAutoFixed(Channel* theChan)
{
for (fixQueueType::iterator ptr = autoFixQ.begin(); ptr != autoFixQ.end(); ptr++) {
  if(ptr->first->getName() == theChan->getName()) return true;
}

return false;
}

bool chanfix::isBeingChanFixed(Channel* theChan)
{
for (fixQueueType::iterator ptr = manFixQ.begin(); ptr != manFixQ.end(); ptr++) {
  if(ptr->first->getName() == theChan->getName()) return true;
}

return false;
}

bool chanfix::removeFromAutoQ(Channel* theChan)
{
for (fixQueueType::iterator ptr = autoFixQ.begin(); ptr != autoFixQ.end(); ptr++) {
  if(ptr->first->getName() == theChan->getName()) {
    ptr = autoFixQ.erase(ptr);
    return true;
  }
}

return false;
}

bool chanfix::removeFromManQ(Channel* theChan)
{
for (fixQueueType::iterator ptr = manFixQ.begin(); ptr != manFixQ.end(); ptr++) {
  if(ptr->first->getName() == theChan->getName()) {
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

return string( tmpBuf ) ;
}

chanfix::clientOpsType* chanfix::findMyOps(iClient* theClient)
{
clientOpsType* myOps = static_cast< clientOpsType* >(theClient->getCustomData(this) );

if(!myOps) myOps = new clientOpsType;

return myOps;
}
void chanfix::startTimers()
{
    /*
     * Start timers
     */
    time_t theTime = time(NULL) + CHECK_CHANS_TIME;
    tidCheckDB = MyUplink->RegisterTimer(time(NULL) + connectCheckFreq, this, NULL);
    tidAutoFix = MyUplink->RegisterTimer(theTime, this, NULL);
    theTime = time(NULL) + SQL_UPDATE_TIME;
    tidUpdateDB = MyUplink->RegisterTimer(theTime, this, NULL);
    theTime = time(NULL) + PROCESS_QUEUE_TIME;
    tidFixQ = MyUplink->RegisterTimer(theTime, this, NULL);
    theTime = time(NULL) + POINTS_UPDATE_TIME;
    tidGivePoints = MyUplink->RegisterTimer(theTime, this, NULL);
    elog    << "chanfix::startTimers::> Started all timers."
            << endl;
}
void chanfix::giveAllOpsPoints()
{
     Channel* thisChan;
     bool hasService = false;
     for (xNetwork::channelIterator ptr = Network->channels_begin(); ptr != Network->channels_end(); ptr++) {
         thisChan = ptr->second;
         if (thisChan->size() >= minClients && !isBeingFixed(thisChan)) {
            for (Channel::userIterator ptr = thisChan->userList_begin(); ptr != thisChan->userList_end(); ptr++) {
                ChannelUser* curUser = ptr->second;
                if (curUser->getClient()->getMode(iClient::MODE_SERVICES)) {
                   hasService = true;
                   break; //Exit the loop fully, and go to the next chan
                }
            }
            if (hasService == false) {
               for (Channel::userIterator ptr = thisChan->userList_begin(); ptr != thisChan->userList_end(); ptr++) {
                  ChannelUser* curUser = ptr->second;
                  if (curUser->isModeO() && curUser->getClient()->getAccount() != "") {
                     //Ok hes an op
                     //Grab an iClient for curUser
                     givePoints(curUser->getClient(), thisChan);
                  }
               }
            }
         }
     }           
                   
} //giveAllOpsPoints
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
  string Query = "host=" + sqlHost + " dbname=" + sqlDb + " port=" + sqlPort;
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

for(glineIterator ptr = rnGlineList.begin();ptr != 
rnGlineList.end();++ptr)
        {
        (ptr->second)->setSqldb(_SQLDb);
        }

for(exceptionIterator ptr = exception_begin();ptr != 
exception_end();++ptr)
        {
        (*ptr)->setSqldb(_SQLDb);
        }

for(usersIterator ptr = usersMap.begin();ptr != usersMap.end();++ptr)
        {
        ptr->second->setSqldb(_SQLDb);
        }

for(serversIterator ptr = serversMap.begin();ptr != 
serversMap.end();++ptr)
        {
        ptr->second->setSqldb(_SQLDb);
        }
}*/

void Command::Usage( iClient* theClient )
{
bot->Notice( theClient, string( "SYNTAX: " ) + getInfo() ) ;
}

} // namespace gnuworld
