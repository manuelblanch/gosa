<?php
ob_start();

/* UNCOMMENT THIS TO ACTIVATE ERROR REPORTING!
error_reporting(E_ALL);
ini_set("display_errors", 1);
*/
error_reporting(0);
ini_set("display_errors", 0);

//$old_level_reporting=error_reporting();
//error_reporting(0);
require_once ("../../include/php_setup.inc");
require_once ("functions.inc");
require_once ("../../plugins/admin/HighSchoolUsers/functions.php");
require_once ("/usr/share/php/fpdf/fpdf.php");
               

session::start();
$config= session::global_get('config');

//*******************************************************************
//**          		  CONFIGURATION								   **			
//*******************************************************************

//GENERAL CONFIG

//URL LINKS
$rulesURL="http://moodle.iesmontsia.org/normesTIC";
$servicesURL="http://moodle.iesmontsia.org/serveisTIC";

$HIGHSCHOOLSNAME="Institut Montsià";
$HIGHSCHOOLSUFFIXEMAIL="iesmontsia.org"; 

/////////////////// DO NOT TOUCH WHEN CONFIGURING 
//PDF DOCUMENT
// DOCUMENT NAME= externalID_internalID_documentNameSufix
$documentNameSufix="_matriculaTIC.pdf";

//WINDOWS HEADER AT PDF DOCUMENT
// TITLE USER FULL NAME
$windowheadertitle="Matrícula TIC de l'alumne";

//IMAGES PATHS
$logo_image="/usr/share/gosa/html/pdfreports/images/logo1.jpeg";
$signature_image="/usr/share/gosa/html/pdfreports/images/signature.jpeg";

//STRINGS
$STR_TITLE=_("MATRÍCULA TIC");
$STR_User=_("Usuari");
$STR_Password=_("Paraula de pas");
$STR_InternalID=_("Identificador del centre");
$STR_PersonalEmail=_("Correu electrònic personal");
$STR_Email=_("Correu electrònic del centre");
$STR_UserSignature=_("Signatura de l'interessat/interessada");
$STR_SchoolSignature=_("Signatura i segell del centre");
$STR_UserPageType=_("Exemplar per a la persona interessada");
$STR_SchoolPageType=_("Exemplar per a l'escola");
$STR_TutorPageType=_("Exemplar per al tutor");

$IMPORTANT_NOTE=_("IMPORTANT: La paraula de pas ha de ser PERSONAL i INTRANSFERIBLE, s'ha d'utilitzar en cura i no es pot deixar-la o prestar-la a altres usuaris. És la vostra responsabilitat no facilitar el vostre usuari o paraula de pas a NINGÚ. Queda expressament prohibit assumir la identitat d'altres usuaris.");

/////////////////// DO NOT TOUCH WHEN CONFIGURING 

$dn=$_GET['dn'];
$password=$_GET['password'];

if ($dn== "") {
	echo "<br/>Fatal Error! No DN provided at query string!";
	exit(1);
}

if ($password== "") {
	echo "<br/>Fatal Error! No Password provided at query string!";
	exit(1);
}

//Obtain enrollment data.
$enrollment_data=getEnrollmentData($dn);

if ($enrollment_data== "") {
	echo "<br/>Fatal Error! No enrollment data found for DN: " . $dn;
	exit(1);
}

$givenName = $enrollment_data['givenName']['0'];
$internalID = $enrollment_data['highSchoolUserId']['0'];
$employeeNumber = $enrollment_data['employeeNumber']['0'];
$externalID = $enrollment_data['irisPersonalUniqueID']['0'];
$personal_email = $enrollment_data['highSchoolPersonalEmail']['0'];
$emailCorporatiu = $enrollment_data['email']['0'];
$uid = $enrollment_data['uid']['0'];
$sn1 = $enrollment_data['sn1']['0'];
$sn2 = $enrollment_data['sn2']['0'];

// Assuming today is March 10th, 2001, 5:16:18 pm, and that we are in the
// Mountain Standard Time (MST) Time Zone
$date= date('j-m-y');	
setlocale(LC_TIME, "ca_ES.UTF-8");
$day_of_month = strftime("%e");
$month = strftime("%B");
$year = strftime("%G");
//$date2= strftime("%B");

/////////////////// END DO NOT TOUCH WHEN CONFIGURING 

//TEXTS

$text1 = <<<EOF
En/Na $givenName $sn1 $sn2, amb número identificatiu $externalID, ha estat matriculat/da el $date per tal de tenir accés als recursos TIC de l'$HIGHSCHOOLSNAME. Les dades que heu d'utilitzar per accedir als recursos TIC del centre són:
EOF;

$text2 = <<<EOF
En firmar aquesta matrícula esteu acceptant les normes d'ús dels recursos TIC del centre. Les normes les podeu consultar a: 


EOF;

$text3 = <<<EOF
Amb el vostre compte d'usuari de centre podeu accedir a una sèrie de serveis que us ofereix el centre i que podeu consultar a:


EOF;

$text4 = <<<EOF
En aquesta pàgina web també podeu trobar les instruccions per tal de modificar la vostra paraula de pas. És important que escolliu una paraula de pas prou segura i que us sigui fàcil de recordar. 

IMPORTANT: Si oblideu la vosta paraula de pas, la forma de recuperar-la serà enviar-vos una de nova a la vostra adreça de correu electrònic personal, per tant és molt important que ens proporcioneu una adreça de correu electrònic vàlida.

EOF;

$text5 = <<<EOF
Amposta, $day_of_month de $month de $year
EOF;
	

//*******************************************************************
//**          		  CONFIGURATION	END							   **			
//*******************************************************************


