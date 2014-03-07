function checkEmail(control) {
	var filter = /^([a-zA-Z0-9_.-])+@(([a-zA-Z0-9-])+.)+([a-zA-Z0-9]{2,4})+$/;
	if (!filter.test(control.value)) {
		alert('Email incorrecte!');
		control.focus
		return false;
	}
}

//Funciones validadoras
 
function validaNif(control) 
{	
  if (control.value=="")
	return;
  var dni=control.value;    
  var numero = dni.substr(0,dni.length-1);
  var let = dni.substr(dni.length-1,1);
  let=let.toUpperCase();
  numero = numero % 23;
  var letra='TRWAGMYFPDXBNJZSQVHLCKET';
  letra=letra.substring(numero,numero+1);    
  //alert(letra);
  if (letra!=let)   
  {
    mensaje("dni","es");
    retornar(control.id);		
  }
}
 
 
function validaCif(control)
{ 
  if (control.value=="")
  {
	return;
  }
        var texto=control.value;
        var pares = 0; 
        var impares = 0; 
        var suma; 
        var ultima; 
        var unumero; 
        var uletra = new Array("J", "A", "B", "C", "D", "E", "F", "G", "H", "I"); 
        var xxx; 
         
        texto = texto.toUpperCase(); 
         
        var regular = new RegExp(/^[ABCDEFGHKLMNPQS]\d\d\d\d\d\d\d[0-9,A-J]$/g); 
         if (!regular.exec(texto)) 
			{
				mensaje("cif","es");
				retornar(control.id);		
			}
		 
              
         ultima = texto.substr(8,1); 
 
         for (var cont = 1 ; cont < 7 ; cont ++){ 
             xxx = (2 * parseInt(texto.substr(cont++,1))).toString() + "0"; 
             impares += parseInt(xxx.substr(0,1)) + parseInt(xxx.substr(1,1)); 
             pares += parseInt(texto.substr(cont,1)); 
         } 
         xxx = (2 * parseInt(texto.substr(cont,1))).toString() + "0"; 
         impares += parseInt(xxx.substr(0,1)) + parseInt(xxx.substr(1,1)); 
          
         suma = (pares + impares).toString(); 
         unumero = parseInt(suma.substr(suma.length - 1, 1)); 
         unumero = (10 - unumero).toString(); 
         if(unumero == 10) unumero = 0; 
          
         if ((ultima == unumero) || (ultima == uletra[unumero])) 
             return true; 
         else 
             {
				mensaje("cif","es");
				retornar(control.id);		
			 }
 
    } 
 
	
function validaNie(control) 
{	
  if (control.value=="")
	return;	
 
	var a=control;		
	var temp=a.value.toUpperCase();
	var cadenadni="TRWAGMYFPDXBNJZSQVHLCKET";
	var v1 = new Array(0,2,4,6,8,1,3,5,7,9);
	var posicion=0;
	var letra=" ";
	
	//Residente en España	
	if (a.value.length==9)
	{
		if (temp.substr(0,1)=="X")
		{
			var temp1=temp.substr(1,7);
 
			posicion = temp1 % 23; /*Resto de la division entre 23 es la posicion en la cadena*/
			letra = cadenadni.substring(posicion,posicion+1);
			if (!/^[A-Za-z0-9]{9}$/.test(temp))
			{ 
				mensaje("nie","es");
				retornar(control.id);	
			}
			else
			{ 
				//Tiene los 9 dígitos, comprobamos si la letra esta bien
				var temp1=temp.substr(1,7);
				posicion = temp1 % 23; /*Resto de la division entre 23 es la posicion en la cadena*/
				letra = cadenadni.charAt(posicion);
				var letranie=temp.charAt(8);
				if (letra != letranie){			
					mensaje("nie","es");
					retornar(control.id);			
				}				
			}
		}
		else
		{
			mensaje("nie","es");
			retornar(control.id);			
		}		
	}else if (a.value.length==14){//14 caracteres, los 2 primeros letras
		var temp1=temp.substr(0,2);
		if (isAlphabetic(temp1)!=true)	
			{
			mensaje("nie","es");
			retornar(control.id);	
			}
	}
	else
	{
			mensaje("nie","es");
			retornar(control.id);			
 
	}
	
}
 
