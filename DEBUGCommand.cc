/**
 * DEBUGCommand.cc
 *
 * 01/16/2006 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Overview command for old ROTATE and UPDATE commands
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

RCSTAG("$Id$");

namespace gnuworld
{

void DEBUGCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
#ifndef CHANFIX_DEBUG
return;
#endif

StringTokenizer st(Message);

std::string option = string_upper(st[1]);

if (option == "ROTATE") {
  bot->logAdminMessage("%s (%s) ordered a manual DB rotation.",
		       theUser->getUserName().c_str(),
		       theClient->getRealNickUserHost().c_str());
  bot->rotateDB(false);
  return;
}

if (option == "UPDATE") {
  bot->logAdminMessage("%s (%s) ordered a manual DB update.",
		       theUser->getUserName().c_str(),
		       theClient->getRealNickUserHost().c_str());
  bot->updateDB(false);
  return;
}

}
} // namespace gnuworld
