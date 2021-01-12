$(document).ready(start);

function start() {

	var initRestart = document.getElementById('restart');
	initRestart.addEventListener('click',initRestartClickedResp,false);

	var initStop = document.getElementById('stop');
	initStop.addEventListener('click',initStopClickedResp,false);

	var initCalibration12 = document.getElementById('calibrate12');
	initCalibration12.addEventListener('click',initCalibration12ClickedResp,false);

	var initCalibration23 = document.getElementById('calibrate23');
	initCalibration23.addEventListener('click',initCalibration23ClickedResp,false);
	
	var initCalibration34 = document.getElementById('calibrate34');
	initCalibration34.addEventListener('click',initCalibration34ClickedResp,false);


	/* Starts: Functions Response */
	function initRestartClickedResp() {
		console.log('Restart');
		var init_main = $.ajax({
			url: 'php/restart.php',
			async: false
		}).responseText;
		console.log(init_main);
	}


	function initStopClickedResp() {
		console.log('Stop');
		var init_main = $.ajax({
			url: 'php/stop.php',
			async: false
		}).responseText;
		console.log(init_main);
	}


	function initCalibration12ClickedResp() {
		console.log('Calibration12');
		var init_calibration = $.ajax({
			url: 'php/init_calibration12.php',
			async: false
		}).responseText;
		console.log(init_calibration);
	}

	function initCalibration23ClickedResp() {
		console.log('Calibration23');
		var init_calibration = $.ajax({
			url: 'php/init_calibration23.php',
			async: false
		}).responseText;
		console.log(init_calibration);
	}

	function initCalibration34ClickedResp() {
		console.log('Calibration34');
		var init_calibration = $.ajax({
			url: 'php/init_calibration34.php',
			async: false
		}).responseText;
		console.log(init_calibration);
	}
	/* Ends: Functions Response */


	var status = getCamStatus();
	for (var i=1; i<5; i++) {
		updateStatus(status['cam'+i.toString()],i);
	}
	
	var status = getCalibrationStatus();
	for (var i=1; i<4; i++) {
		var j = i+1;
		updateStatus(status['cam'+i.toString()+"-"+j.toString()],i,indic='cal');
	}

	function updateStatus(arg,cam,indic="cam") {
		// Updates the color of the indicator of the cemera status
		if(arg==1) {
			document.getElementById(indic+'-'+cam.toString()).style.backgroundColor = '#12ba36';
		} else {
			document.getElementById(indic+'-'+cam.toString()).style.backgroundColor = 'gray';
		};
	};

	function getCamStatus()
	{
		var jsonData = $.ajax({
			url: "php/systemStatus.php",
			dataType: "json",
			async: false
		}).responseText;

		var data = JSON.parse(jsonData);
		return data;//jsonData;
	};

	function getCalibrationStatus()
	{
		var jsonData = $.ajax({
			url: "php/calibrationStatus.php",
			dataType: "json",
			async: false
		}).responseText;

		var data = JSON.parse(jsonData);
		return data;//jsonData;
	};
};