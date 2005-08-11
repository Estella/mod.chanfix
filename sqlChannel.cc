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
const int sqlChannel::EV_BLOCKED	= 3 ; /* Channel blocking */
const int sqlChannel::EV_ALERT		= 4 ; /* Channel alerts */

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

sqlChannel::~sqlChannel()
{
// No heap space allocated
}

} // namespace gnuworld
