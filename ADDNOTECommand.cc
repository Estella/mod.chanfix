/**
 * ADDNOTECommand.cc
 *
 * 08/18/2005 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Adds a note to a channel
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

void ADDNOTECommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

if (st[1][0] != '#') {
  bot->SendTo(theClient, "%s is an invalid channel name.",
	      st[1].c_str());
  return;
}

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan) theChan = bot->newChannelRecord(st[1]);

theChan->addNote(sqlChannel::EV_NOTE, theUser, st.assemble(2));

bot->SendTo(theClient, "Note recorded for channel %s",
	    theChan->getChannel().c_str());

return;
}
} // namespace gnuworld
