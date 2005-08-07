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
 * $Id: sqlUser.cc 1188 2005-07-30 02:29:02Z Compster $
 */

#include	<sstream>
#include	<string>
#include	<iostream>
#include	<cstring>

#include	"libpq++.h"

#include	"ELog.h"
#include	"misc.h"

#include	"chanfix.h"
#include	"sqlUser.h"

namespace gnuworld
{
sqlUser::sqlUser(PgDatabase* _SQLDb)
: Id( 0 ),
  user_name(""),
  created(0),
  last_seen(0),
  last_updated(0),
  last_updated_by(""),
  isServAdmin(0),
  canBlock(0),
  canAlert(0),
  canChanfix(0),
  isOwner(0),
  canManageUsers(0),
  isSuspended(0),
  useNotice(1),
  SQLDb(_SQLDb)
{};

void sqlUser::setAllMembers(int row) 
{
  Id = atoi(SQLDb->GetValue(row, 0));
  user_name = SQLDb->GetValue(row, 1);
  created = atoi(SQLDb->GetValue(row, 2));
  last_seen = atoi(SQLDb->GetValue(row, 3));
  last_updated = atoi(SQLDb->GetValue(row, 4));
  last_updated_by = SQLDb->GetValue(row, 5);
  isServAdmin = (!strcasecmp(SQLDb->GetValue(row, 6),"t") ? 1 : 0 );
  canBlock = (!strcasecmp(SQLDb->GetValue(row, 7),"t") ? 1 : 0 );
  canAlert = (!strcasecmp(SQLDb->GetValue(row, 8),"t") ? 1 : 0 );
  canChanfix = (!strcasecmp(SQLDb->GetValue(row, 9),"t") ? 1 : 0 );
  isOwner = (!strcasecmp(SQLDb->GetValue(row, 10),"t") ? 1 : 0 );
  canManageUsers = (!strcasecmp(SQLDb->GetValue(row, 11),"t") ? 1 : 0 );
  isSuspended = (!strcasecmp(SQLDb->GetValue(row, 12),"t") ? 1 : 0 );
  useNotice = (!strcasecmp(SQLDb->GetValue(row, 13),"t") ? 1 : 0 );
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
		<< "isservadmin = " << (isServAdmin ? "'t'" : "'n'") << ", "
		<< "canblock = " << (canBlock ? "'t'" : "'n'") << ", "
		<< "canalert = " << (canAlert ? "'t'" : "'n'") << ", "
		<< "canchanfix = " << (canChanfix ? "'t'" : "'n'") << ", "
		<< "isowner = " << (isOwner ? "'t'" : "'n'") << ", "
		<< "canmanageusers = " << (canManageUsers ? "'t'" : "'n'") << ", "
		<< "issuspended = " << (isSuspended ? "'t'" : "'n'") << ", "
		<< "usenotice = " << (useNotice ? "'t'" : "'n'")
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
		<< "(user_name, created, last_seen, last_updated, last_updated_by, isservadmin, canblock, canalert, canchanfix, isowner, canmanageusers, issuspended, usenotice) "
		<< "VALUES "
		<< "("
		<< "'" << user_name << "', "
		<< created << ", "
		<< last_seen << ", "
		<< last_updated << ", "
		<< "'" << last_updated_by << "', "
		<< (isServAdmin ? "'t'" : "'n'")
		<< ", "
		<< (canBlock ? "'t'" : "'n'")
		<< ", "
		<< (canAlert ? "'t'" : "'n'")
		<< ", "
		<< (canChanfix ? "'t'" : "'n'")
		<< ", "
		<< (isOwner ? "'t'" : "'n'")
		<< ", "
		<< (canManageUsers ? "'t'" : "'n'")
		<< ", "
		<< (isSuspended ? "'t'" : "'n'")
		<< ", "
		<< (useNotice ? "'t'" : "'n'")
		<< ")"
		;

#ifdef LOG_SQL
elog	<< "sqlUser::insert> "
	<< insertString
	<< endl;
#endif

ExecStatusType status = SQLDb->Exec(insertString.str().c_str());

if(PGRES_COMMAND_OK != status) {
	elog << "sqlUser::insert> " << SQLDb->ErrorMessage();
	return false;
}

return true;
} // sqlUser::insert()

bool sqlUser::Delete()
{
std::stringstream deleteString;
deleteString	<< "DELETE FROM users "
		<< "WHERE user_name = '" << user_name << "'"
		;

ExecStatusType status = SQLDb->Exec(deleteString.str().c_str());

if(PGRES_COMMAND_OK != status) {
	elog << "sqlUser::delete> " << SQLDb->ErrorMessage();
	return false;
}

return true;

}

sqlUser::~sqlUser()
{
// No heap space allocated
}

} //Namespace gnuworld
