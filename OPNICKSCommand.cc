/**
 * OPNICKSCommand.cc
 *
 * 07/18/2005 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Shows the nicks of all ops in <channel>
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
#include "levels.h" 
#include "StringTokenizer.h"

RCSTAG("$Id$");

namespace gnuworld
{

using std::string;

void OPNICKSCommand::Exec(iClient* theClient, const std::string& Message)
{
StringTokenizer st(Message);
if (st.size() < 2) {
  Usage(theClient);
  return;
}

Channel* netChan = Network->findChannel(st[1]);
if (!netChan) {
  bot->Notice(theClient, "No such channel %s.", st[1].c_str());
  return;
}

/* Send list of opped clients. */
bot->Notice(theClient, "Opped clients on channel %s:", 
	    netChan->getName().c_str());

ChannelUser* curUser;
string oppedUsers;
unsigned int numOppedUsers = 0;
for (Channel::userIterator ptr = netChan->userList_begin(); ptr != netChan->userList_end(); ptr++) {
   curUser = ptr->second;
   if (curUser->isModeO()) {
     if (numOppedUsers++)
       oppedUsers += " ";
     oppedUsers += curUser->getNickName();
   }
}

if (numOppedUsers)
  bot->Notice(theClient, "%s", oppedUsers.c_str());

if (numOppedUsers == 1)
  bot->Notice(theClient, "I see 1 opped client in %s.", 
	      netChan->getName().c_str());
else
  bot->Notice(theClient, "I see %u opped clients in %s.",
	      numOppedUsers, netChan->getName().c_str());

/* Log command */
/* ... */

return;
}

} // namespace gnuworld
