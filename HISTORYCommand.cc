/**
 * HISTORYCommand.cc
 *
 * 08/14/2005 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Shows the times that <channel> has been manually fixed
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

void HISTORYCommand::Exec(iClient* theClient, sqlUser*, const std::string& Message)
{
StringTokenizer st(Message);

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan) {
  bot->SendTo(theClient, "Channel %s has never been manually fixed.",
	      st[1].c_str());
  return;
}

/*
 * Perform a query to list the dates/times this channel was last chanfixed.
 */
std::stringstream chanfixQuery;
chanfixQuery	<< "SELECT ts "
		<< "FROM notes "
		<< "WHERE channelID = "
		<< theChan->getID()
		<< " AND event = "
		<< sqlChannel::EV_CHANFIX
		<< " ORDER BY ts DESC"
		;

ExecStatusType status = bot->SQLDb->Exec( chanfixQuery.str().c_str() ) ;

if( PGRES_TUPLES_OK != status )
	{
	elog	<< "HISTORYCommand> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< std::endl;

	bot->SendTo(theClient, "An unknown error occurred while reading this channel's notes.");
	return ;
	}

unsigned int noteCount = bot->SQLDb->Tuples();

if (noteCount <= 0) {
  bot->SendTo(theClient, "Channel %s has never been manually fixed.",
	      theChan->getChannel().c_str());
  return;
}

bot->SendTo(theClient, "Channel %s has been manually fixed on:",
	    theChan->getChannel().c_str());

for (unsigned int i = 0; i < noteCount; i++)
  bot->SendTo(theClient, "%s",
	      bot->tsToDateTime(atoi(bot->SQLDb->GetValue(i,0)), true).c_str());

bot->SendTo(theClient, "End of list.");

return;
}
} // namespace gnuworld
