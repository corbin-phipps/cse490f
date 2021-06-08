/**************
** Variables **
**************/

/* Serial variables */
let serialOptions = { baudRate: 115200  };
let serial;

/* Socket variables */
let socket;

/* SketchRNN variables */
let model; 
let previousPen = "down"; // start by drawing
let x, y; // current location of drawing
let strokePath; // current "stroke" of drawing

/* Drawing variables */
let canvas; // canvas for drawing
let finishButton; // button for submitting player drawing
let goToVoteButton; // button to move to voting stage
let mapPenTypeToName = {
  0: "smallPen",
  1: "largePen"
};
let penType = 0;             // Small pen as default
let penSize = 20;            // Initial pen size
let penX = 0;                // Current pen x location (in pixel coordinates)
let penY = 0;                // Current pen y location (in pixel coordinates)
let penColor;                // Current pen color
let lastPenX = 0;            // Last pen y position (similar to pmouseX but for the pen)
let lastPenY = 0;            // Last pen y position (similar to pmouseY but for the pen)
let showInstructions = true; // If true, shows the app instructions on the screen

// Offscreen graphics buffers: https://p5js.org/reference/#/p5/createGraphics
let offscreenGfxBuffer;       // Displays as drawings are being drawn
let socketPlayerGfxBuffer;    // Stores player's drawing to display on voting screen
let socketComputerGfxBuffer;  // Stores computer drawing to display on voting screen

/* Voting variables */
let votePlayerButton;
let voteComputerButton;
let playerVoteCount;
let computerVoteCount;

/* HTML variables */
let titleHtmlMsg;
let pHtmlMsg;
let titleDiv;
let msgDiv;
let canvasDiv;
let btnDiv;

/* Game variables */
const GameStates = {
  NewGame: "NewGame",
  Choose: "Choose",
  PlayerDraw: "PlayerDraw",
  ComputerDraw: "ComputerDraw",
  Vote: "Vote",
  Voted: "Voted",
  EndGame: "EndGame"
}
let gameState = GameStates.NewGame;
let wordToDraw = "";
let playerDrawData;

/**************************
** Auto-Called Functions **
**************************/

// Handles keyboard presses
function keyPressed() {
  print("keyPressed", key);

  if(key == 'b') {
    penType++;
    if (penType >= Object.keys(mapPenTypeToName).length) {
      penType = 0;
    }
  } else if(key == 'i'){
    showInstructions = !showInstructions;
  } else if(key == 'l'){
    // To clear the screen, simply "draw" over the existing
    // graphics buffer with an empty background
    if (gameState === GameStates.PlayerDraw) {
      offscreenGfxBuffer.background(200);
      socketPlayerGfxBuffer.background(200);
      socket.emit('clearPlayerDrawing', 'clear background of drawing');
    }
  } else if(key == 'o'){
    if (!serial.isOpen() && gameState === GameStates.NewGame) {
      serial.connectAndOpen(null, serialOptions);
    }
  }
}

