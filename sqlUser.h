/**
 * sqlUser.h
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

#ifndef __SQLUSER_H
#define __SQLUSER_H "$Id$"

#include	<string>
#include	<vector>
#include	<ctime>
#include	"libpq++.h"

namespace gnuworld
{

class iClient;

class sqlUser
{

public:

	sqlUser(PgDatabase*);
	virtual ~sqlUser();

	/* Accessors */

	inline const unsigned int&	getID() const
		{ return Id ; }

	inline const std::string& getUserName() const
		{ return user_name; }

	inline const int getCreated() const
		{ return created; }

	inline const unsigned int getLastSeen() const
		{ return last_seen; }

	inline const std::string& getLastUpdatedBy() const
		{ return last_updated_by; }

	inline const unsigned int getLastUpdated() const
		{ return last_updated; }
		
	inline const std::string& getFlags() const
		{
			std::string _flagList;
			if (isServAdmin)
				_flagList += "a";
			
			if (canBlock)
				_flagList += "b";
			
			if (canAlert)
				_flagList += "c";
			
			if (canChanfix)
				_flagList += "f";
			
			if (isOwner)
				_flagList += "o";
			
			if (canManageUsers)
				_flagList += "u";
			
			return _flagList;
		}
		
	inline bool hasFlag(const std::string& _flag) const
		{
		/*
			isServAdmin - a
			isBlocker - b
			isAlerter - c
			isChanfixer - f
			isOwner - o
			isUserMan - u
		*/
			if (isOwner)
				return true;
			else if (_flag == "a" && isServAdmin)
				return true;
			else if (_flag == "b" && canBlock)
				return true;
			else if (_flag == "c" && canAlert)
				return true;
			else if (_flag == "f" && canChanfix)
				return true;
			else if (_flag == "u" && canManageUsers)
				return true;
			else
				return false;
		}

	/* Mutators */

	inline void setUserName(const std::string& _user_name)
		{ user_name = _user_name; }

	inline void setCreated(const unsigned int _created)
		{ created = _created; }

	inline void setLastSeen(const unsigned int _last_seen)
		{ last_seen = _last_seen; }

	inline void setLastUpdatedBy(const std::string& _last_updated_by)
		{ last_updated_by = _last_updated_by; }

	inline void setLastUpdated(const unsigned int _last_updated)
		{ last_updated = _last_updated; }

	inline void addFlag(const std::string& _flag)
		{
			if (_flag == "a")
				isServAdmin = 1;
			else if (_flag == "b")
				canBlock = 1;
			else if (_flag == "c")
				canAlert = 1;
			else if (_flag == "f")
				canChanfix = 1;
			else if (_flag == "o")
				isOwner = 1;
			else if (_flag == "u")
				canManageUsers = 1;
		}
		
	inline void delFlag(const std::string& _flag)
		{
			if (_flag == "a")
				isServAdmin = 0;
			else if (_flag == "b")
				canBlock = 0;
			else if (_flag == "c")
				canAlert = 0;
			else if (_flag == "f")
				canChanfix = 0;
			else if (_flag == "o")
				isOwner = 0;
			else if (_flag == "u")
				canManageUsers = 0;
		}
		
	/* Methods to alter our SQL status */
	void setAllMembers(int);
	bool commit();
	bool Insert();
	bool Delete();

private:

	PgDatabase*	SQLDb;
	unsigned int	Id;
	std::string	user_name;
	unsigned int	created;
	unsigned int	last_seen;
	unsigned int	last_updated;
	std::string	last_updated_by;
	bool		isServAdmin;
	bool		canBlock;
	bool		canAlert;
	bool		canChanfix;
	bool		isOwner;
	bool		canManageUsers;
	bool		isSuspended;
	bool		useNotice;
	bool		atob( std::string str )
			{
				if(str == "y" || str == "true" || str == "yes") return true;
				return false;
			}
	std::string	flags;
}; // class sqlUser

} // namespace gnuworld

#endif // __SQLUSER_H
