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
using std::string ;


class sqlChanOp {

public:
        sqlChanOp(PgDatabase*) ;
        virtual ~sqlChanOp() ;


        /*
         *  Methods to get data attributes.
         */

        inline const std::string&	getChannel() const
                { return channel ; }

        inline const std::string&	getAccount() const
                { return account ; }

	inline const int&	getPoints() const
		{ return points ; }

	inline const std::string&	getLastSeenAs() const
                { return nickUserHost ; }

	inline const time_t	getTimeFirstOpped() const
		{ return ts_firstopped ; }

	inline const time_t	getTimeLastOpped() const
		{ return ts_lastopped ; }

        /*
         *  Methods to set data attributes.
         */

        inline void	setChannel(string _channel)
                { channel = _channel ; }

        inline void	setAccount(string _account)
                { account = _account ; }

        inline void	setPoints(int _points)
                { points = _points ; }

	inline void	addPoint()
		{ points++ ; }

        inline void	setLastSeenAs(string _nickUserHost)
		{ nickUserHost = _nickUserHost ; }

	inline void     setTimeFirstOpped(time_t _ts_firstopped)
		{ ts_firstopped = _ts_firstopped ; }

        inline void	setTimeLastOpped(time_t _ts_lastopped)
                { ts_lastopped = _ts_lastopped ; }

	bool Insert();
	bool commit();
	void setAllMembers(int);

private:

	string		channel;
	string		account;
	string		nickUserHost;
	int		points;
	time_t		ts_firstopped;
	time_t		ts_lastopped;
	short		day0;
	short		day1;
	short		day2;
	short		day3;
	short		day4;
	short		day5;
	short		day6;
	short		day7;
	short		day8;
	short		day9;
	short		day10;
	short		day11;
	short		day12;
	short		day13;
        PgDatabase*     SQLDb;

}; // class

} // namespace gnuworld


#endif // __SQLCHANOP_H
