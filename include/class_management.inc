<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: class_management.inc 20952 2011-07-27 06:38:29Z hickert $$
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

class management
{
  // Public 
  public $config = null;
  public $ui     = null;

  // The plugin description
  public $plugname      = "unconfigured";
  public $plIcon        = "unconfigured";
  public $plDescription = "unconfigured";
  public $plHeadline    = "unconfigured";

  // The currently used object(s) (e.g. in edit, removal)
  public $dn = "";  // this is public due to some compatibility problems with class plugin..
  protected $dns = array();

  // The last used object(s).
  protected $last_dn = "";
  protected $last_dns = array();

  // The common places the displayed objects are stored in. (e.g. array("ou=groups,",".."))
  protected $storagePoints = array();

  // The tab definitions to use for the current object.
  protected $tabClass = "";         // e.g. usertabs
  protected $tabType = "";          // e.g. USERTABS
  protected $aclPlugin = "";        // e.g. generic
  protected $aclCategory = "";      // e.g. users
  protected $objectName = "";       // e.g. users

  // The opened object.
  protected $tabObject = null;
  protected $dialogObject = null;

  // The last opened object.
  protected $last_tabObject = null;
  protected $last_dialogObject = null;

  // Whether to display the apply button or not
  protected $displayApplyBtn = FALSE;

  // Whether to display a header or not.
  protected $skipHeader = false;

  // Whether to display a footer or not.
  protected $skipFooter = false;

  // Copy&Paste handler
  protected $cpHandler = null;

  // Indicates that we want to paste objects right now.
  protected $cpPastingStarted = FALSE;

  // The Snapshot handler class.
  protected $snapHandler = null;

  // The listing handlers
  protected $headpage = null;
  protected $filter = null;

  // A list of configured actions/events
  protected $actions = array();

  // Attributes managed by this plugin, can be used in post events;
  public $attributes = array(); 

  function  __construct(&$config,$ui,$plugname, $headpage)
  {
    $this->plugname = $plugname;
    $this->headpage = $headpage;
    $this->ui = $ui;
    $this->config = $config;
    $this->initTime = microtime(TRUE);

    // Create statistic table entry 
    stats::log('management', $class = get_class($this), $this->getAclCategories(),  $action = 'open', 
            $amount = 1, $duration = (microtime(TRUE) - $this->initTime));

    if($this->cpHandler) $this->headpage->setCopyPasteHandler($this->cpHandler);
    if($this->snapHandler) $this->headpage->setSnapshotHandler($this->snapHandler);

    if(empty($this->plIcon)){
      $this->plIcon = "plugins/".$plugname."/images/plugin.png";
    }

    // Register default actions
    $this->registerAction("new",    "newEntry");
    $this->registerAction("edit",   "editEntry");
    $this->registerAction("apply",  "applyChanges");
    $this->registerAction("save",   "saveChanges");
    $this->registerAction("cancel", "cancelEdit");
    $this->registerAction("cancelDelete", "cancelEdit");
    $this->registerAction("remove", "removeEntryRequested");
    $this->registerAction("removeConfirmed", "removeEntryConfirmed");

    $this->registerAction("copy",   "copyPasteHandler");
    $this->registerAction("cut",    "copyPasteHandler");
    $this->registerAction("paste",  "copyPasteHandler");

    $this->registerAction("snapshot",    "createSnapshotDialog");
    $this->registerAction("restore",     "restoreSnapshotDialog");
    $this->registerAction("saveSnapshot","saveSnapshot");
    $this->registerAction("restoreSnapshot","restoreSnapshot");
    $this->registerAction("removeSnapshotConfirmed","removeSnapshotConfirmed");
    $this->registerAction("cancelSnapshot","closeDialogs");

    $this->registerAction("config-filter","editFilter");
    $this->registerAction("saveFilter","saveFilter");
    $this->registerAction("cancelFilter","cancelFilter");

    // To temporay disable the filter caching UNcomment this line.
    #session::global_un_set(get_class($this)."_filter");
  }

  
  /*! \brief  Returns an array with all ACL-Categories we are responsible for.
   */
  function getAclCategories()
  {
      $ret= $this->aclCategory;
      if(!is_array($ret)) $ret = array($ret);
      return($ret);
  }    


