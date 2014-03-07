<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: functions.inc 13100 2008-12-01 14:07:48Z hickert $$
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
 

/*
 * Returns true, else an array 
 */
function check_templates($templates) {
	global $config;

	$ldap= $config->get_ldap_link();
	
	$notfoundtemplates=array();
	foreach($templates as $templatedn => $templatename) { 
		if ($templatedn=="none") continue;
		$ldap->cat($templatedn,array("cn"));
		if(!$ldap->count()){
			$notfoundtemplates[]=$templatedn;
		}
	}
	
	if (count($notfoundtemplates)) {
		return $notfoundtemplates;
	} else {
		return array();
	}
	
}

function getIrisPersonalUniqueIDTypes() {
	
	return array(	0 => _("DNI"),
                 	1 => _("NIE"),
      				2 => _("Passport"),
                 	3 => _("Others"));
}

function getPositions() {
	
	return array(	0 => _("None"),
					1 => _("Course tutor"),
                 	2 => _("Department Manager"),
      				3 => _("Director"),
                 	4 => _("Assistant director"),
                 	5 => _("Head Teacher"),
                 	6 => _("Administrator"),
                 	7 => _("Coordinador pedagogic"),
                 	8 => _("Coordinador FP"),
                 	9 => _("Coordinadora FP"),
                 	10 => _("Tutoria tècnica web"),
                 	11 => _("Coordinador informàtica"),
                 	12 => _("Coordinador pla estrategic"),
                 	13 => _("Delegat"),
                 	14 => _("Subdelegat"),
                 	15 => _("Consell Escolar"));
}

function js_redirect($url, $seconds=5) {
	
	$output = <<<EOD

<script language="JavaScript">
$this->highSchoolUserPosition1
<!-- hide code from displaying on browsers with JS turned off

function redirect() {
	window.location = "$url";
}

timer = setTimeout('redirect()', '$seconds*1000');
-->

</script>
EOD;

	return $output;
}

function js_fill_selects($selectedIrisPersonalUniqueIDType,$selectedPosition1,$selectedPosition2) {
	
	$irisPersonalUniqueIDTypes=getIrisPersonalUniqueIDTypes();
	$irisPersonalUniqueIDTypesArrayInJavascript= '["'. implode('","',$irisPersonalUniqueIDTypes).'"]';
	
	$positions=getPositions();
	$positions= '["'. implode('","',$positions).'"]';
	
        if ($selectedIrisPersonalUniqueIDType=="") $selectedIrisPersonalUniqueIDType=0;
	
	$output = <<<EOD
	

<script language="JavaScript">

Event.observe(window, 'load', function() {
	var irisPersonalUniqueIDType=document.getElementById("irisPersonalUniqueIDType");
	
	var irisPersonalUniqueIDTypes = $irisPersonalUniqueIDTypesArrayInJavascript;
	
	for(var i=0; i<irisPersonalUniqueIDTypes.length; i++) {
		var value = irisPersonalUniqueIDTypes[i];
		if (i== $selectedIrisPersonalUniqueIDType) {
			irisPersonalUniqueIDType.options[irisPersonalUniqueIDType.length] = 
    			new Option(value,i,true);
    	} else {
    		irisPersonalUniqueIDType.options[irisPersonalUniqueIDType.length] = 
    			new Option(value,i);
    	}
	}
	
	var highSchoolUserPosition1=document.getElementById("highSchoolUserPosition1");
	var highSchoolUserPosition2=document.getElementById("highSchoolUserPosition2");
	
	var positions = $positions;
	
	for(var i=0; i<positions.length; i++) {
		var value = positions[i];
		if (i== $selectedPosition1) {
			highSchoolUserPosition1.options[highSchoolUserPosition1.length] = 
    			new Option(value,i,true);
    	} else	{
    		highSchoolUserPosition1.options[highSchoolUserPosition1.length] = 
    			new Option(value,i);
    	}
	}
	
	for(var i=0; i<positions.length; i++) {
		var value = positions[i];
		if (i== $selectedPosition2) {
			highSchoolUserPosition2.options[highSchoolUserPosition2.length] = 
    			new Option(value,i,true);
    	} else	{
    		highSchoolUserPosition2.options[highSchoolUserPosition2.length] = 
    			new Option(value,i);
    	}
	}
	
});


</script> 
EOD;

	return $output;
}

