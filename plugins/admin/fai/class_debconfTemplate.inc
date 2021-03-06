<?php

class debconf
{
  var $package= "";
  var $language= "";
  var $loaded_template= FALSE;
  var $template= array();


  function debconf($language= "")
  {
    $this->set_language($language);
  }

  function set_language($language)
  {
    $this->language= $language;
  }

  function load_from_string($str)
  {
    $lines                 = explode("\n",$str);
    $this->template        = array();
    $post_name             = 0;
    $langcode              = $this->language.".UTF-8";
    $in_description        = FALSE;
    $got_local_description = FALSE;

    foreach($lines as $line){

      /* Reset description flag */
      if ($in_description && !preg_match("/^ /", $line)){
        $in_description= FALSE;
      }

      /* Template header */
      if (preg_match("/^Template: /", $line)){
        $post_name ++; 
        $name= trim(preg_replace("/^Template: (.*)$/", "\\1", $line));
        $this->template[$post_name]['Name'] = $name;
        $this->template[$post_name]['Default'] ="";

        $got_local_description= FALSE;
        continue;
      }

      /* Get type */
      if (preg_match("/^Type: /", $line)){
        $type= trim(preg_replace("/^Type: (.*)$/", "\\1", $line));
        $this->template[$post_name]['Type']= $type;
        continue;
      }

      /* Get default */
      if (preg_match("/^Default: /", $line)){
        $this->template[$post_name]['Default']= "";
        $default= trim(preg_replace("/^Default: (.*)$/", "\\1", $line));
        $this->template[$post_name]['Default']= $default;
        continue;
      }

      /* Get description */
      if (!$got_local_description && preg_match("/^Description: /i", $line)){
        $description= trim(preg_replace("/^Description: (.*)$/i", "\\1", $line));
        $this->template[$post_name]['Topic']= $description;
        $this->template[$post_name]['Description']= "";
        $in_description= TRUE;
        continue;
      }

      /* Fill description */
      if (!$got_local_description && $in_description){
        $description= preg_replace("/^ (.*)$/", "\\1", $line);
        $this->template[$post_name]['Description'].= $description;
        continue;
      }

      /* Get local description */
      if (preg_match("/^Description-$langcode: /i", $line)){
        $description= trim(preg_replace("/^Description-$langcode: (.*)$/i", "\\1", $line));
        $this->template[$post_name]['Topic']= $description;
        $in_description= TRUE;
        $got_local_description= TRUE;
        $this->template[$post_name]['Description']= "";
        continue;
      }

      /* Fill local description */
      if ($got_local_description && $in_description){
        $description= preg_replace("/^ (.*)$/", "\\1", $line);
        $this->template[$post_name]['Description'].= $description;
        continue;
      }

      /* Get native choices */
      if (preg_match("/^Choices: /", $line)){
        $type= trim(preg_replace("/^Choices: (.*)$/", "\\1", $line));
        $this->template[$post_name]['Choices']= $type;
      }

      /* Get local choices */
      if (preg_match("/^Choices-$langcode: /", $line)){
        $type= trim(preg_replace("/^Choices-$langcode: (.*)$/", "\\1", $line));
        $this->template[$post_name]['Choices-local']= $type;
      }

    }

    $this->loaded_template= TRUE;

    $tmp= array();
    foreach($this->template as $post_name => $template){
      $template['post_name'] = "post_".$post_name;
      $tmp[] = $template;
    }
    $this->template = $tmp;

    return (TRUE);
  }

  function has_template()
  {
    return(count($this->template) != FALSE);
  }


  /* Check if some fields are posted */
  function PostCheck()
  {
    /* Walk through all template variables */
    foreach($this->template as $post_name => $entry){

      /* Check if this var is set*/
      if(isset($_POST[$entry['post_name']])){

        /* special handling for arrays */
        if(is_array($_POST[$entry['post_name']])){
          $str = "";
          foreach($_POST[$entry['post_name']] as $val){
            $str.= $val.", ";
          }
          $str = preg_replace("/\,\ $/","",$str);
          $this->template[$post_name]['Default'] = $str;
        }else{
          $this->template[$post_name]['Default'] = get_post($entry['post_name']);
        }
      }
    }
    
    foreach($this->template as $post_name => $entry){
      if(isset($_POST["multi-".$entry['post_name']])){ 
        $this->template[$post_name]['Default']= "";
        foreach($_POST as $name => $value){
          if(preg_match("/".$entry['post_name']."-multi-/",$name)){
            $this->template[$post_name]['Default'] .= $value.", ";
          }
        } 
        $this->template[$post_name]['Default'] = preg_replace("/, $/","",$this->template[$post_name]['Default']);
      }
    }
  }


  /* This funtion sets the defualt value */
  function SetDefault($var,$val)
  {
    if ($this->loaded_template) {
      foreach($this->template as $key => $tmp){
        if($tmp['Name'] == $var ){
          $this->template[$key]['Default'] = $val;
        }
      }
    }
  }


