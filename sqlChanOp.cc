/**
 * sqlChanOp.cc
 *
 * Author: Matthias Crauwels <ultimate_@wol.be>
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

#include	<sstream>
#include	<string>

#include	"libpq++.h"

#include	"ELog.h"
#include	"misc.h"

#include	"chanfix.h"
#include	"sqlChanOp.h"

namespace gnuworld
{

sqlChanOp::sqlChanOp(sqlManager* _myManager) :
  channel(""),
  account(""),
  nickUserHost(""),
  points(0),
  ts_firstopped(0),
  ts_lastopped(0),
  day()
{
  myManager = _myManager;
};
	
void sqlChanOp::setAllMembers(PgDatabase* theDB, int row)
{
  channel = theDB->GetValue(row, 0);
  account = theDB->GetValue(row, 1);
  nickUserHost = theDB->GetValue(row, 2);
  ts_firstopped = atoi(theDB->GetValue(row, 3));
  ts_lastopped = atoi(theDB->GetValue(row, 4));
  day[0] = atoi(theDB->GetValue(row, 5));
  day[1] = atoi(theDB->GetValue(row, 6));
  day[2] = atoi(theDB->GetValue(row, 7));
  day[3] = atoi(theDB->GetValue(row, 8));
  day[4] = atoi(theDB->GetValue(row, 9));
  day[5] = atoi(theDB->GetValue(row, 10));
  day[6] = atoi(theDB->GetValue(row, 11));
  day[7] = atoi(theDB->GetValue(row, 12));
  day[8] = atoi(theDB->GetValue(row, 13));
  day[9] = atoi(theDB->GetValue(row, 14));
  day[10] = atoi(theDB->GetValue(row, 15));
  day[11] = atoi(theDB->GetValue(row, 16));
  day[12] = atoi(theDB->GetValue(row, 17));
  day[13] = atoi(theDB->GetValue(row, 18));
  calcTotalPoints();
};


void sqlChanOp::calcTotalPoints()
{
  int i;
  points = 0;
  for (i = 0; i < DAYSAMPLES; i++) {
    points += day[i];
  }
}

void sqlChanOp::Insert()
{
static const char* insertHeader = "INSERT INTO chanOps (channel, account, last_seen_as, ts_firstopped, ts_lastopped, day0, day1, day2, day3, day4, day5, day6, day7, day8, day9, day10, day11, day12, day13) VALUES (";

std::stringstream insertString;
insertString	<< insertHeader << "'"
		<< escapeSQLChars(channel) << "','"
		<< escapeSQLChars(account) << "','"
		<< escapeSQLChars(nickUserHost) << "',"
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
		;

myManager->queueCommit(insertString.str());
commit();
};

void sqlChanOp::Delete()
{
static const char* queryHeader =    "DELETE FROM chanOps ";

std::stringstream deleteString;
deleteString	<< queryHeader << "WHERE lower(channel) = '"
		<< string_lower(escapeSQLChars(channel))
		<< "' AND lower(account) = '"
		<< string_lower(escapeSQLChars(account)) << "'"
		;
myManager->queueCommit(deleteString.str());
};

void sqlChanOp::commit()
{
elog	<< "chanfix::sqlChanOp::commit> " << account << " && " << channel << std::endl;

std::stringstream chanOpCommit;
chanOpCommit	<< "UPDATE chanOps "
		<< "SET last_seen_as = " << "'"
		<< escapeSQLChars(nickUserHost)
		<< "', ts_firstopped = " << ts_firstopped
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
		<< string_lower(escapeSQLChars(channel))
		<< "' AND lower(account) = '"
		<< string_lower(escapeSQLChars(account)) << "'"
		;
myManager->queueCommit(chanOpCommit.str());
};

sqlChanOp::~sqlChanOp()
{
// No heap space allocated
}

} // namespace gnuworld
