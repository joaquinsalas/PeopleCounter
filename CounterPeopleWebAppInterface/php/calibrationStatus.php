<?php 
//$files_path = "../data/system_status/";
//$file = $files_path.'status.json';

$json_a = file_get_contents("../config/cfg.json");
$json_a = json_decode($json_a, true);
$file = (string)$json_a['calibrationStatusPath'];

$string = file_get_contents($file);
echo $string;
?>