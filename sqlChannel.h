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
#define __SQLCHANNEL_H "$Id"

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

        /*
         *  Methods to get data atrributes.
         */

        inline const string&	getChannel() const
                { return channel ; }

        inline unsigned int	getSuccessFixes() const
                { return successFixes ; }

        inline unsigned int     getFixAttempts() const
                { return fixAttempts ; }

	inline unsigned int	getMaxScore() const
		{ return maxScore ; }

        /*
         *  Methods to set data atrributes.
         */

        inline void 	setChannel(string _channel)
                { channel = _channel ; }

        inline void     setSuccessFixes (unsigned int _successFixes)
                { successFixes = _successFixes ; }

        inline void     setFixAttempts (unsigned int _fixAttempts)
                { fixAttempts = _fixAttempts ; }

        inline void     addFixAttempt()
                { fixAttempts++ ; }

	inline void	addSuccesFix()
		{ successFixes++ ; }

	inline void	setMaxScore(unsigned int _maxScore)
		{ maxScore = _maxScore ; }

	bool Insert();
	bool Update();
        void setAllMembers(int);

protected:

	string		channel;
	unsigned int	fixAttempts;
	unsigned int	successFixes;

        unsigned int    maxScore;
        PgDatabase*     SQLDb;


}; // class sqlChannel

} // namespace gnuworld

#endif // __SQLCHANNEL_H
