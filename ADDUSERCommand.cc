/**
 * ADDUSERCommand.cc
 *
 * 08/08/2005 - Jimmy Lipham <music0m@alltel.net>
 * Initial Version
 *
 * Adds a new user, without flags, and optionally with this hostmask
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
#include "Network.h"

#include "chanfix.h"
#include "StringTokenizer.h"
#include "sqlUser.h"

RCSTAG("$Id$");

namespace gnuworld
{

void ADDUSERCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

sqlUser* chkUser = bot->isAuthed(st[1]);
if (chkUser) {
  bot->Notice(theClient, "User %s already exists.", st[1].c_str());
  return;
}

sqlUser *newUser = new sqlUser(bot->SQLDb);
assert(newUser != 0);
newUser->setUserName(st[1]);
newUser->setCreated(bot->currentTime());
newUser->setLastSeen(0);
newUser->setLastUpdatedBy(theClient->getAccount());
newUser->setLastUpdated(bot->currentTime());
if (newUser->Insert()) {
  bot->Notice(theClient, "Created user %s.", st[1].c_str());
  bot->usersMap[newUser->getUserName()] = newUser;
} else {
  bot->Notice(theClient, "Error creating user %s. (Insertion failed)", st[1].c_str());
}

} //addusercommand::exec
} //namespace gnuworld