function js_redirect_newwindow($url, $seconds=5,$windowName="newWindow",$forceclose=true) {
	
	$output = <<<EOD

<script language="JavaScript">

<!-- hide code from displaying on browsers with JS turned off

function redirect() {
	window.open("$url","$windowName");
}
EOD;
if ($forceclose) {
	$output = $output.<<<EOD
window.close("$windowName");
EOD;
} 
	$output = $output. <<<EOD

timer = setTimeout('redirect()', '$seconds*1000');
-->

</script>
EOD;

	return $output;
}

return true;

/*
 * Returns "" if no user exists with this externaId
 * Returns user DN if already exists a user with the specified externalID
 * */

function getEnrollmentData($dn) {
	global $config;

	$ldap= $config->get_ldap_link();
	
	$ldap->cd($dn);
	$ldap->cat($dn,array("givenName","sn1","sn2","uid","highSchoolUserId","employeeNumber","irisPersonaluniqueID","email","highSchoolPersonalEmail"));
	
	if($ldap->count()){
        return $ldap->fetch();
   	} else{
        return "";
    }
}

function searchDNByExternalId($externalId,$basesearchdn="") {
	global $config;
	
	if ($basesearchdn == "") {
		//TODO: Obtain from Config file
		$basesearchdn="dc=iesebre,dc=com";
	}

	$ldap= $config->get_ldap_link();
	$ldap->cd ($basesearchdn);           
	$search_string="(&(irisPersonalUniqueID=".normalizeLdap($externalId)."))";

	$ldap->search ($search_string,array("dn","givenName"));
    $val = $ldap->fetch();
    if ($ldap->count () != 0){
     	if(isset($val['dn'])){
			return $val['dn'];
		}
    }
    return "";
}

function get_config_attributes() {
	global $config;
	  
	$ldap = $config->get_ldap_link();	
	$gosaconfigdn= $config->get_cfg_value("core","config");
	
	$ou="highschoolusers";
	$highschoolusersOUconfigdn="ou=$ou,".$gosaconfigdn;

	$ldap->cd($gosaconfigdn);

  	$ldap->search("(&(ou=$ou))",array("ou"));
  	if (!$ldap->success()){
  	    msg_dialog::display(_("Configuration error"), sprintf(_("Cannot found highschoolusers OU config DN!")."<br><br>"._('Error: %s'), "<br><br><i>".$ldap->get_error()."</i>"), ERROR_DIALOG);
	    return;
  	}
  	/* Add config ou if not present*/
  	if ($ldap->count() == 0){
    	add_config_ou($highschoolusersOUconfigdn,$ou);
  	}
  	
  	$ldap->cd($highschoolusersOUconfigdn);
  	$currentAcademicPeriod=calculateCurrentAcademicPeriod();
  	
  	$ldap->search("(&(ObjectClass=highSchoolUsersConfig)(academicPeriod=$currentAcademicPeriod))");
	if (!$ldap->success()){
  	    msg_dialog::display(_("Configuration error"), sprintf(_("Cannot found highschoolusers config DN!")."<br><br>"._('Error: %s'), "<br><br><i>".$ldap->get_error()."</i>"), ERROR_DIALOG);
	    return;
  	}
  	
	/* Add config object if not present*/
  	if ($ldap->count() == 0){
    	add_config_object($highschoolusersOUconfigdn,$ou);
  	}
  	return $ldap->fetch();
  	
}

function getAcademicPeriod() {
	$attrs = get_config_attributes();
  	return $attrs["academicPeriod"][0];
}