  /*! \brief  Execute this plugin
   *          Handle actions/events, locking, snapshots, dialogs, tabs,...
   */
  function execute()
  {
    // Ensure that html posts and gets are kept even if we see a 'Entry islocked' dialog.
    $vars = array('/^act$/','/^listing/','/^PID$/','/^FILTER_PID$/');
    session::set('LOCK_VARS_TO_USE',$vars);

    pathNavigator::registerPlugin($this);

    /* Display the copy & paste dialog, if it is currently open */
    $ret = $this->copyPasteHandler("",array());
    if($ret){
      return($this->getHeader().$ret);
    }

    // Update filter
    if ($this->filter) {
      $this->filter->update();
      session::global_set(get_class($this)."_filter", $this->filter);
      session::set('autocomplete', $this->filter);
    }

    // Handle actions (POSTs and GETs)
    $str = $this->handleActions($this->detectPostActions());
    if($str) return($this->getHeader().$str);

    // Open single dialog objects
    if(is_object($this->dialogObject)){
      if(method_exists($this->dialogObject,'save_object')) $this->dialogObject->save_object(); 
      if(method_exists($this->dialogObject,'execute')){
        $display = $this->dialogObject->execute(); 
        $display.= $this->_getTabFooter();
        return($this->getHeader().$display);
      } 
    }

    // Display tab object.
    if($this->tabObject instanceOf tabs || $this->tabObject instanceOf multi_plug){
#      $this->tabObject->save_object();
      $display = $this->tabObject->execute();
      $display.= $this->_getTabFooter();
      return($this->getHeader().$display);
    }

    // Set current restore base for snapshot handling.
    if(is_object($this->snapHandler)){
      $bases = array();
      foreach($this->storagePoints as $sp){
        $bases[] = $sp.$this->headpage->getBase();
      }

      // No bases specified? Try base  
      if(!count($bases)) $bases[] = $this->headpage->getBase();

      $this->snapHandler->setSnapshotBases($bases);
    }
 
    // Create statistic table entry 
    stats::log('management', $class = get_class($this), $this->getAclCategories(),  $action = 'view',
            $amount = 1, $duration = (microtime(TRUE) - $this->initTime));
 
    // Display list
    return($this->renderList());
  }
 
  function editFilter()
  {
    $this->dialogObject = new userFilter($this->config,$this->getHeadpage());
  }
 
  function renderList()
  {
    $this->headpage->update();
    $display = $this->headpage->render();
    return($this->getHeader().$display);
  }

  function getHeadpage()
  {
    return($this->headpage);
  }

  function getFilter()
  {
    return($this->filter);
  }

  /*! \brief  Generates the plugin header which is displayed whenever a tab object is 
   *           opened.
   */
  protected function getHeader()
  {
    // We do not display any headers right now.
    if(1 || $this->skipHeader) return("");
  }


  /*! \brief  Generates the footer which is used whenever a tab object is 
   *           displayed.
   */
  protected function _getTabFooter()
  {
    // Do not display tab footer for non tab objects 
    if(!($this->tabObject instanceOf tabs || $this->tabObject instanceOf multi_plug)){
      return("");
    }

    // Check if there is a dialog opened - We don't need any buttons in this case. 
    if($this->tabObject->by_object[$this->tabObject->current]){
      $current = $this->tabObject->by_object[$this->tabObject->current];  
      if(isset($current->dialog) && (is_object($current->dialog) || $current->dialog)){
        return("");
      }
    }

    // Skip footer if requested;
    if($this->skipFooter) return("");

    // In case an of locked entry, we may have opened a read-only tab.
    $str = "";
    if(isset($this->tabObject->read_only) && $this->tabObject->read_only == TRUE){
      $str.= "
        <p style=\"text-align:right\">
          <button type=submit name=\"edit_cancel\">".msgPool::cancelButton()."</button>
        </p>";
      return($str);
    }else{

      // Display ok, (apply) and cancel buttons
      $str.= "<p style=\"text-align:right\">\n";
      $str.= "<button type=\"submit\" name=\"edit_finish\">".msgPool::okButton()."</button>\n";
      $str.= "&nbsp;\n";
      if($this->displayApplyBtn){
        $str.= "<button type=\"submit\" name=\"edit_apply\">".msgPool::applyButton()."</button>\n";
        $str.= "&nbsp;\n";
      }
      $str.= "<button type=\"submit\" name=\"edit_cancel\">".msgPool::cancelButton()."</button>\n";
      $str.= "</p>";
    }
    return($str);
  }


