/*
 * Various global variables for saving routes/stops when recieved from API
 * and the saved routes/stops for each preset so switching between presets
 * remembers each combination.
 */
var presets = [[0,0],[0,0],[0,0],[0,0],[0,0]];
var presets_ids = [[0,0],[0,0],[0,0],[0,0],[0,0]];
var routes = [];
var stops = [];
var routes_stops = [];


window.onload = function(){
	//Set up the onchange and onclick functions.
	document.getElementById("save_num").onchange = save_num_change;
	document.getElementById("route").onchange = route_change;
	document.getElementById("stop").onchange = stop_change;
	document.getElementById("submit").onclick = submit;
	document.getElementById("cancel").onclick = cancel;
	//Start the request to load the routes and stops for University of Rochester.
	var req = new XMLHttpRequest();
	var text = 'https://transloc-api-1-2.p.mashape.com/routes.json?agencies=283';
	req.open('GET',text);
	/*
	* NOTE: THE NUMBER IN THIS REQUEST HANDLER IS MY SPECIFIC MASHAPE IDENTIFIER
	* If you want to use this code in your own Transloc bus app, you should probably use your own identifier.
	*/
	req.setRequestHeader("X-Mashape-Authorization","b9ARY1Ni8l7atULy37SUlIdLXuKa4QH7")
	req.onload = function(e) {
		if (req.readyState == 4 && req.status == 200) {
		  	if(req.status == 200) {
			  	var data = JSON.parse(req.responseText).data[283]; //283 is U of R's agency id. Change for use with another agency.
			  	for (var i = 0; i < data.length; i++){
			  		//Load routes that have 1 or more stops into the Route drop down.
			  		if (data[i].stops.length > 0){
						var opt = document.createElement("option");
						opt.value = data[i].route_id;
						opt.innerHTML = data[i].long_name;
						document.getElementById("route").appendChild(opt);	
						routes[data[i].route_id] = data[i].long_name;
			  		}
			  	}
			  	var req2 = new XMLHttpRequest();
			  	text = 'https://transloc-api-1-2.p.mashape.com/stops.json?agencies=283';
				req2.open('GET',text);
				req2.setRequestHeader("X-Mashape-Authorization","b9ARY1Ni8l7atULy37SUlIdLXuKa4QH7");
				req2.onload = function(e){
					if (req2.readyState == 4 && req2.status == 200){
						var data = JSON.parse(req2.responseText).data;
						get_stops(data);
					}
				}
				req2.send();
			}
		}
	};
	req.send();	
};

/*
 * Function to load the presets sent from the Pebble app into the global variables/ 
 */
function load_presets(){
	if (window.location.search !== ""){
		var uri = window.location.search.substring(1);		
		var options = JSON.parse(decodeURIComponent(uri));
		var route_keys = Object.keys(routes);
		for (var i = 1; i <= 5; i++){
			if (options[i+""] !== undefined){
				for (var j = 0; j < route_keys.length; j++){
					if (options[i].route_id == route_keys[j]){
						presets[i-1][0] = j+1;
						presets_ids[i-1][0] = options[i].route_id;
						for (var k = 0; k < routes_stops[route_keys[j]].length; k++){
							if (options[i].stop_id == routes_stops[route_keys[j]][k]){
								presets[i-1][1] = k+1;
								presets_ids[i-1][1] = options[i].stop_id;
								break;
							}
						}
						break;
					}
				}
			}
		}
	}
	document.getElementById("save_num").disabled = false;
	document.getElementById("instruction").innerHTML = "Select a Preset:";
}
/*
 * Function to pull the stop information out of the recieved JSON.
 * Adds the stops into the various global variables for quick access.
 */
function get_stops(data){
	for (var i = 0; i < data.length; i++){
		for (var j = 0; j < data[i].routes.length; j++){
			stops[data[i].stop_id] = data[i].name;
			var opt = document.createElement("option");
			opt.value = data[i].stop_id;
			opt.innerHTML = data[i].name;
			document.getElementById("stop").appendChild(opt);
			if (routes_stops[data[i].routes[j]] == undefined){
				routes_stops[data[i].routes[j]] = [data[i].stop_id];
			}
			else{
				routes_stops[data[i].routes[j]].push(data[i].stop_id);
			}
		}
	}
	load_presets();
}

/*
 * Function for handling when the preset number is changed.
 * If the empty preset is selected, disable everything.
 * Otherwise, load the saved information for the given preset, and enable what fields
 * need to be enabled for what information is available. 
 * This is so the user can switch between presets and it will still remember what they selected for saved presets.
 */
