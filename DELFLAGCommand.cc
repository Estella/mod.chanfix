/**
 * DELFLAGCommand.cc
 *
 * 07/21/2005 - Jimmy Lipham <music0m@alltel.net>
 * Initial Version
 *
 * Shows a list of accounts plus their score of the top ops of this channel
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
 * $Id: DELFLAGCommand.cc
 */

#include <ctime>
#include <iostream>

#include "gnuworld_config.h"
#include "Network.h"

#include "chanfix.h"
#include "levels.h" 
#include "StringTokenizer.h"
#include "sqlChannel.h"
#include "sqlChanOp.h"

RCSTAG("");

namespace gnuworld
{

void DELFLAGCommand::Exec(iClient* theClient, const std::string& Message)
{
if (theClient->getAccount() == "" || !theClient->isOper()) return;
	
StringTokenizer st(Message);

if (st.size() < 2) {
  Usage(theClient);
  return;
}

if (st[2].size() > 1) {
  Usage(theClient);
  return;
}

if (st[2] != "a" && st[2] != "b" && st[2] != "c" && st[2] != "f" && st[2] != "o" && st[2] != "u") {
  Usage(theClient);
  return;
}

sqlUser* theUser = bot->GetOper(theClient->getAccount());
if (!theUser) return;

if (!theUser->hasFlag("u") && !theUser->hasFlag("a")) {
  bot->Notice(theClient, "This command requires one of these flags: ua.");
  return;
}
sqlUser* chkUser = bot->GetOper(st[1]);
if (!chkUser) {
  bot->Notice(theClient, "No such user %s.", st[1].c_str());
  return;
}

if (!chkUser->hasFlag(st[2])) {
  bot->Notice(theClient, "User %s does not have flag %s.", st[1].c_str(), st[2].c_str());
  return;
}

chkUser->delFlag(st[2]);
chkUser->setLastUpdatedBy(theUser->getUserName());
chkUser->setLastUpdated(bot->currentTime());
chkUser->commit();
bot->Notice(theClient, "Deleted flag %s of user %s.", st[2].c_str(), st[1].c_str());
} //DELFLAGCommand::Exec
} //Namespace gnuworld
