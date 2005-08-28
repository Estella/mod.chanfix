/**
 * SETGROUPCommand.cc
 *
 * 08/27/2005 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Sets the group of the user
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

void SETGROUPCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

sqlUser* targetUser = bot->isAuthed(st[1]);
if (!targetUser) {
  bot->SendTo(theClient, "No such user %s.", st[1].c_str());
  return;
}

if (string_lower(targetUser->getGroup()) == string_lower(st[2])) {
  bot->SendTo(theClient, "User %s is already in group %s.", 
	      targetUser->getUserName().c_str(), st[2].c_str());
  return;
}

targetUser->setGroup(st[2]);
targetUser->setLastUpdated(bot->currentTime());
targetUser->setLastUpdatedBy( std::string( "("
	+ theUser->getUserName()
	+ ") "
	+ theClient->getRealNickUserHost() ) );
targetUser->commit();

bot->SendTo(theClient, "Set group %s for user %s.", st[2].c_str(), 
	    targetUser->getUserName().c_str());
bot->logAdminMessage("%s (%s) set the group of user %s to %s.",
	    theUser->getUserName().c_str(),
	    theClient->getRealNickUserHost().c_str(),
	    targetUser->getUserName().c_str(),
	    st[2].c_str());

} //SETGROUPCommand::Exec
} //Namespace gnuworld
