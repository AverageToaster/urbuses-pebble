function getInfo(){
	var req = new XMLHttpRequest();
	req.open('GET','https://transloc-api-1-2.p.mashape.com/arrival-estimates.json?agencies=283&routes=4004990&stops=4148446');
	req.setRequestHeader("X-Mashape-Authorization","b9ARY1Ni8l7atULy37SUlIdLXuKa4QH7");
	req.onload = function(e) {
		if (req.readyState == 4 && req.status == 200) {
		  if(req.status == 200) {
		    var dict = {};
		  	var data = JSON.parse(req.responseText);
		  	if (data.data[0].arrivals[0] == undefined){
		  		dict[0] = -1;
		  	}
			else{ 	
			  	var arrival = new Date(data.data[0].arrivals[0].arrival_at);
			  	var now = new Date();
			  	var minutes = Math.floor(((arrival-now)/1000)/60);
			    dict[0] = minutes;
		    }
		    Pebble.sendAppMessage(dict);
		  } else { console.log("Error"); }
		}
	};
	req.send();
}
// function getStops(){
// 	var dict = {"0":"Stop 1", "1":"Stop 2", "2":"Stop 3"};
// 	Pebble.sendAppMessage(dict);
// }
function getInitInfo(){
	var dict = {};
	dict[0] = 1;
	Pebble.sendAppMessage(dict);
}
Pebble.addEventListener("ready",
	function(e){
		getInfo();
	});
Pebble.addEventListener("appmessage",
	function(e){
		getInfo();
	})
// // Pebble.addEventListener("showConfiguration", 
// // 	function(e){
// // 		Pebble.openURL("https://dl.dropboxusercontent.com/u/10824180/pebble%20config%20pages/sdktut9-config.html");
// // 	});