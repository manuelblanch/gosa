<div style="font-size:18px;">
  {t}Creating a new student using templates{/t}
</div>

<p>
 {t}This form can help you creating a new student.{/t}
</p>

<hr>

<br>

<script type="text/javascript" src="include/highschoolusers_validations.js"></script> 

<table summary="{t}User template selection dialog{/t}" cellpadding=4 border=0>

{if $notfoundtemplates != ""}
  <tr>
    <td><b><font color="red"><LABEL for="Templates not found">{t}Templates not found!{/t}</LABEL></font></b></td>
    <td>
     {foreach from=$notfoundtemplates item=notfoundtemplate}
		<input type='text' size="60" maxlength="80" value="{$notfoundtemplate}" readonly="readonly">
	{/foreach}
    </td>
  </tr>
{/if}  

{if $existentuser != ""}
  <tr>
    <td><b><font color="red"><LABEL for="Existing user">{t}Existent user{/t}</LABEL></font></b></td>
    <td>
     <input type='text' name="existingUser" id="existingUser" size="60" maxlength="80" value="{$existentuser}" readonly="readonly">
     <input type="hidden"  name="copyExistingUser" id="copyExistingUser" />
     <button type='submit' name='template_continue' id='copy'>{t}Copy{/t}</button>
     
     <!-- TODO: backgroun-position: La imatge té totes les icones i s'indica una posició
          Cal mirar de fer-ho programaticament-->
          <!-- 
     <input type='submit' class='img' id='prova' value='' name='prova' 
    title='Edit generic properties' style='background-position:-387px -672px;width:16px;height:16px'>-->
    </td>
  </tr>
{/if}  

{if $conflictiveField != ""}
  <tr>
    <td><b><font color="red"><LABEL for="Conflictive field">{t}Conflictive Field{/t}</LABEL></font></b></td>
    <td>
     <input type='text' name="conflictiveField" id="conflictiveField" size="60" maxlength="80" value="{$conflictiveField}" readonly="readonly">
     <!-- TODO: backgroun-position: La imatge té totes les icones i s'indica una posició
          Cal mirar de fer-ho programaticament-->
    </td>
  </tr>
{/if}  

{if $got_uid eq "true"}
  <tr>
    <td><b><font color="red">{t}Proposed Login{/t}</font></b></td>
    <td>
      {if $edit_uid eq "false"}
      <select size="1" name="uid">
        {html_options output=$uids values=$uids selected=$uid}
      </select>
      {else}
      <input type='text' name="uid" size="30" maxlength="40" value="{$uid}">
      {/if}
    </td>
  </tr>
  {/if}

  <tr>
    <td><b><LABEL for="template">{t}Template{/t}</LABEL></b></td>
    <td>
    <select size="1" name="template" id="template" tabindex="1" readonly>
       {html_options options=$templates selected=$template}
    </select>
    </td>
  </tr>
  
  <!-- Afegit per Sergi Tur 6/6/2010-->
  <tr>
    <td><b><LABEL for="internalID">{t}Internal ID{/t}{$must}</LABEL></b></td>
    <td>
     <input type='text' name="highSchoolUserId" id="highSchoolUserId" size="30" maxlength="40" value="{$highSchoolUserId}" tabindex="2" readonly>
     {if $userType eq "teacher"}
     	<b><LABEL for="teacherCode">{t}Teacher Code{/t}{$must}</LABEL></b> &nbsp;<input type="text" value="{$employeeNumber}" name="employeeNumber" id="employeeNumber" />
     	<input type="hidden" value="1" name="checkTeacherCode" id="checkTeacherCode" />
     {else}
     	<input type="hidden" value="{$employeeNumber}" name="employeeNumber" id="employeeNumber" />
     {/if}
    </td>
  </tr>
  
  <tr>
    <td><b><LABEL for="irisPersonalUniqueID">{t}External ID{/t}{$must}</LABEL></b></td>
    <td>
     <input type='text' name="irisPersonalUniqueID" id="irisPersonalUniqueID" size="30" maxlength="40" value="{$irisPersonalUniqueID}" onblur="checkID(this)" tabindex="3">
     {html_radios name='irisPersonalUniqueIDType' options=$irisPersonalUniqueIDTypes selected=$irisPersonalUniqueIDType separator=' '}
    </td>
  </tr>
  
  <tr>
    <td><b><LABEL for="givenName">{t}First name{/t}{$must}</LABEL></b></td>
    <td><input type='text' name="givenName" id="givenName" size="30" maxlength="40" value="{$givenName}" tabindex="5"></td>
  </tr>
  <tr>
    <td><b><LABEL for="sn1">{t}First Last name{/t}{$must}</LABEL></b></td>
    <td><input type='text' name="sn1" id="sn1" size="30" maxlength="40" value="{$sn1}" 
    onchange="document.getElementById('sn').value = document.getElementById('sn1').value + ' ' + document.getElementById('sn2').value ;" tabindex="6"></td>
  </tr>
  <tr>
    <td><b><LABEL for="sn2">{t}Second Last name{/t}</LABEL></b></td>
    <td><input type='text' name="sn2" id="sn2" size="30" maxlength="40" value="{$sn2}"
    onchange="document.getElementById('sn').value = document.getElementById('sn1').value + ' ' + document.getElementById('sn2').value ;" tabindex="7"></td>
  </tr>

  <tr>
    <td><b><LABEL for="highSchoolPersonalEmail">{t}Email{/t}</LABEL></b></td>
    <td>
     <input type='text' name="highSchoolPersonalEmail" id="highSchoolPersonalEmail" size="30" maxlength="40" value="{$highSchoolPersonalEmail}" onblur="checkEmail(this)" tabindex="8">
     {t}Warning: Users without password cannot recover lost password by themselves{/t}{$must}
    </td>
  </tr>
  <tr>
    <td><b><LABEL for="sn">{t}Calculated Last name{/t} ({t}read only{/t})</LABEL></b></td>
    <td><input type='text' name="sn" id="sn" size="30" maxlength="40" value="{$sn}" readonly="true" tabindex="9">
    </td>
  </tr>
    
  <!-- fi afegit per Sergi Tur-->

</table>


<hr>
<div class="plugin-actions">
{if $existentuser != ""}
	<button type='submit' name='template_continue_reset'">{t}Reset{/t}</button>
	<button type='submit' name='template_continue' onclick="document.getElementById('template').disabled = false;" disabled>{t}Continue{/t}</button>
{else} 
	<button type='submit' name='template_continue' onclick="document.getElementById('template').disabled = false;">{t}Continue{/t}</button> 
{/if} 
	<button type='submit' name='edit_cancel'>{msgPool type=cancelButton}</button> 
</div>

<input type="hidden" name="userType" id="userType" value="{$userType}" />

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('sn');
  -->
</script>
