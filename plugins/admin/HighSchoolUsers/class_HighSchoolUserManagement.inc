<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: class_userManagement.inc 20602 2011-01-18 09:01:38Z hickert $$
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

require_once("/usr/share/php/fpdf/fpdf.php");
 
class HighSchoolUserManagement extends userManagement
{
	var $plHeadline     = "Usuaris de l'Institut";
    var $plDescription  = "Gestiona els usuaris de l'Institut";
    var $plIcon  = "plugins/users/images/user.png";

    // Tab definition 
    protected $tabClass = "HighSchoolUsertabs";
    protected $tabType = "HIGHSCHOOLUSERTABS";
    protected $aclCategory = "highschoolusers";
    protected $aclPlugin   = "highschoolusers";
    protected $objectName   = "highschooluser";
    
    var $sn1 = "";
	var $sn2 = "";
	var $employeeNumber = "";
	var $irisPersonalUniqueID = "";
	var $irisPersonalUniqueType = "";
	var $carLicense="";
    var $email = "";
    
    public $newuser_template = "user_template.tpl";
    
    public $template_dn = "";
    
    public $password_template = "password.tpl";
    
    public $userType = "none";
    
	function HighSchoolUserManagement($config,$ui)
    {
    	
        $this->config = $config;
        $this->ui = $ui;
        
        $this->filter_xml_file = "highschooluser-filter.xml";    
    	$this->list_xml_file = "highschooluser-list.xml";
    	
    	$this->newuser_template="user_template.tpl";
    	$this->password_template="password.tpl";
    	
    	$this->userType="none";

        $this->storagePoints = array(get_ou("core", "userRDN"));

        // Build filter
        if (session::global_is_set(get_class($this)."_filter")){
            $filter= session::global_get(get_class($this)."_filter");
        } else {
            $filter = new filter(get_template_path($this->filter_xml_file, true));
            $filter->setObjectStorage($this->storagePoints);
        }
        $this->setFilter($filter);

        // Build headpage
        $headpage = new listing(get_template_path($this->list_xml_file, true));
        $headpage->registerElementFilter("lockLabel", "HighSchoolUserManagement::filterLockLabel");
        $headpage->registerElementFilter("lockImage", "HighSchoolUserManagement::filterLockImage");
        $headpage->registerElementFilter("filterProperties", "HighSchoolUserManagement::filterProperties");
        $headpage->setFilter($filter);

        // Add copy&paste and snapshot handler.
        if ($this->config->boolValueIsTrue("core", "copyPaste")){
            $this->cpHandler = new CopyPasteHandler($this->config);
        }
        if($this->config->get_cfg_value("core","enableSnapshots") == "true"){
            $this->snapHandler = new SnapshotHandler($this->config);
        }

        management::__construct($config, $ui, "HighSchoolUsers", $headpage);
        
        // Register special user actions 
        $this->registerAction("lock",   "lockEntry");
        $this->registerAction("lockUsers",   "lockUsers");
        $this->registerAction("unlockUsers", "lockUsers");
        $this->registerAction("new_template", "newTemplate");
        $this->registerAction("newfromtpl", "newUserFromTemplate");
        $this->registerAction("templateContinue", "templateContinue");
        $this->registerAction("templatize", "templatizeUsers");
        $this->registerAction("templatizeContinue", "templatizeContinue");

        $this->registerAction("password", "changePassword");
        $this->registerAction("passwordQueue", "handlePasswordQueue");
        $this->registerAction("passwordCancel", "closeDialogs");

        $this->registerAction("sendMessage", "sendMessage");
        $this->registerAction("saveEventDialog", "saveEventDialog");
        $this->registerAction("abortEventDialog", "closeDialogs");

        // Register shortcut icon actions 
        $this->registerAction("edit_user","editEntry");
        $this->registerAction("edit_posixAccount","editEntry");
        $this->registerAction("edit_mailAccount","editEntry");
        $this->registerAction("edit_sambaAccount","editEntry");
        $this->registerAction("edit_netatalk","editEntry");
        $this->registerAction("edit_environment","editEntry");
        $this->registerAction("edit_gofaxAccount","editEntry");
        $this->registerAction("edit_phoneAccount","editEntry");
        
        //REGISTER ACTIONS
   		//Afegit per Sergi Tur 13/02/2011
		$this->registerAction("new_student", "newStudent");
		$this->registerAction("new_teacher", "newTeacher");
		$this->registerAction("new_user", "newUser");
		$this->registerAction("template_continue_reset", "templateContinueReset");
		//TODO
		//$this->registerAction("new_user", "newUser");
		
		// Print enrolment action
		$this->registerAction("print_enrolment","printEnrolment");
		
		//HighShoolActions
		//$this->registerAction("resetPasswords","resetPasswords");
		//$this->registerAction("resetPasswordsConfirmed", "resetPasswordsConfirmed");
		//$this->registerAction("resetPasswordsCancel", "resetPasswordsCancel");    
		//$this->registerAction("printResetPasswordsButton", "printEnrolments");    
        
    }
    
function getTemplatesByUserType($userType="none") {
	
	global $config;
    $ldap = $config->get_ldap_link();
    $templates = array();
    
    switch ($userType) {
    case "student":
        $this->template_dn=getTemplateDN("student");
        $templates= array($this->template_dn => $ldap->getCn($this->template_dn));
        break;
    case "teacher":
        $this->template_dn=getTemplateDN("teacher");
        $templates= array($this->template_dn => $ldap->getCn($this->template_dn));
        break;
    case "none":
        $this->template_dn="none";
        $templates['none']= _("none");
    	$templates = array_merge($templates,$this->get_templates());
        break;
	}
	return $templates;	
}

