//google.charts.load('current', {packages:['corechart','controls']});

$(document).ready(function() {

	callPage('views/dashboard.html');
	//callPage('views/reports.html');

	$("a[id^=nav]").on("click", function(e) {
		e.preventDefault();
		var pageRef = $(this).attr('href');
		callPage(pageRef)
		// The next function changes the URL to Hash-based URL
		// However, if you try to acces the URL it will only point
		// to the home page.
		// TODO: Fix URL behaviour
		//document.location.hash = pageRef.slice(5,-1)
	});

	function callPage( pageInfo ) {
		call(pageInfo,"#main-body");
	}


});

function call( info, place) {
	// "place" should be something like
	// "#chart"
	$.ajax({
		url: info,
		type: "GET",
		dataType: "text",
		success: function( response ) {
			$(place).html(response)
		},
		error: function( errorResponse ) {
			$.ajax({
			url: "views/error.html",
			type: "GET",
			dataType: "text",
			success: function( response ) {
				$(place).html(response);
				console.log("An error occured while loading the page",errorResponse);
			}
			});
		}
	});
}
