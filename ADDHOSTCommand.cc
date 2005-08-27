/**
 * ADDHOSTCommand.cc
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
 * $Id: ADDHOSTCommand.cc 1260 2005-08-26 02:28:50Z Compster $
 */

#include "gnuworld_config.h"
#include "Network.h"

#include "chanfix.h"
#include "StringTokenizer.h"
#include "sqlUser.h"

RCSTAG("");

namespace gnuworld
{

void ADDHOSTCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
  StringTokenizer st(Message);

  if (st[2] == "*!*@*") {
    bot->SendTo(theClient, "I dont think your superiors would like that type of security issue..");
    return;
  }
  
  sqlUser* Target = bot->isAuthed(st[1]);
  
  if (!Target) {
    bot->SendTo(theClient, "No such user %s.", st[1].c_str());
    return;
  }
  
  if (Target->matchHost(st[2].c_str())) {
    bot->SendTo(theClient, "%s already has a host matching the given mask.", st[1].c_str());
    return;
  }
  
  std::stringstream insertString;
  insertString	<< "INSERT INTO hosts "
                << "(user_id, host) VALUES "
                << "("
                << Target->getID()
                << ", '"
                << st[2].c_str()
                << "')"
		;
  ExecStatusType status = bot->SQLDb->Exec(insertString.str().c_str());

  if(PGRES_COMMAND_OK != status) {
    bot->SendTo(theClient, "Could not add host to %s (Insertion failed)", st[1].c_str());
    return;
  }
  
  Target->addHost(st[2].c_str());
  bot->SendTo(theClient, "Added %s to %s's list of registered hostmasks.", st[2].c_str(), st[1].c_str());
  
}

} //namespace gnuworld