     /*! \brief  Intiates Student creation.
     *          -> See 'templateContinue' for further handling.
     */ 
function newUser($action="",$target=array(),$all=array(), 
$altTabClass ="", $altTabType = "", $altAclCategory="", $userType="none") {
	
	$this->userType=$userType;
    
    // Call parent method, it manages everything, locking, object creation...
    $str = management::newEntry($action,$target,$all);
	if(!empty($str)) return($str);
    
    $templates = $this->getTemplatesByUserType($this->userType);
    
    $smarty = get_smarty();
    
    //TODO: Check if template exists!! Show message if not
    $notfoundtemplates=check_templates($templates);
    //$notfoundtemplates= array ("DN DEL TEMPLATE QUE FALTA!");
    $smarty->assign("notfoundtemplates","");
    if (count($notfoundtemplates)) {
		$smarty->assign("notfoundtemplates",$notfoundtemplates);
	}
  
    // Set default variables, normally empty.
    foreach(array("sn", "givenName", "uid", "got_uid") as $attr){
      $smarty->assign($attr, "");
    }
           
    //Propose an employeeNumber value
    $nextInternalID= getNextInternalId();
    $academicPeriod= getAcademicPeriod();
      
    $academicPeriod=substr($academicPeriod,0,4).substr($academicPeriod,7,2);
      
    //$nextInternalID="201011-2000";
    $smarty->assign("highSchoolUserId", $academicPeriod."-".$nextInternalID);
    
	$proposedEmployeeNumber="";
	switch ($userType) {
    case "student":
        $proposedEmployeeNumber=$nextInternalID;
        break;
    case "teacher":
        $proposedEmployeeNumber="";
        break;
    case "none":
        $proposedEmployeeNumber=$nextInternalID;
        break;
	}
    
	$smarty->assign("userType",$this->userType);
    $smarty->assign("employeeNumber", $proposedEmployeeNumber);
    
	$smarty->assign('irisPersonalUniqueIDTypes',getIrisPersonalUniqueIDTypes()); 
			
	$smarty->assign('irisPersonalUniqueIDType', 0);
      
	// Set default "<br/>".__FILE__."(". __LINE__ ."):variables, normally empty.
    foreach(array("conflictiveField","existentuser","irisPersonalUniqueID",
		"givenName","sn1","sn2","highSchoolPersonalEmail","sn") as $attr){
		$smarty->assign($attr, "");
    }
            
    $smarty->assign("templates", $templates);
    $smarty->assign("template", $this->template_dn);
    $smarty->assign("edit_uid", "");
    
    $smarty->assign("userType",$userType);
    
 	return($smarty->fetch(get_template_path($this->newuser_template, TRUE)));
    
    // -> See 'templateContinue' for further handling!	
} 
    
