/**
 * Author: Matthias Crauwels <ultimate_@wol.be>
 *
 */

#include        <sstream>
#include        <string>
#include        <iostream>

#include        <cstring>

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


sqlChannel::sqlChannel(PgDatabase* _SQLDb)
: channel(""),
  fixAttempts(0),
  successFixes(0),
  maxScore(0),
  SQLDb(_SQLDb)
{};

void sqlChannel::setAllMembers(int row) 
{
channel = SQLDb->GetValue(row, 0);
successFixes = atoi(SQLDb->GetValue(row, 1));
};

bool sqlChannel::Insert()
{
static const char* queryHeader = "INSERT INTO channels (channel, fixed) VALUES (";

stringstream queryString;
queryString     << queryHeader << "'"
		<< channel << "',"
		<< successFixes << ")"
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


bool sqlChannel::Update()
{
static const char* queryHeader =    "UPDATE channels ";

stringstream queryString;
queryString     << queryHeader << "SET fixed = "
                << successFixes << " WHERE channel = '"
                << channel << "'"
                << ends;

//#ifdef LOG_SQL
        elog    << "sqlChannel::update> "
                << queryString.str().c_str()
                << endl;
//#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
        {
        // TODO: Log to msgchan here.
        elog    << "sqlChannel::update> Something went wrong: "
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
