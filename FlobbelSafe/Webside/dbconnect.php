<?php
echo "Hello World";
$pdo = new PDO("mysql:host=localhost;dbname=test","Eragon","AtraEsterniOnoThelduin");
#$query = "CREATE TABLE IF NOT EXISTS Alagaesia(id BIGINT, region TEXT, race TEXT);";
#$pdo->exec($query);

#$pdo->exec('INSERT INTO Alagaesia(id,region,race) VALUES(0,"Palancar Tal", "Menschen")');

/*$query = <<<eof
            LOAD DATA INFILE '$filename'
            INTO TABLE tableName
            FIELDS TERMINATED BY ';'
            OPTIONALLY ENCLOSED BY '"'
            LINES TERMINATED BY '\n'
            (...)
            eof;
*/

?>