/**
 * chanfix_config.h
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 * $Id$
 */

#ifndef __CHANFIX_CONFIG_H
#define __CHANFIX_CONFIG_H "$Id$"

/**
 * Defines that deal with fixing channels, both manually and
 * automatically. All the below times are in seconds.
 */

/**
 * The time between consecutive attempts to fix an opless channel
 */
#define AUTOFIX_INTERVAL	600

/**
 * The maximum time to try to fix an opless channel
 */
#define AUTOFIX_MAXIMUM		3600

/**
 * The max number of clients opped by chanfix during an autofix.
 */
#define AUTOFIX_NUM_OPPED	5

/**
 * The time to wait between the removal of modes and the first
 * attempt to give ops to people.
 */
#define CHANFIX_DELAY		30

/**
 * The time between consecutive attempts to manually fix a channel
 */
#define CHANFIX_INTERVAL	300

/**
 * The maximum time to try to manually fix a channel
 */
#define CHANFIX_MAXIMUM		3600

/**
 * The max number of clients opped by chanfix during a manual fix.
 */
#define CHANFIX_NUM_OPPED	5


/**
 * Score values
 * The first 2 values are the minimum scores required at the beginning
 * of a chanfix; the last 2 values are the minimum scores at the very
 * end of the fix. Between these times, there is a linear decrease from
 * the high to the low values.
 */

/**
 * Minimum absolute score required for chanfix to op, relative to
 * the maximum score possible (default: 0.20 * 4032).
 */
#define FIX_MIN_ABS_SCORE_BEGIN	0.20f

/**
 * Minimum score required for chanfix to op, relative to the maximum
 * score for this channel in the database, at the beginning of the
 * fix.
 */
#define FIX_MIN_REL_SCORE_BEGIN	0.90f

/**
 * Minimum absolute score required for chanfix to op, relative to
 * the maximum score possible (default: 0.04 * 4032).
 */
#define FIX_MIN_ABS_SCORE_END	0.04f

/**
 * Minimum score required for chanfix to op, relative to the maximum
 * score for this channel in the database. So, if you have less than
 * 30% of the maximum score, chanfix will never op you.
 */
#define FIX_MIN_REL_SCORE_END	0.30f


/**
 * Other various settings that really should not be changed.
 */

/**
 * Interval between two consecutive database updates.
 */
#define DATABASE_UPDATE_TIME	300

/**
 * Maximum score a client can obtain.
 */
#define MAX_SCORE	((int)(DAYSAMPLES) * 86400 / DATABASE_UPDATE_TIME)

/**
 * The maximum number of days to keep track of per channel -1,
 * e.g. a value of 15 means we track 14.00-14.99 days.
 */
#define DAYSAMPLES	14

/**
 * The maximum number of top ops to return to the user on
 * an OPLIST query. (default: 10)
 */
#define OPCOUNT		10

/**
 * The maximum number of top tops to keep track of per channel.
 * (default: 256)
 */
#define MAXOPCOUNT	256

/**
 * The maximum number of automatic fixes to retain in history per
 * channel. (default: 10)
 */
#define MAXAUTOFIX	10

/**
 * The maximum number of manual fixes to retain in history per channel.
 * (default: 10)
 */
#define MAXMANUALFIX	10

/**
 * Do you want to remember (and never expire) old channel entries that
 * have notes or flags associated with them?
 */
#define REMEMBER_CHANNELS_WITH_NOTES_OR_FLAGS

#endif // __CHANFIX_CONFIG_H
