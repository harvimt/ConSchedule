<?php
// vim: syn=php
/*
 *      tag.inc
 *      
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


/*
   What is a "tag" well a tag is a con or sub-con, it consists of a date range and a set of rooms
   The tag is then divided into days
   This replaces the old $conTimes variable and the $_GET['conday'] variable

   This is a surprisingly complex piece of machinery
   */

function conschedule_tag($tag_id = null, $op='list')
{
	if($op == 'list')
	{
		return conschedule_tag_list();
	}
	else
	{
		return drupal_get_form('conschedule_tag_form',$tag_id, $op);
	}
}

function conschedule_tag_list()
{
	$result = db_query('SELECT t_tagid, t_tagname FROM {conschedule_tags}');
	$row = db_fetch_object($result);
	$rows = array();

	if($row === FALSE)
	{
		drupal_set_message('No Tags set, this is kind of a problem', 'error');
		return '<a href="'.url('schedule/tag/0/add').'">Add Tag</a>';
	}
	do
	{
		$rows[] = array(
			'tag' => $row->t_tagname,
			'edit' => '<a href="'.url('schedule/tag/'.$row->t_tagid.'/edit').'">Edit</a>',
			'delete' => '<a href="'.url('schedule/tag/'.$row->t_tagid.'/delete').'">Delete</a>',
		);
	}
	while($row = db_fetch_object($result));
	return theme_table(array('Tag Name', '', ''), $rows);
}

