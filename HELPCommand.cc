/**
 * HELPCommand.cc
 *
 * 08/08/2005 - Jimmy Lipham <music0m@alltel.net>
 * Initial Version
 *
 * Shows help about <command> or gives a general list of commands
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

#include "chanfix.h"
#include "StringTokenizer.h"
#include "sqlUser.h"

RCSTAG("$Id$");

namespace gnuworld
{

void HELPCommand::Exec(iClient* theClient, sqlUser* theUser, const std::string& Message)
{
StringTokenizer st(Message);

if (st.size() < 2) {
  bot->SendTo(theClient, bot->getHelpMessage(theUser, "<INDEXOPER>"));

  if (theClient->getAccount() != "")
    bot->SendTo(theClient, bot->getHelpMessage(theUser, "<INDEXLOGGEDIN>"));

  if (!theUser) return;

  if (theUser->getFlag(sqlUser::F_SERVERADMIN))
    bot->SendTo(theClient, bot->getHelpMessage(theUser, "<INDEXSERVERADMIN>"));

  if (theUser->getFlag(sqlUser::F_BLOCK))
    bot->SendTo(theClient, bot->getHelpMessage(theUser, "<INDEXBLOCK>"));

  if (theUser->getFlag(sqlUser::F_CHANNEL))
    bot->SendTo(theClient, bot->getHelpMessage(theUser, "<INDEXCHANNEL>"));

  if (theUser->getFlag(sqlUser::F_CHANFIX))
    bot->SendTo(theClient, bot->getHelpMessage(theUser, "<INDEXCHANFIX>"));

  if (theUser->getFlag(sqlUser::F_OWNER))
    bot->SendTo(theClient, bot->getHelpMessage(theUser, "<INDEXOWNER>"));

  if (theUser->getFlag(sqlUser::F_USERMANAGER))
    bot->SendTo(theClient, bot->getHelpMessage(theUser, "<INDEXUSERADMIN>"));

} else {
  std::string msg = bot->getHelpMessage(theUser, string_upper(st.assemble(1)));

  if (!msg.empty())
    bot->SendFmtTo(theClient, msg);
  else
    bot->SendTo(theClient, "There is no help available for that topic.");

}

} //HELPCommand::Exec
} //namespace gnuworld
