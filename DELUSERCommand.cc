/**
 * DELUSERCommand.cc
 *
 * 08/08/2005 - Jimmy Lipham <music0m@alltel.net>
 * Initial Version
 *
 * Deletes this user
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

void DELUSERCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

sqlUser* chkUser = bot->isAuthed(st[1]);
if (!chkUser) {
  bot->SendTo(theClient, "No such user %s.", st[1].c_str());
  return;
}

if (chkUser->getFlag(sqlUser::F_OWNER)) {
  bot->SendTo(theClient, "You cannot delete an owner.");
  return;
}

if (chkUser->getFlag(sqlUser::F_USERMANAGER) && !theUser->getFlag(sqlUser::F_OWNER)) {
  bot->SendTo(theClient, "You cannot delete a user manager.");
  return;
}

/* A serveradmin can only delete users on his own group. */
if (theUser->getFlag(sqlUser::F_SERVERADMIN) &&
    !theUser->getFlag(sqlUser::F_USERMANAGER)) {
  if (chkUser->getGroup() != theUser->getGroup()) {
    bot->SendTo(theClient, "You cannot delete a user in a different group.");
    return;
  }
}

if (chkUser->Delete()) {
  bot->usersMap.erase(bot->usersMap.find(chkUser->getUserName()));
  bot->SendTo(theClient, "Deleted user %s.", chkUser->getUserName().c_str());
  bot->logAdminMessage("%s (%s) deleted user %s.",
		       theClient->getAccount().c_str(),
		       theClient->getRealNickUserHost().c_str(),
		       chkUser->getUserName().c_str());
  delete chkUser; chkUser = 0;
} else {
  bot->SendTo(theClient, "Error deleting user %s.", st[1].c_str());
}
} //DELUSERCommand::exec
} //Namespace gnuworld
