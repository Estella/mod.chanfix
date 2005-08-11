/**
 * flags.h
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

#ifndef __FLAGS_H
#define __FLAGS_H "$Id$"

namespace gnuworld {

 namespace flags {

  /* Block commands (+b) */
  const sqlUser::flagType block		= sqlUser::F_BLOCK;
  const sqlUser::flagType unblock	= sqlUser::F_BLOCK;

  /* Channel commands (+c) */
  const sqlUser::flagType addnote	= sqlUser::F_CHANNEL;
  const sqlUser::flagType delnote	= sqlUser::F_CHANNEL;
  const sqlUser::flagType alert		= sqlUser::F_CHANNEL;
  const sqlUser::flagType unalert	= sqlUser::F_CHANNEL;

  /* Chanfix commands (+f) */
  const sqlUser::flagType chanfix	= sqlUser::F_CHANFIX;
  const sqlUser::flagType oplist	= sqlUser::F_CHANFIX;
  const sqlUser::flagType opnicks	= sqlUser::F_CHANFIX;

  /* Owner commands (+o) */
  const sqlUser::flagType invite	= sqlUser::F_OWNER;
  const sqlUser::flagType quote		= sqlUser::F_OWNER;
  const sqlUser::flagType rehash	= sqlUser::F_OWNER;
  const sqlUser::flagType reload	= sqlUser::F_OWNER;
  const sqlUser::flagType set		= sqlUser::F_OWNER;
  const sqlUser::flagType shutdown	= sqlUser::F_OWNER;

  /* User management commands (+u) (+a for server admins)*/
  const sqlUser::flagType whois		= sqlUser::F_USERMANAGER;
  const sqlUser::flagType adduser	= sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN;
  const sqlUser::flagType deluser	= sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN;
  const sqlUser::flagType addhost	= sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN;
  const sqlUser::flagType delhost	= sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN;
  const sqlUser::flagType addserver	= sqlUser::F_USERMANAGER;
  const sqlUser::flagType delserver	= sqlUser::F_USERMANAGER;
  const sqlUser::flagType addflag	= sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN;
  const sqlUser::flagType delflag	= sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN;
  const sqlUser::flagType whoserver	= sqlUser::F_USERMANAGER | sqlUser::F_SERVERADMIN;

 } // namespace flags

} // namespace gnuworld

#endif /* __FLAGS_H */
