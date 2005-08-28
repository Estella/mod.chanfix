/**
 * WHOISCommand.cc
 *
 * 08/07/2005 - Jimmy Lipham <music0m@alltel.net>
 * Initial Version
 *
 * Shows information about this user
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 *
 * $Id$
 */

#include "gnuworld_config.h"

#include "chanfix.h"
#include "StringTokenizer.h"
#include "sqlUser.h"

RCSTAG("$Id$");

namespace gnuworld
{

void WHOISCommand::Exec(iClient* theClient, sqlUser*, const std::string& Message)
{
StringTokenizer st(Message);

if (st[1] == "*") {
  bot->SendTo(theClient, "List of all users:");
  chanfix::usersIterator ptr = bot->usersMap_begin();
  while (ptr != bot->usersMap_end()) {
    sqlUser* tmpUser = ptr->second;
    bot->SendTo(theClient, "User: %s, Flags: %s, Group: %s",
		tmpUser->getUserName().c_str(), (tmpUser->getFlags()) ?
		std::string("+" + bot->getFlagsString(tmpUser->getFlags())).c_str() : "None", 
		tmpUser->getGroup().c_str());
    ptr++;
  }
  bot->SendTo(theClient, "End of user list.");
  return;
}

sqlUser* theUser = bot->isAuthed(st[1]);
if (!theUser) 
{ 
  bot->SendTo(theClient, "No such user %s.", st[1].c_str());
  return;
}

bot->SendTo(theClient, "User: %s", theUser->getUserName().c_str());
if (!theUser->getFlags())
  bot->SendTo(theClient, "Flags: none.");
else
  bot->SendTo(theClient, "Flags: +%s",
	      bot->getFlagsString(theUser->getFlags()).c_str());

sqlUser::hostListType sqlHostList = theUser->getHostList();
std::stringstream hostlist;

if (sqlHostList.size() > 0) {
  for(sqlUser::hostListType::iterator itr = sqlHostList.begin() ;
    itr != sqlHostList.end() ; ++itr) {
      if (hostlist.str() == "")
        hostlist << *itr;
      else
	hostlist << ", " << *itr;
    }
}
if (hostlist.str() == "") hostlist << "None.";
bot->SendTo(theClient, "Hosts: %s", hostlist.str().c_str());
bot->SendTo(theClient, "Group: %s", theUser->getGroup().c_str());

return;
} //whoiscommand::exec
} //namespace gnuworld
