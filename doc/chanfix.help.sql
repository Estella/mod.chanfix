-- Help information.
--
-- $Id$

DELETE FROM help;

COPY "help" FROM stdin;
ADDFLAG	1	Adds this flag to the user. Possible flags:\nb - can block/unblock channels\nf - can perform manual chanfix\nu - can manage users\nNote: add only a single flag per command.
ADDHOST	1	Adds this hostmask to the user's list of hostmasks.
ADDNOTE	1	Adds a note to a channel.
ADDUSER	1	Adds a new user, without flags, and optionally with this hostmask.
ALERT	1	Puts a channel on ALERT status\nChannels in alert status have information contained in notes that should be considered before fixing the channel. These channels also require the OVERRIDE flag to manually fix them.
BLOCK	1	Blocks a channel from being fixed, both automatically and manually.\nThe reason will be shown when doing INFO <channel>.
CHECK	1	Shows the number of ops and total clients in <channel>.
CSCORE	1	Shows the same as SCORE, but in a compact output. See /msg C HELP SCORE.
CHANFIX	1	Performs a manual fix on <channel>. Append OVERRIDE, YES or an exclamation mark (!) to force this manual fix.
DELFLAG	1	Removes this flag from the user. See /msg C HELP ADDFLAG.
DELHOST	1	Deletes this hostmask from the user's list of hostmasks.
DELNOTE	1	Deletes the note with this note_id from the channel. You can only delete notes you added yourself.
DELUSER	1	Deletes this user.
QUOTE	1	This command should never be used.\nUsing it would be bad.\nHow bad?\nTry to imagine all life stopping instantaniously, and every molecule in your body exploding at the speed of light.
HELP	1	Shows help about <command>.
HISTORY	1	Shows the times that <channel> has been manually fixed.
INFO	1	Shows all notes of this channel, and whether it has been blocked.
OPLIST	1	Shows a list of accounts plus their score of the top ops of this channel.
OPNICKS	1	Shows the nicks of all ops in <channel>.
REHASH	1	Reloads config and users.
RELOAD	1	Completely reloads Chanfix.
SCORE	1	Without extra arguments, shows the top scores of <channel>.\nOtherwise, it shows the score of either the currently online client <nick>, or <*account> for <channel>.
SET	1	Sets <setting> to value <value>.\nBoolean settings: ENABLE_AUTOFIX, ENABLE_CHANFIX, ENABLE_CHANNEL_BLOCKING.\nInteger settings: NUM_SERVERS.
STATUS	1	Shows current status.
SUSPEND	1	Suspends the specified user.
UNALERT	1	Removes a channel from alert status
UNBLOCK	1	Removes the block on a channel.
USET	1	Sets options on your account. Possible settings:\nNOTICE - receive notices, if no then PRIVMSG's will be sent.\nLANG - What language Chanfix will communicate with you in; available languages: EN - English.
WHOIS	1	Shows information about this user.
SETGROUP	1	Sets the group for the specified user.
SHUTDOWN	1	Shuts down Chanfix.
UNSUSPEND	1	UnSuspends the specified user.
WHOGROUP	1	Shows all users in the specified group along with their flags.
<INDEXOPER>	1	\002Oper Level\002: SCORE CSCORE HISTORY INFO STATUS
<INDEXLOGGEDIN>	1	\002Logged In\002: CHECK LISTHOSTS USET WHOIS
<INDEXBLOCK>	1	\002Blocker (+b)\002: BLOCK UNBLOCK
<INDEXCOMMENT>	1	\002Commenter (+c)\002: ADDNOTE DELNOTE ALERT UNALERT
<INDEXCHANFIX>	1	\002Chanfixer (+f)\002: CHANFIX OPLIST OPNICKS
<INDEXOWNER>	1	\002Owner (+o)\002: QUOTE REHASH RELOAD SET SHUTDOWN
<INDEXSERVERADMIN>	1	\002Server Admin (+a)\002: ADDUSER DELUSER ADDHOST DELHOST ADDFLAG DELFLAG WHOGROUP SUSPEND UNSUSPEND
<INDEXUSERADMIN>	1	\002User Manager (+u)\002: ADDUSER DELUSER ADDHOST DELHOST ADDFLAG DELFLAG SETGROUP WHOGROUP SUSPEND UNSUSPEND
\.
