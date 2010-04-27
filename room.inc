<?php
/*
 *		room.inc
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

function conschedule_room($room_id=NULL,$op='list'){
	if($op == 'list')
	{
		return conschedule_room_list();
	}
	else
	{
		assert(!is_null($room_id));
		return drupal_get_form('conschedule_room_form',$room_id,$op);
	}
}

//implements hook_form
function conschedule_room_form($form_state, $room_id, $op)
{
	//Preconditions
	if($op != 'add')
	{
		assert(con_is_int($room_id));
	}
	assert(in_array($op, array('add','edit', 'delete')));
	if($op == 'edit'){
		assert($room_id > 0);
	}else {
		//assert($event_id == 0);
	}

	//branch for different operations
	if($op == 'delete')
	{
		$result = db_query('SELECT r_roomname FROM {conschedule_rooms} WHERE r_roomid = %d', $room_id);
		$data = db_fetch_object($result);
		$room_name = $data->r_roomname;
		return array(
				'title' => array(
					'#value' => "Are you sure you want to delete the room \"$room_name\"?",
				),
				'roomid' => array(
					'#type' => 'hidden',
					'#value' => $room_id,
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

	//more branching
	if($op == 'edit')
	{
		$result = db_query('SELECT 
				r_roomname
				FROM {conschedule_rooms}
				WHERE r_roomid = %d', $room_id);
		$data = db_fetch_object($result);
		if($data === FALSE){
			drupal_set_message('No Room with id, ' . $event_id, 'error');
		}
	}
	else if($op == 'add')
	{
		$data = new stdClass();
		$data->r_roomname = '';
	}

	//return metadata array
	return array(
		'roomname' => array(
			'#type' => 'textfield',
			'#title' => t('Room Name'),
			'#default_value' => $data->r_roomname,
			'#maxlength' => 255,
		),
		'roomid' => array(
			'#type' => 'hidden',
			'#value' => $room_id,
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

function conschedule_room_form_validate($form, &$form_state)
{
	//con_dump($form_state['values']);
	if($form_state['values']['_op'] != 'delete'){
		if($form_state['values']['roomname'] == '')
		{
			form_set_error('roomname', t('You must give a room a name'));
		}
	}
}

function conschedule_room_form_submit($form, &$form_state)
{
	//con_dump($form_state['values']);
	if($form_state['values']['_op'] == 'delete'){
		db_query('DELETE FROM {conschedule_rooms} WHERE r_roomid = %d', $form_state['values']['roomid']);
		drupal_set_message(t('Room Deleted Successfully'));
		drupal_goto('schedule/room');
	}
	else if($form_state['values']['_op'] == 'add')
	{
		db_query("INSERT INTO {conschedule_rooms} ( r_roomname) VALUES ('%s')",
				$form_state['values']['roomname']);

		drupal_set_message(t('Room Added'));
	}
	else if($form_state['values']['_op'] == 'edit')
	{
		db_query("UPDATE {conschedule_rooms} SET r_roomname = '%s' WHERE r_roomid = %d",
				$form_state['values']['roomname'], $form_state['values']['roomid']);
		drupal_set_message(t('Room Updated'));
	}
	else
	{
		drupal_set_message(t('hit else, shouldn\'t do that, op: ' . $form_state['values']['_op']),'error');
	}
}

function conschedule_room_list()
{
	$result = db_query("SELECT r_roomid, r_roomname FROM {conschedule_rooms}");
	$rows = array();
	while($row = db_fetch_object($result))
	{
		$rows[]= array(
			'name' => $row->r_roomname,
			'edit' => '<a href="'.url('schedule/room/'.$row->r_roomid.'/edit').'">Edit</a>',
			'delete' => '<a href="'.url('schedule/room/'.$row->r_roomid.'/delete').'">Delete</a>',
		);
	}
	$headers = array('Room Name','','');
	return theme_table($headers, $rows, array('id'=>'conschedule_row_list'));
}
