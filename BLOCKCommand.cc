/**
 * BLOCKCommand.cc
 *
 * 07/19/2005 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Blocks a channel from being fixed, both automatically and manually
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

void BLOCKCommand::Exec(iClient* theClient, const std::string& Message)
{
StringTokenizer st(Message);
if (st.size() < 3) {
  Usage(theClient);
  return;
}

/* Check if channel blocking has been disabled in the config. */
if (!bot->doChanBlocking()) {
  bot->Notice(theClient, "Channel blocking is disabled.");
  return;
}

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan) theChan = bot->newChannelRecord(st[1]);

if (theChan->getFlag(sqlChannel::F_BLOCKED)) {
  bot->Notice(theClient, "The channel %s is already blocked.",
	      theChan->getChannel().c_str());
  return;
}

theChan->setFlag(sqlChannel::F_BLOCKED);
theChan->commit();

/* Add note to the channel about this command */
/* bot->addNote(theChan, ... "BLOCK by %s: %s" */

bot->Notice(theClient, "The channel %s has been blocked.",
	    theChan->getChannel().c_str());

return;
}
} // namespace gnuworld
