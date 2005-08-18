/**
 * UNALERTCommand.cc
 *
 * 07/19/2005 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Unsets the ALERT flag of this channel
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
#include "sqlChannel.h"
#include "sqlUser.h"

RCSTAG("$Id$");

namespace gnuworld
{

void UNALERTCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan) {
  bot->SendTo(theClient, "There is no entry in the database for %s.",
	      st[1].c_str());
  return;
}

if (!theChan->getFlag(sqlChannel::F_ALERT)) {
  bot->SendTo(theClient, "The channel %s does not have the ALERT flag.",
	      theChan->getChannel().c_str());
  return;
}

theChan->removeFlag(sqlChannel::F_ALERT);
theChan->commit();

/* Add note to the channel about this command */
theChan->addNote(sqlChannel::EV_UNALERT, theUser, "");

bot->SendTo(theClient, "ALERT flag removed from channel %s",
	    theChan->getChannel().c_str());

return;
}
} // namespace gnuworld