     /*! \brief  Intiates Student creation.
     *          -> See 'templateContinue' for further handling.
     */ 
function newTeacher($action="",$target=array(),$all=array(), 
$altTabClass ="", $altTabType = "", $altAclCategory="") {
	 return $this->newUser($action, $target, $all, $altTabClass, $altTabType , 
    			   $altAclCategory, "teacher");	
}    

    /*! \brief  Intiates Student creation.
     *          -> See 'templateContinue' for further handling.
     */ 
function newStudent($action="",$target=array(),$all=array(), 
$altTabClass ="", $altTabType = "", $altAclCategory="")	{
    return $this->newUser($action, $target, $all, $altTabClass, $altTabType , 
    			   $altAclCategory, "student");
    }
    
function templateContinueReset($action="",$target=array(),$all=array(), 
$altTabClass ="", $altTabType = "", $altAclCategory="")	{
	// Preset input fields with user input.
    $smarty = get_smarty();
       		
   	$smarty->assign("irisPersonalUniqueID", "");
   	
   	$smarty->assign("existentuser","");
   	$smarty->assign("conflictiveField","");
   	$smarty->assign("notfoundtemplates","");
            
	foreach(array("highSchoolUserId","employeeNumber",
		"irisPersonalUniqueIDType",	"sn","sn1","sn2", 
		"givenName","highSchoolPersonalEmail", "uid", "template","userType") as $attr)	{
            
    	if(isset($_POST[$attr])){
    		$smarty->assign("$attr", set_post(get_post($attr)));
    	}else{
	    	$smarty->assign("$attr", "");
	    }
	}
            
   	$smarty->assign('irisPersonalUniqueIDTypes',getIrisPersonalUniqueIDTypes());
   	$templates = $this->getTemplatesByUserType($this->userType); 
    $smarty->assign("templates",$templates);
	$smarty->assign("got_uid", false);
	$smarty->assign("edit_uid",false);
   	$smarty->assign("userType",$this->userType);
	return($smarty->fetch(get_template_path($this->newuser_template, TRUE)));
        
    }
    
/* !\brief  This method is called whenever a template selection was displayed.
     *          Here we act on the use selection. 
     *          - Does the user want to create a user from template?
     *          - Create user without template?
     *          - Input correct, every value given and valid? 
     */ 
function templateContinue()
    {
		@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, " Entering..." ,"templateContinue");
        
		global $config;
		$resultsearch = "";
        $ldap = $config->get_ldap_link();
		
		// Get Student Template
        $templates = $this->getTemplatesByUserType($this->userType); 
        
        $smarty = get_smarty();
        $smarty->assign("existentuser","");
        $smarty->assign("conflictiveField","");
        $smarty->assign("notfoundtemplates","");
        $smarty->assign("userType",$this->userType);
        

