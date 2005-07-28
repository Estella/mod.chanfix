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

CREATE TABLE channellog (
        ts INT4,
        channelID INT4 CONSTRAINT channel_log_ref REFERENCES channels ( id ),
        event INT2 DEFAULT '0',
        -- Defines the message event type, so we can filter nice reports.
-- 1  -- EV_MISC - Uncategorised event.
-- 2  -- EV_NOTE - Miscellaneous notes about a channel.
-- 3  -- EV_BLOCKED - When someone blocks a channel.
-- 4  -- EV_ALERT - When somebody
        message TEXT,
        last_updated INT4 NOT NULL,
        deleted INT2 DEFAULT '0'
);

CREATE INDEX channellog_channelID_idx ON channellog(channelID);
CREATE INDEX channellog_event_idx ON channellog(event);

CREATE TABLE users (
	id SERIAL,
	user_name TEXT NOT NULL,
	created INT4 NOT NULL DEFAULT 0,
	last_seen INT4 NOT NULL DEFAULT 0,
	last_updated INT4 NOT NULL DEFAULT 0,
	last_updated_by VARCHAR(128) NOT NULL,
	-- server admin (limited access to +u commands)
	isServAdmin BOOLEAN NOT NULL DEFAULT FALSE,
	-- can block/unblock channels
	canBlock BOOLEAN NOT NULL DEFAULT FALSE,
	-- can add/del notes/alerts to channels
	canAlert BOOLEAN NOT NULL DEFAULT FALSE,
	-- can manual chanfix
	canChanfix BOOLEAN NOT NULL DEFAULT FALSE,
	-- owner
	isOwner BOOLEAN NOT NULL DEFAULT FALSE,
	-- user management rights
	canManageUsers BOOLEAN NOT NULL DEFAULT FALSE,
	-- currently suspended
	isSuspended BOOLEAN NOT NULL DEFAULT FALSE,
	-- use notice instead of privmsg
	useNotice BOOLEAN NOT NULL DEFAULT TRUE,
	PRIMARY KEY (id)
);

CREATE INDEX users_username_idx ON users( lower(user_name) );

CREATE TABLE hosts (
	user_id INT4 CONSTRAINT hosts_user_id_ref REFERENCES users ( id ),
	host VARCHAR(128) NOT NULL
);

CREATE INDEX hosts_user_id_idx ON hosts(user_id);

CREATE TABLE groups (
	group_id SERIAL,
	name VARCHAR(128) NOT NULL,
	PRIMARY KEY (group_id)
);

CREATE TABLE group_members (
	user_id INT4 CONSTRAINT group_members_user_id_ref REFERENCES users ( id ),
	group_id INT4 CONSTRAINT group_members_group_id_ref REFERENCES groups ( group_id ),
	-- currently the main group of the user
	isMain BOOLEAN NOT NULL DEFAULT FALSE,
	-- can administrate this group
	isAdmin BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE INDEX group_members_user_id_idx ON group_members(user_id);
