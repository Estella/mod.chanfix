/**
 * sqlUser.cc
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
#include	"sqlUser.h"

namespace gnuworld
{

sqlUser::sqlUser(PgDatabase* _SQLDb)
: id(0),
  user_name(),
  created(0),
  last_seen(0),
  last_updated(0),
  last_updated_by(),
  flags(0),
  isSuspended(false),
  useNotice(true),
  SQLDb(_SQLDb)
{};

void sqlUser::setAllMembers(int row) 
{
  id = atoi(SQLDb->GetValue(row, 0));
  user_name = SQLDb->GetValue(row, 1);
  created = atoi(SQLDb->GetValue(row, 2));
  last_seen = atoi(SQLDb->GetValue(row, 3));
  last_updated = atoi(SQLDb->GetValue(row, 4));
  last_updated_by = SQLDb->GetValue(row, 5);
  flags = atoi(SQLDb->GetValue(row, 6));
  isSuspended = atob(SQLDb->GetValue(row, 7));
  useNotice = atob(SQLDb->GetValue(row, 8));
};

bool sqlUser::commit()
{

/* Special case if we have no SQLDb
 * ie if we are a fake user
 */
if( !SQLDb ) { return true; }

std::stringstream queryString;
queryString	<< "UPDATE users SET "
		<< "last_seen = " << last_seen << ", "
		<< "last_updated_by = '" << last_updated_by << "', "
		<< "last_updated = " << last_updated << ", "
		<< "flags = " << flags << ", "
		<< "issuspended = " << (isSuspended ? "TRUE" : "FALSE") << ", "
		<< "usenotice = " << (useNotice ? "TRUE" : "FALSE")
		<< " WHERE "
		<< "user_name = '" << user_name << "'"
		;

#ifdef LOG_SQL
elog	<< "sqlUser::commit> "
	<< queryString
	<< endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str());

if(PGRES_COMMAND_OK != status) {
	elog << "sqlUser::commit> " << SQLDb->ErrorMessage();
	return false;
}

return true;

}

bool sqlUser::Insert()
{
std::stringstream insertString;
insertString	<< "INSERT INTO users "
		<< "(user_name, created, last_seen, last_updated, last_updated_by, flags, issuspended, usenotice) "
		<< "VALUES "
		<< "("
		<< "'" << user_name << "', "
		<< created << ", "
		<< last_seen << ", "
		<< last_updated << ", "
		<< "'" << last_updated_by << "', "
		<< flags << ", "
		<< (isSuspended ? "TRUE" : "FALSE") << ", "
		<< (useNotice ? "TRUE" : "FALSE")
		<< ")"
		;

#ifdef LOG_SQL
elog	<< "sqlUser::Insert> "
	<< insertString
	<< endl;
#endif

ExecStatusType status = SQLDb->Exec(insertString.str().c_str());

if(PGRES_COMMAND_OK != status) {
	elog << "sqlUser::Insert> " << SQLDb->ErrorMessage();
	return false;
}

return true;
} // sqlUser::Insert()

bool sqlUser::Delete()
{
std::stringstream deleteString;
deleteString	<< "DELETE FROM users "
		<< "WHERE user_name = '" << user_name << "'"
		;

ExecStatusType status = SQLDb->Exec(deleteString.str().c_str());

if(PGRES_COMMAND_OK != status) {
	elog << "sqlUser::Delete> " << SQLDb->ErrorMessage();
	return false;
}

std::stringstream hostString;
hostString	<< "DELETE FROM hosts "
		<< "WHERE user_id = " << id;

status = SQLDb->Exec(hostString.str().c_str());

if(PGRES_COMMAND_OK != status) {
	elog << "sqlUser::Delete (hosts)> " << SQLDb->ErrorMessage();
	return false;
}

return true;

}

sqlUser::~sqlUser()
{
// No heap space allocated
}

} //namespace gnuworld
