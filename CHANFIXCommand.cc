/**
 * CHANFIXCommand.cc
 *
 * 01/01/2004 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Manually fix a channel
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

#include "config.h"
#include "Network.h"

#include "chanfix.h"
#include "levels.h" 
#include "StringTokenizer.h"

RCSTAG("$Id$");

namespace gnuworld
{

using std::string;

void CHANFIXCommand::Exec(iClient* theClient, const string& Message)
{
StringTokenizer st(Message);
if (st.size() < 2) {
  Usage(theClient);
  return;
}

bool override = false;

if (st.size() > 2) {
  const string flag = string_upper(st[2]);
  if ((flag == "OVERRIDE") || (flag == "NOW") || (st[1][0] == '!'))
    override = true;
}

/* ??
 * if (st[1][0] == '!') {
 *  st[1]++;
 * }
 */

/* Check if manual chanfix has been disabled in the config. */
if (!bot->doChanFix()) {
  bot->Notice(theClient, "Sorry, manual chanfixes are currently disabled.");
  return;
}

/* If not enough servers are currently linked, bail out. */
if (bot->getState() != gnuworld::RUN) {
  bot->Notice(theClient, "Sorry, chanfix is currently disabled because not enough servers are linked.");
  return;
}

Channel* netChan = Network->findChannel(st[1]);
if (!netChan) {
  bot->Notice(theClient, "No such channel %s.", st[1].c_str());
  return;
}

/* Only allow chanfixes for channels that are in the database. */
chanfix::chanOpsType myOps = bot->getMyOps(netChan);
if (myOps.empty()) {
  bot->Notice(theClient, "There are no scores in the database for %s.",
	      st[1].c_str());
  return;
}

sqlChannel* theChan = bot->getChannelRecord(st[1]);
if (!theChan) theChan = bot->newChannelRecord(st[1]);

/* Don't fix a channel being chanfixed. */
if (bot->isBeingChanFixed(netChan)) {
  bot->Notice(theClient, "The channel %s is already being manually fixed.",
	      netChan->getName().c_str());
  return;
}

/* Check if the highest score is high enough for a fix. */
if (myOps.begin() != myOps.end())
  theChan->setMaxScore((*myOps.begin())->getPoints());

if (theChan->getMaxScore() <= (int)(FIX_MIN_ABS_SCORE_END * MAX_SCORE)) {
  bot->Notice(theClient, "The highscore in channel %s is %d which is lower than the minimum score required (%.3f * %d = %d).",
	      theChan->getChannel().c_str(), theChan->getMaxScore(),
	      FIX_MIN_ABS_SCORE_END, MAX_SCORE,
	      (int)(FIX_MIN_ABS_SCORE_END * MAX_SCORE));
  return;
}

/* Don't fix a channel being autofixed without OVERRIDE flag. */
if (bot->isBeingAutoFixed(netChan)) {
  if (!override) {
    bot->Notice(theClient, "The channel %s is being automatically fixed. Append the OVERRIDE flag to force a manual fix.",
		netChan->getName().c_str());
    return;
  } else {
    /* We're going to manually fix this instead of autofixing it,
     * so remove this channel from the autofix queue. */
    /* ... */
  }
}

/* Don't fix a blocked channel. */
if (theChan->getFlag(sqlChannel::F_BLOCKED)) {
  bot->Notice(theClient, "The channel %s is BLOCKED.", 
	      theChan->getChannel().c_str());
  return;
}

/* Don't fix an alerted channel without the OVERRIDE flag. */
/* ... */

/* Fix the channel */
bot->manualFix(netChan);

/* Add note to the channel about this manual fix */
/* bot->addNote(theChan, ... "CHANFIX by %s" */

/* Log the chanfix */
bot->Notice(theClient, "Manual chanfix acknowledged for %s",
	    netChan->getName().c_str());

return;
}

} // namespace gnuworld
