/**
 * LISTTEMPBLOCKEDCommand.cc
 *
 * 26/09/2006 - Neil Spierling <sirvulcan@gmail.com>
 * Initial Version
 *
 * Lists all temp blocked chans that chanfix has stored
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
 * $Id: LISTBLOCKEDCommand.cc 1553 2006-04-18 04:03:59Z sirvulcan $
 */

#include "gnuworld_config.h"
#include "Network.h"

#include "chanfix.h"
#include "responses.h"
#include "StringTokenizer.h"
#include "sqlChannel.h"
#include "sqlcfUser.h"

RCSTAG("$Id: LISTBLOCKEDCommand.cc 1553 2006-04-18 04:03:59Z sirvulcan $");

namespace gnuworld
{
namespace cf
{
void LISTTEMPBLOCKEDCommand::Exec(iClient* theClient, sqlcfUser* theUser, const std::string& Message)
{
int numBlocks = 0;

/* Check if channel blocking has been disabled in the config. */
if (!bot->doChanBlocking()) {
  bot->SendTo(theClient,
              bot->getResponse(theUser,
                              language::channel_blocking_disabled,
                              std::string("Channel blocking is disabled.")).c_str());
  return;
}

bot->SendTo(theClient,
	bot->getResponse(theUser,
		language::list_temp_blocked_chans,
		std::string("List of all temp blocked channels:")).c_str());


for (xNetwork::channelIterator cptr = Network->channels_begin(); cptr != Network->channels_end() ; ++cptr ) {
  Channel* thisChan = cptr->second ;
  bool isBlocked = bot->isTempBlocked(thisChan->getName());

  if (isBlocked) {
      bot->SendTo(theClient, thisChan->getName());
      numBlocks++;
  }
}

bot->SendTo(theClient,
  	bot->getResponse(theUser,
		language::list_total_temp_blocked,
		std::string("%d channels temp blocked.")).c_str(),
		numBlocks);

bot->logAdminMessage("%s (%s) LISTTEMPBLOCKED",
		     theUser->getUserName().c_str(),
		     theClient->getRealNickUserHost().c_str());

bot->logLastComMessage(theClient, Message);

return;
}
} // namespace cf
} // namespace gnuworld
