/**
 * STATUSCommand.cc
 *
 * 04/01/2004 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Display the status of chanfix
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

#include	"config.h"
#include	"StringTokenizer.h"

#include	"chanfix.h"
#include	"levels.h"
#include	"Network.h"

RCSTAG("$Id$");

namespace gnuworld
{

using std::string;
using namespace level;

void STATUSCommand::Exec(iClient* theClient, const string&)
{

bot->Notice(theClient, "[evilnet development's GNUWorld chanfix version %s]",
	    CF_VERSION);
bot->Notice(theClient, "Uptime: \002%s\002",
	    bot->prettyDuration(bot->getUplink()->getStartTime()).c_str());
bot->Notice(theClient, "Automatic fixing is: \002%s\002",
	    bot->doAutoFix() ? "ON" : "OFF");
bot->Notice(theClient, "Manual fixing is: \002%s\002",
	    bot->doChanFix() ? "ON" : "OFF");
bot->Notice(theClient, "Channel blocking is: \002%s\002",
	    bot->doChanBlocking() ? "ON" : "OFF");
bot->Notice(theClient, "Required amount of servers linked is %u%% of %u, which is a minimum of %u servers.",
	    bot->getMinServersPresent(), bot->getNumServers(),
	    ((bot->getMinServersPresent() * bot->getNumServers()) / 100 + 1));
if (bot->getState() == gnuworld::SPLIT) {
  bot->Notice(theClient, "Chanfix is currently disabled because only %i servers are linked.", 
	      Network->serverList_size());
} else {
  bot->Notice(theClient, "There are %i servers linked.",
	      Network->serverList_size());
}

return;
}

} // namespace gnuworld
