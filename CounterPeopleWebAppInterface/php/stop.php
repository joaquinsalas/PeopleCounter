<?php
$json_a = file_get_contents("../config/cfg.json");
$json_a = json_decode($json_a, true);
$scripts_path = (string)$json_a['scriptsPath'];
$script = (string)$json_a['stopPath'];

chdir($scripts_path);
$output = shell_exec('sudo -p " " '.$script);
echo "all done! >> $scripts_path >> $script";
?>
