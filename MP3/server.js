const express = require('express');
const socket = require('socket.io');

const app = express();
const server = app.listen(process.env.PORT || 3000);

const io = socket(server);
io.sockets.on('connection', newConnection);

app.use(express.static('public')); // Serve the p5.js sketch

let firstConnectionId;

let numConnections = 0;
let numVotes = 0;

// Handles all socket event handlers for a particular socket connection once the client connects
function newConnection(socket) {
    numConnections++;
    console.log('Connection #' + numConnections + ': ' + socket.id);

    // Store the first connection's socket.id (main player)
    if (numConnections === 1) {
        firstConnectionId = socket.id;
    }
    
    // Every connection after the first one will wait until voting stage
    if (numConnections > 1) {
        socket.emit('waitForVote', 'waiting for vote...');
    }

    // Receive data from the player's drawing and echo it back to all clients
    socket.on('playerDrawData', data => {
        io.sockets.emit('playerDrawData', data);
    });
    
    // Receive data from the computer's drawing and echo it back to all clients
    socket.on('compLineData', data => {
        io.sockets.emit('compLineData', data);
    });  

    // Receive signal to clear the player's drawing and echo it back to all clients
    socket.on('clearPlayerDrawing', data => {
        io.sockets.emit('clearPlayerDrawing', data);
    });
    
    // Receive signal from main player (first connection) to move to voting state, echo back to all other clients (not the main player)
    socket.on('vote', data => {
        socket.broadcast.emit('vote', data);
    });

    // Receive signal from other players after their vote is placed, send corresponding vote to the main player to keep track of vote counts for player and computer
    socket.on('voted', data => {
        numVotes++;
        if (data === 'player') {
            io.to(firstConnectionId).emit('playerVote', 'received vote for player');
        } else if (data === 'computer') {
            io.to(firstConnectionId).emit('computerVote', 'received vote for computer');
        }
        
        // Send message to main player once all of the votes are counted
        if (numVotes === numConnections - 1) {
            io.to(firstConnectionId).emit('votingDone', 'all votes counted');
        }
    });
}