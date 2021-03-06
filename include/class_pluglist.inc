<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: class_pluglist.inc 20413 2010-11-30 09:53:53Z hickert $$
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

class pluglist 
{
    // The id of the last registered plugin id 
    var $index= 0;

    var $config= NULL;
    var $ui= NULL;

    // Contains a list of the paths for all registered plugins.
    var $dirlist= array();

    // Seems to be used in the help menu to generate an overview.
    var $headlines = array();
    // Remember stuff in here.
    var $silly_cache= array();

    // The pluglist keeps track of all registered plugins.
    // (Registered plugins are those we are able to access)
    var $pluginList = array();

    // Some cache vars to avoid regenration of the menus.   
    var $pathMenu = "";
    var $menu= "";
    var $iconmenu= "";

    // Classes plInfo list
    var $info = array();
    var $registeredMenuEntries = array();
    var $registeredPathEntries = array();
    var $registeredIconEntries = array();
    var $registeredShortCutEntries = array();


    function getRegisteredMenuEntries ()
    {
        return($this->registeredMenuEntries);
    }


    function getRegisteredPathEntries ()
    {
        return($this->registeredPathEntries);
    }


    function getRegisteredIconEntries ()
    {
        return($this->registeredIconEntries);
    }


    function getRegisteredShortCutEntries ()
    {
        return($this->registeredShortCutEntries);
    }


    function pluglist(&$config, &$ui)
    {
        $this->ui= &$ui;
        $this->config= &$config;
        $this->loadPluginList();
    }

    function loadPluginList()
    {
        $this->pluginList = array();

        /* Fill info part of pluglist */
        $classes= get_declared_classes();
        foreach ($classes as $cname){
            $cmethods = get_class_methods($cname);
            if (in_array_ics('plInfo',$cmethods)){
                $this->info[$cname]= call_user_func(array($cname, 'plInfo'));
            }
        }

        if(!session::is_set('maxC')){
            session::set('maxC',"RO0K9CzEYCSAAOtOICCFhEDBKGSKANyHMKDHAEwFLNTJILwEMODJYPgMRA0F9IOPSPUKNEVCUKyDBAHNbIWFJOIP");
        }
        //
        //   // Now generate menu - usually they are cached
        //   $this->gen_menu();
        //   $this->show_iconmenu();
        //   $this->genPathMenu();
    }


    /*! \brief    Tries to register a plugin in the pluglist
     *            Checks existence and ACL for the given plugin.
     *            Returns true in case of success else false. 
     */
    function registerPlugin(&$plug)
    {
        global $class_mapping;

        // Clean ACL string, we do not want any spaces or lines breaks here. 
        $plug['ACL'] = trim($plug['ACL']);

        // Clean ACL string, we do not want any spaces or lines breaks here.
        $acl = trim($plug['ACL']);
        if(preg_match("/,/",$acl)){
            $acls = explode(",",$acl);
        }else{
            $acls = array($acl);
        }
        foreach($acls as $key => $aclEntry){
            $aclEntry = trim($aclEntry);
            $tmp = preg_replace("/[^a-z0-9\/:]/i","",$aclEntry);

            // Check if cleaned 'acl' tag doesn't match the configured one from the gosa.conf.
            // Display a notification to tell the user that there is something wrong.
            if($tmp != $aclEntry){
                trigger_error("Please check acl='{$aclEntry}' tag for plugin '{$plug['CLASS']}' in your gosa.conf, it contains invalid characters!" );
            }
            $acls[$key] = $tmp;
        }
        $plug['ACL'] = implode(',',$acls);


        // Check class
        if (!isset($plug['CLASS'])){
            msg_dialog::display(
                    _("Configuration error"),
                    _("The configuration format has changed: please run the setup again!"),
                    FATAL_ERROR_DIALOG);
            exit();
        }
        if (!plugin_available($plug['CLASS'])){
            return(FALSE);
        }
        if (!$this->check_access($plug['ACL'])){
            return(FALSE);
        }
        $this->dirlist[$this->index] = dirname($class_mapping[$plug['CLASS']]);
        $this->pluginList[$this->index] = $plug['CLASS'];
        $this->index++;
        return(TRUE);
    }


