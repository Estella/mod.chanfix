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
 * $Id$
 */

#ifndef __RESPONSES_H
#define __RESPONSES_H "$Id$"

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

		/* ADDHOST */
		const int cant_add_host_diff_group	= 10;
		const int already_has_hostmask		= 11;
		const int failed_adding_hostmask	= 12;
		const int adding_hostmask_to_user	= 13;

		/* ADDNOTE */
		const int invalid_channel_name		= 14;
		const int note_recorded			= 15;

		/* ADDUSER */
		const int user_already_exists		= 16;
		const int created_user_w_host		= 17;
		const int created_user_wo_host		= 18;
		const int error_creating_user		= 19;

		/* ALERT */
		const int alert_already_set		= 20;
		const int alert_flag_added		= 21;

		/* BLOCK */
		const int channel_blocking_disabled	= 22;
		const int channel_already_blocked	= 23;
		const int channel_has_been_blocked	= 24;

		/* CHANFIX */
		const int manual_fix_disabled		= 25;
		const int not_enough_servers		= 26;
		const int no_such_channel		= 27;
		const int cant_fix_oplevels		= 28;
		const int registered_channel		= 29;
		const int no_scores_for_chan		= 30;
		const int already_being_man_fixed	= 31;
		const int highscore_channel		= 32;
		const int channel_being_auto_fixed	= 33;
		const int channel_blocked		= 34;
		const int channel_has_notes		= 35;
		const int manual_chanfix_ack		= 36;

		/* CHECK */
		const int check_results			= 37;

		/* DELFLAG */
		const int one_flag_per_delflag		= 38;
		const int cant_delete_owner_flag	= 39;
		const int only_owner_del_user_flag	= 40;
		const int cant_delete_flag_diff_group	= 41;
		const int cant_remove_block_flag	= 42;
		const int cant_remove_server_flag	= 43;
		const int user_does_not_have_flag	= 44;
		const int deleted_flag			= 45;

		/* DELHOST */
		const int cant_del_host_diff_group	= 46;
		const int user_doesnt_have_host		= 47;
		const int failed_deleting_host		= 48;
		const int deleted_hostmask		= 49;

		/* DELNOTE */
		const int no_entry_in_db		= 50;
		const int chan_has_no_notes		= 51;
		const int error_checking_noteid		= 52;
		const int no_note_with_id		= 53;
		const int no_note_id_for_chan		= 54;
		const int note_not_added_by_you		= 55;
		const int note_not_manually_added	= 56;
		const int note_deleted			= 57;

		/* DELUSER */
		const int cant_delete_an_owner		= 58;
		const int cant_delete_manager		= 59;
		const int cant_delete_from_diff_group	= 60;
		const int deleted_user			= 61;
		const int error_deleting_user		= 62;

		/* HISTORY */
		const int chan_no_manual_fixes		= 63;
		const int error_occured_notes		= 64;
		const int chan_manually_fix		= 65;
		const int chan_manual_fix		= 66;
		const int end_of_list			= 67;

		/* USET */
		const int lang_set_to			= 122;
		const int send_notices			= 123;
		const int send_privmsgs			= 124;
		const int uset_notice_on_off		= 125;
		const int usetting_doesnt_exist		= 126;

		/* chanfix.cc */
		const int no_commands_during_burst	= 127;
		const int unknown_command		= 128;
		const int access_suspended		= 129;
		const int host_not_matching		= 130;
		const int need_to_auth			= 131;
		const int requires_flag			= 132;
		const int requires_flags		= 133;
		const int syntax			= 134;
	}
}

#endif /* __RESPONSES_H */
