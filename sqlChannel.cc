/**
 * Author: Matthias Crauwels <ultimate_@wol.be>
 *
 */

#include	<sstream>
#include	<string>

#include	"libpq++.h"

#include	"ELog.h"
#include	"misc.h"

#include	"sqlChannel.h"
#include	"chanfix.h"

namespace gnuworld
{

const sqlChannel::flagType sqlChannel::F_BLOCKED	= 0x00000001 ;
const sqlChannel::flagType sqlChannel::F_ALERT		= 0x00000002 ;

const int sqlChannel::EV_MISC		= 1 ; /* Uncategorised event */
const int sqlChannel::EV_NOTE		= 2 ; /* Miscellaneous notes */
const int sqlChannel::EV_CHANFIX	= 3 ; /* Manual chanfixes */
const int sqlChannel::EV_BLOCK		= 4 ; /* Channel block */
const int sqlChannel::EV_UNBLOCK	= 5 ; /* Channel unblock */
const int sqlChannel::EV_ALERT		= 6 ; /* Channel alert */
const int sqlChannel::EV_UNALERT	= 7 ; /* Channel unalert */

sqlChannel::sqlChannel(PgDatabase* _SQLDb)
: id(0),
  channel(),
  last(0),
  start(0),
  maxScore(0),
  modesRemoved(false),
  flags(0),
  SQLDb(_SQLDb)
{};

void sqlChannel::setAllMembers(int row) 
{
id = atoi(SQLDb->GetValue(row, 0));
channel = SQLDb->GetValue(row, 1);
flags = atoi(SQLDb->GetValue(row, 2));
};

bool sqlChannel::Insert()
{
static const char* queryHeader = "INSERT INTO channels (channel, flags) VALUES (";

std::stringstream queryString;
queryString	<< queryHeader << "'"
		<< escapeSQLChars(channel) << "',"
		<< flags << ")"
		<< std::ends;

//#ifdef LOG_SQL
	elog	<< "sqlChannel::Insert> "
		<< queryString.str().c_str()
		<< std::endl;
//#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
	{
	// TODO: Log to msgchan here.
	elog	<< "sqlChannel::Insert> Something went wrong: "
		<< SQLDb->ErrorMessage()
		<< std::endl;

	return false;
	}

return true;

};

bool sqlChannel::Delete()
{
static const char* queryHeader =    "DELETE FROM channels ";

std::stringstream queryString;
queryString	<< queryHeader << "WHERE id = " << id
		<< std::ends;

//#ifdef LOG_SQL
	elog	<< "chanfix::sqlChannel::Delete> "
		<< queryString.str().c_str()
		<< std::endl;
//#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
	{
	// TODO: Log to msgchan here.
	elog	<< "chanfix::sqlChannel::Delete> Something went wrong: "
		<< SQLDb->ErrorMessage()
		<< std::endl;

	return false;
	}

return true;

};

bool sqlChannel::commit()
{
static const char* queryHeader =    "UPDATE channels ";

std::stringstream queryString;
queryString	<< queryHeader << "SET flags = "
		<< flags << " WHERE id = " << id
		<< std::ends;

//#ifdef LOG_SQL
	elog	<< "chanfix::sqlChannel::commit> "
		<< queryString.str().c_str()
		<< std::endl;
//#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_COMMAND_OK != status )
	{
	// TODO: Log to msgchan here.
	elog	<< "chanfix::sqlChannel::commit> Something went wrong: "
		<< SQLDb->ErrorMessage()
		<< std::endl;

	return false;
	}

return true;

};

/**
 * This method writes a 'notes' record, recording an event that has
 * occured in this channel.
 */

void sqlChannel::addNote(unsigned short eventType, sqlUser* theUser,
	const std::string& theMessage)
{
unsigned int num_notes = countNotes(0);
while (num_notes >= MAXNOTECOUNT) {
  if (!deleteOldestNote())
    return;
  num_notes--;
}

std::stringstream theLog;
theLog	<< "INSERT INTO notes (ts, channelID, userID, event, message) "
	<< "VALUES ("
	<< "now()::abstime::int4"
	<< ", "
	<< id
	<< ", "
	<< theUser->getID()
	<< ", "
	<< eventType
	<< ", "
	<< "'"
	<< escapeSQLChars(theMessage)
	<< "')"
	<< std::ends;

#ifdef LOG_SQL
	elog	<< "sqlChannel::addNote> "
		<< theLog.str()
		<< std::endl;
#endif

SQLDb->ExecCommandOk(theLog.str().c_str());
}

const std::string sqlChannel::getLastNote(unsigned short eventType, time_t& eventTime)
{
std::stringstream queryString;

queryString	<< "SELECT message,ts"
			<< " FROM notes WHERE channelID = "
			<< id
			<< " AND event = "
			<< eventType
			<< " ORDER BY ts DESC LIMIT 1"
			<< std::ends;

#ifdef LOG_SQL
	elog	<< "sqlChannel::getLastNote> "
			<< queryString.str().c_str()
			<< std::endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str()) ;

if( PGRES_TUPLES_OK == status )
	{

	if(SQLDb->Tuples() < 1)
		{
		return("");
		}

	std::string note = SQLDb->GetValue(0, 0);
	eventTime = atoi(SQLDb->GetValue(0, 1));

	return (note);
	}

return ("");
}

