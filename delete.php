<?php
/*
 *      delete.php
 *      
 *      Copyright 2008 Dylan Enloe <ninina@Siren>
 *		Copyright 2009 Drew Fisher <kakudevel@gmail.com>
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
function __autoload($class_name) {
    require_once $class_name . '.php';
}

$user = new User();
$page = new Webpage("Delete Event");
$connection = new Connection();

if(!isset($_GET['event']))
{
	echo "You need to provide an event to delete one";
	exit(0);
}

$eventID = $connection->validate_string($_GET['event']);

if($user->is_Admin())
{
	if(isset($_GET['confirm']))
	{
		#they want to delete it so lets delete it
		$query = "
DELETE FROM events
WHERE e_eventID = $eventID;";
		$connection->query($query);
		echo "<center>Event successfully deleted<br>";
		$page->addURL("index.php","Return to main schedule");
	}
	else
	{
		echo "<center>Do you really want to delete this event?<br>";
		$page->addURL("delete.php?event=$eventID&confirm='Yes'","Yes");
		echo "&nbsp;&nbsp;&nbsp;";
		$page->addURL("index.php","No"); 
	}
}
else
{
	echo "You cannot delete events.";
}
?>