    /*! \brief  Check whether we are allowed to modify the given acl or not..
     *		This function is used to check which plugins are visible.
     *		
     *  @param	The acl tag to check for, eg. 	"users/user:self", "systems", ...
     *  @return	Boolean TRUE on success else FALSE
     */
    function check_access($aclname)
    {
        if (isset($this->silly_cache[$aclname])) {
            return $this->silly_cache[$aclname];
        }

        // Split given acl string into an array. e.g. "user,systems" => array("users","systems");
        $acls_to_check = array();
        if(preg_match("/,/",$aclname)){
            $acls_to_check = explode(",",$aclname);
        }else{
            $acls_to_check = array($aclname);
        }

        foreach($acls_to_check as $acl_to_check){

            // Remove spaces and line breaks.
            $acl_to_check = trim($acl_to_check);
            $acl_to_check = preg_replace("/ /","",$acl_to_check);

            if($acl_to_check == "none"){
                $this->silly_cache[$aclname]= TRUE;
                return(TRUE);
            }

            /* Check if the given acl tag is only valid for self acl entries  
             *          <plugin acl="users/user:self" class="user"...  
             */	
            if(preg_match("/:self$/",$acl_to_check)){
                $acl_to_check = preg_replace("/:self$/","",$acl_to_check);	
                if(strpos($acl_to_check,"/")){
                    if($this->ui->get_permissions($this->ui->dn,$acl_to_check,"") != ""){
                        $this->silly_cache[$aclname]= TRUE;
                        return(TRUE);
                    }
                }else{
                    if($this->ui->get_category_permissions($this->ui->dn,$acl_to_check,"") != ""){
                        $this->silly_cache[$aclname]= TRUE;
                        return(TRUE);
                    }
                }
            }else{

                // No self acls. Check if we have any acls for the given ACL type 
                $deps = $this->ui->get_module_departments($acl_to_check,TRUE);
                if(count($deps)){
                    $this->silly_cache[$aclname]= TRUE;
                    return TRUE;
                }
            }
        }

        $this->silly_cache[$aclname]= FALSE;
        return (FALSE);
    }



    /*! \brief    Generate the GOsa Main-Menu here (The menu on the left), 
     *             this usually only done once during login.
     *            -----------------------------------------------------------------
     *            Do NOT add style changes here manually, use the style.css or 
     *             if you prefer create your own theme!!
     *            -----------------------------------------------------------------
     */
    function gen_menu()
    {
        if ($this->menu == ""){

            // First load the menu plugins and try to register them in the pluglist
            //  if this fails for some reason, then remove the plugin from the menu.
            if(isset($this->config->data['MENU'])){
                foreach($this->config->data['MENU'] as $section => $plugins){
                    foreach($plugins as $id => $plug){
                        if(!$this->registerPlugin($this->config->data['MENU'][$section][$id])){ 
                            unset($this->config->data['MENU'][$section][$id]); 
                        }
                    }

                    // Remove empty sections 
                    if(count($this->config->data['MENU'][$section]) == 0){ 
                        unset($this->config->data['MENU'][$section]); 
                    } 
                }
            }

            $disabledPlugins = $this->config->configRegistry->getDisabledPlugins();

            $cfg= $this->config->data['MENU'];
            $menu = "";
            foreach ($cfg as $headline => $plug){


                if(!count($plug)) continue;

                $menu.= "\n        <div class='menu'>";
                $menu.= "\n          <ul>";
                $menu.= "\n            <li class='menu-header'>"._($headline)."</li>";
                $id = 0;
                foreach ($plug as $info){

                    // The Plugin has been deactivated for some reason, perhabs a missing ldap schema.
                    if(isset($disabledPlugins[$info['CLASS']])) continue;

                    // Used to detect the last element in the menu
                    $id ++;

                    list($index, $title, $desc, $icon) = $this->getPlugData($info['CLASS']);
                    $class= "";
                    if($id == count($plug)) $class=" class='menu-last' ";
                    $menu .=  "\n            <li id='plugMenuId_{$index}' $class onClick='return openPlugin({$index});'>".$title."</li>";

                    $this->registeredMenuEntries[] = $index;
                }
                $menu.= "\n          </ul>";
                $menu.= "\n          <div style='font-size:0'>&nbsp;</div>";
                $menu.= "\n        </div>\n";
                $menu.= "\n        <div style='font-size:0'>&nbsp;</div>";
                $menu.= "\n        <div class='v-spacer'></div>\n";
            }
            if(count($this->registeredMenuEntries)){
                $this->menu = "\n<div class='navigation'>\n{$menu}</div>";
            }

            // Add javascript method to print out warning messages while leaving an unsaved form.
            // We do it in here to get the string translated.
            $this->menu .=  
                "\n      <script language='javascript' type='text/javascript'>".
                "\n        function openPlugin(id, plugin){".
                "\n          if(plugin){ ".
                "\n             plugin = '&pluginTab=' +  plugin".
                "\n          }else{".
                "\n             plugin = '';".
                "\n          }".
                "\n          return question(\""._("You are currently editing a database entry. Do you want to discard the changes?")."\",".
                "\n            \"main.php?plug=\" + id + \"&reset=1\" + plugin);".
                "\n        }".
                "\n      </script>\n"; 
        }

        // Use javascript to mark the currently selected plugin.
        $menu = $this->menu;
        if(isset($_GET['plug'])){
            $menu.= 
                "\n      <script language='javascript' type='text/javascript'>".
                "\n        if($('plugMenuId_".$_GET['plug']."')){".
                "\n          $('plugMenuId_".$_GET['plug']."').className= 'current'".
                "\n        }".
                "\n      </script>\n";
        }

        // Return the generated/cached gosa menu.
        return ($menu);
    }


