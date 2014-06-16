function getInfo(route, stop){
	if (route == 0 || stop == 0){
		Pebble.sendAppMessage({"CURRENT_VIEW_TIME":-1});
		return;
	}
	var req = new XMLHttpRequest();
	req.open('GET','https://transloc-api-1-2.p.mashape.com/arrival-estimates.json?agencies=283&routes='+route+'&stops='+stop);
	req.setRequestHeader("X-Mashape-Authorization","b9ARY1Ni8l7atULy37SUlIdLXuKa4QH7");
	req.onload = function(e) {
		if (req.readyState == 4 && req.status == 200) {
		  if(req.status == 200) {
		    var dict = {};
		  	var data = JSON.parse(req.responseText);
		  	if (data.data[0] === undefined || data.data[0].arrivals[0] === undefined){
		  		dict["CURRENT_VIEW_TIME"] = -1;
		  	}
			else{ 	
			  	var arrival = new Date(data.data[0].arrivals[0].arrival_at);
			  	var now = new Date();
			  	var minutes = Math.floor(((arrival-now)/1000)/60);
			    dict["CURRENT_VIEW_TIME"] = minutes;
		    }
		    Pebble.sendAppMessage(dict);
		  } else { console.log("Error"); }
		}
	};
	req.send();
}
Pebble.addEventListener("ready",
	function(e){
		console.log("ready");
	});
Pebble.addEventListener("appmessage",
	function(e){
		var route = e.payload["PRESET_ROUTE_ID"];
		var stop = e.payload["PRESET_STOP_ID"];
		getInfo(route, stop);
	});
Pebble.addEventListener("showConfiguration", 
	function(e){
		var config_url = "http://tjstein.me/dev/urbuses_settings.html";
		if (window.localStorage.getItem("presets") !== null){
			var options = encodeURIComponent(window.localStorage.getItem("presets"));
			config_url = config_url +"?" + options;
		}
		Pebble.openURL(config_url);
	});
Pebble.addEventListener("webviewclosed",
	function(e){
		if (decodeURIComponent(e.response) === ""){
			return;
		}
		var options = JSON.parse(decodeURIComponent(e.response));
		window.localStorage.setItem("presets", JSON.stringify(options));
		var dict = {};
		for (var i = 1; i <= 5; i++){
			if (options[i+""] !== undefined){
				var dict2 = {};
				dict2["PRESET_NUMBER"] = i;
				dict2["PRESET_ROUTE_ID"] = parseInt(options[i].route_id);
				dict2["PRESET_ROUTE_NAME"] = options[i].route_name;
				dict2["PRESET_STOP_ID"] = parseInt(options[i].stop_id);
				dict2["PRESET_STOP_NAME"] = options[i].stop_name;
				dict[i] = dict2;
			}
		}
		sendStuff(dict, 1);
	});

function sendStuff(dict, i){
	var j = i;
	if (i > 5 || i < 0){
		return;
	}
	else{
		if (dict[i] !== undefined){
			Pebble.sendAppMessage(dict[i],
				function(e){
					j = j+1;
					sendStuff(dict, j);
				},
				function(e){
					conosle.log("Settings feedback failed!");
				});
		}
		else{
			j=j+1;
			sendStuff(dict, j);
		}
	}
}