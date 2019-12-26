<?php

$pdo = new PDO("mysql:host=localhost;dbname=alagaesia","eragon","AtraEsterniOnoThelduin");
$return = "";
foreach($pdo->query("SELECT * FROM blacklist") as $row){
    $return = $return . $row['process'] . "/";
}

echo substr($return,0,-1);