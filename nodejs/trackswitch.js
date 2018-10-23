//trackswitch.js

var five = require("johnny-five");
var board = new five.Board();


//Arduino board connection

board.on("ready", function() {	   

    // Track Switch Components
    var trackSwitchLedA = new five.Led(26); // Green for straight, Red for turn
    var trackSwitchLedB = new five.Led(27); // Red for straight, Green for turn
    var trackSwitchServo = new five.Servo(6); // Track Switch
    var trackSwitchButton = new five.Button("A0"); 

    // Add devices to REPL (optional)
    this.repl.inject({
        trackSwitchButton: trackSwitchButton,
        trackSwitch: trackSwitch,
        trackSwitchServo: trackSwitchServo
     });

    // Button
    trackSwitchButton.on("down", function(){
    	console.log("trackSwitchButton pressed");
    	trackSwitch();
    });	 

    // Track Switch with Lights
    var trackSwitchState = "straight";
   	function trackSwitch(){
	    if (trackSwitchState != "straight"){
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
});