function setup() {
  /******************************
  ** Web Page Components Setup **
  ******************************/
  //titleDiv = createDiv();
  //titleDiv.id('title');
  //titleHtmlMsg = createElement("h1", "Draw It!");
  //titleHtmlMsg.parent('title');

  msgDiv = createDiv();
  msgDiv.id('msg');
  pHtmlMsg = createP("Press 'o' key to open the serial connection dialog");
  pHtmlMsg.parent('msg');

  canvasDiv = createDiv();
  canvasDiv.id('canvas');
  canvas = createCanvas(1280, 480);
  canvas.parent('canvas');
  background(250);

  btnDiv = createDiv();
  btnDiv.id('btn-group');

  finishButton = createButton("Finish");
  finishButton.parent('btn-group');
  finishButton.mousePressed(finishPlayerDrawing);

  goToVoteButton = createButton("Start Voting!");
  goToVoteButton.parent('btn-group');
  goToVoteButton.mousePressed(finishComputerDrawing);

  votePlayerButton = createButton("Vote for player's drawing");
  votePlayerButton.parent('btn-group');
  votePlayerButton.mousePressed(votePlayer);

  voteComputerButton = createButton("Vote for computer's drawing");
  voteComputerButton.parent('btn-group');
  voteComputerButton.mousePressed(voteComputer);

  /*****************
  ** Serial Setup **
  *****************/

  // Setup Web Serial using serial.js
  serial = new Serial();
  serial.on(SerialEvents.CONNECTION_OPENED, onSerialConnectionOpened);
  serial.on(SerialEvents.CONNECTION_CLOSED, onSerialConnectionClosed);
  serial.on(SerialEvents.DATA_RECEIVED, onSerialDataReceived);
  serial.on(SerialEvents.ERROR_OCCURRED, onSerialErrorOccurred);

  /********************
  ** Socket Handlers **
  ********************/

  // Use socket to connect to server
  socket = io.connect('http://localhost:3000');

  // Print message if waitForVote event received (received by all connections except the first)
  socket.on('waitForVote', data => {
    console.log(data);
    pHtmlMsg.html('Waiting to vote until drawings are complete');
  });

  socket.on('vote', data => {
    console.log(data);
    pHtmlMsg.html('PLACE YOUR VOTE');
    gameState = GameStates.Vote;
    //votePlayerButton.show();
    //voteComputerButton.show();
    votePlayerButton.style('display', 'inline-block');
    voteComputerButton.style('display', 'inline-block');
  });

  // Draw data received from player drawing to special offscreen graphics buffer
  // when playerDrawData event received
  socket.on('playerDrawData', data => {
    socketPlayerGfxBuffer.fill(penColor);
    socketPlayerGfxBuffer.noStroke();
    socketPlayerGfxBuffer.circle(data.xCenter, data.yCenter, data.size);
  });

  // Draw data received from computer drawing to special offscreen graphics buffer
  // when compLineData event received
  socket.on('compLineData', data => {
    socketComputerGfxBuffer.stroke(0);
    socketComputerGfxBuffer.strokeWeight(3.0);
    socketComputerGfxBuffer.line(data.x1, data.y1, data.x2, data.y2);
  });

  // Clears player graphics buffer when clearPlayerDrawing event received
  socket.on('clearPlayerDrawing', data => {
    socketPlayerGfxBuffer.background(200);
  });

  // Increments vote count for player and sends to Arduino over serial
  // when playerVote event received
  socket.on('playerVote', data => {
    playerVoteCount++;
    serial.writeLine('playerVote');
  });

  // Increments vote count for computer and sends to Arduino over serial
  // when computerVote event received
  socket.on('computerVote', data => {
    computerVoteCount++;
    serial.writeLine('computerVote');
  });

  // Moves game state to EndGame and stores final vote counts when votingDone event received
  socket.on('votingDone', data => {
    gameState = GameStates.EndGame;
  });
  
  /******************
  ** Drawing Setup **
  ******************/

  penColor = color(0);
  // Rather than storing individual paint strokes + paint properties in a
  // data structure, we simply draw immediately to an offscreen buffer
  // and then show this offscreen buffer on each draw call
  // See: https://p5js.org/reference/#/p5/createGraphics
  offscreenGfxBuffer = createGraphics(640, 480);
  offscreenGfxBuffer.background(200); 

  socketPlayerGfxBuffer = createGraphics(640, 480);
  socketPlayerGfxBuffer.background(200);

  socketComputerGfxBuffer = createGraphics(640, 480);
  socketComputerGfxBuffer.background(200);
}

function draw() {
  if (gameState === "NewGame") {
    newGame();
  } else if (gameState === "Choose") {
    choose();
  } else if (gameState === "PlayerDraw") {
    playerDraw();
  } else if (gameState === "ComputerDraw") {
    computerDraw();
  } else if (gameState === "Vote") {
    vote(); 
  } else if (gameState === "Voted") {
    voted();
  } else if (gameState === "EndGame") {
    endGame();
  }
}

/*************************
** Game State Functions **
*************************/

// Handles NewGame game state
// Hides components not in use to get ready for new game
function newGame() {
  canvas.hide();
  finishButton.hide();
  goToVoteButton.hide();
  votePlayerButton.hide();
  voteComputerButton.hide();
}

