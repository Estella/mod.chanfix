/**
 * chanfix.h
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

#ifndef __CHANFIX_H
#define __CHANFIX_H "$Id$"

#include	<string>
#include	<vector>
#include	<map>
#include	<list>

#include	"client.h"
#include	"EConfig.h"
#include	"ELog.h"

#include	"chanfixCommands.h"
#include	"chanfix_config.h"
#include	"sqlChannel.h"
#include	"Timer.h"

/* This must be declared before sqlChanOp.h is #include'd */
namespace gnuworld
{
extern short currentDay;
}
#include	"sqlChanOp.h"
#include	"sqlUser.h"

class PgDatabase;
class Timer;
namespace gnuworld
{

/**
 * Case insensitive comparison struct for use by STL structures/algorithms.
 */
struct noCaseComparePair
{
inline bool operator()( const std::pair<std::string, std::string>& lhs, const std::pair<std::string, std::string>& rhs) const
{
	elog << "gnuworld::noCaseComparePair> DEBUG:"
		<< "[lfirst=" << lhs.first
		<< " lsecond=" << lhs.second
		<< "] [rfirst=" << rhs.first
		<< " rsecond=" << rhs.second << "]"
		<< std::endl;
	if (!strcasecmp(lhs.first, rhs.first) && !strcasecmp(lhs.second, rhs.second)) {
		elog << "gnuworld::noCaseComparePair> DEBUG: This is a match!" << std::endl;
		return true;
	} else {
		elog << "gnuworld::noCaseComparePair> DEBUG: This is not a match!" << std::endl;
		return false;
	}
}
} ;


class cmDatabase : public PgDatabase
{
public:
	cmDatabase(const std::string& conninfo)
	 : PgDatabase(conninfo.c_str()) {}
	virtual ~cmDatabase() {}

};

class chanfix : public xClient {

public:

	/**
	 * Current network state.
	 */
	enum STATE {
		BURST,
		RUN,
		SPLIT,
		INIT
	};

	/**
	 * Constructor receives a configuration file name.
	 */
	chanfix( const std::string& ) ;

	/**
	 * Destructor does normal stuff.
	 */
	virtual ~chanfix() ;

	virtual void OnTimer(const gnuworld::xServer::timerID&, void*) ;

	/**
	 * This method is called when a network client sends
	 * a private message (PRIVMSG or NOTICE) to this xClient.
	 * The first argument is a pointer to the source client,
	 * and the second argument is the actual message (minus
	 * all of the server command stuff).
	 */
	virtual void OnPrivateMessage( iClient*, const std::string&,
		bool secure = false ) ;

	/**
	 * This method is called by the server when a server connection
	 * is established.  The purpose of this method is to inform
	 * the xServer of the channels this client wishes to burst.
	 */
	virtual void BurstChannels() ;

	/**
	 * This method is invoked when this module is first loaded.
	 * This is a good place to setup timers, connect to DB, etc.
	 * At this point, the server may not yet be connected to the
	 * network, so please do not issue join/nick requests.
	 */
	virtual void OnAttach() ;

	/**
	 * This method is called when this module is being unloaded from
	 * the server.  This is a good place to cleanup, including
	 * deallocating timers, closing connections, closing log files,
	 * and deallocating private data stored in iClients.
	 */
	virtual void OnDetach( const std::string& =
			std::string( "Shutting down" ) ) ;

	/**
	 * This method is called when the server connects to the network.
	 * Note that if this module is attached while already connected
	 * to a network, this method is still invoked.
	 */
	virtual void OnConnect() ;

	/**
	 * This method is invoked when the server disconnects from
	 * its uplink.
	 */
	virtual void OnDisconnect() ;

	/**
	 * This method will register a given command handler, removing
	 * (and deallocating) the existing handler for this command,
	 * should one exist.
	 */
	virtual bool RegisterCommand( Command* ) ;

	/**
	 * This method will unregister the command handler for the command
	 * of the given command name, deallocating the object from the
	 * heap as well.
	 */
	virtual bool UnRegisterCommand( const std::string& ) ;