        // Input validation, if someone wants to create a user from a template
        // then validate the given values.
        $message = array();
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $_POST['template'] ,"POST TEMPLATE");
        
        if(isset($_POST['checkTeacherCode'])) {
        	if(!isset($_POST['employeeNumber']) || (empty($_POST['employeeNumber']))){
        		$message[]= msgPool::required(_("Teacher Code"));	
        	}        	
        }
                
    	if(!isset($_POST['highSchoolUserId']) || (empty($_POST['highSchoolUserId']))){
    		$message[]= msgPool::required(_("Internal ID"));
		} else	{
			//TODO: Cheack for internalId duplicates
		}
        
        if(!isset($_POST['sn1']) || (empty($_POST['sn1']))){
			$message[]= msgPool::required(_("First Last Name"));
		}
        
		if(!isset($_POST['givenName']) || (empty($_POST['givenName']))){
            $message[]= msgPool::required(_("Given name"));
        }
        		
		if(!isset($_POST['irisPersonalUniqueID']) || (empty($_POST['irisPersonalUniqueID']))){
			$message[]= msgPool::required(_("External ID"));
		} else {
			//Check for externalID duplicates
			$resultsearch=searchDNByExternalId($_POST['irisPersonalUniqueID']);
			if ( $resultsearch != "") {
				//found duplicate externalID
				@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $resultsearch ,"Duplicate found DN ");;
                new log("view","users/".get_class($this),$this->dn,array(),"Found duplicated user ".$val['dn']);
                $message[]= msgPool::duplicated(_("Name"));
                $smarty->assign("existentuser",$resultsearch);
                $smarty->assign("conflictiveField",_("External ID"));
            }
		}
		
		/**************************
         * 0  Detected duplicated that user want to copy 
         ***************************/
        if(isset($_POST['copyExistingUser'])){
        	
            foreach(array("highSchoolUserId","irisPersonalUniqueIDType","irisPersonalUniqueID","employeeNumber","userType") as $attr){
                $this->$attr = get_post($attr);
                $this->tabObject->$attr = $this->$attr;
                $this->tabObject->by_object[$this->objectName]->$attr = $this->$attr;
            }

            // Adapt template values.
            //Page blank error if I active next line... TODO
            //@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->tabObject ,"tabobject:");

            $template_dn              = get_post('existingUser');
            
            $this->tabObject->by_object[$this->objectName]->originalDnWhenCopying=$template_dn;
            $this->tabObject->adapt_from_template($template_dn, array("highSchoolUserId","irisPersonalUniqueIDType","irisPersonalUniqueID","employeeNumber"));
            
       		$basefornewobject= $this->tabObject->base;
            $this->tabObject->by_object[$this->objectName]->base= $basefornewobject;
			$this->tabObject->by_object[$this->objectName]->baseSelector->setBase($basefornewobject);
			return("");
        }
        
 		/********************
         * 1   We've had input errors - Display errors and show input dialog again. 
         ********************/
        if (count($message) > 0){
            msg_dialog::displayChecks($message);
			// Preset input fields with user input.
            $smarty = get_smarty();
			foreach(array("highSchoolUserId","irisPersonalUniqueID","employeeNumber",
			    "irisPersonalUniqueIDType",	"sn","sn1","sn2", 
			    "givenName","highSchoolPersonalEmail", "uid", "template") as $attr)	{
            
                if(isset($_POST[$attr])){
                    $smarty->assign("$attr", set_post(get_post($attr)));
                }else{
                    $smarty->assign("$attr", "");
                }
			    if ( $resultsearch != "") {
					$smarty->assign("existentuser",$resultsearch);
            	}
            }
            
            $smarty->assign('irisPersonalUniqueIDTypes',getIrisPersonalUniqueIDTypes());
            $smarty->assign("templates",$templates);
            $smarty->assign("got_uid", false);
            $smarty->assign("edit_uid",false);
            $smarty->assign("userType",$this->userType);
            return($smarty->fetch(get_template_path($this->newuser_template, TRUE)));
        }

        /********************
         * 2   There was a template selected, now ask for the uid.
         ********************/
        if (!isset($_POST['uid'])){
			@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $_POST['template'] ,"template Selected");

			//Sergi Tur 17/02/2011
			//Added extra form fields
            // Remember user input.
            $smarty = get_smarty();
            $this->sn             			= get_post('sn');
            $this->sn1         	  			= get_post('sn1');
			$this->sn2         	  			= get_post('sn2');
            $this->givenName      			= get_post('givenName');
            $this->employeeNumber       	= get_post('employeeNumber');
			$this->highSchoolPersonalEmail                	= get_post('highSchoolPersonalEmail');
			$this->irisPersonalUniqueID 	= get_post('irisPersonalUniqueID');
			$this->irisPersonalUniqueIDType = get_post('irisPersonalUniqueIDType');
			$this->highSchoolUserId         = get_post('highSchoolUserId');
			
            // Preset uid field by using the idGenerator 
			$sn=$this->sn;
			$givenName=$this->givenName;
			
			
			$sn_array = explode(" ", $sn);
			$givenName_array = explode(" ", $givenName);
			
			$sn_first = $sn_array[0];
			$givenName_first = $givenName_array[0];
			
			#Pels arabs, no utilitzar "El" com a part del uid--> Massa repetits
			if (strcasecmp($sn_first, "El") == 0) {
				$sn_first = $sn_array[1];
			}
		        
			$stripped_sn = stripAccents ($sn_first);
			$stripped_givenName = stripAccents ($givenName_first);
			
			//Guio baix
			$stripped_sn1= strtr($stripped_sn, "-","_");
			$stripped_givenName1 = strtr($stripped_givenName,  "-","_");

			@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $stripped_sn1 ,"stripped_sn1: ");
			@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $stripped_givenName1 ,"stripped_givenName1: ");
			
			$attributes= array('sn1' => $stripped_sn1, 'givenName' => $stripped_givenName1); 
			
			
			$uids= highschoolusers_gen_uids ($attributes);
			@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $uids ,"uids: ");
			@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, count($uids) ,"countudis: ");		
			if (count($uids)){
				$smarty->assign("edit_uid", "false");
				$smarty->assign("uids", $uids);
				$this->uid= current($uids);
			}
			
            $this->got_uid= true;
            
            // Assign user input 
            foreach(array("highSchoolUserId","employeeNumber","irisPersonalUniqueID","irisPersonalUniqueIDType","sn","sn1","sn2","highSchoolPersonalEmail", "givenName","userType") as $attr){
                $smarty->assign("$attr", set_post($this->$attr));
            }
            
           	$smarty->assign('irisPersonalUniqueIDTypes', getIrisPersonalUniqueIDTypes());
            
            //??????
            foreach(array("uid", "got_uid") as $attr){
				$smarty->assign("$attr", $this->$attr);
			}
            if (isset($_POST['template'])){
                $smarty->assign("template", get_post('template'));
            }
            $smarty->assign("templates",$templates); 
            return($smarty->fetch(get_template_path($this->newuser_template, TRUE)));
		}
		
     	/********************
         * 3   No template - Ok. Lets fill the data into the user object and skip templating here. 
         ********************/
        if (get_post('template') == 'none'){
            foreach(array("sn","sn1","sn2","highSchoolUserId","employeeNumber","irisPersonalUniqueID","irisPersonalUniqueIDType", "givenName", "uid","highSchoolPersonalEmail") as $attr){
                if (isset($_POST[$attr])){
                    $this->tabObject->by_object[$this->objectName]->$attr= get_post($attr);
                }
            }

            // The user Tab object is already instantiated, so just go back and let the 
            //  management class do the rest.
            return("");
        }
    
        /********************
         * 4   Template selected and uid given - Ok, then lets adapt template values. 
         ********************/
        
        if(isset($_POST['uid'])) {
        	$this->tabObject->by_object[$this->objectName]->base= $basefornewobject;
			$this->tabObject->by_object[$this->objectName]->baseSelector->setBase($basefornewobject);
			//$this->tabObject->by_object[$this->objectName]->baseSelector->update(TRUE);
			@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $_POST['template'] ,"Case 4:");
            // Move user supplied data to sub plugins

			foreach(array("highSchoolUserId","employeeNumber","irisPersonalUniqueIDType","irisPersonalUniqueID","uid","sn","sn1","sn2","highSchoolPersonalEmail","givenName") as $attr){
                $this->$attr = get_post($attr);
                $this->tabObject->$attr = $this->$attr;
                $this->tabObject->by_object[$this->objectName]->$attr = $this->$attr;
            }

            // Adapt template values.
            //Page blank error if I active next line... TODO
            //@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->tabObject ,"tabobject:");

            $template_dn              = get_post('template');
             
            $this->tabObject->adapt_from_template($template_dn, array("uid","cn","givenName","sn","highSchoolUserId","irisPersonalUniqueIDType","irisPersonalUniqueID","sn1","sn2","highSchoolPersonalEmail","base"));

            // CODE NOT NECESSARY: We can use /home/%uid at templates to do the same!
            //Adapt homeDirectory from template. Use current uid as home folder at same path as template's homeDirectory
            /*
            $tmp_array= explode ("/",$this->tabObject->by_object['highschooluser']->homeDirectory);
            array_pop($tmp_array);
            $tmp_uid=$this->tabObject->by_object['highschooluser']->uid;
            $adapted_homeDirectory=implode("/",$tmp_array)."/".$tmp_uid;
            $this->tabObject->by_object['highschooluser']->homeDirectory=$adapted_homeDirectory;
            $this->tabObject->by_object['posixAccount']->homeDirectory=$adapted_homeDirectory;
            */
            
            //Enable PosixAccount $ sambaAccount is_account. Allows posix tab active by default.
            $this->tabObject->by_object['posixAccount']->is_account= TRUE;
            $this->tabObject->by_object['sambaAccount']->is_account= TRUE;
            
            //Last option when
            //Esta_cuenta_tiene_extensiones_GOsa_no_validas!
            #http://acacha.org/mediawiki/index.php/Desenvolupament_a_mida_de_Gosa#.C2.A1Esta_cuenta_tiene_extensiones_GOsa_no_validas.21
            //$this->tabObject->by_object['highschooluser']->is_account= TRUE;
                        
            //Base
            
            $basefornewobject= $this->tabObject->base;
            $this->tabObject->by_object[$this->objectName]->base= $basefornewobject;
		    $this->tabObject->by_object[$this->objectName]->baseSelector->setBase($basefornewobject);
			//$this->tabObject->by_object[$this->objectName]->baseSelector->update(TRUE);
			
	        // The user Tab object is already instantiated, so just go back and let the 
            //  management class do the rest.
            return("");
        } 
    }
    
    
    
     
  function printEnrolment() {
	
	if(isset($_POST['proposalSelected'])) $this->proposalSelected = get_post('proposalSelected') == 1;
	
	$report_url="pdfreports/enrollment.php";
	$new_password=get_post('new_password');
	$repeated_password = get_post('repeated_password');
  	//Get posted passwords
    if($this->proposalSelected){
    	$new_password = $this->proposal;
        $repeated_password = $this->proposal;
    }
	if(isset($_POST['proposalSelected'])) $password=get_post(proposalText);
	
	$smarty = get_smarty();
    $smarty->assign("proposal" , set_post($this->proposal));
    $smarty->assign("proposalEnabled" , $this->proposalEnabled);
    $smarty->assign("proposalSelected" , $this->proposalSelected);
    $smarty->assign("passwordChangeForceable" , $this->passwordChangeForceable);
   	$smarty->assign("enforcePasswordChange" , $this->enforcePasswordChange);
	
	
	// Check posted passwords now.            
	$message= array();
    if ($new_password != $repeated_password){
    	$message[]= _("The passwords you've entered as 'New password' and 'Repeated new password' do not match.");
    } else {
    	if ($new_password == ""){
        	$message[] = msgPool::required(_("New password"));
        }
	}

	// Call external check hook to validate the password change
    if(!count($message)){
    	$attrs = array();
        $attrs['current_password'] = '';
        $attrs['new_password'] = $new_password;
        $checkRes = password::callCheckHook($this->config,$this->dn,$attrs);
        if(count($checkRes)){
        	$message[] = sprintf(_("Check-hook reported a problem: %s. Password change canceled!"), 
           	implode($checkRes));
        }
    }

    // Display errors
    if (count($message) != 0){
    	msg_dialog::displayChecks($message);
        return($smarty->fetch(get_template_path($this->password_template, TRUE)));
    }
	
	$report_url=$report_url."?dn=".urlencode($this->dn)."&password=".urlencode($new_password);
  	
	echo js_redirect_newwindow($report_url,0,"enrollmentWindow_" . urlencode($this->dn)."_".rand(),false);
	
	//exit();
	
	// Change password 
	if(isset($this->force_hash_type[$this->dn])){
		if(!change_password ($this->dn, $new_password,0,$this->force_hash_type[$this->dn],'', $message)){
      		msg_dialog::displayChecks(array($message));
            return($smarty->fetch(get_template_path($this->password_template, TRUE)));
   		}
  	}else{
    	if(!change_password ($this->dn, $new_password,0,'','',$message)){
        	msg_dialog::displayChecks(array($message));
            return($smarty->fetch(get_template_path($this->password_template, TRUE)));
     	}
 	}

  }
    
// Inject user actions 
function detectPostActions()
    {
        $action = userManagement::detectPostActions();
        
        //HighSchoolActions
		if(isset($_POST['print_enrolment'])) $action['action'] = "print_enrolment";	
		if(isset($_POST['template_continue_reset'])) $action['action'] = "template_continue_reset";	
		if(isset($_POST['resetPasswords_confirmed'])) $action['action'] = "resetPasswordsConfirmed";   
		if(isset($_POST['resetPasswords_cancel'])) $action['action'] = "resetPasswordsCancel";  
		if(isset($_POST['printResetPasswordsButton'])) $action['action'] = "printResetPasswordsButton";  

        return($action);
    }

}
?>
