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
: channel(""),
  account(""),
  nickUserHost(""),
  points(0),
  ts_firstopped(0),
  ts_lastopped(0),
  day0(0),
  day1(0),
  day2(0),
  day3(0),
  day4(0),
  day5(0),
  day6(0),
  day7(0),
  day8(0),
  day9(0),
  day10(0),
  day11(0),
  day12(0),
  day13(0),
  SQLDb(_SQLDb)
{};

void sqlChanOp::setAllMembers(int row) 
{
channel = SQLDb->GetValue(row, 0);
account = SQLDb->GetValue(row, 1);
nickUserHost = SQLDb->GetValue(row, 2);
points = atoi(SQLDb->GetValue(row, 3));
ts_firstopped = atoi(SQLDb->GetValue(row, 4));
ts_lastopped = atoi(SQLDb->GetValue(row, 5));
day0 = atoi(SQLDb->GetValue(row, 6));
day1 = atoi(SQLDb->GetValue(row, 7));
day2 = atoi(SQLDb->GetValue(row, 8));
day3 = atoi(SQLDb->GetValue(row, 9));
day4 = atoi(SQLDb->GetValue(row, 10));
day5 = atoi(SQLDb->GetValue(row, 11));
day6 = atoi(SQLDb->GetValue(row, 12));
day7 = atoi(SQLDb->GetValue(row, 13));
day8 = atoi(SQLDb->GetValue(row, 14));
day9 = atoi(SQLDb->GetValue(row, 15));
day10 = atoi(SQLDb->GetValue(row, 16));
day11 = atoi(SQLDb->GetValue(row, 17));
day12 = atoi(SQLDb->GetValue(row, 18));
day13 = atoi(SQLDb->GetValue(row, 19));
};

bool sqlChanOp::Insert()
{
static const char* queryHeader = "INSERT INTO chanOps (channel, account, last_seen_as, points, ts_firstopped, ts_lastopped, day0, day1, day2, day3, day4, day5, day6, day7, day8, day9, day10, day11, day12, day13) VALUES (";

stringstream queryString;
queryString     << queryHeader << "'"
		<< escapeSQLChars(channel) << "','"
		<< escapeSQLChars(account) << "','"
		<< escapeSQLChars(nickUserHost) << "',"
		<< points << ","
		<< ts_firstopped << ","
		<< ts_lastopped << ","
		<< day0 << ","
		<< day1 << ","
		<< day2 << ","
		<< day3 << ","
		<< day4 << ","
		<< day5 << ","
		<< day6 << ","
		<< day7 << ","
		<< day8 << ","
		<< day9 << ","
		<< day10 << ","
		<< day11 << ","
		<< day12 << ","
		<< day13 << ")"
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
		<< escapeSQLChars(nickUserHost)
		<< "', points = " << points
		<< ", ts_firstopped = " << ts_firstopped
		<< ", ts_lastopped = " << ts_lastopped
		<< ", day0 = " << day0
		<< ", day1 = " << day1
		<< ", day2 = " << day2
		<< ", day3 = " << day3
		<< ", day4 = " << day4
		<< ", day5 = " << day5
		<< ", day6 = " << day6
		<< ", day7 = " << day7
		<< ", day8 = " << day8
		<< ", day9 = " << day9
		<< ", day10 = " << day10
		<< ", day11 = " << day11
		<< ", day12 = " << day12
		<< ", day13 = " << day13
		<< " WHERE lower(channel) = '"
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
