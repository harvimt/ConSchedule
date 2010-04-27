<?php
/*
 *		event.inc
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

function conschedule_event($event_id,$op='view')
{
	if($op == 'view')
	{
		return conschedule_event_view($event_id);
	}
	else
	{
		return drupal_get_form('conschedule_event_form',$event_id,$op);
	}
}

//implements hook_form
function conschedule_event_form($form_state, $event_id, $op)
{
	//drupal_add_js(drupal_get_path('');
	foreach(array('endTime','startTime') as $js_id)
	{
		//con_dump(array($js_id=>array('timesteps'=>array(1,30,0))));
		//$extraDrupalSettings['datePopup']["edit-$js_id-timeEntry-popup-1"]['settings']['timeSteps'] = array(1,30,0);
		//drupal_add_js($extraDrupalSettings,'setting');
	}

	//Preconditions
	assert(con_is_int($event_id));
	assert(in_array($op, array('add','edit', 'delete')));
	if($op == 'view' || $op == 'edit')
	{
		assert($event_id > 0);
	}
	else
	{
		//assert($event_id == 0);
	}

	//branch for different operations
	if($op == 'delete')
	{
		$result = db_query('SELECT e_eventname FROM {conschedule_events} WHERE e_eventid = %d', $event_id);
		$data = db_fetch_object($result);
		$event_name = $data->e_eventname;
		return array(
			'title' => array(
				'#value' => "Are you sure you want to delete the event \"$event_name\"?",
			),
			'eventid' => array(
				'#type' => 'hidden',
				'#value' => $event_id,
			),
			'_op' => array(
				'#type' => 'hidden',
				'#value' => 'delete',
			),
			'submit' => array(
				'#type' => 'submit',
				'#value' => 'Yes',
			),
		);
	}

	//Load Panel Rooms
	$panelRooms = array();
	$result = db_query("SELECT r_roomid, r_roomname FROM {conschedule_rooms}");
	while($row = db_fetch_object($result))
	{
		$panelRooms[$row->r_roomid] = $row->r_roomname;
	}

	//more branching
	if($op == 'edit')
	{
		$result = db_query('SELECT 
			e_roomid, e_eventname, e_color,
			e_datestart, e_dateend, 
			e_eventdesc, e_panelist
			FROM {conschedule_events}
			WHERE e_eventid = %d', $event_id);
		$data = db_fetch_object($result);
		if($data === FALSE){
			drupal_set_message('No Event with id, ' . $event_id, 'error');
		}
	}
	else if($op == 'add')
	{
		$data = new stdClass();
		$data->e_eventname = '';
		$data->e_color = '#CCCCCC';
		$data->e_eventdesc = '';
		$data->e_panelist = '';

		if(isset($_GET['room']))
		{
			$data->e_roomid = array_search($_GET['room'],$panelRooms);
		}
		else
		{
			$data->e_roomid = '1';
		}

		if(isset($_GET['date']))
		{
			$date = date_create($_GET['date']);
			$data->e_datestart = $date->format('Y-m-d H:i');
			$date->modify('+30 minutes');
			$data->e_dateend = $date->format('Y-m-d H:i');
		}
		else
		{
			$data->e_datestart = date('Y-m-d H:i',strtotime('now'));
			$data->e_dateend = date('Y-m-d H:i',strtotime('+ 1 hour'));
		}
	}

	//return metadata array
	return array(
		'title' => array(
			'#type' => 'textfield',
			'#title' => t('Title'),
			'#default_value' => $data->e_eventname,
			'#maxlength' => 255,
		),
		'description' => array(
			'#type' => 'textarea',
			'#title' => t('Description'),
			'#default_value' => $data->e_eventdesc,
			'#size' => 44,
			'#maxlength' => 255,
		),
		'startTime' => array(
			'#type' => 'date_popup',
			'#title' => t('Start Date'),
			'#default_value' => $data->e_datestart,
			'#date_increment' => 30,
		),
		'endTime' => array(
			'#type' => 'date_popup',
			'#title' => t('End Date'),
			'#default_value' => $data->e_dateend,
			'#date_increment' => 30,
		),
		'color' => array(
			'#type' => 'colorpicker_textfield',
			'#default_value' => $data->e_color,
		),
		'panelRoom' => array(
			'#type' => 'select',
			'#title' => t('Panel Room'),
			'#default_value' => $data->e_roomid,
			'#options' => $panelRooms,
		),
		'panelist' => array(
			'#type' => 'textfield',
			'#title' => t('Panelist'),
			'#default_value' => $data->e_panelist,
		),
		'eventid' => array(
			'#type' => 'hidden',
			'#value' => $event_id,
		),
		'_op' => array(
			'#type' => 'hidden',
			'#value' => $op,
		),
		'submit' => array(
			'#type' => 'submit',
			'#value' => t('Save'),
		),
	);
}

function conschedule_event_form_validate($form, &$form_state)
{
	if($form_state['values']['_op'] != 'delete'){
		$startTime = date_create($form_state['values']['startTime']);
		$endTime = date_create($form_state['values']['endTime']);
		$startTimeArr = getdate($startDate->getTimestamp());
		$endTimeArr = getdate($endDate->getTimestamp());

		if($form_state['values']['title'] == '')
		{
			form_set_error('title', t('You must give an event a title'));
		}
		if( $startTime > $endTime )
		{
			form_set_error('endTime', t('End Date must be after Start Date'));
		}

		if($startTimeArr['seconds'] != 0)
		{
			form_set_error('startTime', t('Start Time must not contain seconds (how did you set them?)'));
		}
		if($endTimeArr['seconds'] != 0)
		{
			form_set_error('endTime', t('End Time must not contain seconds (how did you set them?)'));
		}


		if($startTimeArr['minutes'] != 0 && $startTimeArr['minutes'] != 30)
		{
			form_set_error('startTime', t('Start Time Must be on an interval of 30 minutes'));
		}
		if($endTimeArr['minutes'] != 0 && $endTimeArr['minutes'] != 30)
		{
			form_set_error('endTime', t('End Time Must be on an interval of 30 minutes'));
		}

	}
	
}

function conschedule_event_form_submit($form, &$form_state)
{
	//con_dump($form_state['values']);
	if($form_state['values']['_op'] == 'delete'){
		db_query('DELETE FROM {conschedule_events} WHERE e_eventid = %d', $form_state['values']['eventid']);
		drupal_set_message(t('Event Deleted Successfully'));
		drupal_goto('schedule');
	}
	else if($form_state['values']['_op'] == 'add')
	{
		db_query("INSERT INTO {conschedule_events} (
			e_roomid, e_eventname, e_color,
			e_datestart, e_dateend, 
			e_eventdesc, e_panelist
			) VALUES (%d,'%s','%s','%s','%s','%s','%s')",
			$form_state['values']['panelRoom'], $form_state['values']['title'], $form_state['values']['color'],
			$form_state['values']['startTime'], $form_state['values']['endTime'],
			$form_state['values']['description'], $form_state['values']['panelist']
			);

		drupal_set_message(t('Event Added'));
	}
	else if($form_state['values']['_op'] == 'edit')
	{
		db_query("UPDATE {conschedule_events} SET
			e_roomid = %d, e_eventname = '%s', e_color = '%s',
			e_datestart = '%s', e_dateend = '%s', 
			e_eventdesc = '%s', e_panelist = '%s'
			WHERE e_eventid = %d
			",
			$form_state['values']['panelRoom'], $form_state['values']['title'], $form_state['values']['color'],
			$form_state['values']['startTime'], $form_state['values']['endTime'],
			$form_state['values']['description'], $form_state['values']['panelist'],
			$form_state['values']['eventid']
			);

		drupal_set_message(t('Event Updated'));
	}
	else
	{
		drupal_set_message(t('hit else, shouldn\'t do that, op: ' . $form_state['values']['_op']),'error');
	}

}

function conschedule_event_view($event_id){
	global $user;
	assert(con_is_int($event_id));
	assert($event_id > 0);

	$result = db_query('SELECT 
		r_roomname,
		e_roomid, e_eventname, e_color,
		e_datestart, e_dateend, 
		e_eventdesc, e_panelist
		FROM {conschedule_events}
		LEFT JOIN {conschedule_rooms} ON e_roomid = r_roomid
		WHERE e_eventid = %d', $event_id);
	$data = db_fetch_object($result);

	if($data === FALSE)
	{
		drupal_set_message('No Event with id, ' . $event_id, 'error');
	}

	if($data->e_eventdesc == '')
	{
		$data->e_eventdesc = 'Panelist did not supply a description of this event';
	}

	if($data->e_panelist == '')
	{
		$data->e_panelist = 'Panelist not assigned to this event yet';
	}


	//TODO should probably use the theme engine.
	$time = date('F jS g:i a',strtotime($data->e_datestart)) . ' - ' . date('g:i a',strtotime($data->e_dateend));
	$out = '';
	$out .= "<p><strong>Title:</strong> {$data->e_eventname}</p>";
	$out .= "<p><strong>Room:</strong> {$data->r_roomname}</p>";
	$out .= "<p><strong>Time:</strong> $time</p>";
	//$out .= "<p><strong>Color</strong>: <span style=\"background-color:{$data->e_color}\">{$data->e_color}</span></p>";
	$out .= "<p><strong>Panelist:</strong> {$data->e_panelist}</p>";
	$out .= '<p><strong>Description:</strong><br>';
	$out .= nl2br($data->e_eventdesc) . '</p>';


	//Next/Previous links
	$result = db_query("SELECT e_eventid FROM {conschedule_events} WHERE e_roomid = %d AND e_datestart < '%s' ORDER BY e_datestart DESC",
		$data->e_roomid, $data->e_datestart);
	$row = db_fetch_object($result);
	
	$out .= '<a href="'.url('schedule/event/'.$row->e_eventid.'/view').'">Previous Event</a>&nbsp;';

	$result = db_query("SELECT e_eventid FROM {conschedule_events} WHERE e_roomid = %d AND e_datestart > '%s' ORDER BY e_datestart ASC",
		$data->e_roomid, $data->e_dateend);
	$row = db_fetch_object($result);
	
	$out .= '<a href="'.url('schedule/event/'.$row->e_eventid.'/view').'">Next Event</a>';


	//userschedule
	if(user_is_logged_in())
	{
		$result = db_query('SELECT us_uschedid FROM {conschedule_userschedule} WHERE us_eventid = %d AND us_userid = %d',
			$event_id, $user->uid);

		if( db_fetch_array($result) === FALSE)
		{
			$out .= '<p><a href="'.url('schedule/userschedule/addevent/'.$event_id).'">Add to user Schedule </a></p>';
		}
		else
		{
			$out .= '<p><a href="'.url('schedule/userschedule/delevent/'.$event_id).'">Remove From User Schedule</a></p>';
		}
		$out .= '<p><a href="'.url('schedule/userschedule').'">View your schedule</a></p>';
	}
	else
	{
		//TODO: to the forums? link?
		$out .= '<p>Log in or register to add this event to your own custom schedule!';
	}
	return $out;
}
