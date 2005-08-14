/**
 * CHECKCommand.cc
 *
 * 07/19/2005 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Shows the number of ops and total clients in <channel>
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

RCSTAG("$Id$");

namespace gnuworld
{

void CHECKCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

Channel* netChan = Network->findChannel(st[1]);
if (!netChan) {
  bot->Notice(theClient, "No such channel %s.", st[1].c_str());
  return;
}

/* Reports ops and total clients. */

bot->Notice(theClient, "I see %d opped out of %d total clients in %s.",
	    bot->countChanOps(netChan), netChan->size(),
	    netChan->getName().c_str());

/* Log command */
/* ... */

return;
}

} // namespace gnuworld
