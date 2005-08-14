-- "$Id$"
-- 2005-08-13 : reed
--
-- This script will add an account for user "Admin" with owner (+o) access.
-- Please modify the default user ("Admin") to your account name.
--
-- This script may be used to create (not modify) multiple owner
-- accounts provided that the account name is changed each time.
--

INSERT INTO users (user_name,created,flags)
VALUES ('Admin',now()::abstime::int4,16);

-- END HERE