bool sqlChannel::deleteNote(unsigned int messageId, sqlUser* theUser)
{
std::stringstream queryString;
queryString	<< "DELETE FROM notes WHERE channelID = "
		<< id
		<< " AND userID = "
		<< theUser->getID()
		<< " AND id = "
		<< messageId
		<< std::ends;

#ifdef LOG_SQL
	elog	<< "sqlChannel::deleteNote> "
		<< queryString.str().c_str()
		<< std::endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str());

if( PGRES_COMMAND_OK != status )
	{
	elog	<< "sqlChannel::deleteNote> Something went wrong: "
		<< SQLDb->ErrorMessage()
		<< std::endl;
	return false;
	}

return true;
}

bool sqlChannel::deleteOldestNote()
{
std::stringstream queryString;
queryString	<< "DELETE FROM notes WHERE channelID = "
		<< id
		<< " ORDER BY ts ASC LIMIT 1"
		<< std::ends;

#ifdef LOG_SQL
	elog	<< "sqlChannel::deleteOldestNote> "
		<< queryString.str().c_str()
		<< std::endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str());

if( PGRES_COMMAND_OK != status )
	{
	elog	<< "sqlChannel::deleteOldestNote> Something went wrong: "
		<< SQLDb->ErrorMessage()
		<< std::endl;
	return false;
	}

return true;
}

bool sqlChannel::deleteAllNotes()
{
std::stringstream queryString;
queryString	<< "DELETE FROM notes WHERE channelID = "
		<< id
		<< std::ends;

#ifdef LOG_SQL
	elog	<< "sqlChannel::deleteAllNotes> "
		<< queryString.str().c_str()
		<< std::endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str());

if( PGRES_COMMAND_OK != status )
	{
	elog	<< "sqlChannel::deleteAllNotes> Something went wrong: "
		<< SQLDb->ErrorMessage()
		<< std::endl;
	return false;
	}

return true;
}

size_t sqlChannel::countNotes(unsigned short eventType)
{
std::stringstream queryString;
if (eventType) {
queryString	<< "SELECT count(id) FROM notes WHERE channelID = "
		<< id
		<< " AND event = "
		<< eventType
		<< std::ends;
} else {
queryString	<< "SELECT count(id) FROM notes WHERE channelID = "
		<< id
		<< std::ends;
}

#ifdef LOG_SQL
	elog	<< "sqlChannel::countNotes> "
		<< queryString.str().c_str()
		<< std::endl;
#endif

ExecStatusType status = SQLDb->Exec(queryString.str().c_str());

if( PGRES_TUPLES_OK == status )
	{

	if(SQLDb->Tuples() < 1)
		{
		return 0;
		}

	size_t num_notes = atoi(SQLDb->GetValue(0, 0));

	return num_notes;
	}

return 0;
}

sqlChannel::~sqlChannel()
{
// No heap space allocated
}

} // namespace gnuworld
