// function getInfo(){
// 	var req = new XMLHttpRequest();
// 	req.open('GET','https://transloc-api-1-2.p.mashape.com/stops.json?agencies=283');
// 	req.setRequestHeader("X-Mashape-Authorization","b9ARY1Ni8l7atULy37SUlIdLXuKa4QH7");
// 	req.onload = function(e) {
// 		if (req.readyState == 4 && req.status == 200) {
// 		  if(req.status == 200) {
// 		  	var data = JSON.parse(req.responseText);
// 		    var stops = [];
// 		    for (var i = 0; i < data.data.length; i++){
// 		    	stops.push(data.data[i].name);

// 		    }
// 		    var dict = {};
// 		    for (var i = 0; i < stops.length; i++){
// 		    	// console.log("dict["+i+"] = " + stops[i]);
// 		    	dict[i] = stops[i];
// 		    }
// 		    Pebble.sendAppMessage(dict);
// 		  } else { console.log("Error"); }
// 		}
// 	};
// 	req.send();
// }
// function getStops(){
// 	var dict = {"0":"Stop 1", "1":"Stop 2", "2":"Stop 3"};
// 	Pebble.sendAppMessage(dict);
// }
function getInfo(){
	var dict = {};
	dict[0] = 5;
	Pebble.sendAppMessage(dict);
}
function getInitInfo(){
	var dict = {};
	dict[0] = 1;
	Pebble.sendAppMessage(dict);
}
Pebble.addEventListener("ready",
	function(e){
		getInitInfo();
	});
Pebble.addEventListener("appmessage",
	function(e){
		getInfo();
	})
// // Pebble.addEventListener("showConfiguration", 
// // 	function(e){
// // 		Pebble.openURL("https://dl.dropboxusercontent.com/u/10824180/pebble%20config%20pages/sdktut9-config.html");
// // 	});