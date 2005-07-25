-- ChanFix SQL Database Script
-- (c) 2003 Matthias Crauwels <ultimate_@wol.be>

CREATE TABLE chanOps (
	channel VARCHAR(200) NOT NULL,
        account VARCHAR(24) NOT NULL,
	last_seen_as VARCHAR(128),
	points INT4 NOT NULL,
	ts_firstopped INT4 DEFAULT 0,
	ts_lastopped INT4 DEFAULT 0,
	PRIMARY KEY (channel, account)
);

CREATE TABLE channels (
	channel VARCHAR(200) NOT NULL,
	flags INT4 NOT NULL DEFAULT 0,
	PRIMARY KEY (channel)
);
