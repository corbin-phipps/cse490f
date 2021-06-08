# MP3

## How to Run

Run `npm install` to install any necessary dependencies

Upload `MP3.ino` to Arduino

Start server by running `npm start`

In one browser tab, as the main player of the game, visit `http://localhost:3000`

To simulate other players who will later vote for a drawing, open another browser tab and also visit `http://localhost:3000`


## How to Play

In the main player's browser window, connect to Arduino with Serial

On the Arduino, choose the game mode using the joystick and button

Once a word has been chosen, in the main player's browser window, draw the word and click the finish button

Observe the computer's drawing, then click the start voting button

In the other browser windows/tabs for the other players, vote for the better drawing

See the votes in real-time on the Arduino OLED screen, and see the winner on the main player's screen after all votes are counted!