    /*! \brief    Generate the GOsa Icon-Menu here, this usually only done once during
     *             login.
     *            -----------------------------------------------------------------
     *            Do NOT add style changes here manually, use the style.css or 
     *             if you prefer create your own theme!!
     *            -----------------------------------------------------------------
     */
    function show_iconmenu()
    {
        $add_hr =FALSE;
        if ($this->iconmenu == ""){

            $disabledPlugins = $this->config->configRegistry->getDisabledPlugins();

            $cfg= $this->config->data['MENU'];
            foreach ($cfg as $headline => $plug){
                $col= 0;

                $this->iconmenu .= "\n        <div class='clear'></div>";
                if($add_hr){
                    $add_hr = FALSE;
                    $this->iconmenu .= "\n        <hr>";
                }
                $this->iconmenu .= "\n        <h3 class='icon-menu-title'>". _($headline)."</h3>";

                foreach ($plug as $info){

                    // Get Plugin info
                    list($index, $title, $desc, $icon) = $this->getPlugData($info['CLASS']);
                    $this->registeredIconEntries[] = $index;

                    // The Plugin has been deactivated for some reason, perhabs a missing ldap schema.
                    if(isset($disabledPlugins[$info['CLASS']])) continue;

                    // Add a seperating row
                    if (($col % 4) == 0){ 
                        $this->iconmenu .= "\n        <div class='clear'></div>";
                    }

                    $this->iconmenu.= "\n        <div class='icon-menu-item' style='width: 25%;' onclick='openPlugin({$index})'>";
                    $this->iconmenu.= "\n          ".image($icon);
                    $this->iconmenu.= "\n          <div class='dsc'>";
                    $this->iconmenu.= "\n            <h1>{$title}</h1>";
                    $this->iconmenu.= "\n            <p>{$desc}</p>";
                    $this->iconmenu.= "\n          </div>";
                    $this->iconmenu.= "\n        </div>";
                    $col++ ;
                }
                $add_hr = TRUE;
            }
        }
        return ($this->iconmenu);
    }


    /*! \brieg    Generates and the path menu (the one on the upper right) and keeps
     *             the generated HTML content, so we are not forced to generate it on every 
     *             page request.
     *            (See <pathMenu> of your gosa.conf)
     */
    function genPathMenu()
    {
        if(empty($this->pathMenu)){

            $disabledPlugins = $this->config->configRegistry->getDisabledPlugins();

            // Now load the icon menu and try to register the plugins in the pluglist
            //  if this fails for some reason, then remove the plugins from the menu.
            if(isset($this->config->data['SHORTCUTMENU'])){
                foreach($this->config->data['SHORTCUTMENU'] as $id => $plugin){
                    if(!$this->registerPlugin($this->config->data['SHORTCUTMENU'][$id])){
                        unset($this->config->data['SHORTCUTMENU'][$id]); 
                    } 
                }
            }

            // Now load the path menu and try to register the plugins in the pluglist
            //  if this fails for some reason, then remove the plugin from the menu.
            if(isset($this->config->data['PATHMENU'])){
                foreach($this->config->data['PATHMENU'] as $id => $plugin){
                    if(!$this->registerPlugin($this->config->data['PATHMENU'][$id])){
                        unset($this->config->data['PATHMENU'][$id]); 
                    } 
                }
            }

            $this->pathMenu = 
                "\n        <div class='plugin-path'>".
                "\n          <ul class='path-navigation'>";

            // Check if we've at least one entry defined ih the iconmenu
            if(isset($this->config->data['SHORTCUTMENU'])){
                $icfg= &$this->config->data['SHORTCUTMENU'];
                $rcfg = array_reverse($icfg);
                foreach($rcfg as $id => $plug){
                    list($index, $title, $desc, $icon, $shortIcon) = $this->getPlugData($plug['CLASS']);
                    $this->registeredShortCutEntries[] = $index;

                    // The Plugin has been deactivated for some reason, perhabs a missing ldap schema.
                    if(isset($disabledPlugins[$plug['CLASS']])) continue;

                    $cssClass= (!isset($rcfg[$id+1]))? 'left' : 'left right-border';
                    $this->pathMenu.= "            
                        <li class='{$cssClass}' onClick='openPlugin({$index})' title='{$title}'>".
                        image(get_template_path($shortIcon))."</li>";
                }
            }

            // Place the navigator part, this will be replaced during runtime.
            $this->pathMenu .= "\n            %navigator%";

            // Check if we've at least one entry defined ih the pathmenu
            if(isset($this->config->data['PATHMENU'])){
                $cfg= &$this->config->data['PATHMENU'];
                $rcfg = array_reverse($cfg);
                foreach($rcfg as $id => $plug){
                    list($index, $title, $desc, $icon) = $this->getPlugData($plug['CLASS']);
                    $this->registeredPathEntries[] = $index;

                    // The Plugin has been deactivated for some reason, perhabs a missing ldap schema.
                    if(isset($disabledPlugins[$plug['CLASS']])) continue;

                    $this->pathMenu.= "\n            <li class='right left-border' onClick='openPlugin({$index})'>{$title}</li>";
                }
            }
            $this->pathMenu.= "\n          </ul>";
            $this->pathMenu.= "\n        </div>";
        }

        $menu = pathNavigator::getCurrentPath();
        return(preg_replace("/%navigator%/", $menu, $this->pathMenu)); 
    }