	/**
	 * This method is invoked each time a channel event occurs
	 * for one of the channels for which this client has registered
	 * to receive channel events.
	 */
	virtual void	OnChannelEvent( const channelEventType&, Channel*,
		void* data1 = 0, void* data2 = 0,
		void* data3 = 0, void* data4 = 0 ) ;

	/**
	 * This method is invoked when a user sets or removes
	 * one or more channel mode (o).  Keep in mind that the
	 * source ChannelUser may be NULL if a server is
	 * setting the mode.
	 */
	virtual void OnChannelModeO( Channel*, ChannelUser*,
			const xServer::opVectorType& ) ;

	/**
	 * This method is invoked each time a network event occurs.
	 */
	virtual void	OnEvent( const eventType& theEvent,
		void* data1 = 0, void* data2 = 0,
		void* data3 = 0, void* data4 = 0 ) ;

	virtual void OnCTCP( iClient*, const std::string&, const std::string&, bool ) ;

	/**
	 * Our functions.
	 */

	void readConfigFile(const std::string&);

	sqlChanOp* newChanOp(const std::string&, const std::string&);
	sqlChanOp* newChanOp(Channel*, iClient*);

	sqlChanOp* findChanOp(const std::string&, const std::string&);
	sqlChanOp* findChanOp(Channel*, iClient*);
	
	sqlUser* chanfix::isAuthed(const std::string);

	void preloadChanOpsCache();
	void preloadChannelCache();
	void preloadUserCache();

	void changeState(STATE);

	time_t currentTime() { return ::time(0); }
	
	time_t getSecsTilMidnight() { return 86400 - (currentTime() % 86400); }

	void updatePoints();

	void givePoints(Channel*, iClient*);
	void gotOpped(Channel*, iClient*);
	void lostOps(Channel*, iClient*);

	bool wasOpped(Channel*, iClient*);
	bool hasIdent(iClient*);

	void checkNetwork();
	void chanfix::giveAllOpsPoints();

	void autoFix();
	void manualFix(Channel*);

	bool fixChan(sqlChannel*, bool);

	void chanfix::startScoringChan(Channel*);

	sqlChannel* getChannelRecord(const std::string&);
	sqlChannel* getChannelRecord(Channel*);

	sqlChannel* newChannelRecord(const std::string&);
	sqlChannel* newChannelRecord(Channel*);

	bool deleteChannelRecord(sqlChannel*);

	static size_t countChanOps(const Channel*);

	bool needsModesRemoved(Channel*);

	void startTimers();

	void processQueue();
	
	void rotateDB();

	bool isBeingFixed(Channel*);
	bool isBeingAutoFixed(Channel*);
	bool isBeingChanFixed(Channel*);

	bool removeFromAutoQ(Channel*);
	bool removeFromManQ(Channel*);

	char getFlagChar(const sqlUser::flagType&);
	const std::string getFlagsString(const sqlUser::flagType&);
	sqlUser::flagType getFlagType(const char);

	const std::string prettyDuration( int );

	const std::string tsToDateTime(time_t, bool);

	const std::string getHostList( sqlUser* );
	
	const int getCurrentGMTHour(); /* returns the current hour in GMT (00-23) */

	/* Server notices */
	bool serverNotice( Channel*, const char*, ... );
	bool serverNotice( Channel*, const std::string& );

	/*
	 * Send private messages or notices to authenticated users
	 * By default, send notices
	 */
	void SendTo( iClient*, const char*, ... );
	void SendTo( iClient*, const std::string& );

	/* Admin message logs */
	bool logAdminMessage(const char*, ... );

	void doSqlError(const std::string&, const std::string&);
	
	/**
	 * PostgreSQL Database
	 */
	cmDatabase* SQLDb;

	/**
	 * ChannelOp map
	 */
	//typedef map< std::pair<std::string, std::string>, sqlChanOp*, noCaseComparePair> sqlChanOpsType;
	typedef std::map< std::pair<std::string, std::string>, sqlChanOp*> sqlChanOpsType;
	sqlChanOpsType sqlChanOps;
	
	typedef std::map <std::string, sqlChannel*, noCaseCompare> sqlChannelCacheType;
	sqlChannelCacheType sqlChanCache;

