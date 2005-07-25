/**
 * SETCommand.cc
 *
 * 07/23/2005 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Sets <option> to value <value>
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
#include	"levels.h"
#include	"Network.h"

RCSTAG("$Id$");

namespace gnuworld
{

using std::string;
using namespace level;

void SETCommand::Exec(iClient* theClient, const std::string& Message)
{
StringTokenizer st(Message);

string option = string_upper(st[1]);
string value = string_upper(st[2]);

string::size_type pos;
while ((pos = option.find('_', 0) != string::npos))
  option.erase(pos, 1);

if (option == "NUMSERVERS") {
  int setting = atoi(value.c_str());
  if (setting < 0) {
    bot->Notice(theClient, "Please use SET NUM_SERVERS <integer number>.");
    return;
  }

  bot->setNumServers(setting);
  bot->Notice(theClient, "NUM_SERVERS is now %d.", setting);
  bot->logAdminMessage("%s changed NUM_SERVERS to %d.",
		       theClient->getNickName().c_str(), setting);
  return;
}

if (option == "ENABLEAUTOFIX") {
  if (value == "ON" || value == "1") {
    bot->setDoAutoFix(true);
    bot->Notice(theClient, "Enabled autofix.");
    bot->logAdminMessage("%s enabled autofix.",
			 theClient->getNickName().c_str());
    return;
  } else if (value == "OFF" || value == "0") {
    bot->setDoAutoFix(false);
    bot->Notice(theClient, "Disabled autofix.");
    bot->logAdminMessage("%s disabled autofix.",
			 theClient->getNickName().c_str());
    return;
  } else {
    bot->Notice(theClient, "Please use SET ENABLE_AUTOFIX <on/off>.");
    return;
  }
}

if (option == "ENABLECHANFIX") {
  if (value == "ON" || value == "1") {
    bot->setDoChanFix(true);
    bot->Notice(theClient, "Enabled manual chanfix.");
    bot->logAdminMessage("%s enabled manual chanfix.",
			 theClient->getNickName().c_str());
    return;
  } else if (value == "OFF" || value == "0") {
    bot->setDoChanFix(false);
    bot->Notice(theClient, "Disabled manual chanfix.");
    bot->logAdminMessage("%s disabled manual chanfix.",
			 theClient->getNickName().c_str());
    return;
  } else {
    bot->Notice(theClient, "Please use SET ENABLE_CHANFIX <on/off>.");
    return;
  }
}

if (option == "ENABLECHANNELBLOCKING") {
  if (value == "ON" || value == "1") {
    bot->setDoChanBlocking(true);
    bot->Notice(theClient, "Enabled channel blocking.");
    bot->logAdminMessage("%s enabled channel blocking.",
			 theClient->getNickName().c_str());
    return;
  } else if (value == "OFF" || value == "0") {
    bot->setDoChanBlocking(false);
    bot->Notice(theClient, "Disabled channel blocking.");
    bot->logAdminMessage("%s disabled channel blocking.",
			 theClient->getNickName().c_str());
    return;
  } else {
    bot->Notice(theClient, "Please use SET ENABLE_CHANNEL_BLOCKING <on/off>.");
    return;
  }
}

bot->Notice(theClient, "This setting does not exist.");

return;
}

} // namespace gnuworld
