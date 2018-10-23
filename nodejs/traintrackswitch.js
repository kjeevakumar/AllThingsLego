// traintrackswitch.js

/****************************
// Lego Train Track Switch
// This app uses the Johnny-Five.io framework to control a servo and lights
// The servo will switch the track to the desired location and update the direction lights
// Commands can be sent from the "iol" PubNub channel
// {"command":"track_straight"}
// {"command":"track_turn"}
// {"command":"track_toggle"}

// Written by Cory Guynn
// www.InternetOfLego.com
// 2016
****************************/



/****************************
// Server Code
****************************/
// The config file will be used to store API details. Its separate for security reasons.
var config = require('./config');

var pubnub = require("pubnub")({
    ssl           : true,  // <- enable TLS Tunneling over TCP
    publish_key   : config.pubnub.publish_key, // These will be YOUR PubNub API keys
    subscribe_key : config.pubnub.subscribe_key
});


// Publish Messages

var message = { "news" : "Train Track Switch online!" };
pubnub.publish({
    channel   : 'iol',
    message   : message,
    callback  : function(e) { console.log( "Server: PubNub SUCCESS!", e ); },
    error     : function(e) { console.log( "Server: PubNub FAILED! RETRY PUBLISH!", e ); }
});


/****************************
// Circuit Code
****************************/

// Arduino Johnny-Five
var five = require("johnny-five");
var board = new five.Board();

board.on("ready", function() {

    // Track Switch Components
    var trackSwitchLedA = new five.Led(26); // Green for straight, Red for turn
    var trackSwitchLedB = new five.Led(27); // Red for straight, Green for turn
    var trackSwitchServo = new five.Servo(6); // Track Switch
    var trackSwitchButton = new five.Button(2); // Toggle Button

    // Add devices to REPL (optional)
    this.repl.inject({
        trackSwitch: trackSwitch,  // example: trackSwitch(Straight)
        trackSwitchServo: trackSwitchServo // example: trackSwitchServo.to(90);
     });

    // Button
    trackSwitchButton.on("down", function(){
    	console.log("trackSwitchButton pressed");
    	trackSwitch();
    });

    // Track Switch with Lights
      // Accepts a direction, or will toggle if no parameters sent
    var trackSwitchState = "straight";
    function trackSwitch(trackSwitchState){
        if (trackSwitchState == "straight"){
          console.log("track switched: Straight");
          trackSwitchLedA.on();
          trackSwitchLedB.off();
          trackSwitchServo.to(90);
          trackSwitchState = "straight";
        } else {
          console.log("track switched: Turn");
          trackSwitchLedA.off();
          trackSwitchLedB.on();
          trackSwitchServo.to(130);
          trackSwitchState = "turn";
        }
    }

  /* ---------------------------------------------------------------------------
  Listen for PubNub Messages
  --------------------------------------------------------------------------- */

  pubnub.subscribe({
      channel  : "iol",
      callback : function(msg) {
          console.log( "Server: PubNub Received message and sending to circuit: ", msg );
          if (msg.command === "track_straight"){
   	        trackSwitch("straight");
   	      }
   	      if (msg.command === "track_turn"){
   	        trackSwitch("turn");
   	      }
          if (msg.command === "track_toggle"){
            trackSwitch();
          }
      }
  });

}); //end board.on()