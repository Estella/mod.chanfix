/**
 * RELOADCommand.cc
 *
 * 18/12/2003 - Reed Loden <reed@reedloden.com>
 * Initial Version
 *
 * Unloads the bot's module and reloads it
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

RCSTAG("$Id$");

namespace gnuworld
{

using std::string;
using namespace level;

bool RELOADCommand::Exec(iClient* theClient, const string& Message)
{

StringTokenizer st(Message);

Notice(theClient, "Reloading client...see you on the flip side");

if (st.size() < 2) {
  MyUplink->UnloadClient(this, "Reloading...");
} else {
  MyUplink->UnloadClient(this, st.assemble(1));
}

MyUplink->LoadClient("libchanfix", getConfigFileName());

return true;
}

} // namespace gnuworld
