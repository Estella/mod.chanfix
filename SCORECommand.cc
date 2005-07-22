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

#include <stdlib.h>
#include <iostream>
#include "gnuworld_config.h"
#include "Network.h"

#include "chanfix.h"
#include "levels.h" 
#include "StringTokenizer.h"
#include "sqlChannel.h"
#include "sqlChanOp.h"
#include "sqlUser.h"

RCSTAG("$Id$");

namespace gnuworld
{

using std::endl;
using std::string;
using std::stringstream;
using std::ends;

void SCORECommand::Exec(iClient* theClient, const std::string& Message)
{
StringTokenizer st(Message);
if (st.size() < 2) {
  Usage(theClient);
  return;
}

stringstream strRanks;
string scoreRanks;
unsigned int currentRank;
currentRank = 1;
sqlChanOp* curOp = 0;
iClient* curClient = 0;
bool compact = (string_upper(st[0]) == "CSCORE");

Channel* netChan = Network->findChannel(st[1]);
if (!netChan) {
  if (compact)
    bot->Notice(theClient, "~! %s", st[1].c_str());
  else
    bot->Notice(theClient, "No such channel %s.", st[1].c_str());
  return;
}

ChannelUser* curUser;
for (Channel::userIterator ptr = netChan->userList_begin();
     ptr != netChan->userList_end(); ptr++) {
  curUser = ptr->second;
  if (curUser->getClient()->getMode(iClient::MODE_SERVICES)) {
    if (compact)
      bot->Notice(theClient, "~! %s", netChan->getName().c_str());
    else
      bot->Notice(theClient, "%s is a registered channel.",
		  netChan->getName().c_str());
     return;
  }
}

chanfix::chanOpsType myOps = bot->getMyOps(netChan);
if (myOps.empty()) {
  if (compact)
    bot->Notice(theClient, "~! %s", netChan->getName().c_str());
  else
    bot->Notice(theClient, "There are no scores in the database for %s.",
	      netChan->getName().c_str());
  return;
}

string scUser;
if (st.size() > 2) {
  scUser = st[2];
  if (st[2][0] == '*') {
    //Account
    for (chanfix::chanOpsType::iterator opPtr = myOps.begin(); opPtr != myOps.end(); opPtr++) {
      curOp = *opPtr;
      if (string_lower(curOp->getAccount()) == string_lower(scUser.substr(1))) {
	if (compact)
	  bot->Notice(theClient, "~U %s %s %u", netChan->getName().c_str(), curOp->getAccount().c_str(), curOp->getPoints());
	else
	  bot->Notice(theClient, "Score for %s in channel %s: %u.", curOp->getAccount().c_str(), netChan->getName().c_str(), curOp->getPoints());
	return;
      }
    }
    if (compact)
      bot->Notice(theClient, "~! %s", netChan->getName().c_str());
    else
      bot->Notice(theClient, "There are no scores in the database for %s on channel %s", scUser.c_str(), netChan->getName().c_str());
    return;
  } else {
   //Nickname
   curClient = Network->findNick(scUser);
   if (!curClient) {
     if (compact)
       bot->Notice(theClient, "~U %s no@such.nick 0", netChan->getName().c_str());
     else
       bot->Notice(theClient, "No such nick %s.", st[2].c_str());
     return;
   } else {
     for (chanfix::chanOpsType::iterator opPtr = myOps.begin();
	  opPtr != myOps.end(); opPtr++) {
	curOp = *opPtr;
	curClient = bot->findAccount(curOp->getAccount(), netChan);
	if (string_lower(curClient->getNickName().c_str()) == string_lower(scUser)) {
	  //Score for "reed@local.host" in channel "#coder-com": 4.
	  //Do it like they do on OCF, baby.
	  if (compact) {
	    //~U #coder-com mc@local.host 176
	    bot->Notice(theClient, "~U %s %s %u",
			netChan->getName().c_str(),
			curOp->getAccount().c_str(),
			curOp->getPoints());
	  } else {
	    bot->Notice(theClient, "Score for %s (%s) in channel %s: %u.",
			scUser.c_str(),
			curOp->getAccount().c_str(),
			netChan->getName().c_str(),
			curOp->getPoints());
	  }
	  return;
	}
      }
      if (compact)
	bot->Notice(theClient, "~U %s no@such.nick 0", netChan->getName().c_str());
      else
	bot->Notice(theClient, "No such nick %s.", st[2].c_str());
      return;
    }
  }
}

/* Ok, now lets finally give it to em */
unsigned int minScoreReply;
if (bot->getNumTopScores() <= 0)
  minScoreReply = 10;
else
  minScoreReply = bot->getNumTopScores();

/* 3 different streams for 3 different output types */
stringstream strScoresDB; //For scores in the DB
stringstream strScoresOP; //For scores for users currently opped
stringstream strScoresNOP; //For scores for users NOT currently opped
unsigned int intDBCount = 0; //Rank counter for DB entries
unsigned int intOPCount = 0; //Rank counter for current OP entries
unsigned int intNOPCount = 0; //Rank counter for current non-ops
for (chanfix::chanOpsType::iterator opPtr = myOps.begin();
     opPtr != myOps.end(); opPtr++) {
  curOp = *opPtr;
  curClient = bot->findAccount(curOp->getAccount(), netChan);
  if (intDBCount < minScoreReply) {
    if (intDBCount++) {
      if (compact)
	strScoresDB << " ";
      else
	strScoresDB << ", ";
    }
    strScoresDB << curOp->getPoints();
  }
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
}

if (!compact) {
  strScoresDB << ".";
  strScoresOP << ".";
  strScoresNOP << ".";
}

if (compact) {
  bot->Notice(theClient, "~S %s %s", netChan->getName().c_str(), strScoresDB.str().c_str());
  bot->Notice(theClient, "~O %s %s", netChan->getName().c_str(), strScoresOP.str().c_str());
  bot->Notice(theClient, "~N %s %s", netChan->getName().c_str(), strScoresNOP.str().c_str());
} else {
  bot->Notice(theClient, "Top %u scores for channel %s in the database:", minScoreReply,
	      netChan->getName().c_str());
  if (strScoresDB.str() == ".")
    bot->Notice(theClient, "None.");
  else
    bot->Notice(theClient, strScoresDB.str());

  bot->Notice(theClient, "Top %u scores for ops in %s in the database:", minScoreReply,
	      netChan->getName().c_str());
  if (strScoresOP.str() == ".")
    bot->Notice(theClient, "None.");
  else
    bot->Notice(theClient, strScoresOP.str());
    
  bot->Notice(theClient, "Top %u scores for non-ops in %s in the database:", minScoreReply,
	      netChan->getName().c_str());
  if (strScoresNOP.str() == ".")
    bot->Notice(theClient, "None.");
  else
    bot->Notice(theClient, strScoresNOP.str());
}

return;
}
} // namespace gnuworld
