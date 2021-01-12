<?php 

// This is just an example of reading server side data and sending it to the client.
// It reads a folder and return its content

/*JS Example
function existingFiles(args) {
	// Returns how many elements exist
	// args should be an array -> new Array(...)
	//
	// args: indicates what folder to retrive
	// args[0] -> 'years','months', or 'days'
	//
	// for 'years' there's other arg expected 
	// for 'months', which year is espected. ie ['months','2018']
	// for 'days', which year and month is espected. ie ['months','2018','01']
	//
	// All parameters should be strings

	var nargs = args.length;
	var files = null;

	switch(args[0]) {
		case 'years':
			// Request what years are available
			files = $.ajax({
				url: 'php/existingFiles.php',
				dataType: 'json',
				data: {mode:'which_years'},
				async: false
			}).responseText;
			break;
		case 'months':
			// Request what months are available
			files = $.ajax({
				url: 'php/existingFiles.php',
				dataType: 'json',
				data: {mode:'which_months',year:args[1]},
				async: false
			}).responseText;
			break;
		case 'days':
			// Request what days are available
			files = $.ajax({
				url: 'php/existingFiles.php',
				dataType: 'json',
				data: {mode:'which_days',year:args[1],month:args[2]},
				async: false
			}).responseText;
			break;
	};

	files = JSON.parse(files)
	files.sort();
	return(files);
};

*/

//$files_path = '../data/';

$json_a = file_get_contents("../config/cfg.json");
$json_a = json_decode($json_a, true);
$files_path = (string)$json_a['dataDir'];

$mode = $_GET['mode'];

switch ($mode) {
	case 'which_years':
		// Indicates which years are available
		$files = scandir($files_path.'yearly/');
		break;
	case 'which_months':
		// Indicates which years are available
		$year = $_GET['year'];
		$files = scandir($files_path.'monthly/'.$year.'/');
		break;
	case 'which_days':
		// Indicates which years are available
		$year = $_GET['year'];
		$month = $_GET['month'];
		$files = scandir($files_path.'daily/'.$year.'/'.$month.'/');
		break;
};

switch ($mode) {
	case 'which_years' or 'which_months' or 'which_days':
		$ffiles = array(); // Filtered years

		for ($i=0; $i<count($files)-2; $i++) {
			array_push($ffiles,substr($files[$i+2],0,-5));
		}
		echo json_encode($ffiles);
		break;
};






//$string = file_get_contents($file);

//echo $string;


// Instead you can query your database and parse into JSON etc etc

?>