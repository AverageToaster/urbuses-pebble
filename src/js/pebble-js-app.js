/*
* NOTE: THE VARIABLE BELOW NEEDS A MASHAPE IDENTIFIER TO WORK.
* If you want to use this code in your own Transloc bus app, you need your own identifier
* To get a key, make an account at https://www.mashape.com/ and find your environment keys there.
*/
var identifier = "";
var agency = "283"; // This is the agency id for University of Rochester. If using another agency, change this variable to that id.
var attempts = 0; // Variable to make sure we don't get stuck in a loop if we get nothing but Nacks back from Pebble.
/*
 * Function to get the arrival time estimate for the given route stop combination.
 */
function getTimeEstimate(stop, route) {
	// If either the route or the stop are empty, then simply return -1 (no arrival estimate.)
	if (route == "" || stop == "") {
		Pebble.sendAppMessage({
			"CURRENT_VIEW_TIME": -1
		});
		return;
	}
	// Get in touch with the transloc service for the info.
	var req = new XMLHttpRequest();
	req.open('GET', 'https://transloc-api-1-2.p.mashape.com/arrival-estimates.json?agencies=' + agency + '&routes=' + route + '&stops=' + stop);
	req.setRequestHeader("X-Mashape-Authorization", identifier);
	req.onload = function(e) {
		if (req.readyState == 4 && req.status == 200) {
			if (req.status == 200) {
				// When ready, create a dictionary to return.
				var dict = {};
				dict.group="PRESET";
				dict.operation="PRESET_ETA";
				dict.data = stop+"|"+route+"|";
				var data = JSON.parse(req.responseText);
				// If there's no arrival data, then reply with -1.
				if (data.data[0] === undefined || data.data[0].arrivals[0] === undefined) {
					dict.data += "NO ETA";
				} else {
					// Otherwise, calculate the minutes difference from now to the arrival estimate, and reply with that.
					var arrival = new Date(data.data[0].arrivals[0].arrival_at);
					var now = new Date();
					var minutes = Math.floor(((arrival - now) / 1000) / 60);
					dict.data += minutes;
				}
				// Send the response.
				attempts = 0;
				console.log('Sending preset eta: ' + JSON.stringify(dict));
				sendETA(dict);
			}
		}
	};
	req.send();
}

function getAllTimeEstimates(){
	var presets = JSON.parse(window.localStorage.getItem("presets"));
	var req = new XMLHttpRequest();
	var url = 'https://transloc-api-1-2.p.mashape.com/arrival-estimates.json?agencies=' + agency + '&routes=';
	var routes = [];
	var stops = [];
	for (var i = 1; i <= 5; i++){
		if (presets[i+""].route_id != "0"){
			routes.push(presets[i+""].route_id);
			stops.push(presets[i+""].stop_id);
		}
	}
	var url_routes = routes.join("%2C");
	var url_stops = stops.join("%2C");
	url += url_routes + "&stops=" + url_stops;
	req.open('GET', url);
	req.setRequestHeader("X-Mashape-Key", identifier);
	req.onload = function(e){
		if (req.readyState == 4 && req.status == 200){
			var data=JSON.parse(req.responseText).data;
			var dict = [];
			var dict2;
			for (var i = 0; i < routes.length; i++){
				dict2 = {};
				dict2.group = "PRESET";
				dict2.operation = "PRESET_ETA";
				dict2.data = "";
				for (var j = 0; j < data.length; j++){
					if (data[j].stop_id === stops[i]){
						for (var k = 0; k < data[j].arrivals.length; k++){
							if (data[j].arrivals[k].route_id === routes[i]){
								var arrival = new Date(data[j].arrivals[k].arrival_at);
								var now = new Date();
								var minutes = Math.floor(((arrival-now)/1000)/60);
								dict2.data = stops[i] + "|" + routes[i] + "|" + minutes;
								break;
							}
						}
						break;
					}
				}
				if (dict2.data == ""){
					dict2.data = stops[i] + "|" + routes[i] + "|NO ETA";
				}
				dict.push(dict2);
			}
			dict.push(dict2);
			console.log('Sending all preset etas');
			attempts = 0;
			sendAllETAs(dict, 0);
		}
	}
	req.send();
}

