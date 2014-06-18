/*
* Function to get the arrival time estimate for the given route stop combination.
*/
function getTimeEstimate(route, stop){
	// If either the route or the stop are empty, then simply return -1 (no arrival estimate.)
	if (route == 0 || stop == 0){
		Pebble.sendAppMessage({"CURRENT_VIEW_TIME":-1});
		return;
	}
	// Get in touch with the transloc service for the info.
	var req = new XMLHttpRequest();
	req.open('GET','https://transloc-api-1-2.p.mashape.com/arrival-estimates.json?agencies=283&routes='+route+'&stops='+stop);
	req.setRequestHeader("X-Mashape-Authorization","b9ARY1Ni8l7atULy37SUlIdLXuKa4QH7");
	req.onload = function(e) {
		if (req.readyState == 4 && req.status == 200) {
		  if(req.status == 200) {
		  	// When ready, create a dictionary to return.
		    var dict = {};
		  	var data = JSON.parse(req.responseText);
		  	// If there's no arrival data, then reply with -1.
		  	if (data.data[0] === undefined || data.data[0].arrivals[0] === undefined){
		  		dict["CURRENT_VIEW_TIME"] = -1;
		  	}
			else{ 	
				// Otherwise, calculate the minutes difference from now to the arrival estimate, and reply with that.
			  	var arrival = new Date(data.data[0].arrivals[0].arrival_at);
			  	var now = new Date();
			  	var minutes = Math.floor(((arrival-now)/1000)/60);
			    dict["CURRENT_VIEW_TIME"] = minutes;
		    }
		    // Send the response.
		    Pebble.sendAppMessage(dict);
		  }
		}
	};
	req.send();
}
/*
* Listener and function called when the app first connects the phone.
*/
Pebble.addEventListener("ready",
	function(e){
		// App is connected to the phone.
		console.log("ready");
	});
/*
* Listener and function are called when the phone receives a message from the app. 
* Messages are always a Tuple in the form of (route, stop).
* All messages from the app to the phone are requestion time estimate, so parse the tuple,
* and send it to getTimeEstimate().
*/
Pebble.addEventListener("appmessage",
	function(e){
		var route = e.payload["PRESET_ROUTE_ID"];
		var stop = e.payload["PRESET_STOP_ID"];
		getTimeEstimate(route, stop);
	});

/*
* Listener and function are called when the "Settings" button is selected in the Pebble app on the phone.
* This opens an HTML page on my website which is designed for the URBuses config page.
* If previous presets have been stored, the function sends these to the config page as well, so that
* the user won't override the previous presets they've set.
*/
Pebble.addEventListener("showConfiguration", 
	function(e){
		var config_url = "http://tjstein.me/urbuses/urbuses_settings.html";
		if (window.localStorage.getItem("presets") !== null){
			var options = encodeURIComponent(window.localStorage.getItem("presets"));
			config_url = config_url +"?" + options;
		}
		Pebble.openURL(config_url);
	});

/*
* Listener and function are called when the Config page is closed.
* If the cancel button is selected, or they back out of the page without selecting anything,
* the function simply returns and does nothing.
* If they actually select presets, it is send along in JSON format.
* This information is parse out into a dictionary of dictionaries, and sent to a function to send to the app.
*/
Pebble.addEventListener("webviewclosed",
	function(e){
		// There is no response, so just end.
		if (decodeURIComponent(e.response) === ""){
			return;
		}
		// Otherwise, parse out the response from the app.
		var options = JSON.parse(decodeURIComponent(e.response));
		window.localStorage.setItem("presets", JSON.stringify(options));
		var dict = {};
		for (var i = 1; i <= 5; i++){
			if (options[i+""] !== undefined){
				var dict2 = {};
				dict2["PRESET_NUMBER"] = i;
				if (options[i].route_id != 0){
					dict2["PRESET_ROUTE_ID"] = parseInt(options[i].route_id);
					dict2["PRESET_ROUTE_NAME"] = options[i].route_name;
					dict2["PRESET_STOP_ID"] = parseInt(options[i].stop_id);
					dict2["PRESET_STOP_NAME"] = options[i].stop_name;
				}
				else{
					dict2["PRESET_ROUTE_ID"] = -1;
					dict2["PRESET_ROUTE_NAME"] = -1;
					dict2["PRESET_STOP_ID"] = -1;
					dict2["PRESET_STOP_NAME"] = -1;
				}
				dict[i] = dict2;
			}
		}
		sendStuff(dict, 1);
	});

/*
* As the app can't handle more than one response at a time, this function uses the callback function of the 
* sendAppMessage function to recursively send each preset to the app.
*/
function sendStuff(dict, i){
	var j = i;
	// Base case. Stop when this is reached.
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
					console.log("Settings feedback failed!");
				});
		}
		// Even if dict[i] doesn't exist, they may have set other presets.
		else{
			j=j+1;
			sendStuff(dict, j);
		}
	}
}