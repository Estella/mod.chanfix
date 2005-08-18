/**
 * DELNOTECommand.cc
 *
 * 08/18/2005 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Deletes a note from this channel
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

void DELNOTECommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan) {
  bot->SendTo(theClient, "There is no entry in the database for %s.",
	      st[1].c_str());
  return;
}

if (!theChan->countNotes(0) <= 0) {
  bot->SendTo(theClient, "The channel %s does not have any notes.",
	      theChan->getChannel().c_str());
  return;
}

unsigned int messageId = atoi(st[2].c_str());

std::stringstream noteCheckQuery;
noteCheckQuery	<< "SELECT channelID, userID "
		<< "FROM notes "
		<< "WHERE id = "
		<< messageId
		<< std::ends;

ExecStatusType status = bot->SQLDb->Exec( noteCheckQuery.str().c_str() ) ;

if( PGRES_TUPLES_OK != status )
	{
	elog	<< "DELNOTECommand> SQL Error: "
		<< bot->SQLDb->ErrorMessage()
		<< std::endl ;

	bot->SendTo(theClient, "An unknown error occured while checking the note id.");
	return;
	}

if (bot->SQLDb->Tuples() != 1) {
  bot->SendTo(theClient, "There is no such note with that note_id.");
  return;
}

unsigned int channelID = atoi(bot->SQLDb->GetValue(0,0));
unsigned int userID = atoi(bot->SQLDb->GetValue(0,1));

if (channelID != theChan->getID()) {
  bot->SendTo(theClient, "No such note %d for channel %s.",
	      messageId, theChan->getChannel().c_str());
  return;
}

if (userID != theUser->getID() && !theUser->getFlag(sqlUser::F_OWNER)) {
  bot->SendTo(theClient, "Note %d for channel %s was not added by you. You can only delete notes that you added.",
	      messageId, theChan->getChannel().c_str());
  return;
}

if (!theChan->deleteNote(messageId, theUser))
  return;

bot->SendTo(theClient, "Note %d for channel %s deleted.",
	    messageId, theChan->getChannel().c_str());

return;
}
} // namespace gnuworld