  /* Display all possible options in html*/
  function get_dialog()
  {
    if ($this->loaded_template && count($this->template)) {
      $result= "<table summary=''>";

      foreach ($this->template as $post_name => $entry){

        $types= array("boolean" => "", "multiselect" => "", "note" => "",
            "password" => "", "select" => "", "string" => "", "text" => "", "title" => "");

        /* Check if type is available */
        if ((isset($entry['Type']))&&(isset($types[$entry['Type']]))){

          /* Produce type specific output */
          $fn= "render_".$entry['Type'];
          $str = $this->$fn($entry);
          if(!empty($str)){
            $result.=$str."<tr><td colspan='2'><hr></td></tr>";
          }
        } else {
          //php_error(E_WARNING, "An unknown type has been specified in the debconf template. Please fix.");
        }
      }
      $result.="</table>";

    
      $result .= "<input type='hidden' post_name='check_post' value='1'>";
      return ($result);
    } else {
      return _("This package has no debconf options.");
    }
  }


  function render_boolean($data)
  {
    $post_name= $data['post_name'];
    $result="
      <tr>
      <td valign='top' style='width:100%'>
      <h3>".$data['Topic']."</h3>".$data['Description']."
      </td>
      <td style=\"white-space:nowrap; vertical-align:top; border-left: 1px solid rgb(160, 160, 160);\">";

    foreach(array("true","false") as $value){
      if($data['Default'] == $value){
        $result.="<input type='radio' name='".$data['post_name']."' value='".$value."' checked>"._($value);
      }else{
        $result.="<input type='radio' name='".$data['post_name']."' value='".$value."' >"._($value);
      }
      $result.="<br>";
    }

    $result.= "
      </td>
      </tr>
      ";

    return ($result);
  }


  function render_multiselect($data)
  {
    $post_name= $data['post_name'];
    if (preg_match('/\$\{/', $data['Choices'])){
       $data['Description'].= '<br><br><b>'._('This debconf question is dynamically generated during package installation and requires choosing between specific options which cannot be presented here. The entered text needs to be one of the valid choices in order to take effect.').'</b>';
      $result= $this->render_string($data);
    } else {
      $choices= "";
      foreach (explode(", ", $data['Choices']) as $choice){
        $choices[]= $choice;
      }


      $result="
        <tr>
        <td valign='top'>
        <h3>".$data['Topic']."</h3>".$data['Description']."
        </td>
        <td valign='top'  style=\"white-space:nowrap; border-left: 1px solid rgb(160, 160, 160);\">
          <input type='hidden' name='multi-".$post_name."' value='1'>
        ";
        
      $defs = explode(", ",$data['Default']);
      foreach($choices as $value){
        if(in_array_strict($value,$defs)){
          $result.="\n<input name='".$post_name."-multi-".$value."' type='checkbox' value='".set_post($value)."' checked>".$value."<br>";
        }else{
          $result.="\n<input name='".$post_name."-multi-".$value."' type='checkbox' value='".set_post($value)."'>".$value."<br>";
        }
      }

    $result .=    "</td>
        </tr>
        ";
    }    

    return ($result);
  }


  function render_note($data)
  {
    /* Ignore notes, they do not makes sense, since we don't get any
       chance to test entered values... */
    return ("");
  }


  function render_password($data)
  {
    $result=  "";
    $result.= "<tr><td valign='top'>";
    $result.= "<h3>".$data['Topic']."</h3>".$data['Description']."</td><td style=\"white-space:nowrap; border-left: 1px solid rgb(160, 160, 160);\">&nbsp;<input type='text' name='".$data['post_name']."' value='".$data['Default']."'></b><br><br>";
    $result.= $data['Description'];
    $result.= "</td>";

    return ($result);
  }


  function render_select($data)
  {
    $post_name= $data['post_name'];

    if (preg_match('/\$\{/', $data['Choices'])){
      $result = $this->render_multiselect($data);
    } else {
      $choices= "";
      foreach (explode(", ", $data['Choices']) as $choice){
        $choices[]= $choice;
      }

      $result="
        
        <tr>
        <td valign='top'>
        <h3>".$data['Topic']."</h3>".$data['Description']."
        </td>
        <td  valign='top'  style=\"white-space:nowrap; border-left: 1px solid rgb(160, 160, 160);\">
        ";

      foreach($choices as $value){
        if($data['Default'] == $value){
          $result.="\n<input type='radio' name='".$post_name."' value='".set_post($value)."' checked >".set_post($value)."<br>";
        }else{
          $result.="\n<input type='radio' name='".$post_name."' value='".set_post($value)."'>".set_post($value)."<br>";
        }
      }

      $result.= "
        
        </td>
        </tr>
        ";
    }

    return ($result);
  }


  function render_string($data)
  {
    $result=  "
                <tr>
                  <td valign='top'>
                    <h3>".$data['Topic']."</h3>".$data['Description']."
                  </td>
                  <td  style=\"white-space:nowrap; border-left: 1px solid rgb(160, 160, 160);\" valign='top'>
                    <input type='text' name='".$data['post_name']."' value='".$data['Default']."' style='width:300px;'>
                  </td>
                </tr>
              ";

    return ($result);
  }


  function render_text($data)
  {
    /* Ignore text messages, they are normally used for status hints. */
    return ("");
  }


  function render_title($data)
  {
    /* Ignore text messages, they are normally used for status hints. */
    return ("");
  }

}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
