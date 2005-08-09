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
 * $Id: HELPCommand.cc
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

void HELPCommand::Exec(iClient* theClient, const std::string& Message)
{
if (theClient->getAccount() == "" || !theClient->isOper()) return;
	
StringTokenizer st(Message);

if (st.size() < 2) {
  bot->Notice(theClient, "\002Oper Level:\002 SCORE CSCORE HISTORY INFO STATUS");
  bot->Notice(theClient, "\002Logged In:\002 WHO CHECK");

  sqlUser* theUser = bot->GetOper(theClient->getAccount());
  if (!theUser) return;

  if (theUser->hasFlag("o"))
    bot->Notice(theClient, "\002Owner (+o):\002 REHASH SET");

  if (theUser->hasFlag("b"))
    bot->Notice(theClient, "\002Blocker (+b):\002 BLOCK UNBLOCK");

  if (theUser->hasFlag("u"))
    bot->Notice(theClient, "\002User Manager (+u):\002 WHOIS ADDUSER DELUSER ADDHOST DELHOST ADDSERVER DELSERVER ADDFLAG DELFLAG WHOSERVER");

  if (theUser->hasFlag("f"))
    bot->Notice(theClient, "\002Chanfixer (+f):\002 CHANFIX OPLIST OPNICKS");

  if (theUser->hasFlag("c"))
    bot->Notice(theClient, "\002Channel (+c):\002 ADDNOTE DELNOTE ALERT UNALERT");

} else {
  bot->Notice(theClient, "No help available on that topic.");
}

} //helpcommand::exec
} //namespace gnuworld
