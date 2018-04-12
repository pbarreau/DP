<?php
/*
page : connexion.php
*/
session_start(); //  sert à maintenir la $_SESSION
if (isset($_POST['connexion']))
{
    //empty vérifie si le champs est vide et existe
    if (empty($_POST['pseudo']) OR empty($_POST['mdp']))
    {
        echo "Un des champs est vide.";
    } 
    else 
        {
            //les champs ne sont pas vides et corrects, on sécurise les données rentrées par l'utilisateur
            $pseudo = htmlentities($_POST['pseudo']);
            $mdp = htmlentities($_POST['mdp']);
            //on se connecte à la base de données
            $mysqli = mysqli_connect("domaine.tld", "nom d'utilisateur", "mot de passe", "base de données");
            //on vérifie la connexion
            if(!$mysqli)
            {
                echo "Erreur de connexion à la base de données !";
            }
            else 
            {
                //on effectue une requête à la base de données
                $requete = mysqli_query($mysqli, "SELECT * FROM .... WHERE pseudo = '".$pseudo"' AND mdp = '".$mdp"'" );
                if (mysqli_num_rows($requete)==0)
                {
                    echo "le pseudo ou le mot de passe est incorecte";
                }
                else
                {
                    //on ouvre la session
                    $_SESSION['pseudo'] = $pseudo;
                    echo " vous êtes connecté !";
                }             
            }
        }
} 
?>