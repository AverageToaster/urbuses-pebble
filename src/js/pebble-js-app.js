function getInfo(route, stop){
	var req = new XMLHttpRequest();
	req.open('GET','https://transloc-api-1-2.p.mashape.com/arrival-estimates.json?agencies=283&routes='+route+'&stops='+stop);
	req.setRequestHeader("X-Mashape-Authorization","b9ARY1Ni8l7atULy37SUlIdLXuKa4QH7");
	req.onload = function(e) {
		if (req.readyState == 4 && req.status == 200) {
		  if(req.status == 200) {
		    var dict = {};
		  	var data = JSON.parse(req.responseText);
		  	if (data.data[0] === undefined || data.data[0].arrivals[0] === undefined){
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
Pebble.addEventListener("ready",
	function(e){
		console.log("sup");
	});
Pebble.addEventListener("appmessage",
	function(e){
		console.log(JSON.stringify(e.payload));
		var route = e.payload["PRESET_ROUTE_ID"];
		var stop = e.payload["PRESET_STOP_ID"];
		getInfo(route, stop);
	});
Pebble.addEventListener("showConfiguration", 
	function(e){
		// console.log("here");
		Pebble.openURL("http://tjstein.me/dev/urbuses_settings.html");
	});
Pebble.addEventListener("webviewclosed",
	function(e){
		var options = JSON.parse(decodeURIComponent(e.response));
		for (var i = 1; i <= 5; i++){
			console.log("here");
			if (options[i+""] !== undefined){
				var dict = {};
				dict["PRESET_NUMBER"] = i;
				dict["PRESET_ROUTE_ID"] = parseInt(options[i].route_id);
				dict["PRESET_ROUTE_NAME"] = options[i].route_name;
				dict["PRESET_STOP_ID"] = parseInt(options[i].stop_id);
				dict["PRESET_STOP_NAME"] = options[i].stop_name;
				Pebble.sendAppMessage(dict,
					function(e) {
        				console.log("Sending settings data...");
				    },
      				function(e) {
        				console.log("Settings feedback failed!");
      				});
			}
		}
	});