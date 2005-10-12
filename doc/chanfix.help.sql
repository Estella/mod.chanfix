-- Help information.
--
-- $Id: cservice.help.sql,v 1.3 2002/08/30 10:18:44 nighty Exp $

DELETE FROM help;

COPY "help" FROM stdin;
ADDFLAG	1	/msg C ADDFLAG <user> <flag>\nAdds this flag to the user. Possible flags:\nb - can block/unblock channels\nf - can perform manual chanfix\nu - can manage users\nNote: add only a single flag per command.\nThis command requires one of these flags: ua.
ADDHOST	1	/msg C ADDHOST <user> <hostmask>\nAdds this hostmask to the user's list of hostmasks.\nThis command requires one of these flags: ua.
ADDNOTE	1	/msg C ADDNOTE <channel> <note>\nAdds a note to a channel.\nThis command requires flag c.
ADDUSER	1	/msg C ADDUSER <user> [hostmask]\nAdds a new user, without flags, and optionally with this hostmask.\nThis command requires one of these flags: ua.
BLOCK	1	/msg C BLOCK <channel> <reason>\nBlocks a channel from being fixed, both automatically and manually.\nThe reason will be shown when doing INFO <channel>.\nThis command requires flag b.
CHECK	1	/msg C CHECK <channel>\nShows the number of ops and total clients in <channel>.
CSCORE	1	/msg C CSCORE <channel> [nick|hostmask]\nShows the same as SCORE, but in a compact output. See /msg C HELP SCORE.
CHANFIX	1	/msg C CHANFIX <channel> [override]\nPerforms a manual fix on <channel>. Append OVERRIDE, YES or an exclamation mark (!) to force this manual fix.\nThis command requires flag f.
DELFLAG	1	/msg C DELFLAG<user> <flag>\nRemoves this flag from the user. See /msg C HELP ADDFLAG.\nThis command requires one of these flags: ua.
DELHOST	1	/msg C DELHOST <user> <hostmask>\nDeletes this hostmask from the user's list of hostmasks.\nThis command requires one of these flags: ua.
DELNOTE	1	/msg C DELNOTE <channel> <note_id>\nDeletes the note with this note_id from the channel. You can only delete notes you added yourself.\nThis command requires flag c.
DELUSER	1	/msg C DELUSER <user>\nDeletes this user.\nThis command requires one of these flags: ua.
QUOTE	1	This command should never be used.\nUsing it would be bad.\nHow bad?\nTry to imagine all life stopping instantaniously, and every molecule in your body exploding at the speed of light.
HELP	1	/msg C HELP <command>\nShows help about <command>.
HISTORY	1	/msg C HISTORY <channel>\nShows the times that <channel> has been manually fixed.
INFO	1	/msg C INFO <channel>\nShows all notes of this channel, and whether it has been blocked.
OPLIST	1	/msg C OPLIST <channel>\nShows a list of hostmasks plus their score of the top ops of this channel.\nThis command requires flag f.
OPNICKS	1	/msg C OPNICKS <channel>\nShows the nicks of all ops in <channel>.\nThis command requires flag f.
REHASH	1	/msg C REHASH\nReloads config and users.\nThis command requires flag o.
RELOAD	1	/msg C RELOAD\nCompletely reloads Chanfix.\nThis command requires flag o.
SCORE	1	/msg C SCORE <channel> [nick|hostmask]\nWithout extra arguments, shows the top scores of <channel>.\nOtherwise, it shows the score of either the currently online client <nick>, or <hostmask> for <channel>.
SET	1	/msg C SET <setting> <value>\nSets <setting> to value <value>.\nBoolean settings: ENABLE_AUTOFIX, ENABLE_CHANFIX, ENABLE_CHANNEL_BLOCKING.\nInteger settings: NUM_SERVERS.\nThis command requires flag o.
STATUS	1	/msg C STATUS\nShows current status.
SUSPEND	1	/msg C SUSPEND <username>\nSuspends the specified user.\nThis command requires one of these flags: ua.
UNBLOCK	1	/msg C UNBLOCK <channel>\nRemoves the block on a channel.\nThis command requires flag b.
USET	1	/msg C USET <setting> <value>\nSets options on your account. Possible settings:\nNOTICE - recieve notices, if no then PRIVMSG's will be sent.\nLANG - What language Chanfix will communicate with you in, Available languages: EN - English.
WHOIS	1	/msg C WHOIS <user>\nShows information about this user.
SETGROUP	1	/msg C SETGROUP <user> <group>\nSets the group for the specified user.
SHUTDOWN	1	/msg C SHUTDOWN\nShuts down Chanfix.\nThis command requires flag o.
UNSUSPEND	1	/msg C UNSUSPEND\nUnSuspends the specified user.\nThis command requires one of these flags: ua.
WHOGROUP	1	/msg C WHOGROUP <group>\nShows all users in the specified group along with their flags.
\.

