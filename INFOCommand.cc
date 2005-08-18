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

#include "gnuworld_config.h"
#include "Network.h"

#include "chanfix.h"
#include "StringTokenizer.h"
#include "sqlChannel.h"
#include "sqlUser.h"

RCSTAG("$Id$");

namespace gnuworld
{

void INFOCommand::Exec(iClient* theClient, sqlUser*, const std::string& Message)
{
StringTokenizer st(Message);

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan) {
  bot->SendTo(theClient, "No information on %s in the database.",
	      st[1].c_str());
  return;
}

bot->SendTo(theClient, "Information on %s:",
	    theChan->getChannel().c_str());

if (theChan->getFlag(sqlChannel::F_BLOCKED))
  bot->SendTo(theClient, "%s is BLOCKED.",
	      theChan->getChannel().c_str());
else if (theChan->getFlag(sqlChannel::F_ALERT))
  bot->SendTo(theClient, "%s is ALERTED.",
	      theChan->getChannel().c_str());

Channel* netChan = Network->findChannel(st[1]);
if (netChan) {
  if (bot->isBeingChanFixed(netChan))
    bot->SendTo(theClient, "%s is being chanfixed.",
		theChan->getChannel().c_str());
  if (bot->isBeingAutoFixed(netChan))
    bot->SendTo(theClient, "%s is being autofixed.",
		theChan->getChannel().c_str());
}

/*
 * Perform a query to list all notes belonging to this channel.
 */
std::stringstream allNotesQuery;
allNotesQuery	<< "SELECT notes.id, notes.ts, users.user_name, notes.message "
 		<< "FROM notes,users "
		<< "WHERE notes.userID = users.id "
		<< "AND notes.channelID = "
		<< theChan->getID()
		<< " ORDER BY notes.ts DESC"
		<< std::ends;

ExecStatusType status = bot->SQLDb->Exec( allNotesQuery.str().c_str() ) ;

if( PGRES_TUPLES_OK != status )
	{
	elog	<< "INFOCommand> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< std::endl;

	bot->SendTo(theClient, "An unknown error occurred while reading this channel's notes.");
	return ;
	}

unsigned int noteCount = bot->SQLDb->Tuples();

if (noteCount > 0) {
  bot->SendTo(theClient, "Notes (%d):", noteCount);

  for (unsigned int i = 0; i < noteCount; i++) {
    unsigned int note_id = atoi(bot->SQLDb->GetValue(i,0));
    unsigned int when = atoi(bot->SQLDb->GetValue(i,1));
    std::string from = bot->SQLDb->GetValue(i,2);
    std::string theMessage = bot->SQLDb->GetValue(i,3);

    bot->SendTo(theClient, "[%d:%s] %s %s",
		note_id, from.c_str(),
		bot->tsToDateTime(when, true).c_str(),
		theMessage.c_str());
  }
}

bot->SendTo(theClient, "End of information.");

return;
}
} // namespace gnuworld
