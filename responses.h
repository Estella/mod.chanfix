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
		/* ADDFLAG */
		const int one_flag_per_addflag		= 1;
		const int no_such_user			= 2;
		const int cant_add_owner_flag		= 3;
		const int user_man_add_owner_only	= 4;
		const int cant_add_flag_diff_group	= 5;
		const int cant_add_block_flag		= 6;
		const int cant_add_serveradmin_flag	= 7;
		const int user_already_has_flag		= 8;
		const int added_flag_to_user		= 9;

		/* CHANFIX */
		const int manual_fix_disabled		= 10;
		const int not_enough_servers		= 11;
		const int no_such_channel		= 12;
		const int cant_fix_oplevels		= 13;
		const int registered_channel		= 14;
		const int no_scores_for_chan		= 15;
		const int already_being_man_fixed	= 16;
		const int highscore_channel		= 17;
		const int channel_being_auto_fixed	= 18;
		const int channel_blocked		= 19;
		const int channel_has_notes		= 20;
		const int manual_chanfix_ack		= 21;

		/* USET */
		const int lang_set_to			= 22;
		const int send_notices			= 23;
		const int send_privmsgs			= 24;
		const int uset_notice_on_off		= 25;
		const int usetting_doesnt_exist		= 26;

		/* chanfix.cc */
		const int no_commands_during_burst	= 27;
		const int unknown_command		= 28;
		const int access_suspended		= 29;
		const int host_not_matching		= 30;
		const int need_to_auth			= 31;
		const int requires_flag			= 32;
		const int requires_flags		= 33;
		const int syntax			= 34;
	}
}

#endif
