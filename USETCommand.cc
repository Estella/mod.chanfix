/**
 * USETCommand.cc
 *
 * 08/14/2005 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Sets user <option> to value <value>
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
#include	"sqlUser.h"

RCSTAG("$Id$");

namespace gnuworld
{

void USETCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

std::string option = string_upper(st[1]);
std::string value = string_upper(st[2]);

if (option == "NOTICE") {
  if (value == "ON" || value == "1") {
    theUser->setNotice(true);
    bot->SendTo(theClient, "I will now send you notices.");
    theUser->commit();
    return;
  } else if (value == "OFF" || value == "0") {
    theUser->setNotice(false);
    bot->SendTo(theClient, "I will now send you privmsgs.");
    theUser->commit();
    return;
  } else {
    bot->SendTo(theClient, "Please use USET NOTICE <on/off>.");
    return;
  }
}

bot->SendTo(theClient, "This setting does not exist.");

return;
}

} // namespace gnuworld
