<?php
/*
 * This code is for GOsa (http://www.gosa-project.org)
 * Copyright (C) Sergi Tur Badenas
 *
 * ID: $$Id$$
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

/*!
  \brief   user plugin
  \author  Sergi Tur Badenas <stur@iesebre.com>
  \version 1.0
  \date    13.03.2011

  This class provides the functionality to read and write all attributes
  relevant for person, organizationalPerson, inetOrgPerson and gosaAccount
  from/to the LDAP. It does syntax checking and displays the formulars required.
 */

class highschooluser extends user
{
  var $base;
	
  /* Definitions */
  var $plHeadline= "Generic";
  var $plDescription= "Edit organizational highschool user settings";

  /* Plugin specific values */
  // Added by Sergi Tur
  var $irisPersonalUniqueID = "";
  var $sn1;
  var $sn2;
  var $highSchoolUserId;
  
  var $irisPersonalUniqueIDType = -1;
  var $highSchoolTSI;
  var $highSchoolPersonalEmail;
  
  var $highSchoolUserPosition1 = 0;
  var $highSchoolUserPosition2 = 0;
  
  var $highSchoolUserLastModifiedBy = "null";
  var $highSchoolUserLastModifiedAt = "null";
  var $highSchoolUserLastModifiedAtBackup = "null";
  var $highSchoolUserCreatedBy = "null";
  var $highSchoolUserCreatedAt = "null";
  
  var $searchbasednforduplicateUids;
  
  var $originalDnWhenCopying ="";
  
  
  //employeeNumber
  
  /* attribute list for save action */
  var $attributes= array("highSchoolUserId","sn","sn1","sn2", "givenName", "uid","irisPersonalUniqueID","irisPersonalUniqueIDType", "highSchoolUserPosition1", 
  	  "highSchoolUserPosition2","highSchoolUserCreatedBy","highSchoolUserCreatedAt","highSchoolUserLastModifiedBy","highSchoolUserLastModifiedAt",
      "highSchoolTSI","highSchoolPersonalEmail","homePostalAddress","postalCode", "homePhone", "labeledURI", "ou", "o", "dateOfBirth", "gender","preferredLanguage",
      "departmentNumber", "employeeNumber", "employeeType", "l", "st","jpegPhoto",
      "roomNumber", "telephoneNumber", "mobile","email", "pager","carLicense", "cn", "userPKCS12",
      "postalAddress", "facsimileTelephoneNumber", "userSMIMECertificate", "gosaLoginRestriction", "manager","homeDirectory");

  var $objectclasses= array("top", "gosaAccount", "highSchoolUser", "person", "inetOrgPerson", "organizationalPerson",
  	  "extensibleObject","irisPerson");
  
    /* constructor, if 'dn' is set, the node loads the given
     'dn' from LDAP */
  function highschooluser (&$config, $dn= NULL)
  {
  	global $BASE_DIR;
  	
  	$this->list_template=$BASE_DIR."/plugins/personal/highschoolgeneric/highschoolusergeneric.tpl";
  	$this->searchbasednforduplicateUids="ou=All,".$config->current['BASE'];
  	 	
  	parent::user($config,$dn);
   	
  }


  /* execute generates the html output for this node */
  function execute()
  {
  	date_default_timezone_set('Europe/Madrid');
  	$old_lang = setlocale(LC_ALL, 0);
  	setlocale(LC_TIME, "ca_ES.UTF-8");
  	if ($this->highSchoolUserLastModifiedAt!="null")	{
    	$this->highSchoolUserLastModifiedAt= date("h:i:s d/m/Y",
    		$this->highSchoolUserLastModifiedAt);
    }
    
  	if ($this->highSchoolUserCreatedAt!="null")	{
  		$this->highSchoolUserLastModifiedAtBackup=$this->highSchoolUserCreatedAt;
    	$this->highSchoolUserCreatedAt= date("h:i:s d/m/Y",
    		$this->highSchoolUserCreatedAt);
    }
    setlocale(LC_ALL, $old_lang);
  	
    /* Call parent execute */
    $output = user::execute();
    
    if ($this->irisPersonalUniqueIDType==-1) $this->irisPersonalUniqueIDType=0;
    
    $output = js_fill_selects($this->irisPersonalUniqueIDType,
				$this->highSchoolUserPosition1,$this->highSchoolUserPosition2). $output;
	
    return $output;
    /*
    $smarty->assign("irisPersonalUniqueID",$this->irisPersonalUniqueID);
    return($smarty->fetch (get_template_path('highschoolusergeneric.tpl', TRUE, dirname(__FILE__))));
    */
  }
  
  /* Adapt from given 'dn' */
  function adapt_from_template($dn, $skip= array())
  {
    user::adapt_from_template($dn, $skip); 
    
    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->dn);
    $data = $ldap->get_attribute($this->originalDnWhenCopying,"jpegPhoto");

