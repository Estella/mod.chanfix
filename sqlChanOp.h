/**
 * Author: Matthias Crauwels <ultimate_@wol.be>
 *
 * Class to keep channel-op points
 *
 */

#ifndef __SQLCHANOP_H
#define __SQLCHANOP_H "$Id$"

#include	<string>
#include	"libpq++.h"
#include	"chanfix_config.h"

namespace gnuworld
{

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

	inline const short&	getDay(int _dayval) const
		{ return day[_dayval] ; }

	/*
	 *  Methods to set data attributes.
	 */

	inline void	setChannel(std::string _channel)
		{ channel = _channel ; }

	inline void	setAccount(std::string _account)
		{ account = _account ; }

	inline void	setPoints(short _points)
		{ day[currentDay] = _points; calcTotalPoints(); }

	inline void	addPoint()
		{ day[currentDay]++; calcTotalPoints(); }

	inline void	setLastSeenAs(std::string _nickUserHost)
		{ nickUserHost = _nickUserHost ; }

	inline void     setTimeFirstOpped(time_t _ts_firstopped)
		{ ts_firstopped = _ts_firstopped ; }

	inline void	setTimeLastOpped(time_t _ts_lastopped)
		{ ts_lastopped = _ts_lastopped ; }
		
	inline void	setDay(int _dayval, short _pointsval)
		{ day[_dayval] = _pointsval ; }

	bool Insert();
	bool Delete();
	bool commit();
	void setAllMembers(int);
	void calcTotalPoints();

private:

	std::string	channel;
	std::string	account;
	std::string	nickUserHost;
	int		points;
	time_t		ts_firstopped;
	time_t		ts_lastopped;
	short		day[DAYSAMPLES];
	PgDatabase*	SQLDb;

}; // class

} // namespace gnuworld


#endif // __SQLCHANOP_H
