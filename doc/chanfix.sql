-- ChanFix SQL Database Script
-- (c) 2003 Matthias Crauwels <ultimate_@wol.be>

CREATE TABLE chanOps (
	userHost VARCHAR(128) NOT NULL,
	channel VARCHAR(32) NOT NULL,
	last_seen_as VARCHAR(128),
	points INT4 NOT NULL,
        account VARCHAR(24),
	PRIMARY KEY (account, channel)
);

CREATE TABLE channels (
	channel VARCHAR(32) NOT NULL,
	fixed INT4 DEFAULT 0,
	lastfix INT4 DEFAULT 0,
	PRIMARY KEY (channel)
);
