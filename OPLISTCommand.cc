/**
 * OPLISTCommand.cc
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
 * $Id$
 */

#include "gnuworld_config.h"
#include "Network.h"

#include "chanfix.h"
#include "StringTokenizer.h"
#include "sqlChannel.h"
#include "sqlChanOp.h"

RCSTAG("$Id$");

namespace gnuworld
{

void OPLISTCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

Channel* netChan = Network->findChannel(st[1]);
sqlChanOp* curOp = 0;
if (!netChan) {
  bot->SendTo(theClient, "No such channel %s.", st[1].c_str());
  return;
}

chanfix::chanOpsType myOps = bot->getMyOps(netChan);
if (myOps.empty()) {
  bot->SendTo(theClient, "There are no scores in the database for %s.",
	      netChan->getName().c_str());
  return;
}

unsigned int oCnt = 0;
for (chanfix::chanOpsType::iterator opPtr = myOps.begin();
     opPtr != myOps.end(); opPtr++) {
  curOp = *opPtr;
  if (oCnt < OPCOUNT) oCnt++;
}

/* Technically if they are all 0 scores it will get to this point,
 * dont want a notice saying 0 accounts. */
if (oCnt == 0) {
  bot->SendTo(theClient, "There are no scores in the database for %s.",
	      netChan->getName().c_str());
  return;
}

if (oCnt == OPCOUNT)
  bot->SendTo(theClient, "Top %d unique op accounts in channel %s:",
	      OPCOUNT, netChan->getName().c_str());
else
  bot->SendTo(theClient, "Found %d unique op accounts in channel %s:",
	      oCnt, netChan->getName().c_str());

bot->SendTo(theClient, "Rank Score Account -- Time first opped / Time last opped");

unsigned int opCount = 0;
std::string firstop;
std::string lastop;

for (chanfix::chanOpsType::iterator opPtr = myOps.begin();
     opPtr != myOps.end(); opPtr++) {
  curOp = *opPtr;
  if (opCount < OPCOUNT) {
    opCount++;
    firstop = bot->tsToDateTime(curOp->getTimeFirstOpped(), false);
    lastop = bot->tsToDateTime(curOp->getTimeLastOpped(), true);
    bot->SendTo(theClient, "%2d. %4d %s -- %s / %s", opCount,
		curOp->getPoints(), curOp->getAccount().c_str(),
		firstop.c_str(), lastop.c_str());
  }
}

/* Log command */
bot->logAdminMessage("%s (%s) has requested OPLIST for %s",
		     theUser->getUserName().c_str(),
		     theClient->getRealNickUserHost().c_str(),
		     netChan->getName().c_str());

return;
}

}