// Handles Choose game state
// Hides components not in use. HTML message printed at this stage is handled by onSerialDataReceived
function choose() {
  canvas.hide();
  finishButton.hide();
  goToVoteButton.hide();
  votePlayerButton.hide();
  voteComputerButton.hide();
}

// Handles PlayerDraw game state
// Shows canvas and finish button, draws player drawing to screen as well as onscreen instructions
function playerDraw() {
  canvas.show();
  finishButton.show();
  goToVoteButton.hide();
  votePlayerButton.hide();
  voteComputerButton.hide();

  // Draw the current pen stroke at the given x, y position
  if (mouseIsPressed) {
    drawPenStroke(mouseX, mouseY);
  }

  // Draw the offscreen buffer to the screen
  image(offscreenGfxBuffer, 0, 0);

  // Check to see if we are supposed to draw our instructions
  if(showInstructions) {
    drawInstructions();
  }
}

// Handles ComputerDraw game state
// Shows canvas and displays computer drawing in real-time
function computerDraw() {
  canvas.show();
  finishButton.hide();
  goToVoteButton.hide();
  votePlayerButton.hide();
  voteComputerButton.hide();

  pHtmlMsg.html("Computer is drawing...");

  // If something new to draw
  if (strokePath) {
    // If the pen is down, draw a line
    if (previousPen === "down") {
      offscreenGfxBuffer.stroke(penColor);
      offscreenGfxBuffer.strokeWeight(3.0);
      offscreenGfxBuffer.line(x, y, x + strokePath.dx, y + strokePath.dy);

      let compLineData = {
        x1: x,
        y1: y,
        x2: x + strokePath.dx,
        y2: y + strokePath.dy
      };
      socket.emit('compLineData', compLineData);
      
    }
    // Move the pen
    x += strokePath.dx;
    y += strokePath.dy;
    // The pen state actually refers to the next stroke
    previousPen = strokePath.pen;

    // If the drawing is complete
    if (strokePath.pen !== "end") {
      strokePath = null;
      model.generate(gotStroke);
    }
  } 
  if (strokePath !== undefined && strokePath !== null) {
    goToVoteButton.show();
    pHtmlMsg.html("Computer drew a " + wordToDraw + "!");
  }

  image(offscreenGfxBuffer, 0, 0);
}

// Handles Vote game state
// Displays both finished drawings to the screen
function vote() {
  canvas.show();
  finishButton.hide();
  goToVoteButton.hide();

  image(socketPlayerGfxBuffer, 0, 0);
  image(socketComputerGfxBuffer, 640, 0);
}

// Handles Voted game state
// Displays message to other players (not main player) after they place their vote
function voted() {
  canvas.show();
  finishButton.hide();
  goToVoteButton.hide();
  votePlayerButton.hide();
  voteComputerButton.hide();

  pHtmlMsg.html("Your vote has been placed!");
}

// Handles EndGame game state
// Hides components not in use and displays message of who won the game based on number of votes
function endGame() {
  canvas.hide();
  finishButton.hide();
  goToVoteButton.hide();
  votePlayerButton.hide();
  voteComputerButton.hide();

  if (playerVoteCount > computerVoteCount) {
    pHtmlMsg.html("Player wins!");
  } else if (computerVoteCount > playerVoteCount) {
    pHtmlMsg.html("Computer wins!");
  } else {
    pHtmlMsg.html("It's a tie!");
  }
}

/*********************
** Serial Functions **
*********************/

function onSerialErrorOccurred(eventSender, error) {
  console.log("onSerialErrorOccurred", error);
  pHtmlMsg.html(error);
}

function onSerialConnectionOpened(eventSender) {
  console.log("onSerialConnectionOpened");
  pHtmlMsg.html("Serial connection opened successfully");
}

function onSerialConnectionClosed(eventSender) {
  console.log("onSerialConnectionClosed");
  pHtmlMsg.html("onSerialConnectionClosed");
}

function onSerialDataReceived(eventSender, newData) {
  console.log("onSerialDataReceived", newData);
  if (newData === 'choosing word') {
    gameState = GameStates.Choose;
    pHtmlMsg.html("Waiting for player to choose a word...");
  } else {
    gameState = GameStates.PlayerDraw;
    wordToDraw = newData;
    pHtmlMsg.html("Your word to draw is: " + newData);
  }
}

