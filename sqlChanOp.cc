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
  SQLDb(_SQLDb)
{};

void sqlChanOp::setAllMembers(int row) 
{
channel = SQLDb->GetValue(row, 0);
userHost = SQLDb->GetValue(row, 1);
nickUserHost = SQLDb->GetValue(row, 2);
points = atoi(SQLDb->GetValue(row, 3));
account = SQLDb->GetValue(row, 4);
};

bool sqlChanOp::Insert()
{
static const char* queryHeader = "INSERT INTO chanOps (channel, userHost, account, last_seen_as, points) VALUES (";

stringstream queryString;
queryString     << queryHeader << "'"
		<< channel << "','"
                << userHost << "','"
		<< account << "','"
		<< nickUserHost << "',"
		<< points << ")"
                << ends;

//#ifdef LOG_SQL
        elog    << "sqlChanOp::Insert> "
                << queryString.str().c_str()
                << endl;
//#endif

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


bool sqlChanOp::Update()
{
static const char* queryHeader =    "UPDATE chanOps ";

stringstream queryString;
queryString     << queryHeader << "SET last_seen_as = "<< "'"
                << escapeSQLChars(nickUserHost) << "', points = "
                << points << " WHERE lower(channel) = '"
                << string_lower(channel) << "' AND lower(account) = '"
                << string_lower(account) << "'"
                << ends;

//#ifdef LOG_SQL
        elog    << "sqlChanOp::update> "
                << queryString.str().c_str()
                << endl;
//#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
        {
        // TODO: Log to msgchan here.
        elog    << "sqlChanOp::update> Something went wrong: "
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
