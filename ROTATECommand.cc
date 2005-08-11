/**
 * ROTATECommand.cc
 *
 * 07/26/2005 - Jimmy Lipham <music0m@alltel.net>
 * Initial Version
 *
 * Rotate the bot's score database (keep only DAYSAMPLES days)
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
 * $Id: ROTATECommand.cc 1165 2005-07-26 00:52:31Z Compster $
 */

#include	<string>

#include	"gnuworld_config.h"
#include	"StringTokenizer.h"

#include	"chanfix.h"
#include	"flags.h"

RCSTAG("$Id: ROTATECommand.cc 1165 2005-07-26 00:52:31Z Compster $");

namespace gnuworld
{

void ROTATECommand::Exec(iClient* theClient, sqlUser* theUser, const std::string&)
{
bot->logAdminMessage("%s (%s) ordered a manual DB rotation.",
		     theClient->getNickName().c_str(),
		     theClient->getNickUserHost().c_str());
bot->rotateDB();
return;
}

} // namespace gnuworld
