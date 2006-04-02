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
#include "responses.h"
#include "StringTokenizer.h"
#include "sqlChannel.h"
#include "sqlChanOp.h"

RCSTAG("$Id$");

namespace gnuworld
{
namespace cf
{

void OPLISTCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

bool all = false;
bool dots = false;
bool dotscolor = false;

unsigned int pos = 2;
while(pos < st.size()) {
  if (!strcasecmp(st[pos],"-all"))
    all = true;

  if (!strcasecmp(st[pos],"all"))
    all = true;

  if (!strcasecmp(st[pos],"!"))
    all = true;

  if (!strcasecmp(st[pos],"-dots"))
    dots = true;

  if (!strcasecmp(st[pos],"-dotscolor")) {
    dotscolor = true;
    dots = true;
  }

  pos++;
}

sqlChanOp* curOp = 0;

chanfix::chanOpsType myOps = bot->getMyOps(st[1]);
if (myOps.empty()) {
  bot->SendTo(theClient,
              bot->getResponse(theUser,
                              language::no_scores_for_chan,
                              std::string("There are no scores in the database for %s.")).c_str(),
                                          st[1].c_str());
  return;
}

unsigned int oCnt = myOps.size();

/**
 * Technically, if there are all 0 scores, it will get to this point.
 * We don't want a notice saying 0 accounts.
 */
if (oCnt == 0) {
  bot->SendTo(theClient,
              bot->getResponse(theUser,
                              language::no_scores_for_chan,
                              std::string("There are no scores in the database for %s.")).c_str(),
	      st[1].c_str());
  return;
}

if (oCnt > OPCOUNT && !all)
  bot->SendTo(theClient,
              bot->getResponse(theUser,
                              language::top_unique_op_accounts,
                              std::string("Top %d unique op accounts in channel %s:")).c_str(),
                                          OPCOUNT, st[1].c_str());
else
  bot->SendTo(theClient,
              bot->getResponse(theUser,
                              language::found_unique_op_accounts,
                              std::string("Found %d unique op accounts in channel %s:")).c_str(),
                                          oCnt, st[1].c_str());

bot->SendTo(theClient,
            bot->getResponse(theUser,
                            language::rank_score_acc_header,
			     std::string("Rank Score Account -- Time first opped / Time last opped / Nick")).c_str());

unsigned int opCount = 0;
bool inChan = false;
std::string firstop;
std::string lastop;
std::string nickName = "";
std::string dotStr;
std::stringstream dotString;
int cScore;
for (chanfix::chanOpsType::iterator opPtr = myOps.begin();
     opPtr != myOps.end() && (all || opCount < OPCOUNT); opPtr++) {
  curOp = *opPtr;
  opCount++;
  firstop = bot->tsToDateTime(curOp->getTimeFirstOpped(), false);
  lastop = bot->tsToDateTime(curOp->getTimeLastOpped(), true);
  inChan = bot->accountIsOnChan(st[1], curOp->getAccount());
  if (inChan)
    nickName = bot->getChanNickName(st[1], curOp->getAccount());

  if (dots || dotscolor) {
    dotString.str("");

    for (int i = 0; i < DAYSAMPLES; i++) {
      cScore = curOp->getDay(i);

      if (dotscolor) {
        if ((cScore > 0) && (cScore < 40))
          dotString << "\0032,2" << (cScore ? "*" : ".") << "\003"; // BLUE 1-39
        else if ((cScore >= 40) && (cScore < 80))
          dotString << "\0033,3" << (cScore ? "*" : ".") << "\003"; // GREEN 40-79
        else if ((cScore >= 80) && (cScore < 120))
          dotString << "\0034,4" << (cScore ? "*" : ".") << "\003"; // RED 80-119
        else if ((cScore >= 120) && (cScore < 160))
          dotString << "\0035,5" << (cScore ? "*" : ".") << "\003"; // BROWN 120-159
        else if ((cScore >= 160) && (cScore < 200))
          dotString << "\0036,6" << (cScore ? "*" : ".") << "\003"; // PURPLE 160-199
        else if ((cScore >= 200) && (cScore < 240))
          dotString << "\0037,7" << (cScore ? "*" : ".") << "\003"; // ORANGE 200-240
        else if ((cScore >= 240) && (cScore < 280))
          dotString << "\0038,8" << (cScore ? "*" : ".") << "\003"; // YELLOW 240-280
        else if ((cScore >= 280) && (cScore < 300))
          dotString << "\0039,9" << (cScore ? "*" : ".") << "\003"; // NEON GREEN 280-300 (MAX 288)
        else
          dotString << "\0030,0" << (cScore ? "*" : ".") << "\003"; // WHITE 0
      } else {
        dotString << (cScore ? "*" : ".");
      }
    }

    dotString << std::ends;
    dotStr = dotString.str();
  }

  bot->SendTo(theClient, "%3d. %4s%4d  %15s -- %s / %s%s%s%s %s%s%s", opCount,
	      inChan ? "\002" : "", curOp->getPoints(),
	      curOp->getAccount().c_str(), firstop.c_str(),
	      lastop.c_str(), inChan ? " / " : "",
	      inChan ? nickName.c_str() : "", inChan ? "\002" : "",
              (dots || dotscolor) ? "[" : "",
              (dots || dotscolor) ? dotStr.c_str() : "",
              (dots || dotscolor) ? "]" : "");


}

bot->logAdminMessage("%s (%s) OPLIST %s %s",
		     theUser ? theUser->getUserName().c_str() : "!NOT-LOGGED-IN!",
		     theClient->getRealNickUserHost().c_str(),
		     st[1].c_str(), all ? "ALL" : "");

return;
}

} //namespace cf
} //namespace gnuworld
