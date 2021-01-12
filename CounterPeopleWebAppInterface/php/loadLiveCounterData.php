<?php
//$files_path = "../data/";
//$file = $files_path."eventos.txt";
$json_a = file_get_contents("../config/cfg.json");
$json_a = json_decode($json_a, true);
$file = (string)$json_a['liveCounterDataPath'];

$string = file_get_contents($file);
echo $string;
?>