  /*! \brief  Initiates the removal for the given entries
   *           and displays a confirmation dialog.
   *      
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  protected function removeEntryRequested($action="",$target=array(),$all=array())
  {
    // Close dialogs and remove locks for currently handled dns
    $this->cancelEdit();
  
    $disallowed = array();
    $this->dns = array();

    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$target,"Entry removel requested!");

    // Check permissons for each target 
    $h = $this->getHeadpage();
    $oTypes = array_reverse($h->objectTypes);
    foreach($target as $dn){
      $entry = $h->getEntry($dn);
      $obj   = $h->getObjectType($oTypes, $entry['objectClass']);
      $acl = $this->ui->get_permissions($dn, $obj['category']."/".$obj['class']);
      if(preg_match("/d/",$acl)){
        $this->dns[] = $dn;
      }else{
        $disallowed[] = $dn;
      }
    }
    if(count($disallowed)){
      msg_dialog::display(_("Permission"),msgPool::permDelete($disallowed),INFO_DIALOG);
    }

    // We've at least one entry to delete.
    if(count($this->dns)){

      // check locks
      if ($user= get_multiple_locks($this->dns)){
        return(gen_locked_message($user,$this->dns));
      }

      // Add locks
      $dns_names = array();
      $types = array();

      // Build list of object -labels
      foreach($h->objectTypes as $type){
        $map[$type['objectClass']]= $type['label'];
      }

      foreach($this->dns as $dn){
        $tmp = $h->getType($dn);
        if(isset($map[$tmp])){
          $dns_names[LDAP::fix($dn)] = _($map[$tmp]);
        }else{
          $dns_names[] =LDAP::fix($dn);
        }
      }
      add_lock ($this->dns, $this->ui->dn);

      // Display confirmation dialog.
      $smarty = get_smarty();
      $smarty->assign("info", msgPool::deleteInfo($dns_names));
      $smarty->assign("multiple", true);
      return($smarty->fetch(get_template_path('removeEntries.tpl')));
    }
  }  


  /*! \brief  Object removal was confirmed, now remove the requested entries. 
   *      
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function removeEntryConfirmed($action="",$target=array(),$all=array(),
      $altTabClass="",$altTabType="",  $altAclCategory="",$altAclPlugin="")
  {
    $tabType = $this->tabType;
    $tabClass = $this->tabClass;
    $aclCategory = $this->aclCategory;
    $aclPlugin = $this->aclPlugin;
    if(!empty($altTabClass)) $tabClass = $altTabClass;
    if(!empty($altTabType)) $tabType = $altTabType;
    if(!empty($altAclCategory)) $aclCategory = $altAclCategory;
    if(!empty($altAclPlugin)) $aclPlugin = $altAclPlugin;

    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$target,"Entry removel confirmed!");

    // Check permissons for each target 
    $h = $this->getHeadpage();
    $oTypes = array_reverse($h->objectTypes);
    foreach($this->dns as $key => $dn){
      $entry = $h->getEntry($dn);
      $obj   = $h->getObjectType($oTypes, $entry['objectClass']);
      $acl = $this->ui->get_permissions($dn, $obj['category']."/".$obj['class']);

      // Check permissions, are we allowed to remove this object? 
      if(preg_match("/d/",$acl)){

        // Delete the object
        $this->dn = $dn;
        $this->tabObject= new $tabClass($this->config,$this->config->data['TABS'][$tabType], $this->dn, 
            $aclCategory, true, true);
        $this->tabObject->set_acl_base($this->dn);
        $this->tabObject->parent = &$this;
        $this->tabObject->delete ();

        // Remove the lock for the current object.
        del_lock($this->dn);        
      } else {
        msg_dialog::display(_("Permission error"), msgPool::permDelete(), ERROR_DIALOG);
        new log("security","groups/".get_class($this),$dn,array(),"Tried to trick deletion.");
      }
    }

    // Cleanup
    $this->remove_lock();
    $this->closeDialogs();
  }


  /*! \brief  Detects actions/events send by the ui
   *           and the corresponding targets.
   */
  function detectPostActions()
  {
    if(!is_object($this->headpage)){
      trigger_error("No valid headpage given....!");
      return(array());
    }
    $action= $this->headpage->getAction();
    if(isset($_POST['edit_apply']))  $action['action'] = "apply";    
    if(isset($_POST['edit_finish'])) $action['action'] = "save";    
    if(isset($_POST['edit_cancel'])) $action['action'] = "cancel";    
    if(isset($_POST['delete_confirmed'])) $action['action'] = "removeConfirmed";   
    if(isset($_POST['delete_snapshot_confirm'])) $action['action'] = "removeSnapshotConfirmed";   
    if(isset($_POST['delete_cancel'])) $action['action'] = "cancelDelete";   
    if(isset($_POST['saveFilter'])) $action['action'] = "saveFilter";   
    if(isset($_POST['cancelFilter'])) $action['action'] = "cancelFilter";   

    // Detect Snapshot actions
    if(isset($_POST['CreateSnapshot'])) $action['action'] = "saveSnapshot";   
    if(isset($_POST['CancelSnapshot'])) $action['action'] = "cancelSnapshot";   
    foreach($_POST as $name => $value){
      $once =TRUE;
      if(preg_match("/^RestoreSnapShot_/",$name) && $once){
        $once = FALSE;
        $entry = base64_decode(preg_replace("/^RestoreSnapShot_(.*)$/i","\\1",$name));
        $action['action'] = "restoreSnapshot";
        $action['targets'] = array($entry);
      }
    }

    return($action);
  }


