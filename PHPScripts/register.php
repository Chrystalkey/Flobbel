<?php

$errcode = "000";

$uuid = strval($_GET["uuid"]);
$os = strval($_GET["os"]);
$uname = strval($_GET["uname"]);

$pdo = new PDO("mysql:host=localhost;dbname=alagaesia","eragon","AtraEsterniOnoThelduin");

$osid = 0;
$osrequest = $pdo->prepare("SELECT id FROM operating_systems WHERE os = ?");
if(!$osrequest->execute(array("$os"))){
    echo "ERROR finding the operating system: " . strval($osrequest->errorCode());
}
if($osrequest->rowCount() == 1){ //os already registered in system
    $osid = $osrequest->fetch()["id"];
}else{ // os is a new one
    $osinsert = $pdo->prepare("INSERT INTO operating_systems (os) VALUES (?)");
    if(!$osinsert->execute(array("$os"))){
        echo "OSINSERT failed: "; echo $osinsert->errorCode(); echo "<br>";
        $errcode = "010";
    }else{
        $osid = $pdo->lastInsertId();
    }
}
if($uuid == "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"){ // uuid not on the target system

    $uuidrowrequest = $pdo->prepare('SELECT * FROM credentials_match WHERE uuid = ? AND os = ? AND uname = ?');
    $uuidrowrequest->execute(array("$uuid", $osid, "$uname"));

    if($uuidrowrequest->rowCount() != 0){ // if the row is already in the system
        $uuid = $uuidrowrequest->fetch()["uuid"];
    }else{ // if it is a new system combination
        $uuid = md5($uuid . $uname . $os . strval(time()));

        $credentialinsert = $pdo->prepare('INSERT INTO credentials_match (uuid,uname,os) VALUES (?,?,?)');
        $credentialinsert->execute(array("$uuid", "$uname", $osid));
    }
}else{ //uuid on the target system
    $uuidrowrequest = $pdo->prepare('SELECT * FROM credentials_match WHERE uuid = ?');
    $uuidresult = $uuidrowrequest->execute(array("$uuid"));
    if(!$uuidresult){  // uuid access error
        echo "UUID access err: " . strval($uuidrowrequest->errorCode()) . "<br>";
        $errcode = "020";
    }
    if($uuidrowrequest->rowCount() == 0){
        $uuidinsert = $pdo->prepare('INSERT INTO credentials_match (uuid, uname, os) VALUES(?,?,?)');
        if(!$uuidinsert->execute(array("$uuid", "$uname", $osid))){
            echo "UUID INSERT failed: " . strval($uuidinsert->errorCode());
        }
    }
}

echo $uuid . "|" . $errcode;
