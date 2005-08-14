/**
 * WHOISCommand.cc
 *
 * 08/07/2005 - Jimmy Lipham <music0m@alltel.net>
 * Initial Version
 *
 * Shows information about this user
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

void WHOISCommand::Exec(iClient* theClient, sqlUser*, const std::string& Message)
{
StringTokenizer st(Message);

sqlUser* theUser = bot->isAuthed(st[1]);
if (!theUser) 
{ 
  bot->Notice(theClient, "No such user %s.", st[1].c_str());
  return;
}

bot->Notice(theClient, "User: %s", theUser->getUserName().c_str());
if (!theUser->getFlags())
  bot->Notice(theClient, "Flags: none.");
else
  bot->Notice(theClient, "Flags: +%s",
	      bot->getFlagsString(theUser->getFlags()).c_str());
bot->Notice(theClient, "Hosts: %s", bot->getHostList(theUser).c_str()); //Fix this
bot->Notice(theClient, "Main server: NA"); //This too
bot->Notice(theClient, "Other servers: NA"); //Yep, this
} //whoiscommand::exec
} //namespace gnuworld