  /*! \brief  Calls the registered method for a given action/event.
   */
  function handleActions($action)
  {
    // Start action  
    if(isset($this->actions[$action['action']])){
      $func = $this->actions[$action['action']];
      if(!isset($action['targets']))$action['targets']= array(); 

      // Create statistic table entry 
      stats::log('management', $class = get_class($this), $this->getAclCategories(),  $action['action'],
              $amount = count($action['targets']), $duration = (microtime(TRUE) - $this->initTime));

      return($this->$func($action['action'],$action['targets'],$action));
    }
  } 


  /*! \brief  Opens the snapshot creation dialog for the given target.
   *      
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function createSnapshotDialog($action="",$target=array(),$all=array())
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$target,"Snaptshot creation initiated!");

    foreach($target as $entry){
      if(!empty($entry) && $this->ui->allow_snapshot_create($entry,$this->aclCategory)){
        $this->dialogObject = new SnapShotDialog($this->config,$entry,$this);
        $this->dialogObject->aclCategories = array($this->aclCategory);
        $this->dialogObject->parent = &$this;

      }else{
        msg_dialog::display(_("Permission"),sprintf(_("You are not allowed to create a snapshot for %s!"), bold($entry)),
            ERROR_DIALOG);
      }
    }
  }


  /*! \brief  Creates a snapshot new entry - This method is called when the somebody
   *           clicks 'save' in the "Create snapshot dialog" (see ::createSnapshotDialog).
   *      
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function saveSnapshot($action="",$target=array(),$all=array())
  { 
    if(!is_object($this->dialogObject)) return;
    $this->dialogObject->save_object();
    $msgs = $this->dialogObject->check();
    if(count($msgs)){
      foreach($msgs as $msg){
        msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
      }
    }else{
      $this->dn =  $this->dialogObject->dn;
      $this->snapHandler->create_snapshot( $this->dn,$this->dialogObject->CurrentDescription);
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$this->dn,"Snaptshot created!");
      $this->closeDialogs();
    }
  }


  /*! \brief  Restores a snapshot object.
   *          The dn of the snapshot entry has to be given as ['target'] parameter.  
   *      
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function restoreSnapshot($action="",$target=array(),$all=array())
  {
    $entry = array_pop($target);
    if(!empty($entry) && $this->ui->allow_snapshot_restore($entry,$this->aclCategory)){
      $this->snapHandler->restore_snapshot($entry);
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$this->dn,"Snaptshot restored!");
      $this->closeDialogs();
    }else{
      msg_dialog::display(_("Permission"),sprintf(_("You are not allowed to restore a snapshot for %s!"), bold($entry)),
          ERROR_DIALOG);
    }
  }


  /*! \brief  Removes a snapshot object.
   */
  function removeSnapshotConfirmed($action="",$target=array(),$all=array())
  {
    $entry = $this->dialogObject->del_dn;
    if(!empty($entry) && $this->ui->allow_snapshot_create($entry,$this->aclCategory)){
      $this->snapHandler->remove_snapshot($entry);
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$entry,"Snaptshot removed!");
    }else{
      msg_dialog::display(_("Permission"),sprintf(_("You are not allowed to remove a snapshot for %s!"), bold($entry)),
          ERROR_DIALOG);
    }
  }


  /*! \brief  Displays the "Restore snapshot dialog" for a given target. 
   *          If no target is specified, open the restore removed object 
   *           dialog.
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function restoreSnapshotDialog($action="",$target=array(),$all=array())
  {
    // Set current restore base for snapshot handling.
    if(is_object($this->snapHandler)){
      $bases = array();
      foreach($this->storagePoints as $sp){
        $bases[] = $sp.$this->headpage->getBase();
      }
    }

    // No bases specified? Try base  
    if(!count($bases)) $bases[] = $this->headpage->getBase();

    // No target, open the restore removed object dialog.
    if(!count($target)){ 
      $entry = $this->headpage->getBase();
      if(!empty($entry) && $this->ui->allow_snapshot_restore($entry,$this->aclCategory)){
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$entry,"Snaptshot restoring initiated!");
        $this->dialogObject = new SnapShotDialog($this->config,$entry,$this);
        $this->dialogObject->set_snapshot_bases($bases);
        $this->dialogObject->display_all_removed_objects = true;
        $this->dialogObject->display_restore_dialog = true;
        $this->dialogObject->parent = &$this;
      }else{
        msg_dialog::display(_("Permission"),sprintf(_("You are not allowed to restore a snapshot for %s!"), bold($entry)),
            ERROR_DIALOG);
      } 
    }else{

      // Display the restore points for a given object.
      $entry = array_pop($target);
      if(!empty($entry) && $this->ui->allow_snapshot_restore($entry,$this->aclCategory)){
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$entry,"Snaptshot restoring initiated!");
        $this->dialogObject = new SnapShotDialog($this->config,$entry,$this);
        $this->dialogObject->set_snapshot_bases($bases);
        $this->dialogObject->display_restore_dialog = true;
        $this->dialogObject->parent = &$this;
      }else{
        msg_dialog::display(_("Permission"),sprintf(_("You are not allowed to restore a snapshot for %s!"), bold($entry)),
            ERROR_DIALOG);
      } 
    }
  }


  /*! \brief  This method intiates the object creation.
   *          
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function newEntry($action="",$target=array(),$all=array(), $altTabClass ="", $altTabType = "", $altAclCategory="")
  {
    /* To handle mutliple object types overload this method.
     * ...
     *   registerAction('newUser', 'newEntry');
     *   registerAction('newGroup','newEntry');
     * ... 
     * 
     * function newEntry($action="",$target=array(),$all=array(), $altTabClass ="", $altTabType = "", $altAclCategory)
     * {
     *   switch($action){
     *     case 'newUser' : {
     *       mangement::newEntry($action,$target,$all,"usertabs","USERTABS","users");
     *     }
     *     case 'newGroup' : {
     *       mangement::newEntry($action,$target,$all,"grouptabs","GROUPTABS","groups");
     *     }
     *   }
     * }
     **/ 
    $tabType = $this->tabType;
    $tabClass = $this->tabClass;
    $aclCategory = $this->aclCategory;
    if(!empty($altTabClass)) $tabClass = $altTabClass;
    if(!empty($altTabType)) $tabType = $altTabType;
    if(!empty($altAclCategory)) $aclCategory = $altAclCategory;

    // Check locking & lock entry if required 
    $this->displayApplyBtn = FALSE;
    $this->dn = "new";
    $this->is_new = TRUE;
    $this->is_single_edit = FALSE;
    $this->is_multiple_edit = FALSE;

    set_object_info($this->dn);

    // Open object.
    if(empty($tabClass) || empty($tabType)){
      // No tab type defined
    }else{

        if (isset($this->config->data['TABS'][$tabType])) {

            // Check if the base plugin is available - it is mostly responsible for object creation and removal.
            $first  = $this->config->data['TABS'][$tabType][0];
            if(!class_available($first['CLASS'])){
                msg_dialog::display(_("Internal error"),
                        sprintf(_("Cannot instantiate tabbed-plug-in, the base plugin (%s) is not available!"), $first['CLASS']),
                        ERROR_DIALOG);
            }else{
                $this->tabObject= new $tabClass($this->config,$this->config->data['TABS'][$tabType], $this->dn, $aclCategory);
                $this->tabObject->set_acl_base($this->headpage->getBase());
                $this->tabObject->parent = &$this;
                @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$this->dn,"Create new entry initiated!");
            }
      } else {
        msg_dialog::display(_("Error"), sprintf(_("No tab definition for %s found in configuration file: cannot create plugin instance!"), bold($tabType)), ERROR_DIALOG);
      }
    }
  }


  /*! \brief  This method opens an existing object or a list of existing objects to be edited. 
   *                  
   * 
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function editEntry($action="",$target=array(),$all=array(), $altTabClass ="", $altTabType = "", $altAclCategory="")
  {
    /* To handle mutliple object types overload this method.
     * ...
     *   registerAction('editUser', 'editEntry');
     *   registerAction('editGroup','editEntry');
     * ... 
     * 
     * function editEntry($action="",$target=array(),$all=array(), $altTabClass ="", $altTabType = "", $altAclCategory)
     * {
     *   switch($action){
     *     case 'editUser' : {
     *       mangement::editEntry($action,$target,$all,"usertabs","USERTABS","users");
     *     }
     *     case 'editGroup' : {
     *       mangement::editEntry($action,$target,$all,"grouptabs","GROUPTABS","groups");
     *     }
     *   }
     * }
     **/

    // Do not create a new tabObject while there is already one opened,
    //  the user may have just pressed F5 to reload the page.
    if(is_object($this->tabObject)){
      return;
    }
 
    $tabType = $this->tabType;
    $tabClass = $this->tabClass;
    $aclCategory = $this->aclCategory;
    if(!empty($altTabClass)) $tabClass = $altTabClass;
    if(!empty($altTabType)) $tabType = $altTabType;
    if(!empty($altAclCategory)) $aclCategory = $altAclCategory;

    $this->displayApplyBtn = count($target) == 1;

    // Single edit - we only got one object dn.
    if(count($target) == 1){
      $this->is_new = FALSE;
      $this->is_single_edit = TRUE;
      $this->is_multiple_edit = FALSE;

      // Get the dn of the object and creates lock
      $this->dn = array_pop($target);
      set_object_info($this->dn);
      $user = get_lock($this->dn);
      if ($user != ""){
        return(gen_locked_message ($user, array($this->dn),TRUE));
      }
      add_lock ($this->dn, $this->ui->dn);

      // Open object.
      if(empty($tabClass) || empty($tabType)){
        trigger_error("We can't edit any object(s). 'tabClass' or 'tabType' is empty!");
      }else{

        $tab = $tabClass;
        
        // Check if the base plugin is available - it is mostly responsible for object creation and removal.
        $first  = $this->config->data['TABS'][$tabType][0];
        if(!class_available($first['CLASS'])){
            msg_dialog::display(_("Internal error"), 
                    sprintf(_("Cannot instantiate tabbed-plug-in, the base plugin (%s) is not available!"), $first['CLASS']), 
                    ERROR_DIALOG);
        }else{
            $this->tabObject= new $tab($this->config,$this->config->data['TABS'][$tabType], $this->dn,$aclCategory);
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$this->dn,"Edit entry initiated!");
            $this->tabObject->set_acl_base($this->dn);
            $this->tabObject->parent = &$this;
        }
      }
    }else{

      // We've multiple entries to edit.
      $this->is_new = FALSE;
      $this->is_singel_edit = FALSE;
      $this->is_multiple_edit = TRUE;

      // Open multiple edit handler.
      if(empty($tabClass) || empty($tabType)){
        trigger_error("We can't edit any object(s). 'tabClass' or 'tabType' is empty!");
      }else{
        $this->dns = $target;
        $tmp = new multi_plug($this->config,$tabClass,$this->config->data['TABS'][$tabType],
            $this->dns,$this->headpage->getBase(),$aclCategory);

        // Check for locked entries
        if ($tmp->entries_locked()){
          return($tmp->display_lock_message());
        }

        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$this->dns,"Edit entry initiated!");

        // Now lock entries.
        if($tmp->multiple_available()){
          $tmp->lock_entries($this->ui->dn);
          $this->tabObject = $tmp;
          set_object_info($this->tabObject->get_object_info());
        }
      }
    }
  }


  /*! \brief  Close filter dialog
   */
  protected function cancelFilter()
  {
      if($this->dialogObject instanceOf userFilter){
          $this->remove_lock();
          $this->closeDialogs();
      }
  }


  /*! \brief  Save filter modifcations.
   */
  protected function saveFilter()
  {
    if($this->dialogObject instanceOf userFilter){
      $msgs = $this->dialogObject->check();
      if(count($msgs)){
        msg_dialog::displayChecks($msgs); 
        return("");
      }else{
        $this->dialogObject->save();
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$this->dns,"Entry saved!");
        $this->remove_lock();
        $this->closeDialogs();

        // Ask filter to reload information
        $this->filter->reloadFilters();
      }
    }
  }


  /*! \brief  Save object modifications and closes dialogs (returns to object listing).
   *          - Calls '::check' to validate the given input.
   *          - Calls '::save' to save back object modifications (e.g. to ldap).
   *          - Calls '::remove_locks' to remove eventually created locks.
   *          - Calls '::closeDialogs' to return to the object listing.
   */
  protected function saveChanges()
  {
    if($this->tabObject instanceOf tabs || $this->tabObject instanceOf multi_plug){
      $this->tabObject->save_object();
      $msgs = $this->tabObject->check();
      if(count($msgs)){
        msg_dialog::displayChecks($msgs); 
        return("");
      }else{
        $this->tabObject->save();
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$this->dns,"Entry saved!");
        $this->remove_lock();
        $this->closeDialogs();
      }
    }elseif($this->dialogObject instanceOf plugin){
      $this->dialogObject->save_object();
      $msgs = $this->dialogObject->check();
      if(count($msgs)){
        msg_dialog::displayChecks($msgs); 
        return("");
      }else{
        $this->dialogObject->save();
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$this->dns,"Entry saved!");
        $this->remove_lock();
        $this->closeDialogs();
      }
    }
  }


  /*! \brief  Save object modifications and keep dialogs opened. 
   *          - Calls '::check' to validate the given input.
   *          - Calls '::save' to save back object modifications (e.g. to ldap).
   */
  protected function applyChanges()
  {
    if($this->tabObject instanceOf tabs || $this->tabObject instanceOf multi_plug){
      $this->tabObject->save_object();
      $msgs = $this->tabObject->check();
      if(count($msgs)){
        msg_dialog::displayChecks($msgs); 
        return("");
      }else{
        $this->tabObject->save();
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$this->dns,"Modifications applied!");
        $this->tabObject->re_init();
      }
    }
  }


  /*! \brief  This method closes dialogs
   *           and cleans up the cached object info and the ui.
   */
  protected function closeDialogs()
  {
    $this->last_dn = $this->dn;
    $this->last_dns = $this->dns;
    $this->last_tabObject = $this->tabObject;
    $this->last_dialogObject = $this->dialogObject;
    $this->dn = "";
    $this->dns = array();
    $this->tabObject = null;
    $this->dialogObject = null;
    $this->skipFooter = FALSE;
    set_object_info();
  }


  /*! \brief  Editing an object was caneled. 
   *          Close dialogs/tabs and remove locks.
   */
  protected function cancelEdit()
  {
    $this->remove_lock();
    $this->closeDialogs();
  }


  /*! \brief  Every click in the list user interface sends an event
   *           here can we connect those events to a method. 
   *          eg.  ::registerEvent('new','createUser')
   *          When the action/event new is send, the method 'createUser' 
   *           will be called.
   */
  function registerAction($action,$target)
  {
    $this->actions[$action] = $target;
  }


  /*! \brief  Removes ldap object locks created by this class.
   *          Whenever an object is edited, we create locks to avoid 
   *           concurrent modifications.
   *          This locks will automatically removed here.
   */
  function remove_lock()
  {
    if(!empty($this->dn) && $this->dn != "new"){
      del_lock($this->dn);
    }
    if(count($this->dns)){
      del_lock($this->dns);
    }
  }


  /*! \brief  This method is used to queue and process copy&paste actions. 
   *          Allows to copy, cut and paste mutliple entries at once.
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function copyPasteHandler($action="",$target=array(),$all=array(), 
      $altTabClass ="", $altTabType = "", $altAclCategory="",$altAclPlugin="")
  {
    // Return without any actions while copy&paste handler is disabled.
    if(!is_object($this->cpHandler))  return("");

    $tabType = $this->tabType;
    $tabClass = $this->tabClass;
    $aclCategory = $this->aclCategory;
    $aclPlugin = $this->aclPlugin;
    if(!empty($altTabClass)) $tabClass = $altTabClass;
    if(!empty($altTabType)) $tabType = $altTabType;
    if(!empty($altAclCategory)) $aclCategory = $altAclCategory;
    if(!empty($altAclPlugin)) $aclPlugin = $altAclPlugin;

    // Save user input
    $this->cpHandler->save_object();

    // Add entries to queue 
    if($action == "copy" || $action == "cut"){
      $this->cpHandler->cleanup_queue();
      foreach($target as $dn){
        if($action == "copy" && $this->ui->is_copyable($dn,$aclCategory,$aclPlugin)){
          $this->cpHandler->add_to_queue($dn,"copy",$tabClass,$tabType,$aclCategory,$this);
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$dn,"Entry copied!");
        }
        if($action == "cut" && $this->ui->is_cutable($dn,$aclCategory,$aclPlugin)){
          $this->cpHandler->add_to_queue($dn,"cut",$tabClass,$tabType,$aclCategory,$this);
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$dn,"Entry cutted!");
        }
      }
    }

    // Initiate pasting
    if($action == "paste"){
      $this->cpPastingStarted = TRUE;
    }

    // Display any c&p dialogs, eg. object modifications required before pasting.
    if($this->cpPastingStarted && $this->cpHandler->entries_queued()){
      $this->cpHandler->SetVar("base",$this->headpage->getBase());
      $data = $this->cpHandler->execute();
      if(!empty($data)){
        return($data);
      }
    }

    // Automatically disable pasting process since there is no entry left to paste.
    if(!$this->cpHandler->entries_queued()){
      $this->cpPastingStarted = FALSE;
    }
    return("");
  }


  function setFilter($str) {
    $this->filter = $str;
  }


  function postcreate() {
    $this->handle_post_events('add');
  }
  function postmodify(){
    $this->handle_post_events('modify');
  }
  function postremove(){
    $this->handle_post_events('remove');
  }

  function is_modal_dialog()
  {
    return(is_object($this->tabObject) || is_object($this->dialogObject));
  }


  /*! \brief    Forward command execution request
   *             to the correct method.
   */
  function handle_post_events($mode, $addAttrs= array())
  {
    if(!in_array_strict($mode, array('add','remove','modify'))){
      trigger_error(sprintf("Invalid post event type given %s! Valid types are [add,modify,remove].", bold($mode)));
      return;
    }
    switch ($mode){
      case "add":
        plugin::callHook($this,"POSTCREATE", $addAttrs);
      break;

      case "modify":
        plugin::callHook($this,"POSTMODIFY", $addAttrs);
      break;

      case "remove":
        plugin::callHook($this,"POSTREMOVE", $addAttrs);
      break;
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
