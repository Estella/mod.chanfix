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
#include	"sqlChanOp.h"
#include	"Timer.h"

using std::string;

class PgDatabase;
class Timer;

namespace gnuworld
{

using std::string ;
using std::vector ;
using std::map ;
using std::list ;

enum CHANFIX_STATE {
	BURST,
	RUN,
	SPLIT,
	INIT
};

/**
 * Case insensitive comparison struct for use by STL structures/algorithms.
 */
struct noCaseComparePair
{
inline bool operator()( const std::pair<string, string>& lhs, const std::pair<string, string>& rhs) const
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
	cmDatabase(const string& conninfo)
	 : PgDatabase(conninfo.c_str()) {}
	virtual ~cmDatabase() {}

};

class chanfix : public xClient {

public:
	/**
	 * Constructor receives a configuration file name.
	 */
	chanfix( const string& ) ;

	/**
	 * Destructor does normal stuff.
	 */
	virtual ~chanfix() ;

	virtual void OnTimer(xServer::timerID, void*);

	/**
	 * This method is called when a network client sends
	 * a private message (PRIVMSG or NOTICE) to this xClient.
	 * The first argument is a pointer to the source client,
	 * and the second argument is the actual message (minus
	 * all of the server command stuff).
	 */
	virtual void OnPrivateMessage( iClient*, const string&,
		bool secure = false ) ;

	/**
	 * This method is called by the server when a server connection
	 * is established.  The purpose of this method is to inform
	 * the xServer of the channels this client wishes to burst.
	 */
	virtual bool BurstChannels() ;

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
        virtual void OnDetach( const string& =
			string( "Shutting down" ) ) ;

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
	virtual bool UnRegisterCommand( const string& ) ;

        /**
         * This method is invoked each time a channel event occurs
         * for one of the channels for which this client has registered
         * to receive channel events.
         */
        virtual void    OnChannelEvent( const channelEventType&, Channel*,
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
        virtual void    OnEvent( const eventType& theEvent,
                void* data1 = 0, void* data2 = 0,
                void* data3 = 0, void* data4 = 0 ) ;

        virtual void OnCTCP( iClient*, const string&, const string&, bool ) ;

	/**
	 * Our functions.
	 */

	sqlChanOp* newChanOp(const string&, const string&);
	sqlChanOp* newChanOp(iClient*, Channel*);

	sqlChanOp* findChanOp(const string&, const string&);
	sqlChanOp* findChanOp(iClient*, Channel*);

	void preloadChanOpsCache();
	void preloadChannelCache();

	void changeState(CHANFIX_STATE);

	time_t currentTime() { return ::time(0); }

	void burstOps();

	void givePoints(sqlChanOp*);

	void gotOpped(iClient*, Channel*);

	sqlChanOp* wasOpped(iClient*, Channel*);
        sqlChanOp* wasOpped(sqlChanOp*);

	void checkNetwork();

	void autoFix();
	void manualFix(Channel*);

	bool fixChan(Channel*, bool);

	iClient* findAccount(const string&, Channel*);

	sqlChannel* getChannelRecord(const string&);
	sqlChannel* getChannelRecord(Channel*);

	sqlChannel* newChannelRecord(const string&);
	sqlChannel* newChannelRecord(Channel*);

	static size_t countChanOps(const Channel*);

	void processQueue();

	bool isBeingFixed(Channel*);
	bool isBeingAutoFixed(Channel*);
	bool isBeingChanFixed(Channel*);

	bool removeFromAutoQ(Channel*);
	bool removeFromManQ(Channel*);

	const string prettyDuration( int );
	
	/**
	 * PostgreSQL Database
	 */
	cmDatabase* SQLDb;

	/**
	 * ChannelOp map
	 */
//        typedef map< std::pair<string, string>, sqlChanOp*, noCaseComparePair> sqlChanOpsType;
        typedef map< std::pair<string, string>, sqlChanOp*> sqlChanOpsType;
	sqlChanOpsType sqlChanOps;

	typedef map <string, sqlChannel*, noCaseCompare> sqlChannelCacheType;
	sqlChannelCacheType sqlChanCache;

	typedef list< sqlChanOp* > chanOpsType;
	chanOpsType opList;	

	chanOpsType	getMyOps(Channel*);

	/**
	 * Queues to process.
	 */
	typedef list< std::pair<Channel*, time_t> > fixQueueType;
	fixQueueType	autoFixQ;
	fixQueueType	manFixQ;

	typedef list< std::pair <Channel*, iClient*> > opQueueType;
	opQueueType	opQ;

	string          consoleChan;
	string          operChan;
	string          supportChan;

protected:
	/**
	 * Commands map
	 */
	typedef map< string, Command*, noCaseCompare> commandMapType;
	commandMapType commandMap;

	/**
	 * Configuration file.
	 */
	EConfig*	chanfixConfig;

	/**
	 * Configuration variables
	 */
	string		consoleChanModes;
	string		operChanModes;
	string		supportChanModes;
	bool		enableAutoFix;
	bool		enableChanFix;
	bool		enableChannelBlocking;
	string		defaultChannelModes;
	unsigned int	numServers;
	unsigned int	minServersPresent;
	unsigned int	numTopScores;
	unsigned int	minClients;
	bool		clientNeedsIdent;
	bool		clientNeedsReverse;
	string          sqlHost;
	string          sqlPort;
	string          sqlUser;
	string          sqlPass;
	string          sqlDB;

	/**
	 * State variable
	 */
	CHANFIX_STATE	currentState;

	/**
	 * Timer declarations
	 */
	xServer::timerID tidCheckOps;
	xServer::timerID tidAutoFix;
	xServer::timerID tidUpdateDB;
	xServer::timerID tidFixQ;

	/**
	 * Internal timer
	 */
	Timer *theTimer;

public:
	bool doAutoFix() { return enableAutoFix; }
	bool doChanFix() { return enableChanFix; }
	bool doChanBlocking() { return enableChannelBlocking; }
	CHANFIX_STATE getState() { return currentState; }
	unsigned int getNumServers() { return numServers; }
	unsigned int getMinServersPresent() { return minServersPresent; }
	unsigned int getNumTopScores() { return numTopScores; }
	unsigned int getMinClients() { return minClients; }

}; // class chanfix

const string escapeSQLChars(const string& theString);

} // namespace gnuworld

#endif // __CHANFIX_H