//PDF Document Name when downloading:
$documentName=$externalID."_".$internalID.$documentNameSufix;
$fullName= $givenName ." ". $sn1 . " " . $sn2;

//uncomment when debugging
//exit();

//FPDF needs a clean output --> force:
ob_end_clean();

//CREATE PDF OUPUT:
$pdf=new FPDF();

//DOCUMENT TITLE: Appears at PDF window title 
$pdf->SetTitle(utf8_decode($windowheadertitle)." ". utf8_decode($fullName), false);


//CREATE PAGES: Multiple similar pages with some changes

$numPages=3;
$pageTypes=array("user","school","tutor");

$pdf->SetMargins(20, 20, 20);
$pdf->SetLeftMargin(20);

for ($i = 1; $i <= $numPages; $i++) {
	$pdf->AddPage();
	$pdf->SetFont('Times','',18);

	//HEADER IMAGE
	$pdf->Image($logo_image,$pdf->GetX(),$pdf->GetY());
	
	//TITLE
	$pdf->SetY(45);
	$pdf->Cell(170,10,utf8_decode($STR_TITLE),1,2,'C');
	
	//TEXT1
	$pdf->SetFont('Times','',10);	
	$pdf->Ln();
	$pdf->write(5,utf8_decode($text1));
	
	//ENROLLMENT DATA
	//USER
	$pdf->Ln();
	$pdf->Ln();
	$pdf->SetX($pdf->GetX()+10);	
	$pdf->SetFont('Times','B',10); 
	$pdf->write(5,"- ". utf8_decode($STR_User).": ",0);
	$pdf->SetFont('Times','',10); 
	$pdf->write(5,utf8_decode($uid),0);
	$pdf->Ln();
	
	$pdf->SetX($pdf->GetX()+10);	
	$pdf->SetFont('Times','B',10); 
	$pdf->write(5,"- ". utf8_decode($STR_Password).": ",0);
	$pdf->SetFont('Times','',10); 
	$pdf->write(5,utf8_decode($password),0);
	$pdf->Ln();
	
	$pdf->SetX($pdf->GetX()+10);	
	$pdf->SetFont('Times','B',10); 
	$pdf->write(5,"- ". utf8_decode($STR_InternalID).": ",0);
	$pdf->SetFont('Times','',10); 
	$pdf->write(5,utf8_decode($internalID),0);
	$pdf->Ln();
	
	$pdf->SetX($pdf->GetX()+10);	
	$pdf->SetFont('Times','B',10); 
	$pdf->write(5,"- ". utf8_decode($STR_PersonalEmail).": ",0);
	$pdf->SetFont('Times','',10); 
	$pdf->write(5,utf8_decode($personal_email),0);
	$pdf->Ln();
	
	$pdf->SetX($pdf->GetX()+10);	
	$pdf->SetFont('Times','B',10); 
	$pdf->write(5,"- ". utf8_decode($STR_Email).": ",0);
	$pdf->SetFont('Times','',10); 
	$pdf->write(5,utf8_decode($uid."@".$HIGHSCHOOLSUFFIXEMAIL),0);
	$pdf->Ln();
	
	//TEXT 2
	$pdf->Ln();		
	$pdf->write(5,utf8_decode($text2));
	
	//RULES URL
	$pdf->SetX($pdf->GetX()+10);	
	$pdf->SetFont('Times','B',10); 
	$pdf->write(5,utf8_decode($rulesURL));
	$pdf->Ln();
	$pdf->Ln();

	//IMPORTANT NOTE
	$pdf->SetFont('Times','',10);	
	$pdf->SetLeftMargin(20+10); 
	$pdf->SetRightMargin(20+10); 
    $pdf->MultiCell(0,5,utf8_decode($IMPORTANT_NOTE),1,"L");
    $pdf->SetLeftMargin(20); 
    $pdf->SetRightMargin(20); 
	$pdf->Ln();
	
	//TEXT3
	$pdf->write(5,utf8_decode($text3));
	
	//SERVICES URL
	$pdf->SetX($pdf->GetX()+10);	
	$pdf->SetFont('Times','B',10); 
	$pdf->write(5,utf8_decode($servicesURL));
	$pdf->Ln();
	$pdf->Ln();
	
	//TEXT 4
	$pdf->SetFont('Times','',10); 
	$pdf->write(5,utf8_decode($text4));
	$pdf->Ln();
	
	//USER_SIGNATURE
	$pdf->SetFont('Times','',10); 
	$pdf->write(5,utf8_decode($STR_UserSignature. ","));
	$pdf->Ln();
	
	//FOOTNOTE
	$pdf->SetY(-50);
	$pdf->SetFont('Times','',10);	
	$pdf->write(5,utf8_decode($text5));
	
	//OFICIAL SIGNATURE
	$pdf->Ln();
    $pdf->Image($signature_image,$pdf->GetX()-3, $pdf->GetY());
    $pdf->write(5,utf8_decode($STR_SchoolSignature),0);
	
    //TYPE    
	$pdf->Ln();
	$pdf->Line($pdf->GetX(), $pdf->GetY(), $pdf->GetX()+170, $pdf->GetY());
	$pdf->SetX(133);
	
	switch ($pageTypes[$i-1]) {
    	case "user":
	        $pdf->write(5,utf8_decode($STR_UserPageType),0);
        	break;
    	case "school":
        	$pdf->write(5,utf8_decode($STR_SchoolPageType),0);
        	break;
    	case "tutor":
	        $pdf->write(5,utf8_decode($STR_TutorPageType),0);
        	break;
	}
}
	
$pdf->Output($documentName,"D");
//Si afegeixo el exit aleshores tarda molt més en obrir el PDF!!!
//exit();
?>