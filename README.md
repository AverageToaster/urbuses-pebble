# urbuses (Pebble)

This Pebble Smartwatch application provides up to date time estimates (ETAs) for up to 5 user-customizable stops on any route on the University of Rochester bus system. The app also serves as a template for any organization that utilizes the [TransLoc API](http://transloc.com/), available [here](https://www.mashape.com/transloc/openapi-1-2#!documentation). To see what agencies TransLoc supports, look [here](http://transloc.com/agencies/).

# Whats new?
* v2.3
  * Fixed bug where multi-line routes would overflow into eta text.
  * Added Shake to refresh support in main menu and single preset screen.
* v2.2
  * Fixed bug where ETA would not display on single preset window.
* v2.1
  * Minor changes to text in windows
* v2.0
  * Updated app name from 'urbuses' to 'UR Buses'
  * Complete code redesign
  * UI update
    * Now has a Menu displaying all presets with ETA in one window.
    * Can go to old style single Preset ETA by selecting.
    * Can cycle through single Preset ETAs still.
    * Now has a footer menu below presets.
      * Contains About window, Clear Presets window, and No Presets Set window.
  * Clear All Presets option inside the app.
  * Altered settings page to not allow duplicate Presets.
  * Shows Error screens on certain catchable errors, like Broken persistent storage.
  * Still functions when persistent storage is broken. Loads presets from phone instead of storage.
* v1.1
  * No longer displays "Preset #" when scrolling to unset preset. Now simply finds the next set preset.
  * Shows warning page when there are no presets set, prompting the user to go to the Settings page.
  * 'Clear All Presets' button added to Settings Page, with confirmation. (Don't want people accidentally clearing their presets.
  * Shows error page when app detects broken persistent storage.
* v1.0
  * inital release

# Are you planning any new features?
- [ ] Show direction of line (ie towards Rush Rhees Library) 
- [ ] Option to hold SELECT on a preset's window to delete preset on watch.
- [ ] Additional optional preset that displays the closest stop and next arriving line.
- [ ] Additional optional preset that displays the closest stop that connects with home stop.
- [ ] Separate menu that displays all stops and the next X number of arriving lines.

# Can I use this project?
Yep. To use this project with your own TransLoc agency, there is only a few things you need to change and understand about the Pebble application.
## configuration/urbuses_settings.html/js
To allow for user customization, Pebble applications use an HTML page for configuration. This page must be a publicly visible page, so you'll have to host these files somewhere.

To use another agency and supply the user with correct bus routes and stops, you will have to change the agency in the line
```javascript
var agency = "283";
```
to whatever agency you are building the application for. (If you don't know the id of the agency, you'll have to look through the agencies resource available on their [API page](https://www.mashape.com/transloc/openapi-1-2) to find it.)

Additionally, the line
```javascript
var identifier = "";
```
will have to be set to your unique Mashape authorization key, which you will need to create in order to use the API.

Also, you'll most likely want to change the portions of the HTML page that refere to the University of Rochester, since that won't make much sense in another agency's application.
## src/js/pebble-js-app.js
This is the code that runs in the Pebble phone application. It recieves messages from the Pebble watch application, and gets the response from the configuration page regarding the presets.

Similarly to the configuartion page, you'll need to change the lines
```javascript
var identifier = "";
var agency = "283";
```
to your respective agency id and Mashape key.

Additionally, you will need to change the line 
```javascript
var config_url = "http://tjstein.me/urbuses/urbuses_settings.html";
```
in the showConfiguration eventListener to the URL of your own configuration page.
## appinfo.json
Since you will have to make your own Pebble app, this file will be generated for you. 

The uuid, shortName, longName, companyName, versionCode, and versionLabel will all be unique to your own project.

Everything else you will need to add to your appinfo.json file:
```json
{
  "capabilities": [ "configurable" ],
  "watchapp": {
    "watchface": false
  },
  "appKeys": {
    "CURRENT_VIEW_TIME": 0,
    "PRESET_NUMBER": 1,
    "PRESET_ROUTE_ID": 2,
    "PRESET_ROUTE_NAME": 3,
    "PRESET_STOP_ID": 4,
    "PRESET_STOP_NAME": 5
  },
  "resources": {
    "media": [
      {
        "menuIcon": true,
        "type": "png",
        "name": "MENU_ICON",
        "file": "images/menu_icon.png"
      }
    ]
  }
}
```
## What else?
That's it! Just make sure to adhere to the license when using this project. 