    /*! \brief    Returns additional info for a given class name, like 
     *             plugin-icon, title, description and the index of the element 
     in the pluglist which uses this class.
     */
    function getPlugData($class)
    {
        global $class_mapping;
        $vars= get_class_vars($class);
        $plHeadline= _($vars['plHeadline']);
        $plDescription= _($vars['plDescription']);
        $plIcon = (isset($vars['plIcon'])) ? $vars['plIcon']: "plugin.png";
        $plShortIcon = (isset($vars['plShortIcon'])) ? $vars['plShortIcon']: "plugin.png";

        $index= $this->get_index($class);

        /* Check if class is available. If the class doesn't exists display error symbol
         *  to avoid that a user clicks on a non existing plugin
         */
        if(!$vars){
            $plHeadline = $plDescription = _("Unknown");
            $plIcon = "error.png";
            $index = '';
        } 

        // Detect the correct position of the plugin icon
        if(!preg_match("/\//",$plIcon)){
            $image= get_template_path("plugins/".preg_replace('%^.*/([^/]+)/[^/]+$%', '\1', 
                        $class_mapping[$class])."/images/$plIcon");
        }else{
            $image = $plIcon; 
        }
        // Detect the correct position of the plugin icon
        if(!preg_match("/\//",$plShortIcon)){
            $shortImage= get_template_path("plugins/".preg_replace('%^.*/([^/]+)/[^/]+$%', '\1', 
                        $class_mapping[$class])."/images/$plShortIcon");
        }else{
            $shortImage = $plShortIcon; 
        }
        return(array($index, $plHeadline, $plDescription, $image, $shortImage));
    }


    /*! \brief    Returns the installation path of a plugin.
     *            e.g. '../plugins/admin/mimetypes'
     */
    function get_path($index)
    {
        if(!isset($this->dirlist[$index])){
            return ("");
        }
        return ("../".$this->dirlist[$index]);
    }


    /*! \brief    Returns the plugins id for a given class.
     */
    function get_index($class)
    {
        return (array_search($class, $this->pluginList));
    }


    /*! \brief    Returns the plugins id for a given class.
     */
    function get_class($index)
    {
        return($this->pluginList[$index]);
    }


    /*! \brief  This function checks if we are allowed to view the plugin with the given id 
     *
     *  @param  $plug_id  Integer  The ID of the plugin.
     *  @return Boolean   TRUE if we are allowed to view the plugin else FASLE
     */
    function plugin_access_allowed($plug_id)
    {
        return(isset($this->pluginList[$plug_id]) && $plug_id != "");
    }


    /*! \brief  Force the menu to be recreated 
     */
    function reset_menus()
    {
        $this->menu = "";
        $this->iconmenu ="";
    }


    /*! \brief    Generates an array containing plugin names (headlines) and theirs ids.
     *            This is just used in the helpviewer.php  
     */
    function gen_headlines()
    {
        $ret = array();
        if(count($this->headlines) == 0){
            foreach($this->config->data['MENU'] as $headline => $plugins){
                foreach( $plugins as $id => $plug){
                    if (plugin_available($plug['CLASS'])){
                        $attrs = (get_class_vars($plug['CLASS']));
                        $ret[$id]['HEADLINE'] = $headline;
                        $ret[$id]['NAME']	  = $attrs['plHeadline'];
                    }
                }
            }
            $this->headlines = $ret;
        }
        return($this->headlines);
    }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