function getNextInternalId() {
	$attrs = get_config_attributes();
	return $attrs["nextInternalId"][0];
}

function incrementNextInternalId() {
	global $config;
	
	$attrs = get_config_attributes();
	  
	$ldap = $config->get_ldap_link();
		
	$ldap->cd($attrs["dn"]);
	$insert['nextInternalId'] = ((int) $attrs["nextInternalId"][0]) +1;
    $ldap->modify($insert);
	if (!$ldap->success()){
  	    msg_dialog::display(_("Configuration error"), sprintf(_("Cannot insert new nextInternalId value!")."<br><br>"._('Error: %s'), "<br><br><i>".$ldap->get_error()."</i>"), ERROR_DIALOG);
	    return;
  	}
}

function getTemplateDN($type="student") {
	$attrs = get_config_attributes();
	switch ($type) {
		case "student":
			$templateDN= $attrs["studentTemplateDN"][0];
			break;
		case "teacher":
			$templateDN= $attrs["teacherTemplateDN"][0];
			break;
		default:
			$templateDN= "";
		break;
	}
	
	return $templateDN;
}

function add_config_ou($dn,$ou) {
		global $config;
	  	$ldap = $config->get_ldap_link();

	  	$attrs= array();
		$attrs["objectClass"] = array("top","organizationalUnit");
    	$attrs["ou"] = $ou;
    	
    	$ldap->cd($dn);
    	$ldap->add($attrs);
    	if (!$ldap->success()){
      		msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), "cn=$name,".$config->get_cfg_value("core","config"), 0, ERROR_DIALOG));
      		return;
    	}
}

function add_config_object($dn,$ou) {
		global $config;
	  	$ldap = $config->get_ldap_link();

	  	$attrs= array();
		$attrs["objectClass"] = array("top","highSchoolUsersConfig");
    	$attrs["academicPeriod"] = calculateCurrentAcademicPeriod();
    	$attrs["nextInternalId"] = "1";
    	
    	$studentTemplateDN="cn=Nou Alumne,ou=people,ou=Alumnes,ou=All,dc=iesebre,dc=com";
    	$teacherTemplateDN="cn=Nou profe,ou=people,ou=Profes,ou=All,dc=iesebre,dc=com";
    	
    	if (isset ($config) && $config->get_cfg_value("core",'studentTemplateDN') != "") {
			$studentTemplateDN=$config->get_cfg_value("core",'studentTemplateDN');
		}
    	if (isset ($config) && $config->get_cfg_value("core",'teacherTemplateDN') != "") {
			$teacherTemplateDN=$config->get_cfg_value("core",'teacherTemplateDN');
		}
    	
    	$attrs["studentTemplateDN"] = $studentTemplateDN;
    	$attrs["teacherTemplateDN"] = $teacherTemplateDN;
    	
    	
    	$ldap->cd("academicPeriod=".$attrs["academicPeriod"].",".$dn);
    	$ldap->add($attrs);
    	if (!$ldap->success()){
      		msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), "cn=$name,".$config->get_cfg_value("core","config"), 0, ERROR_DIALOG));
      		return;
    	}
}

function calculateCurrentAcademicPeriod() {
	global $config;
	/* Check for global language settings in gosa.conf */
	//print_a($config);
	$strdaylimit="25 June 2011";
	if (isset ($config) && $config->get_cfg_value("core",'changePeriodDaylimit') != "") {
		$strdaylimit=$config->get_cfg_value("core",'changePeriodDaylimit');
	}
	
	$year= (int) date("o");
	$daylimit= (int) date("z",strtotime($strdaylimit));
	$dayOfTheYear= (int) date("z");
	$previousyear=$year-1;
	$nextyear=$year+1;
	if ($dayOfTheYear>$daylimit) {
		return "$year-$nextyear";
	} else {
		return "$previousyear-$year";
	}
}
 