	typedef std::map <std::string, Channel*, noCaseCompare> clientOpsType;
	clientOpsType*  findMyOps(iClient*);

	typedef std::list< sqlChanOp* > chanOpsType;
	chanOpsType	getMyOps(Channel*);

	typedef std::map <std::string, bool> ScoredOpsMapType;
	ScoredOpsMapType scoredOpsList;
	
	/**
	 * The db clients map
	 */
	typedef std::map <std::string ,sqlUser* ,noCaseCompare> usersMapType;

	/**
	 * Holds the authenticated user list
	 */
	usersMapType	usersMap;
	
	typedef usersMapType::iterator	usersIterator;

	usersIterator		usersMap_begin()
		{ return usersMap.begin(); }

	usersIterator		usersMap_end()
		{ return usersMap.end(); }

	/**
	 * Queues to process.
	 */
	typedef std::list< std::pair<Channel*, time_t> > fixQueueType;
	fixQueueType	autoFixQ;
	fixQueueType	manFixQ;

	typedef std::vector< iClient* > acctListType; //For reopping all logged in users to an acct.
	acctListType findAccount(Channel*, const std::string&);

	std::string	consoleChan;
	std::string	operChan;
	std::string	supportChan;

protected:
	/**
	 * Commands map
	 */
	typedef std::map< std::string, Command*, noCaseCompare> commandMapType;
	commandMapType commandMap;

	/**
	 *  Time of the last cache
	 */
	std::map < std::string , time_t > lastUpdated;

	/**
	 * Configuration file.
	 */
	EConfig*	chanfixConfig;

	/**
	 * Configuration variables
	 */
	std::string		consoleChanModes;
	std::string		operChanModes;
	std::string		supportChanModes;
	bool		enableAutoFix;
	bool		enableChanFix;
	bool		enableChannelBlocking;
	unsigned int	version;
	unsigned int	numServers;
	unsigned int	minServersPresent;
	unsigned int	numTopScores;
	unsigned int	minClients;
	bool		clientNeedsIdent;
	bool		clientNeedsReverse;
	unsigned int	connectCheckFreq;
	std::string	sqlHost;
	std::string	sqlPort;
	std::string	sqlUsername;
	std::string	sqlPass;
	std::string	sqlDB;
	

	/**
	 * State variable
	 */
	STATE		currentState;

	/**
	 * Timer declarations
	 */
	xServer::timerID tidCheckOps;
	xServer::timerID tidAutoFix;
	xServer::timerID tidUpdateDB;
	xServer::timerID tidFixQ;
	xServer::timerID tidCheckDB;
	xServer::timerID tidGivePoints;
	xServer::timerID tidRotateDB;

	/**
	 * Internal timer
	 */
	Timer *theTimer;

public:

	/*
	 *  Methods to get data attributes.
	 */
	bool doAutoFix() { return enableAutoFix; }
	bool doChanFix() { return enableChanFix; }
	bool doChanBlocking() { return enableChannelBlocking; }
	STATE getState() { return currentState; }
	unsigned int getNumServers() { return numServers; }
	unsigned int getMinServersPresent() { return minServersPresent; }
	unsigned int getNumTopScores() { return numTopScores; }
	unsigned int getMinClients() { return minClients; }
	short getCurrentDay() { return currentDay; }

	/*
	 *  Methods to set data attributes.
	 */
	inline void	setNumServers(int _numServers)
		{ numServers = _numServers; }
	inline void	setDoAutoFix(bool _enableAutoFix)
		{ enableAutoFix = _enableAutoFix; }
	inline void	setDoChanFix(bool _enableChanFix)
		{ enableChanFix = _enableChanFix; }
	inline void	setDoChanBlocking(bool _enableChannelBlocking)
		{ enableChannelBlocking = _enableChannelBlocking; }
	inline void	setCurrentDay()
		{ currentDay = currentTime() / 86400 % DAYSAMPLES; }


}; // class chanfix

const std::string escapeSQLChars(const std::string&);
bool atob(std::string);

} // namespace gnuworld

#endif // __CHANFIX_H
