/**
 * chanfix_misc.h
 *
 * 17/12/2003 - Matthias Crauwels <ultimate_@wol.be>
 * Initial Version
 *
 * Miscellaneous functions
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

#ifndef __CHANFIX_MISC_H
#define __CHANFIX_MISC_H "$Id$"

#include "match.h"
#include "ELog.h"

namespace gnuworld
{

using std::endl;

struct MatchPair
{
inline bool operator()( std::pair<string, string> lhs, std::pair<string, string> rhs ) const
	{
	elog << "DEBUG: Match called!!" << endl;
	return (match( lhs.first, rhs.first ) < 0 && match ( lhs.second, rhs.second ) < 0) ;
	}
} ;

bool atob( string str )
{
if(str == "y" || str == "true" || str == "yes") return true;
return false;
}

bool compare_points(sqlChanOp* a, sqlChanOp* b)
        {
        return a->getPoints() > b->getPoints();
        }

} //namespace gnuworld

#endif // __CHANFIX_MISC_H
