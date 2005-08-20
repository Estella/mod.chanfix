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

const sqlUser::flagType sqlUser::F_SERVERADMIN =	0x01; /* +a */
const sqlUser::flagType sqlUser::F_BLOCK =		0x02; /* +b */
const sqlUser::flagType sqlUser::F_CHANNEL =		0x04; /* +c */
const sqlUser::flagType sqlUser::F_CHANFIX =		0x08; /* +f */
const sqlUser::flagType sqlUser::F_OWNER =		0x10; /* +o */
const sqlUser::flagType sqlUser::F_USERMANAGER =	0x20; /* +u */
const sqlUser::flagType sqlUser::F_LOGGEDIN =		0x40;

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

/*
 *  Load all data for this user from the backend. (Key: userID)
 */

bool sqlUser::loadData(unsigned int userID)
{
/*
 *  With the open database handle 'SQLDb', retrieve information about
 *  'userID' and fill our member variables.
 */

#ifdef LOG_DEBUG
	elog	<< "sqlUser::loadData> Attempting to load data for user-id: "
		<< userID
		<< std::endl;
#endif

std::stringstream queryString;
queryString	<< "SELECT "
		<< "id, user_name, created, last_seen, last_updated, last_updated_by, flags, issuspended, usenotice"
		<< " FROM users WHERE id = "
		<< userID
		<< std::ends;

#ifdef LOG_SQL
	elog	<< "sqlUser::loadData> "
		<< queryString.str().c_str()
		<< std::endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_TUPLES_OK == status )
	{
	/*
	 *  If the user doesn't exist, we won't get any rows back.
	 */

	if(SQLDb->Tuples() < 1)
		{
		return (false);
		}

	setAllMembers(0);

	return (true);
	}

return (false);
}

bool sqlUser::loadData(const std::string& userName)
{
/*
 *  With the open database handle 'SQLDb', retrieve information about
 *  'userID' and fill our member variables.
 */

#ifdef LOG_DEBUG
	elog	<< "sqlUser::loadData> Attempting to load data for user-name: "
		<< userName
		<< std::endl;
#endif

std::stringstream queryString;
queryString	<< "SELECT "
		<< "id, user_name, created, last_seen, last_updated, last_updated_by, flags, issuspended, usenotice"
		<< " FROM users WHERE lower(user_name) = '"
		<< escapeSQLChars(string_lower(userName))
		<< "'"
		<< std::ends;

#ifdef LOG_SQL
	elog	<< "sqlUser::loadData> "
		<< queryString.str().c_str()
		<< std::endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_TUPLES_OK == status )
	{
	/*
	 *  If the user doesn't exist, we won't get any rows back.
	 */

	if(SQLDb->Tuples() < 1)
		{
		return (false);
		}

	setAllMembers(0);

	return (true);
	}

return (false);
}

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
		<< "id = " << id
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
	if (!loadData(user_name))
		return false;
	return true;
}

elog << "sqlUser::Insert> " << SQLDb->ErrorMessage();
return false;
} // sqlUser::Insert()

bool sqlUser::Delete()
{
std::stringstream deleteString;
deleteString	<< "DELETE FROM users "
		<< "WHERE id = '" << id << "'"
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