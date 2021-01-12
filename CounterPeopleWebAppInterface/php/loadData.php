<?php 

// This is just an example of reading server side data and sending it to the client.
// It reads a json formatted text file and outputs it.

// mode: indicates if you want to retrieve information of
//    a day, month, year, or the full registery

/*JS Example
function getJsonData() {
	var jsonData = $.ajax({
		url: "php/loadData.php",
		data: {mode:period,year:year,month:month,day:day},
		dataType: "json",
		async: false
	}).responseText;

	var data = new google.visualization.DataTable(jsonData);
	formatDate.format(data,0)
	return data;
};
*/

//$files_path = "../data/";

$json_a = file_get_contents("../config/cfg.json");
$json_a = json_decode($json_a, true);
$files_path = (string)$json_a['dataDir'];


$mode = $_GET['mode'];

switch ($mode) {
	case 'year':
		$year = $_GET['year'];
		$file = $files_path."yearly/".$year.".json";
		break;
	case 'month':
		$year = $_GET['year'];
		$month = $_GET['month'];
		$file = $files_path."monthly/".$year."/".$month.".json";
		break;
	case 'day':
		$year = $_GET['year'];
		$month = $_GET['month'];
		$day = $_GET['day'];
		$file = $files_path."daily/".$year."/".$month."/".$day.".json";
		break;
};

switch ($mode) {
	case 'year' or 'month' or 'day':
		$string = file_get_contents($file);
		echo $string;
		break;
}
