/**
 * Author: Matthias Crauwels <ultimate_@wol.be>
 *
 * Class to keep channel-op points
 *
 */

#include        <sstream>
#include        <string>
#include        <iostream>
#include        <cstring>

#include	"libpq++.h"

#include        "ELog.h"
#include        "misc.h"
#include        "sqlChanOp.h"
#include	"chanfix.h"

namespace gnuworld
{

using std::string ;
using std::endl ;
using std::ends ;
using std::stringstream ;


sqlChanOp::sqlChanOp(PgDatabase* _SQLDb)
: userHost(""),
  channel(""),
  nickUserHost(""),
  account(""),
  points(0),
  ts_firstopped(0),
  ts_lastopped(0),
  SQLDb(_SQLDb)
{};

void sqlChanOp::setAllMembers(int row) 
{
channel = SQLDb->GetValue(row, 0);
userHost = SQLDb->GetValue(row, 1);
nickUserHost = SQLDb->GetValue(row, 2);
points = atoi(SQLDb->GetValue(row, 3));
account = SQLDb->GetValue(row, 4);
ts_firstopped = atoi(SQLDb->GetValue(row, 5));
ts_lastopped = atoi(SQLDb->GetValue(row, 6));
};

bool sqlChanOp::Insert()
{
static const char* queryHeader = "INSERT INTO chanOps (channel, userHost, account, last_seen_as, points, ts_firstopped, ts_lastopped) VALUES (";

stringstream queryString;
queryString     << queryHeader << "'"
		<< escapeSQLChars(channel) << "','"
                << escapeSQLChars(userHost) << "','"
		<< escapeSQLChars(account) << "','"
		<< escapeSQLChars(nickUserHost) << "',"
		<< points << ","
		<< ts_firstopped << ","
		<< ts_lastopped << ")"
                << ends;

#ifdef LOG_SQL
        elog    << "sqlChanOp::Insert> "
                << queryString.str().c_str()
                << endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
        {
        // TODO: Log to msgchan here.
        elog    << "sqlChanOp::Insert> Something went wrong: "
                << SQLDb->ErrorMessage()
                << endl;

        return false;
        }

return true;

};


bool sqlChanOp::commit()
{
static const char* queryHeader =    "UPDATE chanOps ";

elog    << "chanfix::sqlChanOp::commit> " << account << " && " << channel << endl;

stringstream queryString;
queryString	<< queryHeader << "SET last_seen_as = "<< "'"
		<< escapeSQLChars(nickUserHost) << "', points = "
		<< points << ", ts_firstopped = "
		<< ts_firstopped << ", ts_lastopped = "
		<< ts_lastopped << " WHERE lower(channel) = '"
		<< string_lower(escapeSQLChars(channel)) << "' AND lower(account) = '"
		<< string_lower(escapeSQLChars(account)) << "'"
		<< ends;

//#ifdef LOG_SQL
        elog    << "chanfix::sqlChanOp::commit> "
                << queryString.str().c_str()
                << endl;
//#endif
if (points <= 0) {
   return true;
}
ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
        {
        // TODO: Log to msgchan here.
        elog    << "chanfix::sqlChanOp::commit> Something went wrong: "
                << SQLDb->ErrorMessage()
                << endl;

        return false;
        }

return true;

};

sqlChanOp::~sqlChanOp()
{
// No heap space allocated
}

} // namespace gnuworld
