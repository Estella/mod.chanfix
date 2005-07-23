/**
 * Author: Matthias Crauwels <ultimate_@wol.be>
 *
 */

#include        <sstream>
#include        <string>
#include        <iostream>
#include        <cstring>

#include	"libpq++.h"

#include        "ELog.h"
#include        "misc.h"

#include        "sqlChannel.h"
#include	"chanfix.h"

namespace gnuworld
{

using std::string ;
using std::endl ;
using std::ends ;
using std::stringstream ;

const sqlChannel::flagType sqlChannel::F_BLOCKED	= 0x00000001 ;
const sqlChannel::flagType sqlChannel::F_ALERT		= 0x00000002 ;

sqlChannel::sqlChannel(PgDatabase* _SQLDb)
: channel(""),
  last(0),
  start(0),
  maxScore(0),
  modesRemoved(false),
  flags(0),
  SQLDb(_SQLDb)
{};

void sqlChannel::setAllMembers(int row) 
{
channel = SQLDb->GetValue(row, 0);
flags = atoi(SQLDb->GetValue(row, 1));
};

bool sqlChannel::Insert()
{
static const char* queryHeader = "INSERT INTO channels (channel, flags) VALUES (";

stringstream queryString;
queryString     << queryHeader << "'"
		<< escapeSQLChars(channel) << "',"
		<< flags << ")"
                << ends;

//#ifdef LOG_SQL
        elog    << "sqlChannel::Insert> "
                << queryString.str().c_str()
                << endl;
//#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
        {
        // TODO: Log to msgchan here.
        elog    << "sqlChannel::Insert> Something went wrong: "
                << SQLDb->ErrorMessage()
                << endl;

        return false;
        }

return true;

};


bool sqlChannel::commit()
{
static const char* queryHeader =    "UPDATE channels ";

stringstream queryString;
queryString     << queryHeader << "SET flags = "
		<< flags << " WHERE channel = '"
                << escapeSQLChars(channel) << "'"
                << ends;

//#ifdef LOG_SQL
        elog    << "chanfix::sqlChannel::commit> "
                << queryString.str().c_str()
                << endl;
//#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
        {
        // TODO: Log to msgchan here.
        elog    << "chanfix::sqlChannel::commit> Something went wrong: "
                << SQLDb->ErrorMessage()
                << endl;

        return false;
        }

return true;

};

sqlChannel::~sqlChannel()
{
// No heap space allocated
}

} // namespace gnuworld