function save_num_change(){
	if (document.getElementById("save_num").value != "NONE"){
		document.getElementById("stop").disabled = true;
		document.getElementById("submit").disabled = true;
		document.getElementById("instruction").innerHTML = "Select a Route:";
		document.getElementById("route").disabled = false;
		document.getElementById("route").selectedIndex = presets[parseInt(document.getElementById("save_num").value)-1][0];
		if (document.getElementById("route").selectedIndex != 0){
			document.getElementById("stop").innerHTML="";
			var opt = document.createElement("option");
			opt.value = "NONE";
			document.getElementById("stop").innerHTML = "";
			document.getElementById("stop").appendChild(opt);
			for (var i = 0; i < routes_stops[document.getElementById("route").value].length; i++){
				opt = document.createElement("option");
  				opt.value = routes_stops[document.getElementById("route").value][i];
  				opt.innerHTML = stops[opt.value];
  				document.getElementById("stop").appendChild(opt);
			}
			document.getElementById("stop").selectedIndex = presets[parseInt(document.getElementById("save_num").value)-1][1];
			document.getElementById("stop").disabled = false;
			document.getElementById("instruction").innerHTML = "Select a Stop:";
			if (document.getElementById("stop").selectedIndex != 0){
				document.getElementById("instruction").innerHTML = "Hit Submit to Save Presets.";
				document.getElementById("submit").disabled = false;
			}
		}
		else{
			document.getElementById("stop").selectedIndex = 0;
		}
	}
	else{
		document.getElementById("route").selectedIndex = 0;
		document.getElementById("stop").selectedIndex = 0;
		document.getElementById("route").disabled = true;
		document.getElementById("stop").disabled = true;
		document.getElementById("submit").disabled = true;
		document.getElementById("instruction").innerHTML = "Select a Preset:";
	}
}

/*
 * Function to handle when the route changes.
 * If it is nothing, reflect that by setting it and the stop to nothing, and disabling what needs to be disabled.
 * If it is someting, load in the stops for the selected route from the global variable, and enable that field.
 */
function route_change(){
	presets[parseInt(document.getElementById("save_num").value)-1][0] = document.getElementById("route").selectedIndex;
	presets_ids[parseInt(document.getElementById("save_num").value)-1][0] = document.getElementById("route").value;
	document.getElementById("submit").disabled = true;
	if (document.getElementById("route").value != "NONE"){
		var opt = document.createElement("option");
		opt.value = "NONE";
		document.getElementById("stop").innerHTML = "";
		document.getElementById("stop").appendChild(opt);
		for (var i = 0; i < routes_stops[document.getElementById("route").value].length; i++){
			opt = document.createElement("option");
			opt.value = routes_stops[document.getElementById("route").value][i];
			opt.innerHTML = stops[opt.value];
			document.getElementById("stop").appendChild(opt);
		}
		document.getElementById("stop").disabled = false;
	  	document.getElementById("instruction").innerHTML = "Select a Stop:";
	}
	else{
		document.getElementById("stop").disabled = true;
		var opt = document.createElement("option");
		opt.value = "NONE";
		document.getElementById("stop").innerHTML = "";
		document.getElementById("stop").appendChild(opt);
		document.getElementById("instruction").innerHTML = "Select a Route:";
		
	}
}
/*
 * Function for when the stop value changes.
 * If it is anything other than blank, allow the user to submit. Otherwise, don't.
 */
function stop_change(){
	presets[parseInt(document.getElementById("save_num").value)-1][1] = document.getElementById("stop").selectedIndex;
	presets_ids[parseInt(document.getElementById("save_num").value)-1][1] = document.getElementById("stop").value;
	if (document.getElementById("stop").value != "NONE"){
		document.getElementById("submit").disabled = false;
		document.getElementById("instruction").innerHTML = "Hit Submit to Save Presets.";
		
	}
	else{
		document.getElementById("submit").disabled = true;
		document.getElementById("instruction").innerHTML = "Select a Stop:";
		
	}
}
/*
 * Function to turn the arrays of presets into JSON format for sending to the Pebble app.
 */
function save_options(){
	var options = {};
	var sub_opt = {};
	var str = "";
	for (var i = 0; i < presets_ids.length; i++){
		sub_opt = {};
		if (presets[i][0] !== 0 && presets[i][1] !== 0){
			sub_opt["route_id"] = presets_ids[i][0];
			sub_opt["route_name"] = routes[presets_ids[i][0]];
			sub_opt["stop_id"] = presets_ids[i][1];
			sub_opt["stop_name"] = stops[presets_ids[i][1]].replace("\/", " \/ ");
		}
		else{
			sub_opt["route_id"] = 0;
			sub_opt["route_name"] = 0;
			sub_opt["stop_id"] = 0;
			sub_opt["stop_name"] = 0;
		}
		options[i+1] =  sub_opt;
	}
	return options;
}
/*
* Function for the submit button. Get the selected presets, and send them back to the phone.
*/
function submit(){
	var location = "pebblejs://close#" + encodeURIComponent(JSON.stringify(save_options()));
    document.location = location;
}
/*
* Function for the cancel button. Send back nothing.
*/
function cancel(){
	document.location = "pebblejs://close#";
}