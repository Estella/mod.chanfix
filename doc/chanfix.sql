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
	channel VARCHAR(200) NOT NULL,
	flags INT4 NOT NULL DEFAULT 0,
	PRIMARY KEY (channel)
);

CREATE TABLE users (
	id SERIAL,
	user_name VARCHAR(24) NOT NULL,
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

CREATE TABLE hosts (
	user_id INT4 NOT NULL,
	host VARCHAR(128) NOT NULL
);

CREATE TABLE groups (
	group_id SERIAL,
	name VARCHAR(128) NOT NULL,
	PRIMARY KEY (group_id)
);

CREATE TABLE group_members (
	user_id INT4 NOT NULL,
	group_id INT4 NOT NULL,
	-- currently the main group of the user
	isMain BOOLEAN NOT NULL DEFAULT FALSE,
	-- can administrate this group
	isAdmin BOOLEAN NOT NULL DEFAULT FALSE
);
