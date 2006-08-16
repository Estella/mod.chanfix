<?
/**
 * update.notes.php
 *
 * 30/04/2006 - Neil Spierling <sirvulcan@gmail.com>
 *
 * Updates the chanfix notes table (pre r1552) to the latest schema
 * Run via a web browser or in a shell do this:
 * php update.notes.php
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 *
 * $Id: chanfix.cc 1553 2006-04-18 04:03:59Z sirvulcan $
 *
 *
 * ----------------------------------------------------------------------
 * Configuration
 */

$dbhost = "127.0.0.1";
$dbport = "5432";
$dbusername = "gnuworld";
$dbpassword = "";
$cservicedatabase = "chanfixtest";
$chanfixdatabase = "chanfixtest";

/* Dont change anything below this */
/* --------------------------------------------------------------------- */

$cservicedb = pg_connect('host=' . $dbhost . ' port=' . $dbport . ' dbname=' . $cservicedatabase . ' user=' . $dbusername . ' password=' . $dbpassword) or die('Could not connect to the cservice db: ' . pg_last_error());
$chanfixdb = pg_connect('host=' . $dbhost . ' port=' . $dbport . ' dbname=' . $chanfixdatabase . ' user=' . $dbusername . ' password=' . $dbpassword) or die('Could not connect to the chanfix db: ' . pg_last_error());
echo "Connected.)\n";

/* Check the existing notes table for notes */
echo "Checking existing notes table...\n";
$q = "SELECT * FROM notes";
$query = pg_query($chanfixdb, $q);
$result = pg_fetch_assoc($query);
if (!$result) {
  $notesempty = true;
} else {
  $type = pg_field_type($query, 3);
  if ($type == "varchar") {
    echo "Table has already been updated\n";
    die();
  }
}

echo "Creating new notes table...\n";
/* Create the new notes table */
$seq = "DROP SEQUENCE notes_id_seq";
$seqd = pg_query($chanfixdb, $seq);
$schema = "CREATE TABLE newnotes (id SERIAL, ts INT4, channelID INT4 CONSTRAINT notes_channelID_ref REFERENCES channels ( id ), user_name VARCHAR(128), event INT2 DEFAULT 0, message TEXT, PRIMARY KEY(id, channelID))";
$create = pg_query($chanfixdb, $schema);


/* If theres notes then insert them into the new table
   using usernames from the cservice database */
if ($notesempty != true) {
  echo "Inserting data into new notes table...\n";
  $q = "SELECT * FROM notes";
  $query = pg_query($chanfixdb, $q);
  while ($result = pg_fetch_row($query)) {
    $usernameq = pg_query($cservicedb, "SELECT user_name FROM users WHERE id='". $result[3] ."'");
    while ($username = pg_fetch_row($usernameq)) {
      $insertq = "INSERT INTO newnotes (id, ts, channelID, user_name, event, message) VALUES($result[0], $result[1], $result[2], '$username[0]', $result[4], '$result[5]')";
      echo $insertq . "\n";
      $insert = pg_query($chanfixdb, $insertq);
    }
  }
}

/* Rename the tables, we will keep the old one just
   in case for now */
echo "Renaming existing notes table to oldnotes...\n";
$renoldtonew = pg_query($chanfixdb, "ALTER TABLE notes RENAME TO oldnotes");
echo "Renaming new notes table to notes...\n";
$rennewtoold = pg_query($chanfixdb, "ALTER TABLE newnotes RENAME TO notes");

echo "\nComplete.";
?>

