/**
 * RELOADCommand.cc
 *
 * 18/12/2003 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Unloads the bot's module and reloads it
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 * $Id$
 */

#include	<string>

#include	"gnuworld_config.h"
#include	"StringTokenizer.h"

#include	"chanfix.h"
#include	"flags.h"

RCSTAG("$Id$");

namespace gnuworld
{

void RELOADCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

bot->Notice(theClient, "Reloading client...see you on the flip side");
bot->logAdminMessage("%s (%s) is reloading the chanfix module.", theClient->getNickName().c_str(), theClient->getNickUserHost().c_str());

if (st.size() < 2) {
  server->UnloadClient(bot, "Reloading...");
} else {
  server->UnloadClient(bot, st.assemble(1));
}

server->LoadClient("libchanfix", bot->getConfigFileName());

return;
}

} // namespace gnuworld
