<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: tabs_acl_role.inc 19254 2010-07-29 13:54:19Z hickert $$
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

class aclroletab extends tabs
{

  function aclroletab($config, $data, $dn)
  {
    $data = array(array("CLASS" => "aclrole" , "NAME" => _("ACL Templates")));
    tabs::tabs($config, $data, $dn,"acl");
  }

  function save($ignore_account= FALSE)
  {
    $baseobject= $this->by_object['aclrole'];
    $cn      = preg_replace('/,/', '\,', $baseobject->cn); 
    $cn      = preg_replace('/"/', '\"', $cn); 

    /* Check for new 'dn', in order to propagate the
       'dn' to all plugins */
    $new_dn=  @LDAP::convert('cn='.$cn.",".get_ou("aclrole", "aclRoleRDN").$baseobject->base);

    if ($this->dn != $new_dn){

      /* Write entry on new 'dn' */
      if ($this->dn != "new"){
        $baseobject->move($this->dn, $new_dn);
        $this->by_object['aclrole']= $baseobject;
      }

      /* Happen to use the new one */
      $this->dn= $new_dn;
    }

    $ret= tabs::save();

    return $ret;
  }

  function save_object($ignore_account= FALSE)
  {
    tabs::save_object();
  }
  
  function execute()
  {
    $display= tabs::execute();
    if($this->read_only){
      $display.= "<div class='plugin-actions'>";
      $display.= "  <button type=submit name=\"edit_cancel\">".msgPool::cancelButton()."</button>\n";
      $display.= "</div>";
    }elseif(!$this->by_object['aclrole']->dialog){
      $display.= "<p style=\"text-align:right\">\n";
      $display.= "  <button type=submit name=\"edit_finish\">".msgPool::okButton()."</button>\n";
      $display.= "  <button type=submit name=\"edit_cancel\">".msgPool::cancelButton()."</button>\n";
      $display.= "</p>";
    }
    return($display);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