    if((!$data) || ($data == "*removed*")){ 
      /* In case we don't get an entry, load a default picture */
      $this->set_picture ();
      $this->jpegPhoto= "*removed*";
    }else{
      /* Set picture */
      $this->photoData= $data;
      session::set('binary',$this->photoData);
      session::set('binarytype',"image/jpeg");
      $this->jpegPhoto= "";
    }
  
  }
  
  /* Save data to LDAP*/
  function save() {
  	
  	//Obtain current nextInternalId
  	$nextInternalID= getNextInternalId();
  	
  	$ui = get_userinfo();
    //print_a($ui);
 	//echo "<br/>user DN: " . $ui->dn;
 	
  	$this->highSchoolUserLastModifiedBy=$ui->dn;
  	$this->highSchoolUserLastModifiedAt=time();
  	
  	//Restore unix timestamp Before save
  	$this->highSchoolUserCreatedAt=$this->highSchoolUserLastModifiedAtBackup;
 
  	if ($this->highSchoolUserCreatedBy=="null")	{
  		$this->highSchoolUserCreatedBy=$ui->dn;  		
  		$this->highSchoolUserCreatedAt=time();
  	}
  	
  	/* First use parents methods to do some basic fillup in $this->attrs */
    if (!user::save ()){
    	//Save is Ok
    	//Increment nextInternalId
    	if ($this->is_new)	{
  			incrementNextInternalId();
  		}
  	}
    
    return (0);
  
  }
  
  function save_object()	{
  	
  	if(isset($_POST['highschoolgeneric']) || isset($_POST['multiple_user_posted'])){
		user::save_object();  			
  	}
  }
  
  static function plInfo()
  {
  
    $govattrs= array(
        "gouvernmentOrganizationalUnit"             =>  _("Unit"), 
        "houseIdentifier"                           =>  _("House identifier"), 
        "vocation"                                  =>  _("Vocation"),
        "ivbbLastDeliveryCollective"                =>  _("Last delivery"), 
        "gouvernmentOrganizationalPersonLocality"   =>  _("Person locality"),
        "gouvernmentOrganizationalUnitDescription"  =>  _("Unit description"),
        "gouvernmentOrganizationalUnitSubjectArea"  =>  _("Subject area"),
        "functionalTitle"                           =>  _("Functional title"),
        "certificateSerialNumber"                   =>  _("Certificate serial number"),
        "publicVisible"                             =>  _("Public visible"),
        "street"                                    =>  _("Street"),
        "role"                                      =>  _("Role"),
        "postalCode"                                =>  _("Postal code"));

    $ret = array(
        "plShortName" => _("Generic"),
        "plDescription" => _("Generic highschol user settings"),
        "plSelfModify"  => TRUE,
        "plDepends"     => array(),
        "plPriority"    => 1,
        "plSection"     => array("personal" => _("My account")),
        "plCategory"    => array("highschoolusers" => array("description" => _("Highschool Users "),
                                                  "objectClass" => "highSchoolUser")),

        "plRequirements"=> array(
            'ldapSchema' => array(
    			'highSchoolUser' => '>=0.1',
                'gosaAccount' => '>=2.7',
                'gosaUserTemplate' => '>=2.7'
                ),
            'onFailureDisablePlugin' => array(get_class(),'HighSchoolUserManagement', 'highschooluser')
            ),

        "plProperties" => array(
            array(
                "name"          => "accountRDN",
                "type"          => "string",
                "default"       => "",
                "description"   =>  _("Pattern for the generation of user DNs. Please read the FAQ for details."),
                "check"         => "gosaProperty::isString",
                "migrate"       => "",
                "group"         => "plugin",
                "mandatory"     => FALSE
                )

            ),
        "plProvidedAcls" => array(

          "sn"                => _("Surname"),
          "givenName"         => _("Given name"),
          "uid"               => _("Login"),
          
          "sn1"               => _("First last name"),
          "sn2"               => _("Second last name"),
          "fullName"               => _("Full name"),
          "irisPersonalUniqueID"    => _("External ID"),
          "irisPersonalUniqueIDType" => _("External ID type"),
          "highSchoolUserId"    => _("Internal ID"),         
          "highSchoolUserPosition1"  => _("Position1"),
          "highSchoolUserPosition2"  => _("Position2"),
          "highSchoolPersonalEmail"  => _("Email"),
          
          "gosaUserDefinedFilter"  => _("Allow definition of custom filters"),

          "personalTitle"     => _("Personal title"),
          "academicTitle"     => _("Academic title"),

          "dateOfBirth"       => _("Date of birth"),
          "gender"            => _("Sex"),
          "preferredLanguage" => _("Preferred language"),
          "base"              => _("Base"), 

          "userPicture"       => _("User picture"),

          "gosaLoginRestriction" => _("Login restrictions"),         

          "o"                 => _("Organization"),
          "ou"                => _("Department"),
          "departmentNumber"  => _("Department number"),
          "manager"           => _("Manager"),
          "employeeNumber"    => _("Employee number"),
          "employeeType"      => _("Employee type"),
		  "highSchoolTSI"	  => _("TSI"),
          "roomNumber"        => _("Room number"),
          "telephoneNumber"   => _("Telephone number"),
          "pager"             => _("Pager number"),
          "carLicense"        => _("carLicense"),
          "mobile"            => _("Mobile number"),
          "facsimileTelephoneNumber"     => _("Fax number"),

          "st"                => _("State"),
          "l"                 => _("Location"),
          "postalAddress"     => _("Postal address"),
          "postalCode"        => _("Postal code"),
          "email"             => _("Email"),
          

          "homePostalAddress" => _("Home postal address"),
          "homePhone"         => _("Home phone number"),
          "labeledURI"        => _("Homepage"),
          "userPassword"      => _("User password method"), 
          "Certificate"       => _("User certificates"))

        );

#   /* Append government attributes if required */
#   global $config;
#   if($config->get_cfg_value("core","honourIvbbAttributes") == "true"){
#     foreach($govattrs as $attr => $desc){
#       $ret["plProvidedAcls"][$attr] = $desc;
#     }
#   }
    return($ret);
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