function mensaje(msg,lang)
{
	if (lang=="es")
	{
		if(msg=="dni")	
			{
			alert("DNI no válido");
			}
		else if (msg=="cif")
					{
			alert("CIF no valido");
			}
		else if (msg=="nie")
					{
			alert("NIE no valido");
			}			
		else if (msg=="dc")
					{
			alert("El digito de control no es correcto");
			}						
		else if (msg=="completo")
					{
			alert("Rellene todos los datos de la cuenta");
			}									
		else if (msg=="formato")
					{
			alert("Formato incorrecto");
			}									
		else
			alert("Formato no valido");
	}
	
}
 
 
 
function validar(ibanco,isucursal,idc,icuenta) 
  {  
	var banco=(document.getElementById(ibanco).value);
	var sucursal=(document.getElementById(isucursal).value);
	var dc=(document.getElementById(idc).value);
	var cuenta=(document.getElementById(icuenta).value);	  
  
		if (banco == ""  || sucursal == "" || dc == "" || cuenta == ""){
		  return;
		}
		else 
		{
			if (banco.length != 4 || sucursal.length != 4 ||
				dc.length != 2 || cuenta.length != 10)
				{
				mensaje("completo","es");
				retornar(idc);							
			}
			else {
			  if (!numerico(banco) || !numerico(sucursal) ||
				  !numerico(dc) || !numerico(cuenta)){
				mensaje("formato","es");
				retornar(ibanco);
				retornar(isucursal);					  					  	
				retornar(idc);
				retornar(icuenta);
				  	}				
			  else {
			  	//alert(obtenerDigito("00" + banco + sucursal));
			  	//alert(obtenerDigito(cuenta));
				if (!(obtenerDigito("00" + banco + sucursal) ==
					  parseInt(dc.charAt(0))) || 
					!(obtenerDigito(cuenta) ==
					  parseInt(dc.charAt(1))))
					  {
					mensaje("dc","es");
					retornar(idc);					  					  	
				  	} 
				else
				  return;
			  }
			}
		}
}
 
function numerico(valor){
  cad = valor.toString();
  for (var i=0; i<cad.length; i++) {
    var caracter = cad.charAt(i);
	if (caracter<"0" || caracter>"9")
	  return false;
  }
  return true;
}
 
function obtenerDigito(valor){
  valores = new Array(1, 2, 4, 8, 5, 10, 9, 7, 3, 6);
  control = 0;
  for (i=0; i<=9; i++)
    control += parseInt(valor.charAt(i)) * valores[i];
  control = 11 - (control % 11);
  if (control == 11) control = 0;
  else if (control == 10) control = 1;
  return control;
}
 
 
function validaNif2(c1,c2)
{	
	var campo1=(document.getElementById(c1).value);
	var campo2=(document.getElementById(c2).value);	  
  
	if (campo1 == ""  || campo2 == ""){
	  return;
	}else{		
		var dni=campo1+campo2;  
		var numero = dni.substr(0,dni.length-1);
		var let = dni.substr(dni.length-1,1);
		numero = numero % 23;
		var letra='TRWAGMYFPDXBNJZSQVHLCKET';
		letra=letra.substring(numero,numero+1);  
		if (letra!=let)   
		{
			mensaje("dni","es");	
			retornar(c2);			
		}
	}
}
 
