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

#include <stdlib.h>
#include <iostream>
#include <time.h>
#include "gnuworld_config.h"
#include "Network.h"

#include "chanfix.h"
#include "levels.h" 
#include "StringTokenizer.h"
#include "sqlChannel.h"
#include "sqlChanOp.h"

RCSTAG("$Id$");

namespace gnuworld
{

using std::string;

void OPLISTCommand::Exec(iClient* theClient, const std::string& Message)
{
StringTokenizer st(Message);
if (st.size() < 2) {
  Usage(theClient);
  return;
}

Channel* netChan = Network->findChannel(st[1]);
sqlChanOp* curOp = 0;
if (!netChan) {
  bot->Notice(theClient, "No such channel %s.", st[1].c_str());
  return;
}

chanfix::chanOpsType myOps = bot->getMyOps(netChan);
if (myOps.empty()) {
  bot->Notice(theClient, "There are no scores in the database for %s.",
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
  bot->Notice(theClient, "There are no scores in the database for %s.",
              netChan->getName().c_str());
  return;
}

if (oCnt == OPCOUNT)
  bot->Notice(theClient, "Top %d unique op accounts in channel %s:",
	      OPCOUNT, netChan->getName().c_str());
else
  bot->Notice(theClient, "Found %d unique op accounts in channel %s:",
	      oCnt, netChan->getName().c_str());

unsigned int opCount = 0;
static char sdatetimestring[24];
static char ldatetimestring[24];
struct tm * stms;
struct tm * stml;
time_t tmVars;
time_t tmVarl;

for (chanfix::chanOpsType::iterator opPtr = myOps.begin();
     opPtr != myOps.end(); opPtr++) {
  curOp = *opPtr;
  if (opCount < OPCOUNT) {
    opCount++;
    tmVars = curOp->getTimeFirstOpped();
    tmVarl = curOp->getTimeLastOpped();
    stms = localtime(&tmVars);
    strftime(sdatetimestring, 24, "%Y-%m-%d", stms);
    stml = localtime(&tmVarl);
    strftime(ldatetimestring, 24, "%Y-%m-%d %H:%M:%S", stml);

    bot->Notice(theClient, "%d. %4d %s -- %s / %s", opCount,
		curOp->getPoints(), curOp->getAccount().c_str(),
		sdatetimestring, ldatetimestring);
  }
}

/* Log command */
/* ... */

return;
}

}
