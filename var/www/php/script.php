<?php
header("Content-Type: text/plain");

$name = $_POST['name'] ?? 'non fourni';
$age = $_POST['age'] ?? 'non fourni';

echo "Nom : $name\n";
echo "Âge : $age\n";
?>
