<?php
/*
 *		conschedule.inc
 *		
 *		Copyright © 2010 Mark Harviston <infinull@gmail.com>
 *		ALL RIGHTS RESERVED
 *		
 *		This program is free software; you can redistribute it and/or modify
 *		it under the terms of the GNU General Public License as published by
 *		the Free Software Foundation; either version 2 of the License, or
 *		(at your option) any later version.
 *		
 *		This program is distributed in the hope that it will be useful,
 *		but WITHOUT ANY WARRANTY; without even the implied warranty of
 *		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *		GNU General Public License for more details.
 *		
 *		You should have received a copy of the GNU General Public License
 *		along with this program; if not, write to the Free Software
 *		Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *		MA 02110-1301, USA.
 */

class ConSchedule {
	public $embedded = FALSE;
	private $schedule = array();
	private $roomNames = array();
	private $startDate = NULL;
	private $endDate = NULL;
	private $custom_startStop = FALSE;
	private $in_usched = array();

	public function calcStartAndEndTimes()
	{
		global $conschedule_global;
		$conDayCount = count($conschedule_global->conTimes);

		// used if _GET['startTime'] or _GET['endTime'] are specified,
		// filled with conTimes span otherwise.

		$this->startDate = NULL;
		$this->endDate = NULL;
		$date = NULL;

		// check for index.php?date=YYYYMMDD[&startTime=HHMM][&endTime=HHMM]
		// or index.php?conday=#[&startTime=HHMM][&endTime=HHMM]

		if( isset($_GET['conday']) && strlen($_GET['conday']) <= 2) //I doubt we'll ever have more than 99 days of con...
		{
			$conday = $_GET['conday'];
			if( isset($conschedule_global->conTimes[$conday]) )
			{
				$date = substr($conschedule_global->conTimes[$conday]['start'],0,10);
			}
			else
			{
				$conday = NULL;
			} 
		}

		if( isset($_GET['date']) && strlen($_GET['date']) == 8 && ! isset($conday) )
		{
			$date = $_GET['date'];
		} 

		//if either ?conday or ?date were passed, check for ?startTime and ?endTime
		if( isset($conday) || isset($date) )
		{ 
			if( isset($_GET['startTime']) && strlen($_GET['startTime']) == 4 )
			{
				$this->startTime = $_GET['startTime'];
				$conschedule_global->defaultStartTime = $this->startTime . "00";
			}

			if( isset($_GET['endTime']) && strlen($_GET['endTime']) == 4 )
			{
				$this->endTime = $_GET['endTime'];
				$conschedule_global->defaultEndTime = $this->endTime . "00";
			}

			$this->startDate = date_create($date ." ". $conschedule_global->defaultStartTime);
			$this->endDate = date_create($date ." ". $conschedule_global->defaultEndTime);

			// if the end time is before or equal to the start time, assume 
			// they meant the end time was on the following day.

			$diff = $this->endDate->format("U") - $this->startDate->format("U");

			if( $diff <= 0 )
			{
				$this->endDate->modify("+1 day");
			}
		}


		/* special check when ?conday is passed, possibly with a ?startTime, in case
		 * the $conTimes[$conday]['end'] is before the $defaultEndTime.
		 * we don't want to print out more times than the official con runs.
		 *
		 * (NOTE that if a user specifies an ?endTime that goes beyond the conday time,
		 * we assume the user knows what they're doing and wants to see till, say, 4am
		 * even if the con only goes till 2am.) 
		 */
		if( isset($conday) && ! isset($_GET['endTime']) )
		{
			$conEndTime = date_create( $conschedule_global->conTimes[$conday]['end'] );

			$diff = $conEndTime->format("U") - $this->endDate->format("U");

			if( $diff < 0)
			{
				$this->endDate = date_create( $conschedule_global->conTimes[$conday]['end'] );
			}
		}

		//fill in the start and end times with conTimes if not specified via url params
		if( is_null($this->startDate) && is_null($this->endDate) )
		{
			$this->startDate = date_create( $conschedule_global->conTimes[0]['start'] );
			$this->endDate = date_create( $conschedule_global->conTimes[$conDayCount-1]['end'] ); 
		}
		else
		{
			$this->custom_startStop = TRUE;
		}
	}

