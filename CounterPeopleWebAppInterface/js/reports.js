$(document).ready(start);

function start() {
	var period = null;
	var year = null;
	var month = null;
	var day = null;

	if($('.main').is('#reports')) {
		period = 'year';
		$('#month_form').hide();
		$('#day_form').hide();

		var years = ut.existingFiles(new Array('years'));
		ut.populateForm(years,0);

		var periodSelect = document.getElementById('period_select');
		periodSelect.addEventListener('change',periodSelectedResp,false);

		var yearSelect = document.getElementById('year_select');
		yearSelect.addEventListener('change',yearSelectedResp,false);
		//yearSelect.on("change",yearSelectedResp);

		var monthSelect = document.getElementById('month_select');
		monthSelect.addEventListener('change',monthSelectedResp,false);

		var daySelect = document.getElementById('day_select');
		daySelect.addEventListener('change',daySelectedResp,false);

		var downloadClick = document.getElementById('download');
		downloadClick.addEventListener('click',downloadSelectedResp,false);		
	};


	////////////// Start: Response Functions
	function periodSelectedResp() {
		if (this.value.length>0) {
			// Save selected year
			period = this.value;
			console.log(period);
		};
		switch(period){
			case 'year':
				$('#month_form').hide();
				$('#day_form').hide();
			break;
			case 'month':
				$('#month_form').show();
				$('#day_form').hide();
			break;
			case 'day':
				$('#month_form').show();
				$('#day_form').show();
			break;
		}
	};

	function yearSelectedResp() {
		if (this.value.length>0) {
			// Save selected year
			year = this.value;

			// Errase all old content
			ut.cleanForm(1);
			ut.cleanForm(2);

			// Populate form
			var months = ut.existingFiles(new Array('months',this.value));
			ut.populateForm(months,1);
		};
	};

	function monthSelectedResp() {
		if (this.value.length>0) {
			// Save selected month
			month = this.value;

			// Errase all old content
			ut.cleanForm(2);

			// Populate form
			var months = ut.existingFiles(new Array('days',year,this.value));
			ut.populateForm(months,2);
		};
	};

	function daySelectedResp() {
		if (this.value.length>0) {
			day = this.value;
		};
	};

	function downloadSelectedResp() {
		var GET_DATA = false;
		switch(period) {
			case 'year':
				if (year!=null) {
					GET_DATA = true;
				};
				break;
			case 'month':
				if(year!=null && month!=null) {
					GET_DATA = true;
				};
				break;
			case 'day':
				if(year!=null && month!=null && day!=null) {
					GET_DATA = true;
				};
				break
		};
		if(GET_DATA) {
			var data = ut.getJson(new Array(period,year,month,day));
			var args = null
			if (day!=null){
				args = [period,year,month,day]
			}
			else if (month!=null) {
				args = [period,year,month]	
			}
			else if (year!=null){
				args = [period,year]
			}
			downloadJSON(data,args);
		}
	};
	////////////// Ends: Response Functions
}


function leadZeros(str) {
		if (str.length<2) {
			str = '0'+str;
		}
		return str;
	};

function downloadJSON(data,args) {
	// https://code-maven.com/create-and-download-csv-with-javascript
	// data should be a JSON object with two objects inside; 
	//  'cols' and 'rows'
	console.log('HERE')
	console.log(args)
	console.log(data)
	var csv = 'Fecha,Hora,Entradas,Salidas\n';
	
	for (var i = 0; i<data['rows'].length; i++) {
		if(args[0]!='day'){
			var row = data['rows'][i]['c'];
			
			var dateTime = row[0]['v'];
			var ins = row[1]['v'];
			var outs = row[2]['v'];

			dateTime = dateTime.replace('Date(','');
			dateTime = dateTime.replace(')','');
			
			dateTime = dateTime.split(',');
			
			var MM = leadZeros((parseInt(dateTime[1])+1).toString());
			var DD = leadZeros(dateTime[2]);

			var date = dateTime[0]+'-'+MM+'-'+DD+',';
			



			var hh = leadZeros(dateTime[3]);
			var mm = leadZeros(dateTime[4]);
			var ss = leadZeros(dateTime[5]);

			var time = hh+':'+mm+':'+ss+',';
			
			csv += date+time+ins+','+outs+'\n';
		}
		else
		{
			var row = data['rows'][i]['c'];
			
			var dateTime = row[1]['v'];
			var ins = row[2]['v'];
			var outs = row[3]['v'];
			
			var MM = leadZeros((parseInt(args[2])).toString());
			var DD = leadZeros((parseInt(args[3])).toString());
			var date = args[1]+'-'+MM+'-'+DD+',';



			var hh = leadZeros(dateTime[0].toString());
			var mm = leadZeros(dateTime[1].toString());
			var ss = leadZeros(dateTime[2].toString());

			var time = hh+':'+mm+':'+ss+',';
			
			csv += date+time+ins+','+outs+'\n';
		}
	};

	/* file name */
	var fname = 'reporte.csv';
	switch (args[0]){
		case ('year'):
			fname = 'reporte-'+args[1]+'.csv';
			break;
		case ('month'):
			fname = 'reporte-'+args[1]+'-'+args[2]+'.csv';
			break;
		case ('day'):
			fname = 'reporte-'+args[1]+'-'+args[2]+'-'+args[3]+'.csv';
			break;
	};
	/**/

	var hiddenElement = document.createElement('a');
	hiddenElement.href = 'data:text/csv;charset=utf-8,'+encodeURI(csv);
	hiddenElement.target = '_blank';
	hiddenElement.download = fname;
	document.getElementById('csv_download').appendChild(hiddenElement);
	hiddenElement.click();
};
////////