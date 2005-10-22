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
const sqlUser::flagType sqlUser::F_NORMALUSER =		0x10; /* +n */
const sqlUser::flagType sqlUser::F_OWNER =		0x20; /* +o */
const sqlUser::flagType sqlUser::F_USERMANAGER =	0x40; /* +u */
const sqlUser::flagType sqlUser::F_LOGGEDIN =		0x80;

unsigned long int sqlUser::maxUserId = 0;

sqlUser::sqlUser(PgDatabase* _SQLDb)
: id(0),
  user_name(),
  created(0),
  last_seen(0),
  last_updated(0),
  last_updated_by(),
  language_id(0),
  group(),
  flags(0),
  isSuspended(false),
  useNotice(true)
{
  myManager = _myManager;
};

void sqlUser::setAllMembers(PgDatabase* theDB, int row)
{
  id = atoi(theDB->GetValue(row, 0));
  user_name = theDB->GetValue(row, 1);
  created = atoi(theDB->GetValue(row, 2));
  last_seen = atoi(theDB->GetValue(row, 3));
  last_updated = atoi(theDB->GetValue(row, 4));
  last_updated_by = theDB->GetValue(row, 5);
  language_id = atoi(theDB->GetValue(row, 6));
  group = theDB->GetValue(row, 7);
  flags = atoi(theDB->GetValue(row, 8));
  isSuspended = (!strcasecmp(theDB->GetValue(row,9),"t") ? true : false);
  useNotice = (!strcasecmp(theDB->GetValue(row,10),"t") ? true : false);

  if (id > maxUserId) maxUserId = id;
};

void sqlUser::commit()
{
std::stringstream userCommit;
userCommit	<< "UPDATE users SET "
		<< "last_seen = " << last_seen << ", "
		<< "last_updated = " << last_updated << ", "
		<< "last_updated_by = '" << last_updated_by << "', "
		<< "language_id = " << language_id << ", "
		<< "faction = '" << group << "', "
		<< "flags = " << flags << ", "
		<< "issuspended = " << (isSuspended ? "'t'" : "'f'") << ", "
		<< "usenotice = " << (useNotice ? "'t'" : "'f'")
		<< " WHERE "
		<< "id = " << id
		;
myManager->queueCommit(userCommit.str());
}

/**
 * This function inserts a brand new user into the DB.
 * It is a slight fudge, in that it first creates a blank record then
 * calls commit() to update the data fields for that record. This is done
 * so that any new fields added will automatically be dealt with in commit()
 * instead of in 50 different functions.
 */
void sqlUser::Insert()
{
/* Grab the next available user id */
id = ++maxUserId;

std::stringstream insertString;
insertString	<< "INSERT INTO users "
		<< "(id, user_name) "
		<< "VALUES "
		<< "("
		<< id << ", "
		<< "'" << user_name << "'"
		<< ")"
		;

myManager->queueCommit(insertString.str());
commit();
} // sqlUser::Insert()

void sqlUser::Delete()
{
std::stringstream deleteString;
deleteString	<< "DELETE FROM users "
		<< "WHERE id = '" << id << "'"
		;
myManager->queueCommit(deleteString.str());

std::stringstream hostString;
hostString	<< "DELETE FROM hosts "
		<< "WHERE user_id = " << id
		;
myManager->queueCommit(hostString.str());
}

void sqlUser::loadHostList()
{
  static const char* queryHeader
	= "SELECT host FROM hosts WHERE user_id = ";

  std::stringstream theQuery;
  theQuery	<< queryHeader 
		<< id
		;
// DEBUG
elog	<< "sqlUser::loadHostList> "
	<< theQuery.str()
	<< std::endl;
//END DEBUG
  ExecStatusType status = SQLDb->Exec( theQuery.str().c_str() ) ;

  if( PGRES_TUPLES_OK != status )
	{
	elog	<< "sqlUser::loadHostList> SQL Error: "
		<< SQLDb->ErrorMessage()
		<< std::endl ;
	return;
	}

// SQL Query succeeded
  for (int i = 0 ; i < SQLDb->Tuples(); i++) {
    hostList.push_back(SQLDb->GetValue(i, 0));
  }
  
};

void sqlUser::delHost(const std::string& _theHost)
{
  if (hostList.size() < 1) return;
  hostListType::iterator ptr = find( hostList.begin(), hostList.end(), string_lower(_theHost) );
  if (ptr == hostList.end()) return;
  hostList.erase(ptr);
}

bool sqlUser::matchHost(const std::string& _host)
{
  if (hostList.size() < 1) return false;
  for(hostListType::iterator itr = hostList.begin() ;
    itr != hostList.end() ; ++itr) {
      if (match(*itr,_host) == 0) return true;
  }
  return false;
}

bool sqlUser::hasHost(const std::string& _host)
{
  if (hostList.size() < 1) return false;
  hostListType::iterator ptr = find( hostList.begin(), hostList.end(), string_lower(_host) );
  if ( ptr == hostList.end() ) return false;
  return true;
}

sqlUser::~sqlUser()
{
// No heap space allocated
}

} //namespace gnuworld