	function getData()
	{
		global $conschedule_global;
		global $user;

		$this->schedule = NULL;

		// events query
		if(user_is_logged_in())
		{
			//,
			//
			//
					 
			$result = db_query("
			SELECT 
				CASE WHEN us_uschedid IS NULL THEN 0 ELSE 1 END as in_u_sched,
				e_eventid, e_eventname, r_roomname, e_datestart, 
				e_dateend, e_eventname, e_eventdesc, e_color, e_panelist
			FROM {conschedule_events}
			INNER JOIN {conschedule_rooms} ON e_roomid = r_roomid 
			LEFT JOIN {conschedule_userschedule} ON us_eventid = e_eventid
			WHERE 
				e_datestart BETWEEN '%s' AND '%s'
				AND (us_userid = %d OR us_uschedid IS NULL)
			ORDER BY
				r_roomid ASC
			;", $this->startDate->format("Y-m-d H:i:s"), $this->endDate->format("Y-m-d H:i:s"), $user->uid);
		}
		else
		{
			$result = db_query("
			SELECT 
				0 as in_u_sched,
				e_eventid, e_eventname, r_roomname, e_datestart, 
				e_dateend, e_eventname, e_eventdesc, e_color, e_panelist
			FROM 
				{conschedule_events}, {conschedule_rooms}
			WHERE 
				e_roomid = r_roomid AND
				e_datestart BETWEEN '%s' AND '%s'
			ORDER BY
				r_roomID ASC
			;", $this->startDate->format("Y-m-d H:i:s"), $this->endDate->format("Y-m-d H:i:s"));
		}

		$row = db_fetch_object($result);
		if( $row == NULL ) 
		{
			if( isset($_GET['date']) )
			{
				drupal_set_message("No events scheduled for ". $this->startDate->format("F d, Y"),'error');
				/*
					echo "<center>";
					$page->addURL("index.php","Return to non-filtered event schedule.");
					echo "</center>";
				*/
				return;
			}
			else
			{
				drupal_set_message("No events scheduled.",'error');
			}


			if( ! user_access('administer conschedule') )
			{
				drupal_set_message("Please check back later. We'll be posting events soon =^.^=",'status');
			}
			else
			{
				echo 'Oi, you:	<a href="'.url('schedule/event/0/add').'">Add an Event</a>!'; 
			}
			return;
		}

		// create the events and this->roomNames arrays
		$this->roomNames = array();
		$events = array();
		$events_in_usched = array();
		do
		{
			$events[] = new Event( 
				$row->e_eventid, $row->e_eventname, $row->r_roomname, $row->e_datestart, $row->e_dateend,
				$row->e_eventdesc, $row->e_panelist, $row->e_color 
				);

			if( ! in_array($row->r_roomname, $this->roomNames))
			{
				$this->roomNames[] = $row->r_roomname;
			}
			if( $row->in_u_sched == 1)
			{
				$this->in_usched[] = $row->e_eventid;
			}

		}
		while( $row = db_fetch_object($result));

		// set up the schedule var
		foreach( $this->roomNames as $roomName )
		{
			foreach( $events as $event )
			{
				if( $event->getRoomName() == $roomName )
				{
					$fmt = $event->getStartDate()->format("Y-m-d H:i:s");
					$this->schedule[$fmt][$roomName] = $event;
				}
			}
		}

	}

	function printSchedule($embedded=TRUE)
	{
		global $conschedule_global;
		$this->embedded = $embedded;
		$this->calcStartAndEndTimes();
		$this->getData();

		ob_start();

		$this->printHeader();
		if( $this->custom_startStop )
		{
			$this->printDaySchedule($this->startDate, $this->endDate);
		}
		else
		{
			foreach( $conschedule_global->conTimes as $the_conTime )
			{
				$dayStarts = date_create( $the_conTime['start'] );
				$dayEnds = date_create( $the_conTime['end'] );

				$this->printDaySchedule($dayStarts, $dayEnds); 
			}
		}
		$this->printFooter();

		return ob_get_clean();
	}

	public function printHeader()
	{
		global $conschedule_global;
		$consched_path = drupal_get_path('module','conschedule');
		if(!$this->embedded)
		{
			$consched_path = base_path().$consched_path;
		}
		$js_file = "$consched_path/conschedule.js";
		$css_file = "$consched_path/conschedule.css";
		
		if (!$this->embedded)
		{
		echo 
"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"
\"http://www.w3.org/TR/html4/loose.dtd\">
<head>
	<title>$title</title>
	<meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">
	<link href=\"$css_file\" rel=\"stylesheet\" type=\"text/css\">
	<script type=\"text/javascript\" language=\"javascript\" src=\"$js_file\"></script>
</head>
<body>";
		}
		else
		{
			drupal_add_css($css_file);
			drupal_add_js($js_file);
		}
		echo '<div id="conschedule_body"';
		if (!$this->embedded)
		{
			echo ' class="conschedule_kiosk"';
		}
		echo '>';
	}

	public function printFooter()
	{
		echo '</div>';
		if(!$this->embedded)
		{
			echo "</body></html>";
		}
	}

	public function printDaySchedule($startDate=NULL,$endDate=NULL)
	{
		if(is_null($startDate))
		{
			$startDate = $this->startDate;
		}
		if(is_null($endDate))
		{
			$endDate = $this->endDate;
		}

		assert(!is_null($startDate) && !is_null($endDate) && !is_null($this->roomNames) && !is_null($this->schedule));

		// only print 24 hours from beginning time
		$dayCheck = clone($startDate);
		$dayCheck->modify("+1 days");
		if( $dayCheck->format("U") < $endDate->format("U") )
		{
			$endDate = $dayCheck;
		}
		
		$halfHoursOpen = ((($endDate->format("U") - $startDate->format("U"))/60/60)*2)+1;
		$tableTime = clone($startDate);
		
		echo '<table class="daySchedule" cellpadding=0 cellspacing=0><thead>';
		echo "<caption><h2>Schedule for {$startDate->format('F d, Y')}</h2></caption>";
		echo '<tr><th class="timeColumn">Time</th>';
		//initialize the wait on each room to zero
		//might as well print out the top row too
		foreach($this->roomNames as $roomName)
		{
			echo "<th>$roomName</th>";
			$wait[$roomName] = 0;
		}
		echo "</thead>";
		
		//table body printout
		for($i=0; $i < $halfHoursOpen; $i+=1)
		{
			echo "<tr>";
			$tF = $tableTime->format("g:i a");
			echo "<td class=\"timeColumn\">" . $tF . "</td>";
		
			foreach($this->roomNames as $roomName)
			{
				if($wait[$roomName] == 0)
				{
					$tF = $tableTime->format("Y-m-d H:i:s");
					
					/* if an event for the current looped room starts
					 * at the current looped time, print the event and
					 * set the wait so that table printout skips cells it
					 * shouldn't print because of an event's rowspan.
					 */ 
					if(isset($this->schedule[$tF][$roomName]))
					{			
						$timeFormat = "H:i / g:i a";
						
						$event = $this->schedule[$tF][$roomName];
						$name = $event->getEventName();
						$color = $event->getColor();
						$size = $event->getEventLengthInHalfHours();
						$eventID = $event->getEventID();
						
						$extraclass = '';
						if(in_array($eventID, $this->in_usched))
						{
							$extraclass = ' inusched';
						}

						echo "<td class=\"foundEvent$extraclass\" rowspan=\"$size\" style=\"background-color:$color\">
						<div class=\"event_container\">";

						if($extraclass == ' inusched')
						{
							echo '<div class="usched_badge"></div>';
						}
						
						if($size > 1){
							echo "<div class=\"startTime\">
							{$event->getStartDate()->format($timeFormat)}
							</div>";
						}
						
						echo "<div class=\"event\">";
						$path = url("schedule/event/$eventID/view");
						echo "<a href=\"$path\">$name</a>";
						echo "</div>";
						
						if($size > 1){
							echo "<div class=\"endTime\">
							{$event->getEndDate()->format($timeFormat)}
							</div>";
						}

						echo "</div>
						</td>";
						$wait[$roomName] = $size - 1;
					}
					else
					{
						if( user_access('administer conschedule') )
						{
							echo "<td>";
							$path = url("schedule/event/0/add/",array('query'=>array('room'=>$roomName,'date'=>$tableTime->format('YmdHis'))));
							echo "<a href=\"$path\">+</a>";
							echo "</td>";
						}
						else
						{
							echo "<td>&nbsp;</td>";
						}
					}
				}
				else
				{
					$wait[$roomName] -= 1;
				}
			}
			echo '</tr>';
			$tableTime->modify("+30 minutes");
		}
		echo '</table>';
	}
}

function conschedule_home(){
	$conschedule_path = drupal_get_path('module','conschedule');
	include_once($conschedule_path . '/Event.php');
	
	$cs = new ConSchedule();
	return $cs->printSchedule(TRUE);
}

//:=> /schedule/kiosk
function conschedule_kiosk(){
	$conschedule_path = drupal_get_path('module','conschedule');
	include_once($conschedule_path . '/Event.php');

	$cs = new ConSchedule();
	echo $cs->printSchedule(FALSE);
}

