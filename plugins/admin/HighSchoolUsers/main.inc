<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: main.inc 19817 2010-09-28 06:01:13Z hickert $$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* Remove locks created by this plugin
 */

require_once ("functions.php");

if ($remove_lock){
    if(session::is_set('HighSchoolUserManagement')){
        $macl = session::get('HighSchoolUserManagement');
        $macl->remove_lock();
    }
}

/* Remove this plugin from session
 */
if ( $cleanup ){
	session::un_set('HighSchoolUserManagement');
}else{

    /* Create HighSchoolUserManagement object on demand */
    if (!session::is_set('HighSchoolUserManagement')){
	    $HighSchoolUserManagement= new HighSchoolUserManagement ($config, $ui);
        session::set('HighSchoolUserManagement',$HighSchoolUserManagement);
    }
    $HighSchoolUserManagement = session::get('HighSchoolUserManagement');
    $display= $HighSchoolUserManagement->execute();
    /* Reset requested? */
    if (isset($_GET['reset']) && $_GET['reset'] == 1){
        session::un_set ('HighSchoolUserManagement');
    }

    /* Show and save dialog */
    session::set('HighSchoolUserManagement',$HighSchoolUserManagement);
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