function sendAllETAs(dict, i){
	var j = i;
	// Base case. Stop when this is reached.
	if (i > dict.length || i < 0)
		return;
	else {
 		if (dict[i] !== undefined) {
			Pebble.sendAppMessage(dict[i],
				function(e) {
					attempts = 0;
					j = j + 1;
					sendAllETAs(dict, j);
				},
				function(e) {
					if (attempts < 5){
						attempts++;
						sendAllETAs(dict, j);
					}
					else{
					}
				});
		}
		else {
			j = j + 1;
			sendAllETAs(dict, j);
		}
	}
}

function sendETA(dict){
	Pebble.sendAppMessage(dict,
		function(e){
		},
		function(e){
			if (attempts < 5){
				attempts++;
				sendETA(dict);
			}
			else{
			}
		})
}

/*
 * Listener and function called when the app first connects the phone.
 */
Pebble.addEventListener("ready",
	function(e) {
		// App is connected to the phone.
		console.log('Ready');
	});
/*
 * Listener and function are called when the phone receives a message from the app.
 * Messages are always a Tuple in the form of (route, stop).
 * All messages from the app to the phone are requesting time estimate, so parse the tuple,
 * and send it to getTimeEstimate().
 */
Pebble.addEventListener("appmessage",
	function(e) {
		if (e.payload.group === "PRESET"){
			if (e.payload.operation === "PRESET_ETA"){
				if (e.payload.data === " "){
					console.log('Received appmessage for all');
					getAllTimeEstimates();
				}
				else{
					console.log('Received appmessage for one');
					var split = e.payload.data.split("|");
					getTimeEstimate(split[0], split[1]);
				}
			}
		}
	});

/*
 * Listener and function are called when the "Settings" button is selected in the Pebble app on the phone.
 * This opens an HTML page on my website which is designed for the URBuses config page.
 * If previous presets have been stored, the function sends these to the config page as well, so that
 * the user won't override the previous presets they've set.
 */
Pebble.addEventListener("showConfiguration",
	function(e) {
		var config_url = "http://tjstein.me/urbuses/urbuses_settings.html"; // Change this URL to the URL of your own configuration page.
		if (window.localStorage.getItem("presets") !== null) {
			var options = encodeURIComponent(window.localStorage.getItem("presets"));
			config_url = config_url + "?" + options;
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
	function(e) {
		// There is no response, so just end.
		if (decodeURIComponent(e.response) === "") {
			return;
		}
		// Otherwise, parse out the response from the app.
		var options = JSON.parse(decodeURIComponent(e.response));
		window.localStorage.setItem("presets", JSON.stringify(options));
		var dict = [];
		var count = 1;
		var dict2 = {};
		dict2.group = "PRESET";
		dict2.operation = "PRESET_CLEAR";
		dict2.data = "";
		dict.push(dict2);
		for (var i = 1; i <= 5; i++) {
			if (options[i + ""] !== undefined) {
				dict2 = {};
				if (options[i].route_id != 0) {
					dict2.group = "PRESET";
					dict2.operation = "PRESET_SET";
					dict2.data = options[i].stop_id
					+ "|" + options[i].stop_name
					+ "|" + options[i].route_id
					+ "|" + options[i].route_name;
					dict.push(dict2);
					count++;
				}
			}
		}
		dict2 = {};
		dict2.group="PRESET";
		dict2.operation = "PRESET_SET";
		dict2.data = "END";
		dict.push(dict2);
		attempts = 0;
		sendStuff(dict, 0);
	});


/*
 * As the app can't handle more than one response at a time, this function uses the callback function of the
 * sendAppMessage function to recursively send each preset to the app.
 */
function sendStuff(dict, i) {
	var j = i;
	// Base case. Stop when this is reached.
	if (i >= dict.length || i < 0) {
		return;
	} else {
 		if (dict[i] !== undefined) {
			Pebble.sendAppMessage(dict[i],
				function(e) {
					attempts = 0;
					j = j + 1;
					sendStuff(dict, j);
				},
				function(e) {
					if (attempts < 5){
						attempts++;
						sendStuff(dict, j);
					}
					else{
					}
				});
		}
		// Even if dict[i] doesn't exist, they may have set other presets.
		else {
			j = j + 1;
			sendStuff(dict, j);
		}
	}
}
