-- mod.chanfix SQL Database Script
-- (c) 2003 Matthias Crauwels <ultimate_@wol.be>
-- (c) 2005 Reed Loden <reed@reedloden.com>
-- $Id$

CREATE TABLE chanOps (
	channel VARCHAR(200) NOT NULL,
	account VARCHAR(24) NOT NULL,
	last_seen_as VARCHAR(128),
	ts_firstopped INT4 DEFAULT 0,
	ts_lastopped INT4 DEFAULT 0,
	day0 INT2 NOT NULL DEFAULT 0,
	day1 INT2 NOT NULL DEFAULT 0,
	day2 INT2 NOT NULL DEFAULT 0,
	day3 INT2 NOT NULL DEFAULT 0,
	day4 INT2 NOT NULL DEFAULT 0,
	day5 INT2 NOT NULL DEFAULT 0,
	day6 INT2 NOT NULL DEFAULT 0,
	day7 INT2 NOT NULL DEFAULT 0,
	day8 INT2 NOT NULL DEFAULT 0,
	day9 INT2 NOT NULL DEFAULT 0,
	day10 INT2 NOT NULL DEFAULT 0,
	day11 INT2 NOT NULL DEFAULT 0,
	day12 INT2 NOT NULL DEFAULT 0,
	day13 INT2 NOT NULL DEFAULT 0,
	PRIMARY KEY (channel, account)
);

CREATE TABLE channels (
	id SERIAL,
	channel TEXT NOT NULL UNIQUE,
	flags INT4 NOT NULL DEFAULT 0,
	PRIMARY KEY (id)
);

CREATE UNIQUE INDEX channels_name_idx ON channels(LOWER(channel));

CREATE TABLE users (
	id SERIAL,
	user_name TEXT NOT NULL,
	created INT4 NOT NULL DEFAULT 0,
	last_seen INT4 NOT NULL DEFAULT 0,
	last_updated INT4 NOT NULL DEFAULT 0,
	last_updated_by VARCHAR(128) NOT NULL,
	faction VARCHAR(128) NOT NULL DEFAULT 'undernet.org',
	flags INT2 NOT NULL DEFAULT 0,
	-- 0x01 - server admin (limited access to +u commands)
	-- 0x02 - can block/unblock channels
	-- 0x04 - can add/del notes/alerts to channels
	-- 0x08 - can manual chanfix
	-- 0x10 - owner
	-- 0x20 - user management rights
	isSuspended BOOLEAN NOT NULL DEFAULT FALSE,
	-- currently suspended
	useNotice BOOLEAN NOT NULL DEFAULT TRUE,
	-- use notice instead of privmsg
	PRIMARY KEY (id)
);

CREATE INDEX users_username_idx ON users( lower(user_name) );

CREATE TABLE hosts (
	user_id INT4 CONSTRAINT hosts_user_id_ref REFERENCES users ( id ),
	host VARCHAR(128) NOT NULL
);

CREATE INDEX hosts_user_id_idx ON hosts(user_id);

CREATE TABLE notes (
	id SERIAL,
	ts INT4,
	channelID INT4 CONSTRAINT notes_channelID_ref REFERENCES channels ( id ),
	userID INT4 CONSTRAINT notes_userID_ref REFERENCES users ( id ),
	event INT2 DEFAULT 0,
	-- Defines the note event type, so we can filter nice reports.
-- 1  -- EV_MISC - Uncategorised event.
-- 2  -- EV_NOTE - Miscellaneous notes about a channel.
-- 3  -- EV_CHANFIX - When someone manual chanfixes a channel.
-- 4  -- EV_BLOCK - When someone blocks a channel.
-- 5  -- EV_UNBLOCK - When somebody unblocks a channel.
-- 6  -- EV_ALERT - When someone sets alert flag on a channel.
-- 7  -- EV_UNALERT - When somebody removes alert flag from a channel.
	message TEXT,

	PRIMARY KEY(id, channelID, userID)
);

CREATE INDEX notes_channelID_idx ON notes(channelID);
CREATE INDEX notes_userID_idx ON notes(userID);
CREATE INDEX notes_event_idx ON notes(event);
