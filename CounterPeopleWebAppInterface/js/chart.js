google.charts.load('current', {packages:['corechart','controls']}).then(chart);

//$(document).ready(chart);
function chart() {
	var chart = new google.visualization.ChartWrapper({
		'containerId': 'chart_div'
	});

	
	var optionsDay = {
		'chartArea': {'height':'80%','width':'80%'},
		'animation':{
        /*'duration': 1000,*/
        'easing': 'in',
      },
		'hAxis': {
			'title': "Hora del día",
			'slantedText': false
		},
		'vAxis': {
			'title': 'Número de Ususarios',
			'viewWindow': {
				'min': 0 //,'max': 150
			}
		}/*,
		'legend': {
			'position': 'none'
		}*/
	};

	var options = {
		'chartArea': {'height':'80%','width':'80%'},
		'animation':{
        /*'duration': 1000,*/
        'easing': 'in',
      },
		'hAxis': {
			'title': "Fecha",
			'slantedText': false
		},
		'vAxis': {
			'title': 'Número de Ususarios',
			'viewWindow': {
				'min': 0 //,'max': 150
			}
		}/*,
		'legend': {
			'position': 'none'
		}*/
	};

	// Which chart to be displayed
	var drawChart = null;
	// Indicates if it's visible or not
	var visibilityChart = false;
	// Indicates which period to display
	var period = null;
	// Indicates which year to load
	var year = null;
	// Indicates which month to load
	var month = null;
	// indicates which day to load
	var day = null;
	
	if($('.main').is('#dashboard')) {
		period = 'year';
		selectChart('bars')
		alert(period);
		// Loads the available years
		var years = ut.existingFiles(new Array('years'));
		console.log(years);	
		if (years.length>1)
		{
			year = years[0];
			yearSelectedResp2(year);
		};
		ut.populateForm(years,0);

		var periodSelect = document.getElementById('period_select');
		periodSelect.addEventListener('change',periodSelectedResp,false);

		var chartSelect = document.getElementById('chart_select');
		chartSelect.addEventListener('change',chartSelectedResp,false);

		var yearSelect = document.getElementById('year_select');
		yearSelect.addEventListener('change',yearSelectedResp,false);

		var monthSelect = document.getElementById('month_select');
		monthSelect.addEventListener('change',monthSelectedResp,false);

		var daySelect = document.getElementById('day_select');
		daySelect.addEventListener('change',daySelectedResp,false);
	};

	$(window).resize(function(){
		if(visibilityChart) {
			drawChart();
		}
	});

	///////////////// Starts: Response Functions
	function periodSelectedResp() {
		if (this.value.length>0) {
			// Save selected year
			period = this.value;

			switch(period) {
				case 'year':
					if (year!=null) {
						drawChart();
					}
					break;
				case 'month':
					if (month!=null) {
						drawChart();
					}
					break;
				case 'day':
					if (day!=null) {
						drawChart();
					}
					break;
			};
		};
	};

	function chartSelectedResp() {
		selectChart(this.value);
		if (visibilityChart) {
			drawChart();
		};
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

			if(period=='year') {
				drawChart();
			};
		};
	};

	function yearSelectedResp2(year) {
		if(period=='year') {
			drawChart();
		};
	}

	function monthSelectedResp() {
		if (this.value.length>0) {
			// Save selected month
			month = this.value;

			// Errase all old content
			ut.cleanForm(2);

			// Populate form
			var months = ut.existingFiles(new Array('days',year,this.value));
			ut.populateForm(months,2);

			if(period=='month') {
				drawChart();
			};
		};
	};

	function daySelectedResp() {
		if (this.value.length>0) {
			day = this.value;

			if(period=='day') {
				drawChart();
			};
		};
	};
	///////////////// Ends: Response Functions

	/* FUNCTIONS */


	/*
	var chart = new google.visualization.ChartWrapper({
		'containerId': 'chart_div'
	});
	*/

	function selectChart(text) {
		// Select which chart is going to be used based on a text
		//  message
		var chart = null;
		switch(text) {
			case 'area':
				drawChart = drawArea;
				break;
			case 'lines':
				drawChart = drawLines;
				break;
			case 'bars':
				drawChart = drawBars;
				break;
		};
	};

	function drawArea() {
		chart.setChartType("AreaChart");
		data = ut.getJsonData([period,year,month,day]);
		data = new google.visualization.DataView(data);
		var chart_options = options;
		if(period=='day') {
    		data.setColumns([1,2,3]);
    		chart_options = optionsDay;
		};
		chart.setDataTable(data);
		chart.setOptions(chart_options);
		chart.draw();

		//console.log('Area');
		visibilityChart = true;
	};

	function drawLines() {
		chart.setChartType("LineChart");
		data = ut.getJsonData([period,year,month,day]);
		data = new google.visualization.DataView(data);
		var chart_options = options;
		if(period=='day') {
    		data.setColumns([1,2,3]);
    		chart_options = optionsDay;
		};
		chart.setDataTable(data);
		chart.setOptions(chart_options);
		chart.draw();

		//console.log('Area');
		visibilityChart = true;
	};

	function drawBars() {
		chart.setChartType("ColumnChart");
		data = ut.getJsonData([period,year,month,day]);
		data = new google.visualization.DataView(data);
		var chart_options = options;
		if(period=='day') {
    		data.setColumns([1,2,3]);
    		chart_options = optionsDay;
		};
		chart.setDataTable(data);
		chart.setOptions(chart_options);
		chart.draw();

		//console.log('Bars')
		visibilityChart = true;
	};
};

function drawBasic () {
	var chart = new google.visualization.ChartWrapper({
		'containerId': 'chart_div'
	});

	chart.setChartType("ColumnChart");
	var data = ut.getJsonData(['day','2018','01','01']);
	var data = new google.visualization.DataView(data);
    data.setColumns([1,2,3]);
  	
	chart.setDataTable(data);
	chart.draw();

	$(window).resize(function(){
	  chart.draw();

	});
};