/**********************
** Utility Functions **
**********************/

// Resets and starts the computer drawing
function startDrawing() {
  offscreenGfxBuffer.background(200);
  // Start in the middle
  x = 640 / 2;
  y = 480 / 2;
  model.reset();
  // Generate the first stroke path
  model.generate(gotStroke);
}

// Sets new stroke path for computer drawing
function gotStroke(err, s) {
  strokePath = s;
}

// Clears buffer and moves game state to computer drawing
function finishPlayerDrawing() {
  // Clear the background
  offscreenGfxBuffer.background(200);
  
  // Move to next game state
  gameState = GameStates.ComputerDraw;
  serialUpdateGameState();
  model = ml5.sketchRNN(wordToDraw);
  startDrawing();
}

// Clears buffer and moves game state to voting stage
function finishComputerDrawing() {
  console.log("computer drawing finished");
  pHtmlMsg.html("Counting votes...");

  offscreenGfxBuffer.background(200);

  gameState = GameStates.Vote;
  serialUpdateGameState();

  socket.emit('vote', 'start voting');
  playerVoteCount = 0;
  computerVoteCount = 0;
}

// Sends event to server telling it that the player on this socket connection has voted for the player's drawing
function votePlayer() {
  gameState = GameStates.Voted;
  socket.emit('voted', 'player');
}

// Sends event to server telling it that the player on this socket connection has voted for the computer's drawing
function voteComputer() {
  gameState = GameStates.Voted;
  socket.emit('voted', 'computer');
}

// Draws a circle at the given location, with size based on penType
function drawPenStroke(xPen, yPen){
  // set the fill and outline pen settings
  offscreenGfxBuffer.fill(penColor);
  offscreenGfxBuffer.noStroke();

  // draw the specific pen size depending on penType
  let xCenter = xPen;
  let yCenter = yPen;
  let halfShapeSize = penSize / 2;
  switch (penType) {
    case 0: // draw small pen
      offscreenGfxBuffer.circle(xCenter, yCenter, halfShapeSize);
      playerDrawData = {
        xCenter: xCenter,
        yCenter: yCenter,
        size: halfShapeSize
      };
      socket.emit('playerDrawData', playerDrawData);
      break;
    case 1: // draw large pen
      // Draw rectangle based on center coordinates
      offscreenGfxBuffer.circle(xCenter, yCenter, penSize);
      playerDrawData = {
        xCenter: xCenter,
        yCenter: yCenter,
        size: penSize
      };
      socket.emit('playerDrawData', playerDrawData);
      break;
  } 
}

// Draws on-screen instructions
function drawInstructions() {
  // Some instructions to the user
  noStroke();
  fill(0);
  let tSize = 10;

  textSize(tSize);
  let yText = 2;
  let yBuffer = 1;
  let xText = 3;
  text("KEYBOARD COMMANDS", xText, yText + tSize);
  yText += tSize + yBuffer;
  text("'i' : Show/hide instructions", xText, yText + tSize);
  
  yText += tSize + yBuffer;
  text("'l' : Clear the screen", xText, yText + tSize);
  
  yText += tSize + yBuffer;
  let strPenType = "'b' : Set pen size (" + mapPenTypeToName[penType] + ")";
  text(strPenType, xText, yText + tSize);

  yText += tSize + yBuffer;
  let strConnectToSerial = "'o' : Open serial (";
  if (serial.isOpen()) {
    strConnectToSerial += "connected";
  } else {
    strConnectToSerial += "not connected";
  }
  strConnectToSerial += ")";
  text(strConnectToSerial, xText, yText + tSize);
}

// Sends a serial message to Arduino to update the game state on the Arduino side, based on the game state on the JavaScript side
async function serialUpdateGameState() {
  let strData;
  if (serial.isOpen()) {
    if (gameState === "ComputerDraw") {
      strData = "ComputerDraw";
    } else if (gameState === "Vote") {
      strData = "Vote";
    }
    serial.writeLine(strData);
  }
}