function conschedule_tag_form($form_state, $tag_id, $op)
{
	assert(in_array($op, array('add','edit','delete')));
	assert(is_int($tag_id));

	//con_dump($form_state);
	//con_dump($tag_id);
	//con_dump($op);

	$form = array(
		'_op' => array(
			'#type' => 'hidden',
			'#value' => $op,
		),
		'tagid' => array(
			'#type' => 'hidden',
			'#value' => $tag_id,
		),
	);

	if($op == 'delete')
	{
		$result = db_query('SELECT t_tagname FROM {conschedule_tags} WHERE t_tagid = %d', $tag_id);
		$data = db_fetch_object($result);
		$form['sure'] = array('#value'=>'Are you sure you want to delete tag "'.$data->t_tagname.'"');
		$form['submit'] = array('#type'=>'submit','#value'=>'yes');
		return $form;
	}
	else if($op == 'edit')
	{
		$result = db_query('SELECT t_tagname FROM {conschedule_tags} WHERE t_tagid = %d', $tag_id);
		$data = db_fetch_object($result);
		if($data === FALSE)
		{
			drupal_set_message('No such Tag', 'error');
			drupal_goto('schedule/tag/0/list');
		}
		//con_dump($data);
	}
	else if($op == 'add')
	{
		$data = new stdClass();
		$data->t_tagname = '';
	}

	$form['general'] = array (
		'#type' => 'fieldset',
		'#title' => 'General',
		't_tagname' => array(
			'#title' => 'Tag Name',
			'#type' => 'textfield',
			'#maxlength' => 40,
			'#default_value' => $data->t_tagname,
		)
	);

	if($op == 'edit')
	{
		//Rooms
		$form['rooms'] = array ('#type'=>'fieldset','#title'=>'Rooms');
		$result = db_query('SELECT
			CASE WHEN tr_roomid IS NULL THEN 1 ELSE 0 END is_in_tag,
		   	r_roomid, r_roomname
			FROM {conschedule_rooms}
			LEFT JOIN {conschedule_tags_rooms} ON tr_roomid = r_roomid
			WHERE tr_tagid = %d OR tr_tagid IS NULL
			',$tag_id);

		$rows = array();
		$rooms = array();

		$headers = array('Name','');

		while($row = db_fetch_object($result))
		{
			if($row->is_in_tag == 1)
			{
				$url = url('schedule/tag/'.$tag_id.'/roomrem/'.$row->r_roomid);
				$rows[] = array(
					'name' => $row->r_roomname,
					'add' => '<a href="'.$url.'">Remove from Tag</a>',
				);
			}
			else
			{
				//$rooms[] = $row->r_roomname;
				$rooms[$row->r_roomid] = $row->r_roomname;
			}
		}

		if(empty($rows))
		{

			$form['rooms']['table']['#value'] = 'No rooms attached yet';
		}
		else
		{
			$form['rooms']['table']['#value'] = theme_table($headers, $rows);
		}

		if(empty($rooms))
		{
			$form['rooms']['addroom']['#value'] = 'No rooms to add';
		}
		else
		{
			$form['rooms']['addroom'] = array(
				'#type'=>'select',
				'#title' => 'Rooms not attached',
				'#options' => $rooms,
			);
			/*
			$form['rooms']['addroom_btn'] = array(
				'#type'=>'submit',
				'#value'=>'Button',
			)
			*/
		}
		//Tag Days
	}

	$form['submit'] = array('#type'=>'submit', '#value'=>'Save');

	return $form;
}

function conschedule_tag_form_validate($form,&$form_state)
{
	if($form_state['values']['t_tagname'] == '')
	{
		form_set_error('t_tagname', 'Must set a tagname');
	}
}

function conschedule_tag_form_submit($form,&$form_state)
{
	if($form_state['values']['_op'] == 'delete')
	{
		db_query('DELETE FROM {conschedule_tags_rooms} WHERE tr_tagid = %d', $form_state['values']['tagid']);
		db_query('DELETE FROM {conschedule_tags_days} WHERE td_tagid = %d', $form_state['values']['tagid']);
		$result = db_query('DELETE FROM {conschedule_tags} WHERE t_tagname = %d', $form_state['values']['tagid']);
		if(db_affected_rows($result) == 1)
		{
			drupal_set_message('Tag Deleted Successfully');
		}
		else
		{
			drupal_set_message('Tag not deleted successfully','error');
		}
		drupal_goto('schedule/tag/0/list');
	}
	else if($form_state['values']['_op'] == 'add')
	{
		$result = db_query('INSERT INTO {conschedule_tags} (t_tagname) VALUES (\'%s\')', $form_state['values']['t_tagname']);
		if(db_affected_rows($result) == 1)
		{
			drupal_set_message('Tag Added Successfully');
		}
		else
		{
			drupal_set_message('Tag not added successfully','error');
		}
		drupal_goto('schedule/tag/0/list');
	}
	else if($form_state['values']['_op'] == 'edit')
	{
		$result = db_query('UPDATE {conschedule_tags} SET t_tagname = \'%s\' WHERE t_tagid = %d',
			$form_state['values']['t_tagname'], $form_state['values']['tagid']);
		if(db_affected_rows($result) == 1)
		{
			drupal_set_message('Tag Updated Successfully');
		}
		else
		{
			drupal_set_message('Tag not updated successfully','error');
		}
		drupal_set_message('Tag Updated');
		drupal_goto('schedule/tag/'.$form_state['values']['t_tagname'].'/edit');
	}
	else
	{
		drupal_set_message('Hit else, that shouldn\'t happen');
	}
}

//##TAG ROOM HANDLING
function conschedule_tag_roomadd($tag_id, $room_id)
{
	assert(is_int($room_id));
	assert(is_int($tag_id));

	$result = db_query('DELETE FROM {conschedule_tags_rooms} WHERE tr_tagid = %d AND tr_roomid = %d',
		$tag_id, $room_id);

	if(db_affected_rows($result) == 1)
	{
		drupal_set_message('Room removed from Tag');
	}
	else
	{
		drupal_set_message('Room not attached to Tag','error');
	}

	drupal_goto('schedule/tag/'.$tag_name.'/edit');
}

function conschedule_tag_roomrem($tag_id, $room_id)
{
	assert(is_int($room_id));
	assert(is_int($tag_id));

	$result = db_query('SELECT tr_trid FROM {conschedule_tags_rooms} WHERE tr_tagid = %d AND tr_roomid = %d',
		$tag_id, $room_id);

	if( db_fetch_array($result) === FALSE)
	{
		db_query('INSERT INTO {conschedule_tags_rooms} (tr_tagid, tr_roomid) VALUES (%d, %d)',
			$tag_id, $room_id);
		drupal_set_message('Room added to Tag');
	}
	else
	{
		drupal_set_message('Room already attached','error');
	}

	drupal_goto('schedule/tag/'.$tag_name.'/edit');
}

//##TAG DAY
function conschedule_tagday($tag_id = null, $tag_day = 0, $op='edit')
{
	return drupal_get_form('conschedule_tagday_form', $tag_id, $tag_day $op);
}

function conschedule_tagday_form($form_state, $tag_id, $tag_day, $op)
{
	//Preconditions
	assert(is_int($tag_id));
	assert($tag_id >= 0);
	assert(is_int($tag_day));
	assert($tag_day >= 0);
	assert(in_array($op, array('add','edit','delete')));

	$form = array(
		'tagid' => array(
			'#type' => 'hidden',
			'#value' => $tag_id,
		),
		'tagday' => array(
			'#type' => 'hidden',
			'#value' => $tag_day
		)
		'_op' => array(
			'#type' => 'hidden',
			'#value' => $op,
		),
		'submit' => array(
			'#type' => 'submit',
			'#value' => t('Save'),
		),
	);

	if($op == 'delete')
	{
		$form['title'] = array('#value'=>'Are you sure you want to delete this tag day?');
		$form['submit']['#value'] = 'Yes';
	}

	if($op == 'edit')
	{
		$result = db_query('SELECT td_datestart, td_dateend FROM {conschedule_tagdays} WHERE td_tagid = %d AND td_day = %d',
			$tag_id, $tag_day);
		$data = db_fetch_object($result);
		if($data === FALSE)
		{
			drupal_set_message('No such Tag Day','error');
		}
	}
	else
	{
		$data = new stdClass();
		$data->td_datestart = date('Y-m-d H:i',strtotime('now'));
		$data->td_dateend = date('Y-m-d H:i',strtotime('+ 1 day'));
	}
	
	$form['startTime'] = array(
		'#type' => 'date_popup',
		'#title' => t('Start Date'),
		'#default_value' => $data->e_datestart,
		'#date_increment' => 30,
	);
	$form['endTime' = array(
		'#type' => 'date_popup',
		'#title' => t('End Date'),
		'#default_value' => $data->e_dateend,
		'#date_increment' => 30,
	);

	return $form;
}

function conschedule_tagday_form_validate($form, &$form_state)
{

}

function conschedule_tagday_form_submit($form, &$form_state)
{
	if($form_state['values']['_op'] == 'delete')
	{
		$result = db_query('DELETE FROM {conschedule_tagdays} WHERE td_tagid = %d AND td_day',
			$form_state['values']['tagid'], $form_state['values']['tagday']);

		if(db_affected_rows($result) == 1)
		{
			db_set_message('Tag day deleted successfully');
		}
		else
		{
			db_set_message('There was an error while deleting a Tag Day','error');
		}
	}
	else if($form_state['values']['_op'] == 'add')
	{
		$result = db_query('INSERT INTO {conschedule_tagdays} (td_tagid,td_day,td_datestart,td_dateend) VALUES (%d,%d,\'%s\',\'%s\')',
			$form_state['values']['tagid'], $form_state['values']['tagday'],
			$form_state['values']['td_datestart'], $form_state['values']['td_dateend']);

		if(db_affected_rows($result))
		{
			drupal_set_message('Tag day added successfully');
			drupal_goto('schedule/tag/'.$form_state['values']['tagid'].'/edit');
		}else{
			db_set_message('There was an error while deleting a Tag Day','error');
		}
	}
	else if($form_state['values']['_op'] == 'edit')
	{
		
		$result = db_query('UPDATE {conschedule_tagdays} SET td_tagid = %d,td_day = %d,td_datestart = \'%s\',td_dateend = \'%s\')',
			$form_state['values']['tagid'], $form_state['values']['tagday'],
			$form_state['values']['td_datestart'], $form_state['values']['td_dateend']);

		if(db_affected_rows($result))
		{
			drupal_set_message('Tag day updated successfully');
			drupal_goto('schedule/tag/'.$form_state['values']['tagid'].'/edit');
		}else{
			db_set_message('There was an error while deleting a Tag Day','error');
		}
	}
	else
	{
		drupal_set_message('Hit else, this shouldn\' happen','error');
	}
}
