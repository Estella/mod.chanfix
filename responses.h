/**
 * responses.h
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
 * $Id: responses.h,v 1.17 2003/06/28 01:21:20 dan_karrels Exp $
 */

#ifndef __RESPONSES_H
#define __RESPONSES_H "$Id: responses.h,v 1.17 2003/06/28 01:21:20 dan_karrels Exp $"

namespace gnuworld
	{
	namespace language
	{
		const int lang_set_to		= 1;
		const int send_notices		= 2;
		const int send_privmsgs		= 3;
		const int uset_notice_on_off	= 4;
		const int usetting_doesnt_exist	= 5;
	}
}

#endif
