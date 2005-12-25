/**
 * SCORECommand.cc
 *
 * 07/15/2005 - Jimmy Lipham <music0m@alltel.net>
 * Initial Version
 *
 * Shows the top scores or an individual score of <channel>
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

#include <sstream>
#include <vector>

#include "gnuworld_config.h"
#include "Network.h"

#include "chanfix.h"
#include "responses.h"
#include "StringTokenizer.h"
#include "sqlChannel.h"
#include "sqlChanOp.h"
#include "sqlUser.h"

RCSTAG("$Id$");

namespace gnuworld
{

void SCORECommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

std::stringstream strRanks;
std::string scoreRanks;
unsigned int currentRank;
currentRank = 1;
sqlChanOp* curOp = 0;
iClient* curClient = 0;
chanfix::acctListType acctToScore;
chanfix::acctListType acctToShow;
bool compact = (string_upper(st[0]) == "CSCORE");
Channel* netChan = Network->findChannel(st[1]);
if (!netChan) {
  if (compact)
    bot->SendTo(theClient, "~! %s", st[1].c_str());
  else
    bot->SendTo(theClient,
                bot->getResponse(theUser,
                                language::no_such_channel,
                                std::string("No such channel %s.")).c_str(), st[1].c_str());
  return;
}
if (!bot->canScoreChan(netChan, false)) {
  if (compact)
    bot->SendTo(theClient, "~! %s", netChan->getName().c_str());
  else
    bot->SendTo(theClient,
		bot->getResponse(theUser,
				 language::registered_channel,
				 std::string("%s is a registered channel.")).c_str(),
				 netChan->getName().c_str());
  return;
}
chanfix::chanOpsType myOps = bot->getMyOps(netChan);
if (myOps.empty()) {
  if (compact)
    bot->SendTo(theClient, "~! %s", netChan->getName().c_str());
  else
    bot->SendTo(theClient,
                bot->getResponse(theUser,
                                language::no_scores_for_chan,
                                std::string("There are no scores in the database for %s.")).c_str(),
                                            netChan->getName().c_str());
  return;
}
if (st.size() > 2) {
  const char* scUser = st[2].c_str();
  if (st[2][0] == '*') {
    //Account
    ++scUser;
    for (chanfix::chanOpsType::iterator opPtr = myOps.begin();
	 opPtr != myOps.end(); opPtr++) {
      curOp = *opPtr;
      if (string_lower(curOp->getAccount()) == string_lower(scUser)) {
	if (compact)
	  bot->SendTo(theClient, "~U %s %s %u", netChan->getName().c_str(), curOp->getAccount().c_str(), curOp->getPoints());
	else
	  bot->SendTo(theClient,
		bot->getResponse(theUser,
			language::score_for_channel_account,
			std::string("Score for account %s in channel %s: %u.")).c_str(),
				curOp->getAccount().c_str(),
				netChan->getName().c_str(),
				curOp->getPoints());
	
	return;
      }
    }
    if (compact)
      bot->SendTo(theClient,
		  bot->getResponse(theUser,
				   language::compact_no_such_account,
				   std::string("~U %s no@such.account 0")).c_str(), netChan->getName().c_str());
    else
      bot->SendTo(theClient,
		  bot->getResponse(theUser,
				   language::no_such_account,
				   std::string("No such account %s.")).c_str(), scUser);
    return;
  } else {
   //Nickname
   curClient = Network->findNick(scUser);
   if (!curClient) {
     if (compact)
       bot->SendTo(theClient,
                   bot->getResponse(theUser,
                                   language::compact_no_such_nick,
                                   std::string("~U %s no@such.nick 0")).c_str(), netChan->getName().c_str());
     else
       bot->SendTo(theClient,
                   bot->getResponse(theUser,
                                   language::no_such_nick,
                                   std::string("No such nick %s.")).c_str(), scUser);
     return;
   } else {
     iClient* curClientOp;
     for (chanfix::chanOpsType::iterator opPtr = myOps.begin();
	  opPtr != myOps.end(); opPtr++) {
	curOp = *opPtr;
	acctToScore = bot->findAccount(netChan, curOp->getAccount());
	std::vector< iClient* >::const_iterator acctPtr = acctToScore.begin();
	if (acctPtr == acctToScore.end()) {
	  if (compact)
            bot->SendTo(theClient,
                        bot->getResponse(theUser,
                                        language::compact_no_such_nick,
                                        std::string("~U %s no@such.nick 0")).c_str(), netChan->getName().c_str());
	  else
            bot->SendTo(theClient,
                        bot->getResponse(theUser,
                                        language::no_such_nick,
                                        std::string("No such nick %s.")).c_str(), scUser);
	  return;
	}
	while (acctPtr != acctToScore.end()) {
		curClientOp = *acctPtr;
		if (curClientOp && (string_lower(curClientOp->getNickName()) == string_lower(scUser)))
		  break;
		++acctPtr;
	}
	if (curClientOp && (string_lower(curClientOp->getNickName()) == string_lower(scUser))) {
	  //Score for "reed@local.host" in channel "#coder-com": 4.
	  //Do it like they do on OCF, baby.
	  if (compact) {
	    //~U #coder-com mc@local.host 176
	    bot->SendTo(theClient, "~U %s %s %u",
			netChan->getName().c_str(),
			curOp->getAccount().c_str(),
			curOp->getPoints());
	  } else {
            bot->SendTo(theClient,
                        bot->getResponse(theUser,
                                        language::score_for_channel_nick,
                                        std::string("Score for %s (%s) in channel %s: %u.")).c_str(),
                                                    curClientOp->getNickName().c_str(),
                                                    curOp->getAccount().c_str(),
                                                    netChan->getName().c_str(),
                                                    curOp->getPoints());
	  }
	  return;
	}
      }
      acctToScore.clear();
      if (compact)
        bot->SendTo(theClient,
                    bot->getResponse(theUser,
                                    language::compact_no_such_nick,
                                    std::string("~U %s no@such.nick 0")).c_str(), netChan->getName().c_str());
      else
        bot->SendTo(theClient,
                    bot->getResponse(theUser,
                                    language::no_such_nick,
                                    std::string("No such nick %s.")).c_str(), scUser);
      return;
    } // for
  } //else
} //else

/* Ok, now lets finally give it to em */
unsigned int minScoreReply;
if (bot->getNumTopScores() <= 0)
  minScoreReply = 10;
