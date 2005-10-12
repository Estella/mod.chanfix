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
		/* CHANFIX */
		const int manual_fix_disabled		= 1;
		const int not_enough_servers		= 2;
		const int no_such_channel		= 3;
		const int cant_fix_oplevels		= 4;
		const int registered_channel		= 5;
		const int no_scores_for_chan		= 6;
		const int already_being_man_fixed	= 7;
		const int highscore_channel		= 9;
		const int channel_being_auto_fixed	= 9;
		const int channel_blocked		= 10;
		const int channel_has_notes		= 11;
		const int manual_chanfix_ack		= 12;

		/* USET */
		const int lang_set_to			= 13;
		const int send_notices			= 14;
		const int send_privmsgs			= 15;
		const int uset_notice_on_off		= 16;
		const int usetting_doesnt_exist		= 17;
	}
}

#endif
