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
  day(),
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
day[0] = atoi(SQLDb->GetValue(row, 6));
day[1] = atoi(SQLDb->GetValue(row, 7));
day[2] = atoi(SQLDb->GetValue(row, 8));
day[3] = atoi(SQLDb->GetValue(row, 9));
day[4] = atoi(SQLDb->GetValue(row, 10));
day[5] = atoi(SQLDb->GetValue(row, 11));
day[6] = atoi(SQLDb->GetValue(row, 12));
day[7] = atoi(SQLDb->GetValue(row, 13));
day[8] = atoi(SQLDb->GetValue(row, 14));
day[9] = atoi(SQLDb->GetValue(row, 15));
day[10] = atoi(SQLDb->GetValue(row, 16));
day[11] = atoi(SQLDb->GetValue(row, 17));
day[12] = atoi(SQLDb->GetValue(row, 18));
day[13] = atoi(SQLDb->GetValue(row, 19));
};
void sqlChanOp::rotatePointSet()
{
	//Yes this is messy until I can figure out a good method/loop
	points = points - day[13];
	day[13] = day[12];
	day[12] = day[11];
	day[11] = day[10];
	day[10] = day[9];
	day[9] = day[8];
	day[8] = day[7];
	day[7] = day[6];
	day[5] = day[4];
	day[3] = day[2];
	day[1] = day[0];
	day[0] = 0;
	points = day[0] + day[1] + day[2] + day[3] + day[4] + day[5] + day[6] + day[7] + day[8] + day[9] + day[10] + day[11] + day[12] + day[13];
}
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
		<< day[0] << ","
		<< day[1] << ","
		<< day[2] << ","
		<< day[3] << ","
		<< day[4] << ","
		<< day[5] << ","
		<< day[6] << ","
		<< day[7] << ","
		<< day[8] << ","
		<< day[9] << ","
		<< day[10] << ","
		<< day[11] << ","
		<< day[12] << ","
		<< day[13] << ")"
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
		<< ", day0 = " << day[0]
		<< ", day1 = " << day[1]
		<< ", day2 = " << day[2]
		<< ", day3 = " << day[3]
		<< ", day4 = " << day[4]
		<< ", day5 = " << day[5]
		<< ", day6 = " << day[6]
		<< ", day7 = " << day[7]
		<< ", day8 = " << day[8]
		<< ", day9 = " << day[9]
		<< ", day10 = " << day[10]
		<< ", day11 = " << day[11]
		<< ", day12 = " << day[12]
		<< ", day13 = " << day[13]
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