function stripAccents($string){
	#return preg_replace("/[^\x9\xA\xD\x20-\x7F]/", "", $string);
	#return strtr($string,'àáâãäçèéêëìíîïñòóôõöùúûüýÿÀÁÂÃÄÇÈÉÊËÌÍÎÏÑÒÓÔÕÖÙÚÛÜÝ','aaaaaceeeeiiiinooooouuuuyyAAAAACEEEEIIIINOOOOOUUUUY');
	$table = array(
        'Š'=>'S', 'š'=>'s', 'Đ'=>'Dj', 'đ'=>'dj', 'Ž'=>'Z', 'ž'=>'z', 'Č'=>'C', 'č'=>'c', 'Ć'=>'C', 'ć'=>'c',
        'À'=>'A', 'Á'=>'A', 'Â'=>'A', 'Ã'=>'A', 'Ä'=>'A', 'Å'=>'A', 'Æ'=>'A', 'Ç'=>'C', 'È'=>'E', 'É'=>'E',
        'Ê'=>'E', 'Ë'=>'E', 'Ì'=>'I', 'Í'=>'I', 'Î'=>'I', 'Ï'=>'I', 'Ñ'=>'N', 'Ò'=>'O', 'Ó'=>'O', 'Ô'=>'O',
        'Õ'=>'O', 'Ö'=>'O', 'Ø'=>'O', 'Ù'=>'U', 'Ú'=>'U', 'Û'=>'U', 'Ü'=>'U', 'Ý'=>'Y', 'Þ'=>'B', 'ß'=>'Ss',
        'à'=>'a', 'á'=>'a', 'â'=>'a', 'ã'=>'a', 'ä'=>'a', 'å'=>'a', 'æ'=>'a', 'ç'=>'c', 'è'=>'e', 'é'=>'e',
        'ê'=>'e', 'ë'=>'e', 'ì'=>'i', 'í'=>'i', 'î'=>'i', 'ï'=>'i', 'ð'=>'o', 'ñ'=>'n', 'ò'=>'o', 'ó'=>'o',
        'ô'=>'o', 'õ'=>'o', 'ö'=>'o', 'ø'=>'o', 'ù'=>'u', 'ú'=>'u', 'û'=>'u', 'ý'=>'y', 'ý'=>'y', 'þ'=>'b',
        'ÿ'=>'y', 'Ŕ'=>'R', 'ŕ'=>'r',
    );
   
    return strtr($string, $table);
}


function highschoolusers_gen_uids($attributes)
{
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
	// Strip out non ascii chars
    foreach($attributes as $name => $value){
        if ( $config->get_cfg_value("core", "forceTranslit") == "true" ) {
             $value = cyrillic2ascii($value);
        } else {
             $value = iconv('UTF-8', 'US-ASCII//TRANSLIT', $value);
        }
        $value = preg_replace('/[^(\x20-\x7F)]*/','',$value);
        $attributes[$name] = strtolower($value);
    }
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $attributes, "Prepare"); 

    // Create proposal array
    /*
    $rules = array($rule);
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, print_r($replacements) ,"replacements ");
    foreach($replacements as $tag => $values){
        $rules = gen_uid_proposals($rules, $tag,$values);
    }
    */
    // Create proposal array
    $baseuid=$attributes['givenName'].$attributes['sn1'];
    $rules = array($baseuid);
    for ($i = 1; $i < 100; $i++) {
    	$rules[] = $baseuid."$i";	
    }
    // Create result set by checking which uid is already used and which is free.
    $ret = array();
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $rules ,"rules: ");
    foreach($rules as $rule){
		@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $rule ,"rule");
		@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, 'uid='.normalizeLdap($rule) ,"Searching user with: ");
        $ldap->search('uid='.normalizeLdap($rule));
		@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $ldap->count() ,"Ldap count: ");
	    if(!$ldap->count()){
            $ret[] =  $rule;
            break;
        }
    }
    return($ret);
}

?>
