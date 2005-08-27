/**
 * DELHOSTCommand.cc
 *
 * 08/26/2005 - Jimmy Lipham <music0m@alltel.net>
 * Initial Version
 *
 * Adds a new user, without flags, and optionally with this hostmask
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
 * $Id: DELHOSTCommand.cc 1260 2005-08-26 02:28:50Z Compster $
 */

#include "gnuworld_config.h"
#include "Network.h"

#include "chanfix.h"
#include "StringTokenizer.h"
#include "sqlUser.h"

RCSTAG("");

namespace gnuworld
{

void DELHOSTCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
  StringTokenizer st(Message);
  sqlUser* Target = bot->isAuthed(st[1]);
  
  if (!Target) {
    bot->SendTo(theClient, "No such user %s.", st[1].c_str());
    return;
  }
  
  if (!Target->hasHost(st[2].c_str())) {
    bot->SendTo(theClient, "That host is not in %s's host list.", st[1].c_str());
    return;
  }

  std::stringstream insertString;
  insertString	<< "DELETE FROM hosts "
                << "WHERE user_id = "
                << Target->getID()
                << " AND lower(host) = '"
                << string_lower(st[2].c_str())
                << "'"
		;
  ExecStatusType status = bot->SQLDb->Exec(insertString.str().c_str());

  if(PGRES_COMMAND_OK != status) {
    bot->SendTo(theClient, "Could not delete host from %s (Deletion query failure)", st[1].c_str());
    return;
  }
  
  Target->delHost(st[2].c_str());
  bot->SendTo(theClient, "Deleted %s from %s's host list.", st[2].c_str(), st[1].c_str());
}

} //namespace gnuworld
