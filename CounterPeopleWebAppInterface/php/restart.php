<?php
$json_a = file_get_contents("../config/cfg.json");
$json_a = json_decode($json_a, true);
$scripts_path = (string)$json_a['scriptsPath'];
$script = (string)$json_a['restartPath'];

# S: Test
/*
$my_file = '/home/orbbec/contador/test_result.txt';
$handle = fopen($my_file,'w') or die('Cannot open file: '.$my_file);
echo fread($handle,filesize($my_file));
$data = 'Trying to execute: '.$script;
$data  = $data.' from '.$scripts_path;
fwrite($handle,$data);
*/
# E: Test

#chdir('/home/orbbec/contador/');
#$output = shell_exec('./test.sh');

chdir($scripts_path);
$output = shell_exec($script);
#$gcd = getcwd();
echo "all done!";# >> $gcd  >> $script";
?>
