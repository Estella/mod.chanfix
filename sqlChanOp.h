/**
 * Author: Matthias Crauwels <ultimate_@wol.be>
 *
 * Class to keep channel-op points
 *
 */

#ifndef __SQLCHANOP_H
#define __SQLCHANOP_H "$Id$"

#include        <string>
#include        "libpq++.h"

namespace gnuworld
{

class sqlChanOp {

public:
        sqlChanOp(PgDatabase*) ;
        virtual ~sqlChanOp() ;


        /*
         *  Methods to get data atrributes.
         */

        inline const string&      getChannel() const
                { return channel ; }

        inline const string&      getUserHost() const
                { return userHost ; }

        inline const string&      getAccount() const
                { return account ; }

	inline const unsigned long& getPoints() const
		{ return points ; }

	inline const string&	  getLastSeenAs() const
                { return nickUserHost ; }

        inline const time_t	  getTimeOpped() const
                { return ts_opped ; }

        /*
         *  Methods to set data atrributes.
         */

        inline void setChannel(string _channel)
                { channel = _channel ; }

        inline void setUserHost(string _userHost)
                { userHost = _userHost ; }

        inline void setAccount(string _account)
                { account = _account ; }

        inline void setPoints(unsigned long _points)
                { points = _points ; }

	inline void addPoints(unsigned long _points)
		{ points += _points ; }

        inline void setLastSeenAs(string _nickUserHost)
		{ nickUserHost = _nickUserHost ; }

        inline void setTimeOpped(time_t _ts_opped)
                { ts_opped = _ts_opped ; }

	bool Insert();
	bool Update();
	void setAllMembers(int);

private:

	string		userHost;
	string		channel;
	string		nickUserHost;
	string		account;
	unsigned long	points;
	time_t		ts_opped;

        PgDatabase*     SQLDb;

}; // class

} // namespace gnuworld


#endif // __SQLCHANOP_H

