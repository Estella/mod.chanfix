/*
 * (c) 2003 Matthias Crauwels
 *
 * Special functions 
 */

#include "match.h"
#include "ELog.h"
namespace gnuworld
{

using std::endl;

struct MatchPair
{
inline bool operator()( pair<string, string> lhs, pair<string, string> rhs ) const
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

} //namespace gnuworld
