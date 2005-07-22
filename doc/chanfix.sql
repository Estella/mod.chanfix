-- ChanFix SQL Database Script
-- (c) 2003 Matthias Crauwels <ultimate_@wol.be>

CREATE TABLE chanOps (
	userHost VARCHAR(128) NOT NULL,
	channel VARCHAR(200) NOT NULL,
	last_seen_as VARCHAR(128),
	points INT4 NOT NULL,
        account VARCHAR(24),
	ts_firstopped INT4 DEFAULT 0,
	ts_lastopped INT4 DEFAULT 0,
	PRIMARY KEY (account, channel)
);

CREATE TABLE channels (
	channel VARCHAR(200) NOT NULL,
	fixed INT4 DEFAULT 0,
	lastfix INT4 DEFAULT 0,
	flags INT4 NOT NULL DEFAULT 0,
	PRIMARY KEY (channel)
);