function validaCif2(c1,c2)
{	
	var campo1=(document.getElementById(c1).value);
	var campo2=(document.getElementById(c2).value);	  
  
	if (campo1 == ""  || campo2 == ""){
	  return;
	}else{
		//alert("Cif "+campo1+" "+campo2);
		
		var texto=campo1+campo2; 
        var pares = 0; 
        var impares = 0; 
        var suma; 
        var ultima; 
        var unumero; 
        var uletra = new Array("J", "A", "B", "C", "D", "E", "F", "G", "H", "I"); 
        var xxx; 
         
        texto = texto.toUpperCase(); 
         
        var regular = new RegExp(/^[ABCDEFGHKLMNPQS]\d\d\d\d\d\d\d[0-9,A-J]$/g); 
         if (!regular.exec(texto)) 
			{
				mensaje("cif","es");
				retornar(control.id);		
			}
		 
              
         ultima = texto.substr(8,1); 
 
         for (var cont = 1 ; cont < 7 ; cont ++){ 
             xxx = (2 * parseInt(texto.substr(cont++,1))).toString() + "0"; 
             impares += parseInt(xxx.substr(0,1)) + parseInt(xxx.substr(1,1)); 
             pares += parseInt(texto.substr(cont,1)); 
         } 
         xxx = (2 * parseInt(texto.substr(cont,1))).toString() + "0"; 
         impares += parseInt(xxx.substr(0,1)) + parseInt(xxx.substr(1,1)); 
          
         suma = (pares + impares).toString(); 
         unumero = parseInt(suma.substr(suma.length - 1, 1)); 
         unumero = (10 - unumero).toString(); 
         if(unumero == 10) unumero = 0; 
          
         if ((ultima == unumero) || (ultima == uletra[unumero])) 
             return true; 
         else 
             {
				mensaje("cif","es");
				retornar(c2);		
			 }
 
    } 		
		
		
	
}
 
function validaNie2(c1,c2)
{	
	var campo1=(document.getElementById(c1).value);
	var campo2=(document.getElementById(c2).value);	  
  
	if (campo1 == ""  || campo2 == ""){
	  return;
	}else{
		//alert("Nie "+campo1+" "+campo2);		
 
			var a=campo1+campo2; 		
			var temp=a.toUpperCase();
			var cadenadni="TRWAGMYFPDXBNJZSQVHLCKET";
			var v1 = new Array(0,2,4,6,8,1,3,5,7,9);
			var posicion=0;
			var letra=" ";
 
			//Residente en España	
			if (a.length==9)
			{
				if (temp.substr(0,1)=="X")
				{
					var temp1=temp.substr(1,7);
 
					posicion = temp1 % 23; /*Resto de la division entre 23 es la posicion en la cadena*/
					letra = cadenadni.substring(posicion,posicion+1);
					if (!/^[A-Za-z0-9]{9}$/.test(temp))
					{ 
						mensaje("nie","es");
						retornar(c2);	
					}
					else
					{ 
						//Tiene los 9 dígitos, comprobamos si la letra esta bien
						var temp1=temp.substr(1,7);
						posicion = temp1 % 23; /*Resto de la division entre 23 es la posicion en la cadena*/
						letra = cadenadni.charAt(posicion);
						var letranie=temp.charAt(8);
						if (letra != letranie){			
							mensaje("nie","es");
							retornar(c2);			
						}				
					}
				}
				else
				{
					mensaje("nie","es");
					retornar(c2);		
				}		
			}else if (a.length==14){//14 caracteres, los 2 primeros letras
				var temp1=temp.substr(0,2);
				if (isAlphabetic(temp1)!=true)	
					{
					mensaje("nie","es");
					retornar(c2);	
					}
			}
			else
			{
					mensaje("nie","es");
					retornar(c2);		
 
			}
 
	}
}

function checkID(control)
{	
	
	var val = 0;
	
	for( i = 0; i < document.mainform.irisPersonalUniqueIDType.length; i++ )
	{
		if( document.mainform.irisPersonalUniqueIDType[i].checked == true )
		val = document.mainform.irisPersonalUniqueIDType[i].value;
	}
	//alert( "val = " + val );
	
	if ( val == "0" || val == "DNI" || val == "NIF" ) {
		validaNif(control);
	}
	if ( val == "1" || val =="NIE") {
		validaNie(control);
	}
	
}
