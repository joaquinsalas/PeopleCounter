var ut = {};

ut.getJson = function(args) 
{
	// Returns a json
	// args is an array of strings 
	// new Array(period,year,month,day)
  	//
	// period: 'year', 'month', or 'day'
	// year: 2018 | month: 01 | day: 01

	var period = args[0];
	var year = args[1];
	var month = args[2];
	var day = args[3];

	var jsonData = $.ajax({
		url: "php/loadData.php",
		data: {mode:period,year:year,month:month,day:day},
		dataType: "json",
		async: false
	}).responseText;

	var data = JSON.parse(jsonData);
	return data;//jsonData;
};

ut.getJsonData = function(args) 
{
	// Returns a Google DataTable
	// args is an array of strings 
	// new Array(period,year,month,day)
    //
	// period: 'year', 'month', or 'day'
	// year: 2018 | month: 01 | day: 01

	var period = args[0];
	var year = args[1];
	var month = args[2];
	var day = args[3];

	var jsonData = $.ajax({
		url: "php/loadData.php",
		data: {mode:period,year:year,month:month,day:day},
		dataType: "json",
		async: false
	}).responseText;
	console.log(jsonData);
	var data = new google.visualization.DataTable(jsonData);
	//formatDate.format(data,0)
	return data;
};

ut.existingFiles = function(args) 
{
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
	console.log(files);	
	files = JSON.parse(files);
	files.sort();
	return(files);
};

ut.populateForm = function (ymd,id) {
	// ymd: array containing the strings to be displayed
	// id: 0='year_selector',1='month_selector', 
	//     2='day_selector'

	var months = ['Enero','Febrero','Marzo','Abril','Mayo',
								'Junio','Julio','Agosto','Septiembre',
								'Octubre','Noviembre','Diciembre'];

	var cls_str = null;
	var dest_id = null;

	switch(id) {
		case 0:
			cls_str = 'year_select_item';
			dest_id = 'year_select';
			break;
		case 1:
			cls_str = 'month_select_item';
			dest_id = 'month_select';
			break;
		case 2:
			cls_str = 'day_select_item';
			dest_id = 'day_select';
			break;
	};
	
	for (var i=0;i<ymd.length;i++) {
		var oi = document.createElement('option');
		oi.setAttribute('class',cls_str);
		oi.setAttribute('value',ymd[i]);
		//oi.setAttribute('id',id_str);
		if(id!=1) {
			var txt = document.createTextNode(ymd[i]);
		} else {
			var txt = document.createTextNode(months[parseInt(ymd[i])-1]);
		}
		oi.appendChild(txt);
		document.getElementById(dest_id).appendChild(oi);
	};
};

ut.cleanForm = function (id) {
	// id: 0='year_selector',1='month_selector', 
	//     2='day_selector'
	var cls_str = null;
	switch(id) {
		case 0:
			cls_str = 'year_select_item';
			break;
		case 1:
			cls_str = 'month_select_item';
			break;
		case 2:
			cls_str = 'day_select_item';
			break;
	};

	$('.'+cls_str).remove();
};
