/**
 * chanfixCommands.h
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

#ifndef __CHANFIXCOMMANDS_H
#define __CHANFIXCOMMANDS_H "$Id$"

namespace gnuworld {

class chanfix;

class Command {
public:
	Command( chanfix* _bot, const std::string& _commandName, const std::string& _help ) :
		bot(_bot), server(0), commandName(_commandName), help(_help)
		{ }

	virtual ~Command() { }
	
	virtual void Exec(iClient*, const std::string&) = 0;
	
	void setServer(xServer *_server)
		{ server = _server; }
	
	virtual inline std::string getInfo() const
		{ return commandName + ' ' + help; }
	
	virtual void Usage(iClient*);
	
	inline const std::string& getName() const
		{ return commandName; }
	
	inline const std::string& getHelp() const
		{ return help; }
	
protected:
	chanfix		*bot;
	xServer		*server;
	std::string		commandName;
	std::string		help;

};

#define DECLARE_COMMAND(commandName) \
class commandName##Command : public Command \
{ \
  public: \
    commandName##Command(chanfix* _bot, \
      const std::string& _commandName, \
      const std::string& _help) : \
        Command(_bot, _commandName, _help) {} \
    virtual ~commandName##Command() {} \
    virtual void Exec(iClient*, const std::string&); \
};

/* Oper commands */
DECLARE_COMMAND( CHANFIX )
DECLARE_COMMAND( STATUS )

/* Coder commands */
DECLARE_COMMAND( INVITE )
DECLARE_COMMAND( QUOTE )
DECLARE_COMMAND( RELOAD )
DECLARE_COMMAND( SHUTDOWN )

} // namespace gnuworld

#endif
