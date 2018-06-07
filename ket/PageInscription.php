<?php
if(!empty($_POST['pseudo'])) //il faut vérifier si le formulaire a été rempli
{
// D'abord, je me connecte à la base de données.
$host="192.168.13.107";
$user="ketzia";
$passwd = "admin";
$database="ketzia";

$identifiant=$_POST['identifiant'];
$mdp=$_POST['mdp'];

$Cible = 'mysql:host='.$host.';dbname=ketzia;charset=utf8';
$conect= new PDO($Cible, $user, $passwd);

try
{
    $conect;
    $sql = "INSERT INTO Agent ('', '$identifiant', '$mdp')";
    $conect->exec($sql); // exécute la requête sql
    echo "Inscription terminee !";
   
}
catch (Exception $a)
{
    die('Erreur :'.$a->getMessage());
}

}

?>
<html>
<head>
                <link rel="stylesheet" href="style.css" />
        </head>
        <body>
        <p><strong><u><font color="teal"><h1 align="center"><t>Digeq Entreprise</t></h1></font></u></strong></p><hr/>

<form method="post" action="PageInscription.php">
<h2 align="center"><label>Identifiant: <input type="text" name="identifiant"/></label></h2><br/>
<h2 align="center"><label>Mot de passe: <input type="password" name="mdp"/></label></h2><br/>

<h2 align="center"><input type="submit" value="M'inscrire"/></h2>
</form>
</body>
</html>