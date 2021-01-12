<?php 
	$date = $_GET['date'];
	exec("sudo date -s '$date'");
	$updated_date = exec('date');
	echo $updated_date;
?>