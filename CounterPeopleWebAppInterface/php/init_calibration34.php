<?php
$json_a = file_get_contents("../config/cfg.json");
$json_a = json_decode($json_a, true);
$calib_path = (string)$json_a['calibration34Path'];

shell_exec($calib_path)
echo "all done!";
?>