else
  minScoreReply = bot->getNumTopScores();

/* 3 different streams for 3 different output types */
std::stringstream strScoresDB; //For scores in the DB
std::stringstream strScoresOP; //For scores for users currently opped
std::stringstream strScoresNOP; //For scores for users NOT currently opped
unsigned int intDBCount = 0; //Rank counter for DB entries
unsigned int intOPCount = 0; //Rank counter for current OP entries
unsigned int intNOPCount = 0; //Rank counter for current non-ops
for (chanfix::chanOpsType::iterator opPtr = myOps.begin();
     opPtr != myOps.end(); opPtr++) {
  curOp = *opPtr;
  
  if (intDBCount < minScoreReply) {
    if (intDBCount++) {
      if (compact)
	strScoresDB << " ";
      else
	strScoresDB << ", ";
    }
    strScoresDB << curOp->getPoints();
  }
  
  acctToShow = bot->findAccount(netChan, curOp->getAccount());
  std::vector< iClient* >::const_iterator acctPtr = acctToShow.begin();
  if (acctPtr == acctToShow.end())
    continue;
  
  curClient = *acctPtr;
  if (curClient && netChan->findUser(curClient)->isModeO()) {
    if (intOPCount < minScoreReply) {
      if (intOPCount++) {
	if (compact)
	  strScoresOP << " ";
	else
	  strScoresOP << ", ";
      }
      strScoresOP << curOp->getPoints();
    }
  } else {
    if (!curClient)
      continue;
    if (intNOPCount < minScoreReply) {
      if (intNOPCount++) {
	if (compact)
	  strScoresNOP << " ";
	else
	  strScoresNOP << ", ";
      }
      strScoresNOP << curOp->getPoints();
    }
  }
  currentRank++;
  acctToShow.clear();
}

if (!compact) {
  strScoresDB << ".";
  strScoresOP << ".";
  strScoresNOP << ".";
}

if (compact) {
  bot->SendTo(theClient, "~S %s %s", netChan->getName().c_str(), strScoresDB.str().c_str());
  bot->SendTo(theClient, "~O %s %s", netChan->getName().c_str(), strScoresOP.str().c_str());
  bot->SendTo(theClient, "~N %s %s", netChan->getName().c_str(), strScoresNOP.str().c_str());
} else {
  bot->SendTo(theClient,
              bot->getResponse(theUser,
                              language::top_scores_for_chan,
                              std::string("Top %u scores for channel %s in the database:")).c_str(),
                                          minScoreReply, netChan->getName().c_str());
  if (strScoresDB.str() == ".")
    bot->SendTo(theClient,
                bot->getResponse(theUser,
                                language::score_none,
                                std::string("None.")).c_str());
  else
    bot->SendTo(theClient, strScoresDB.str());

  bot->SendTo(theClient,
              bot->getResponse(theUser,
                              language::top_scores_for_ops,
                              std::string("Top %u scores for ops in %s in the database:")).c_str(),
                                          minScoreReply, netChan->getName().c_str());
  if (strScoresOP.str() == ".")
    bot->SendTo(theClient,
                bot->getResponse(theUser,
                                language::score_none,
                                std::string("None.")).c_str());
  else
    bot->SendTo(theClient, strScoresOP.str());

  bot->SendTo(theClient,
              bot->getResponse(theUser,
                              language::top_scores_for_nonops,
                              std::string("Top %u scores for non-ops in %s in the database:")).c_str(),
                                          minScoreReply, netChan->getName().c_str());
  if (strScoresNOP.str() == ".")
    bot->SendTo(theClient,
                bot->getResponse(theUser,
                                language::score_none,
                                std::string("None.")).c_str());
  else
    bot->SendTo(theClient, strScoresNOP.str());
}

return;
}
} // namespace gnuworld
