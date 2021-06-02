/**************
** Variables **
**************/

/* Constants */
const MAX_PEN_SIZE = 150; // the maximum pen size

/* Serial variables*/
let serialOptions = { baudRate: 115200  };
let serial;

/* SketchRNN variables */
let model; 
let previousPen = "down"; // start by drawing
let x, y; // current location of drawing
let strokePath; // current "stroke" of drawing

/* Drawing variables */
let canvas; // canvas for drawing
let finishButton;
let mapPenTypeToName = {
  0: "smallPen",
  1: "largePen"
};
let penType = 0;             // Small pen as default
let penSize = 50;            // Initial pen size
let penX = 0;                // Current pen x location (in pixel coordinates)
let penY = 0;                // Current pen y location (in pixel coordinates)
let penColor;                // Current pen color
let lastPenX = 0;            // Last pen y position (similar to pmouseX but for the pen)
let lastPenY = 0;            // Last pen y position (similar to pmouseY but for the pen)
let showInstructions = true; // If true, shows the app instructions on the screen
let offscreenGfxBuffer;      // Offscreen graphics buffer: https://p5js.org/reference/#/p5/createGraphics

/* Game variables */
let pHtmlMsg;
let titleHtmlMsg;
const GameStates = {
  NewGame: "NewGame",
  Choose: "Choose",
  PlayerDraw: "PlayerDraw",
  ComputerDraw: "ComputerDraw",
  Vote: "Vote",
  EndGame: "EndGame"
}
let gameState = GameStates.NewGame;
let wordToDraw = "";

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
    offscreenGfxBuffer.background(255);
    offscreenGfxBuffer.stroke(0);
    offscreenGfxBuffer.fill(255);
    offscreenGfxBuffer.rect(0, 0, 640, 480);
  } else if(key == 'o'){
    if (!serial.isOpen()) {
      serial.connectAndOpen(null, serialOptions);
    }
  }
}

function setup() {
  canvas = createCanvas(640, 480);
  background(255);

  finishButton = createButton("Finish");
  finishButton.mousePressed(finishPlayerDrawing);

  // Setup Web Serial using serial.js
  serial = new Serial();
  serial.on(SerialEvents.CONNECTION_OPENED, onSerialConnectionOpened);
  serial.on(SerialEvents.CONNECTION_CLOSED, onSerialConnectionClosed);
  serial.on(SerialEvents.DATA_RECEIVED, onSerialDataReceived);
  serial.on(SerialEvents.ERROR_OCCURRED, onSerialErrorOccurred);

  penColor = color(0);
  // Rather than storing individual paint strokes + paint properties in a
  // data structure, we simply draw immediately to an offscreen buffer
  // and then show this offscreen buffer on each draw call
  // See: https://p5js.org/reference/#/p5/createGraphics
  offscreenGfxBuffer = createGraphics(width, height);
  offscreenGfxBuffer.background(255); 
  offscreenGfxBuffer.rect(0, 0, 640, 480);

  titleHtmlMsg = createElement("h1", "Let's Draw");
  pHtmlMsg = createP("Press 'o' key to open the serial connection dialog");
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
  } else if (gameState === "EndGame") {
    endGame();
  }
}

/*************************
** Game State Functions **
*************************/

function newGame() {
  canvas.hide();
  finishButton.hide();
}

function choose() {

}

function playerDraw() {
  canvas.show();
  finishButton.show();

  // Draw the current pen stroke at the given x, y position
  // But we don't draw to canvas, we draw to the offscreenGfxBuffer
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

function computerDraw() {
  finishButton.hide();

  pHtmlMsg.html("Computer is drawing...");

  // If something new to draw
  if (strokePath) {
    // If the pen is down, draw a line
    if (previousPen === "down") {
      stroke(0);
      strokeWeight(3.0);
      line(x, y, x + strokePath.dx, y + strokePath.dy);
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
}

function vote() {

}

function endGame() {

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
    titleHtmlMsg.html("Waiting for player to choose a word...");
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
  background(220);
  // Start in the middle
  x = width / 2;
  y = height / 2;
  model.reset();
  // Generate the first stroke path
  model.generate(gotStroke);
}

// Sets new stroke path for computer drawing
function gotStroke(err, s) {
  strokePath = s;
}

// Saves the player's drawing and moves game state to computer drawing
function finishPlayerDrawing() {
  // TODO: somehow save player's drawing

  // Clear the background
  offscreenGfxBuffer.background(255);
  offscreenGfxBuffer.stroke(0);
  offscreenGfxBuffer.fill(255);
  offscreenGfxBuffer.rect(0, 0, 640, 480);
  image(offscreenGfxBuffer, 0, 0);
  
  // Move to next game state
  gameState = GameStates.ComputerDraw;
  serialUpdateGameState();
  model = ml5.sketchRNN(wordToDraw);
  startDrawing();
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
      break;
    case 1: // draw large pen
      // Draw rectangle based on center coordinates
      offscreenGfxBuffer.circle(xCenter, yCenter, penSize)
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

async function serialUpdateGameState() {
  let strData;
  if (serial.isOpen()) {
    if (gameState === "ComputerDraw") {
      strData = "ComputerDraw";
    }
    serial.writeLine(strData);
  }
}