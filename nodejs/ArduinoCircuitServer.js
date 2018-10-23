// ArduinoCircuitServer.js

// This program creates the server for controlling Arduino or similar projects. 
// You can either use a web socket for real-time communication, or GET requests to send commands easily.
// Refer to the following "elevator.js" file for a working circuit file that accepts the commands.
// https://gist.github.com/dexterlabora/4a4692d0954eab107dd2
// More info at www.internetoflego.com

// includes
	var Primus = require('primus');
	var PrimusEmitter = require('primus-emitter');
	var express = require('express');
	var http = require('http');
	var path = require('path');
	var circuit = require('./elevator'); // replace this with your Arduino circuit file.

	var app = express();
	app.use(express.static(path.join(__dirname,"public")));

// define API
	// example: http://192.168.0.15:8080/command/elevator_toggle

	app.get('/command/:command', function(req,res){
	     var command = req.params.command;
	     console.log("received command from api: ");
	     // do something with command
	     circuit.emit('command', command); 
	      //
	      res.send('ok');
	});

// Create server
	var server = http.createServer(app);

// Add WebSockets support to http server
	var primus = new Primus(server);
	primus.use('emitter', PrimusEmitter);
	primus.on('connection', function(socket){
	     // If WebSockets server receives a ‘command’ event, it will process it
	     socket.on('command', function(command){

	    // We have the command, send it to the elevator code
	    circuit.emit('command', command);
	    //
	     });
	});

// Turn on sever
	server.listen(8080);