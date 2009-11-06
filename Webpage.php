<?php
/*
 *      Webpage.php
 *      
 *      Copyright � 2008 Dylan Enloe <ninina@koneko-hime>
 *		Copyright � 2009 Drew Fisher <kakudevel@gmail.com>
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

class Webpage {
	function __construct($title)
	{
		echo "
<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"
  \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">
<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">
<head>
	<title>Mewcon: $title</title>
	<meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" />
	<LINK href='MEWschedule.css' rel='stylesheet' type='text/css'>
</head>
<body>";
	}
	
	function __destruct()
	{
		echo "</body></html>";
	}
	
	public function addURL($location, $text)
	{
		echo "<a href=\"$location\">$text</a>";
	}
	
	public function printNoEvents()
	{
		echo "<center>";
		echo "<h2>No events have yet been planned =T.T=</h2>";
		echo "</center>";
	}
	
	// conOpens and conCloses are epxected to be of type Date
	public function printDaySchedule($schedule, $roomNames, $conOpens, $conCloses) 
	{
	
		// only print 24 hours from beginning time maximum
		$dayCheck = clone($conOpens);
		$dayCheck->modify("+1 days");
		if( $dayCheck->format("U") < $conCloses->format("U") )
		{
			$conCloses = $dayCheck;
		}
		
		$halfHoursOpen = ((($conCloses->format("U") - $conOpens->format("U"))/60/60)*2)+1;
		$tableTime = clone($conOpens);
		
		echo '<table cellpadding=0 cellspacing=0><thead><td></td>';
		//initialize the wait on each room to zero
		//might as well print out the top row too
		foreach($roomNames as $roomName)
		{
			echo "<td style=\"width: 13%;\">$roomName</td>";
			$wait[$roomName] = 0;
		}
		echo "</thead>";
		for($i=0; $i < $halfHoursOpen; $i+=1)
		{
			echo '<tr>';
			$tF = $tableTime->format("H:i");
			echo "<td class=\"timeColumn\" style=\"width: 5%;\" align=\"center\">" . $tF . "</td>";
		
			foreach($roomNames as $roomName)
			{
				if($wait[$roomName] == 0)
				{
					$tF = $tableTime->format("Y-m-d H:i:s");
					if(isset($schedule[$tF][$roomName]))
					{			
						//print the item
						$event = $schedule[$tF][$roomName];
						$name = $event->getEventName();
						$color = $event->getColor();
						$size = $event->getEventLengthInHalfHours();
						$eventID = $event->getEventID();
						
						echo "<td class=\"foundEvent\" rowspan=\"" . $size
							. "\" bgcolor=\"" . $color . "\">";
						
						echo "<div class=\"startTime\">";
						echo $event->getStartDate()->format("H:i");
						echo "</div>";
						
						echo "<div class=\"event\">"; 
						$this->addURL("view.php?event=$eventID",$name);
						echo "</div>";
						
						echo "<div class=\"endTime\">";
						echo $event->getEndDate()->format("H:i");
						echo "</div>";
						echo"</td>";
						$wait[$roomName] = $size - 1;
					}
					else
					{
						echo "<td> </td>";
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
	
	public function createEventForm($connection)
	{
		$query ="
SELECT r_roomID, r_roomname
FROM rooms
ORDER BY (r_roomID);";
		$connection->query($query);
	
		echo "
<form action=\"add.php?action=add\" method=\"post\" enctype=\"multipart/form-data\">
Event Name: 
<br>
<input type=\"text\" name=\"name\">
<br><br />
NOTE: If an event spans multiple days, you'll have to add the event to each day individually.
<br><br />
Time of the event:<br />
Format is: YYYY-MM-DD HH:MM (e.g. 2009-08-02 14:30)<br />
Start : <input type=\"text\" name=\"start\" size=\"16\"> 
End: <input type=\"text\" name=\"end\" size=\"16\">
<br>
Color to make event (html color, including the #):
<br>
<input type=\"text\" name=\"color\">
<br>
Primary panelist's forum name(may be empty):
<br>
<input type=\"text\" name=\"panalist\">
<br>
Description of the event (may be empty):
<br>
<textarea name=\"desc\" rows=\"10\" cols=\"60\"></textarea>
<br>
Select Room:
<br>
<select name=\"room\">";
		while($row = $connection->fetch_row())
		{
			$roomID = $row[0];
			$roomname = $row[1];
			echo "<option value=\"$roomID\">$roomname</option>";
		}
		echo "</select>
<br><br>
<input type=\"submit\" name=\"add\" value=\"Finished\">
</form>
";	
	}
	
	public function printEvent($event)
	{
		$eventID = $event->getEventID();
		$name = $event->getEventName();
		$room = $event->getRoomName();
		$start = $event->getStartDate();
		$end = $event->getEndDate();
		$desc = $event->getDesc();
		$panelist = $event->getPanelist();
		
		echo "<center>";
		echo "<table id=\"viewEvent\" cellpadding=0 cellspacing=0>";
		echo "<colgroup>";
		echo "<col class=\"property\" />";
		echo "<col class=\"value\" />";
		echo "</colgroup>";
		echo "<tr><td>Panel<br />Name</td><td>" . $name . "</td></tr>";
		echo "<tr><td>Room</td><td>" . $room . "</td></tr>";
		echo "<tr><td>Date</td><td>" . $start->format("l, F d Y") . "</td></tr>";
		echo "<tr><td>Start Time</td><td>" . $start->format("H:i");
			echo "&nbsp; &nbsp; &nbsp;(" . $start->format("g:i a") . ")</td></tr>";
		echo "<tr><td>End Time</td><td>" . $end->format("H:i");
			echo "&nbsp; &nbsp; &nbsp;(" . $end->format("g:i a") . ")</td></tr>";
		
		if( $panelist != "")
		{
			echo "<tr><td>Panelist</td><td>" . $panelist . "</td></tr>";
		}
		
		if( $desc != "")
		{
			$desc = str_replace("\\n","<br /><br />",$desc);
			echo "<tr><td>Description</td><td>" . $desc . "</td></tr>";
		}
		
		echo "</table>";
		echo "</center>";
	}
	
	public function printAdminEdit($event, $eventID, $connection)
	{
		$name = $event->getEventName();
		$room = $event->getRoomName();
		$start = $event->getStartDate();
		$end = $event->getEndDate();
		$desc = $event->getDesc();
		$panelist = $event->getPanelist();
		$color = $event->getColor();
		echo "
<form action=\"edit.php?update=admin&event=$eventID\" method=\"post\" enctype=\"multipart/form-data\">
Update Event Name: 
<br>
<input type=\"text\" name=\"name\" value=\"$name\">
<br><br>
Color to make event (html color):
<br>
<input type=\"text\" name=\"color\" value=\"$color\">
<br><br>
Change the time of the event:
<br>
Format is: YYYY-MM-DD HH:MM (e.g. 2009-08-02 14:30)<br />
Start : <input type=\"text\" name=\"start\" size=\"16\" value = \"" . $start->format("Y-m-d H:i") . "\"> 
End: <input type=\"text\" name=\"end\" size=\"16\" value = \"" . $end->format("Y-m-d H:i") . "\">
<br><br>
Primary panelist's forum name (may be empty):
<br>
<input type=\"text\" name=\"panalist\" value= \"$panelist\">
<br><br>
Change Room:
<br>
<select name=\"room\">";
		$query ="
SELECT r_roomID, r_roomname
FROM rooms
ORDER BY (r_roomID);";
		$connection->query($query);
		while($row = $connection->fetch_row())
		{
			$roomID = $row[0];
			$roomname = $row[1];
			if($room == $roomname)
			{
				echo "<option value=\"$roomID\" selected=\"yes\">$roomname</option>";
			}
			else
			{
				echo "<option value=\"$roomID\">$roomname</option>";
			}
		}
		echo "</select>
<br><br>
Edit the Description of this Panel:<br>
<textarea name=\"desc\" rows=\"10\" cols=\"60\">$desc</textarea>
<br><br>
<input type=\"submit\" name=\"add\" value=\"Update\">
</form><br />";
	echo "<form action=\"delete.php?event=$eventID\" method=\"post\" enctype=\"multipart/form-data\">";
	echo "<input type=\"submit\" value=\"Delete Event\"></form><br />";
	}

	public function printPanelistEdit($event, $eventID)
	{
		$name = $event->getEventName();
		$desc = $event->getDesc();
		echo "
<form action=\"edit.php?update=panelist&event=$eventID\" method=\"post\" enctype=\"multipart/form-data\">
Update Event Name: 
<br>
<input type=\"text\" name=\"name\" value=\"$name\">
<br><br>
Edit the Description of this Panel:<br>
<textarea name=\"desc\" rows=\"10\" cols=\"60\">$desc</textarea>
<br><br>
<input type=\"submit\" name=\"add\" value=\"Update\"></form><br>";
	}
	
	public function printError($err)
	{
		echo "<center>";
		echo "<h2>". $err ."</h2>";
		echo "</center>";
	}
	
	// _GET_checkEventID:
	// checks the passed get param for validity.
	// prints an error if the param isn't set,
	// or the eventID doesn't exist.
	// On Success returns an Event object when 
	// $createEvent is TRUE, or the EventID when $createEvent is FALSE.
	// On failure always returns NULL.
	public function _GET_checkEventID($_GETvar, $connection, $createEventObj = TRUE)
	{
		// make sure the provided event is valid
		if( ! isset($_GETvar) )
		{
			$this->printError("EventID must be supplied.");
			echo "<center>";
			$this->addURL("index.php","Return to event schedule.");
			echo "</center>";
			return NULL;
		}
		
		$eID = $connection->validate_string($_GETvar);
		
		// type check eventID
		if( is_null($eID) || $eID == '' || ! is_numeric($eID) )
		{
			$this->printError("Problem with passed EventID.");
			echo "<center>";
			$this->addURL("index.php","Return to event schedule.");
			echo "</center>";
			return NULL;
		}
		
		$q = NULL;
		
		if( $createEventObj == FALSE)
		{
			// we only care the the event exists, so keep the query simple.
			$q = "SELECT e_eventID FROM events WHERE e_eventID = $eID;";
		}
		else
		{
			// get the actual event from the db
			$q = "
			SELECT
				e_eventID, r_roomName, e_dateStart, e_dateEnd, 
				e_eventName, e_color, e_eventDesc, e_panelist
			FROM
				events, rooms
			WHERE 
				e_eventID = ". $eID ."
				AND
				r_roomID = e_roomID
			;";
		}
		$connection->query($q);

		if( $connection->result_size() != 1 )
		{
			$this->printError("EventID doesn't exist in the database.");
			echo "<center>";
			$this->addURL("index.php","Return to event schedule.");
			echo "</center>";
			return NULL;
		}
		
		if( $createEventObj == FALSE)
		{
			return $eID;
		}
		else
		{
			$row = $connection->fetch_assoc();

			return new Event(
				$row['e_eventID'], $row['e_eventName'], $row['r_roomName'], 
				$row['e_dateStart'],$row['e_dateEnd'], $row['e_eventDesc'], 
				$row['e_panelist'], $row['e_color']
			);
		}
	}
}
?>
