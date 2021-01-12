$(document).ready(start);

function start() {
	showDate();
	counterUpdateClickedResp();
	var counterUpdate = document.getElementById('liveCountBtn');
	counterUpdate.addEventListener('click',counterUpdateClickedResp,false);
	
	function showDate(){
		var actual_date = $.ajax({
			url: "php/getDate.php",
			async: false
		}).responseText;
		//alert(actual_date);
		$('#date').text(actual_date);
	}

	function counterUpdateClickedResp() {
		var jsonData = $.ajax({
			url: "php/loadLiveCounterData.php",
			dataType: "json",
			async: false
		}).responseText;
		jsonData = JSON.parse(jsonData)
		document.getElementById("in").textContent=jsonData['entradas'];
		document.getElementById("out").textContent=jsonData['salidas'];
	};

	$('#update').click(function(){
		var currentdate = new Date(); 
		var datetime = currentdate.getFullYear() + "-"
		                + (currentdate.getMonth()+1)  + "-" 
		                + currentdate.getDate() + " "  
		                + currentdate.getHours() + ":"  
		                + currentdate.getMinutes() + ":" 
		                + currentdate.getSeconds();
		var response = $.ajax({
			url: "php/update_date.php",
			data: { date:datetime },
			async: false
		}).responseText;
		//alert(response);
	});
}
