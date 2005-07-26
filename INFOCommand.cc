/**
 * INFOCommand.cc
 *
 * 07/20/2005 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Shows all notes of this channel, and whether it has been blocked
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

#include <stdlib.h>
#include <iostream>
#include "gnuworld_config.h"
#include "Network.h"

#include "chanfix.h"
#include "levels.h" 
#include "StringTokenizer.h"
#include "sqlChannel.h"
#include "sqlUser.h"

RCSTAG("$Id$");

namespace gnuworld
{

using std::endl;
using std::string;
using std::stringstream;
using std::ends;

void INFOCommand::Exec(iClient* theClient, const std::string& Message)
{
StringTokenizer st(Message);
if (st.size() < 2) {
  Usage(theClient);
  return;
}
sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan) {
  bot->Notice(theClient, "No information on %s in the database.",
	      st[1].c_str());
  return;
}

bot->Notice(theClient, "Information on %s:",
	    theChan->getChannel().c_str());

if (theChan->getFlag(sqlChannel::F_BLOCKED))
  bot->Notice(theClient, "%s is BLOCKED.",
	      theChan->getChannel().c_str());
else if (theChan->getFlag(sqlChannel::F_ALERT))
  bot->Notice(theClient, "%s is ALERTED.",
	      theChan->getChannel().c_str());

Channel* netChan = Network->findChannel(st[1]);
if (netChan) {
  if (bot->isBeingChanFixed(netChan))
    bot->Notice(theClient, "%s is being chanfixed.",
		theChan->getChannel().c_str());
  if (bot->isBeingAutoFixed(netChan))
    bot->Notice(theClient, "%s is being autofixed.",
		theChan->getChannel().c_str());
}

/* need to check for notes here -reed */
/* ... */

bot->Notice(theClient, "End of information.");

return;
}
} // namespace gnuworld
