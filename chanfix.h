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

#ifndef __chanfix_H
#define __chanfix_H "$Id$"

#include	<string>

#include	"client.h"
#include	"sqlChanOp.h"
#include	"functor.h"

using std::string ;
class PgDatabase;

namespace gnuworld
{

class cmDatabase : public PgDatabase
{
public:
        cmDatabase(const string& conninfo)
         : PgDatabase(conninfo.c_str()) {}
        virtual ~cmDatabase() {}

};


/**
 * A dummy xClient.  This is just a skeleton from which developers
 * may create new services clients.
 */
class chanfix : public xClient
{

public:
	/**
	 * Constructor receives a configuration file name.
	 */
	chanfix( const string& ) ;

	/**
	 * Destructor does normal stuff.
	 */
	virtual ~chanfix() ;

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

        virtual void OnAttach() ;

        virtual void OnDetach( const string& =
                        string( "Server Shutdown" ) ) ;

        virtual void OnConnect() ;

        virtual void OnDisconnect() ;

//        virtual bool RegisterCommand( Command* ) ;

        /**
         * This method will unregister the command handler for the command
         * of the given command name, deallocating the object from the
         * heap as well.
         */
//        virtual bool UnRegisterCommand( const string& ) ;

        /**
         * This method is invoked each time a channel event occurs
         * for one of the channels for which this client has registered
         * to receive channel events.
         */
        virtual void    OnChannelEvent( const channelEventType&, Channel*,
                void* data1 = 0, void* data2 = 0,
                void* data3 = 0, void* data4 = 0 ) ;

        /**
         * This method is invoked each time a network event occurs.
         */
        virtual void    OnEvent( const eventType& theEvent,
                void* data1 = 0, void* data2 = 0,
                void* data3 = 0, void* data4 = 0 ) ;

        virtual void OnCTCP( iClient*, const string&, const string&, bool ) ;

	sqlChanOp* findChanOp(const string&, const string&);
	sqlChanOp* findChanOp(iClient*, Channel*);

	void preCacheChanOps();
	void BurstOps();

	void OnChannelModeO( Channel*, ChannelUser*, const xServer::opVectorType&);

	const string getClientUserHost(iClient*);

	void givePoints(iClient*, Channel*);
        void givePoints(sqlChanOp*);

	void gotOpped(iClient*, Channel*);

        /**
         * PostgreSQL Database
         */
        cmDatabase* SQLDb;

	/**
	 * Channel-op map
	 */
        typedef map< pair<string, string>, sqlChanOp*, MatchPair> sqlChanOpsType;
	sqlChanOpsType 	sqlChanOps;


protected:
        string          coderChan;
        string          debugChan;
        string          operChan;
        string          relayChan;
        string          chanModes;

        string          sqlHost;
        string          sqlPort;
        string          sqlUser;
        string          sqlPass;
        string          sqlDb;

} ;

const string escapeSQLChars(const string& theString);

} // namespace gnuworld

#endif // __chanfix_H
