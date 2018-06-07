
<h3 align="right"><a href="PageHtml1.html">Deconnexion</a></h3>
<h3 align="right"><a href="PageElectromenager.php">Electromenager</a></h3>
<h3 align="right"><a href="PageBricolage.php">Bricolage</a></h3>
<h3 align="right"><a href="PageJardin.php">Jardin</a></h3>
<h3 align="right"><a href="PageMenage.php">Menage</a></h3>

<?php

$host="192.168.13.107";
$user="ketzia";
$passwd = "admin";
$database="ketzia";

$Cible = 'mysql:host='.$host.';dbname=ketzia;charset=utf8';
$conect= new PDO($Cible, $user, $passwd); //mysql_connect($host, $user,$passwd, $database) or die("erreur de connexion au serveur");

//mysql_select_db($database) or die("erreur de connexion a la base de donnees");

try
{
    $conect;
}
catch (Exception $a)
{
    die('Erreur :'.$a->getMessage());
}

//on récupère le contenu de la table inventaire
$reponse = $conect->query('SELECT * FROM inventaire');

//on affiche chaque entrée une à une
while ($donnees = $reponse->fetch())
{
?>


<meta http-equiv="refresh" content="10">
<table border="1" cellpadding="10" cellspacing="1" width="100%">
   <tr>
      <th width="20%" align="center">Nom</th>
      <th width="10%" align="center">Reference</th>
      <th width="10%" align="left">Marque</th>
      <th width="10%" align="left">Categorie</th>
   </tr>
   <tr>
      <td align="center" valign="top"><?php echo $donnees['nom']; ?></td>
      <td align="center" valign="top"> <?php echo $donnees['reference']; ?></td>
      <td align="left" valign="top"><?php echo $donnees['marque']; ?></td>
      <td align="left" valign="top"><?php echo $donnees['categorie']; ?></td>
   </tr>
</table>

<?php
}
 $reponse->closeCursor(); //termine le traitement de la requête

 ?>