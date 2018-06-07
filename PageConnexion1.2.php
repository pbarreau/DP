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

<p>
<meta http-equiv="refresh" content="10">
<strong>Produit</strong> : <?php echo $donnees['nom']; ?><br/>
Reference : <?php echo $donnees['reference']; ?><br/>
Marque : <?php echo $donnees['marque']; ?><br/>
Categorie : <?php echo $donnees['categorie']; ?>
</p>
<?php
}
 $reponse->closeCursor(); //termine le traitement de la requête

 ?>