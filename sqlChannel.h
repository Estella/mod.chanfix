/**
 * sqlChannel.h
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 * $Id$
 */

#ifndef __SQLCHANNEL_H
#define __SQLCHANNEL_H "$Id$"

#include        <string>
#include        <vector>
#include        <ctime>
#include        "libpq++.h"

namespace gnuworld
{

using std::string;
using std::vector;

class iClient;

class sqlChannel
{
public:

	sqlChannel(PgDatabase*);
	virtual ~sqlChannel();

	typedef unsigned int	flagType ;

	static const flagType	F_BLOCKED;
	static const flagType	F_ALERT;

        /*
         *  Methods to get data atrributes.
         */

        inline const std::string&	getChannel() const
                { return channel ; }

	inline const flagType&	getFlags() const
		{ return flags ; }

	inline bool	getFlag( const flagType& whichFlag ) const
		{ return (flags & whichFlag) ; }

        inline unsigned int	getSuccessFixes() const
                { return successFixes ; }

        inline time_t		getLastAttempt() const
                { return last ; }

        inline time_t		getFixStart() const
                { return start ; }

	inline unsigned int	getMaxScore() const
		{ return maxScore ; }

	inline bool		getModesRemoved() const
		{ return modesRemoved ; }

        /*
         *  Methods to set data atrributes.
         */

        inline void 	setChannel(string _channel)
                { channel = _channel; }

	inline void	setFlag( const flagType& whichFlag )
		{ flags |= whichFlag; }

	inline void	removeFlag( const flagType& whichFlag )
		{ flags &= ~whichFlag; }

	inline void	clearFlags()
		{ flags = 0; }

        inline void     setSuccessFixes (unsigned int _successFixes)
                { successFixes = _successFixes; }

        inline void     setLastAttempt (time_t _last)
                { last = _last; }

	inline void	setFixStart(time_t _start)
                { start = _start; }

	inline void	addSuccessFix()
		{ successFixes++; }

	inline void	setMaxScore(unsigned int _maxScore)
		{ maxScore = _maxScore; }

	inline void	setModesRemoved(bool _modesRemoved)
		{ modesRemoved = _modesRemoved; }

	bool Insert();
	bool Update();
        void setAllMembers(int);

protected:

	string		channel;
	time_t		last;
	time_t		start;
	unsigned int	successFixes;
        unsigned int    maxScore;
	bool		modesRemoved;
	flagType	flags;
        PgDatabase*     SQLDb;


}; // class sqlChannel

} // namespace gnuworld

#endif // __SQLCHANNEL_H
