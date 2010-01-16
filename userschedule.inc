<?php
// vim: syn=php
/*
 *      userschedule.inc
 *      
 *      Copyright © 2009, 2010 Drew Fisher <kakudevel@gmail.com>
 *      Copyright © 2010 Mark Harviston <infinull@gmail.com>
 *		ALL RIGHTS RESERVED
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

function conschedule_usersched()
{
	global $user;
	$conschedule_path = drupal_get_path('module','conschedule');
	include_once($conschedule_path . '/Event.php');

	//return 'Under Construction, User Schedule for ' . $user->uid;

	$result = db_query('SELECT
	e_eventid, e_eventname, r_roomname, e_datestart, 
	e_dateend, e_eventdesc, e_panelist, e_color
FROM
	{conschedule_events}, {conschedule_rooms}, {conschedule_userschedule}
WHERE
	us_userid = %d
	AND
	us_eventid = e_eventid
	AND
	e_roomid = r_roomid
ORDER BY
	e_dateStart
	ASC', $user->uid);
	$row = db_fetch_object($result);

	$userEvents = array();
	if($row === FALSE)
	{
		drupal_set_message('Silly ' . $user->name . ', you have no events scheduled. =^.^=','error');
		return '<a href="'.url('schedule').'">Return to Schedule</a>';
	}

	do
	{
		$userEvents[] = new Event( 
			$row->e_eventid, $row->e_eventname, $row->r_roomname, 
			$row->e_datestart,$row->e_dateend, $row->e_eventdesc, 
			$row->e_panelist, $row->e_color 
		);

	}
	while($row = db_fetch_object($result));

	ob_start();

	echo '<h2>Custom Schedule for ' . $user->name . '</h2>';

	echo '<center>';
	echo '<table class="userSchedule" cellpadding=0 cellspacing=0>';
	echo '<thead><tr><td class="eventName">';
	echo 'Event Name';
	echo '</td><td class="room">';
	echo 'Room';
	echo '</td><td class="day">';
	echo 'Day';
	echo '</td><td class="startTime">';
	echo 'Start Time';
	echo '</td><td class="endTime">';
	echo 'End Time';
	echo '</td></tr></thead>';

	$prevE = NULL; // holder for previous event as we run through the loop
	$maxLen = 44; // max length for event name

	foreach( $userEvents as $e )
	{	
		$id = $e->getEventID();
		$name = $e->getEventName();
		$day = $e->getStartDate()->format("D, d M 'y");
		$startTime = $e->getStartDate()->format("H:i");
		$endTime = $e->getEndDate()->format("H:i");
		
		$tdClass = "";
		
		if( isset($prevE) )
		{
			// check for conflict
			if( conschedule_isTimeConflict($prevE, $e) )
			{
				if( isset($conflicts)){
					if( ! in_array($prevE, $conflicts) )
					{
						$conflicts[] = $prevE;
					}
					
					if( ! in_array($e, $conflicts) )
					{
						$conflicts[] = $e;
					}
				}
				else {
					$conflicts[0] = $prevE;
					$conflicts[1] = $e;
				}
			}
		
			// check for day change
			$prevSDF = $prevE->getStartDate()->format("Y-m-d");
			$currSDF = $e->getStartDate()->format("Y-m-d");
			
			if( $prevSDF != $currSDF )
			{
				$tdClass = 'class="dayBreak"';	
			}
		}
		
		echo '<tr><td '. $tdClass .'>';
		
		if( strlen($name) > $maxLen )
		{
			$tName = subStr( $name, 0, $maxLen );
			echo '<a href="'.url('schedule/event/'.$id.'/view').'">'.$tName.'&#133;</a>';
		}
		else 
		{
			echo '<a href="'.url('schedule/event/'.$id.'/view').'">'.$name.'</a>';
		}
		
		echo '</td><td '. $tdClass .'>';
		echo $e->getRoomName();
		echo '</td><td '. $tdClass .'>';
		echo $day;
		echo '</td><td '. $tdClass .'>';
		echo $startTime;
		echo '</td><td '. $tdClass .'>'; 
		echo $endTime;
		echo '</td></tr>';
		
		$prevE = $e;
	}

	echo '</table><br>';

	// exit if no conflicts were found
	if( count($conflicts) == 0 )
	{
		//echo 
		//$page->addURL("index.php","Return to event schedule.");
		echo '</center>';
		return ob_get_clean();
	}

	echo '</center>';
	echo '<hr><hr>';

	echo '<h2>Conflicts</h2>';

	echo '<center>';
	echo '<table class="userSchedule" cellpadding=0 cellspacing=0>';
	echo '<thead><tr><td class="eventName">';
	echo 'Event Name';
	echo '</td><td class="room">';
	echo 'Room';
	echo '</td><td class="day">';
	echo 'Day';
	echo '</td><td class="startTime">';
	echo 'Start Time';
	echo '</td><td class="endTime">';
	echo 'End Time';
	echo '</td></tr></thead>';

	$prevE = NULL; // holder for previous event as we run through the loop
	$maxLen = 44; // max length for event name

	foreach( $conflicts as $e )
	{	
		$id = $e->getEventID();
		$name = $e->getEventName();
		$day = $e->getStartDate()->format("D, d M 'y");
		$startTime = $e->getStartDate()->format("H:i");
		$endTime = $e->getEndDate()->format("H:i");
		
		$tdClass = "";
		
		if( isset($prevE) )
		{
			// check for day change
			$prevSDF = $prevE->getStartDate()->format("Y-m-d");
			$currSDF = $e->getStartDate()->format("Y-m-d");
			
			if( $prevSDF != $currSDF )
			{
				$tdClass = 'class="dayBreak"';	
			}
		}
		
		echo '<tr><td '. $tdClass .'>';
		
		if( strlen($name) > $maxLen )
		{
			$tName = subStr( $name, 0, $maxLen );
			echo '<a href="'.url('schedule/event/'.$id.'/view').'">'.$tName.'&#133;</a>';
		}
		else 
		{
			echo '<a href="'.url('schedule/event/'.$id.'/view').'">'.$name.'</a>';
		}
		
		echo '</td><td '. $tdClass .'>';
		echo $e->getRoomName();
		echo '</td><td '. $tdClass .'>';
		echo $day;
		echo '</td><td '. $tdClass .'>';
		echo $startTime;
		echo '</td><td '. $tdClass .'>'; 
		echo $endTime;
		echo '</td></tr>';
		
		$prevE = $e;
	}

	echo '</table><br>';
	echo '</center>';

	//$page->addURL("index.php","Return to event schedule.");
	return ob_get_clean();

}

function conschedule_usersched_add_event($event_id)
{
	global $user;
	assert(is_numeric($event_id));

	$result = db_query('SELECT us_uschedid FROM {conschedule_userschedule} WHERE us_eventid = %d AND us_userid = %d',
		$event_id, $user->uid);

	if( db_fetch_array($result) === FALSE)
	{
		db_query('INSERT INTO {conschedule_userschedule} (us_eventid, us_userid) VALUES (%d, %d)',
			$event_id, $user->uid);
		drupal_set_message('Event added to your schedule');
	}
	else
	{
		drupal_set_message('Event already on your schedule','error');
	}

	drupal_goto('schedule/event/' . $event_id . '/view');
}

function conschedule_usersched_del_event($event_id)
{
	global $user;
	assert(is_numeric($event_id));

	$result = db_query('DELETE FROM {conschedule_userschedule} WHERE us_eventid = %d AND us_userid = %d',
		$event_id, $user->uid);

	if(db_affected_rows($result) == 1)
	{
		drupal_set_message('Event removed from your schedule');
	}
	else
	{
		drupal_set_message('Event not on your user schedule','error');
	}

	drupal_goto('schedule/event/' . $event_id . '/view');
}


function conschedule_isTimeConflict($reqEvent, $schedEvent) {
	$req_start = $reqEvent->getStartDate()->format("U");
	$req_end = $reqEvent->getEndDate()->format("U");
	
	$se_start = $schedEvent->getStartDate()->format("U");
	$se_end = $schedEvent->getEndDate()->format("U");
	
	// requested time lies within scheduled time
	if( $req_start >= $se_start && $req_start < $se_end )
	{	
		return TRUE;
	} 
	// scheduled time lies within requested time
 	else if( $se_start >= $req_start && $se_start < $req_end )
 	{
 		return TRUE;
 	}
 	else
 	{
 		return FALSE;
 	}
}
