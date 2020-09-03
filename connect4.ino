/* 
 *  arduino_connect4: sketch for connect4 game using 8x8 LED matrix backpack
 *  by mit-mit
 *  
 *  Code uses Adafruit LED Backpack and GFX Library
 *  https://github.com/adafruit/Adafruit_LED_Backpack
 *  https://github.com/adafruit/Adafruit-GFX-Library
 *  
 */

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 4

// MATRIX DECLARATION:
// Parameter 1 = width of NeoPixel matrix
// Parameter 2 = height of matrix
// Parameter 3 = pin number (most are valid)
// Parameter 4 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the matrix; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs are arranged in horizontal
//     rows or in vertical columns, respectively; pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns proceed
//     in the same order, or alternate lines reverse direction; pick one.
//   See example below for these values in action.
// Parameter 5 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_GRBW    Pixels are wired for GRBW bitstream (RGB+W NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)


// Example for NeoPixel Shield.  In this application we'd like to use it
// as a 5x8 tall matrix, with the USB port positioned at the top of the
// Arduino.  When held that way, the first pixel is at the top right, and
// lines are arranged in columns, progressive order.  The shield uses
// 800 KHz (v2) pixels that expect GRB color data.
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, MATRIX_PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

#define rot2 1
#define rot1 2
#define Button_drop  0

#define color_p1 matrix.Color(255, 0, 0)
#define color_p2     matrix.Color(0, 0, 255)
#define color_win matrix.Color(255, 255, 255)

int board_state[49];
int board_stateAI[49];
long col_wins[7];
long col_losses[7];
long score_rec[7];

uint8_t score_p1 = 0;
uint8_t score_p2 = 0;

void ClearBoard() {
  for (int i = 0; i < 49; i++) {
    board_state[i] = 0;
  }
  //board_state[2] = 1;
  //board_state[3] = 1;
  for (int i = 0; i < 49; i++) {
    if (board_state[i] == 1) {
      matrix.drawPixel(i/7, i%7, 1);
      matrix.show();
    }
    else if (board_state[i] == 2) {
      matrix.drawPixel(i/7, i%7, 2);
      matrix.show();
    }
  }
}


// PlaceToken: controls token place animation and board state update
uint8_t PlaceToken(uint8_t col, uint8_t player, uint16_t color)
{
  if (board_state[7*6+col] > 0) { // can't place token here, already full on this column
    return(0);
  }
  uint8_t row = 6;
  while (1) {
    if (row == 0) { // at the bottom, place the token
      board_state[col] = player;
      matrix.drawPixel(0, col, color);
      matrix.show();
      delay(100);
      return(1);
    }
    if (board_state[7*(row-1)+col] > 0) { // token already played at row immediately under us
      board_state[7*row+col] = player;
      matrix.drawPixel(row, col, color);
      matrix.show();
      delay(100);
      return(1);
    }
    else {
      matrix.drawPixel(row, col, color); // update animation of token falling
      matrix.show();
      delay(100);
      matrix.drawPixel(row, col, 0);
      matrix.show();
      row--;
    }
  }
}

// CheckforWin: checks if last move resulted in victory
uint8_t CheckforWin()
{
  uint8_t p;
  for (int c = 0; c < 4; c++) {
    for (int r = 0; r < 6; r++) {
      p = board_state[7*r + c];
      if (p == 0) {
        continue;
      }
      if (board_state[7*r+c+1] == p &&
          board_state[7*r+c+2] == p && 
          board_state[7*r+c+3] == p) { // horizontal line
        matrix.drawPixel(r, c, color_win);
        matrix.drawPixel(r, c+1, color_win);
        matrix.drawPixel(r, c+2, color_win);
        matrix.drawPixel(r, c+3, color_win);
        matrix.show();
        delay(1000);
        return p;
      }
    }
  }
  for (int c = 0; c < 7; c++) {
    for (int r = 0; r < 4; r++) {
      p = board_state[7*r+c];
      if (p == 0) {
        continue;
      }
      if (board_state[7*(r+1)+c] == p &&
          board_state[7*(r+2)+c] == p && 
          board_state[7*(r+3)+c] == p) { // vertical line
        matrix.drawPixel(r, c, color_win);
        matrix.drawPixel(r+1, c, color_win);
        matrix.drawPixel(r+2, c, color_win);
        matrix.drawPixel(r+3, c, color_win);
        matrix.show();
        delay(1000);
        return p;
      }
    }
  }
  for (int c = 0; c < 4; c++) {
    for (int r = 0; r < 4; r++) {
      p = board_state[7*r+c];
      if (p == 0) {
        continue;
      }
      if (board_state[7*(r+1)+c+1] == p &&
          board_state[7*(r+2)+c+2] == p && 
          board_state[7*(r+3)+c+3] == p) { // diagonal line
        matrix.drawPixel(r, c, color_win);
        matrix.drawPixel(r+1, c+1, color_win);
        matrix.drawPixel(r+2, c+2, color_win);
        matrix.drawPixel(r+3, c+3, color_win);
        matrix.show();
        delay(1000);
        return p;
      }
      p = board_state[7*r+6-c];
      if (p == 0) {
        continue;
      }
      if (board_state[7*(r+1)+6-c-1] == p &&
          board_state[7*(r+2)+6-c-2] == p &&
          board_state[7*(r+3)+6-c-3] == p) { // diagonal line (opposite direction)
        matrix.drawPixel(r, 6-c, color_win);
        matrix.drawPixel(r+1, 6-c-1, color_win);
        matrix.drawPixel(r+2, 6-c-2, color_win);
        matrix.drawPixel(r+3, 6-c-3, color_win);
        matrix.show();
        delay(1000);
        return p;
      }
    }
  }
  // check for tie
  if ((board_state[42] > 0) && (board_state[43] > 0) && (board_state[44] > 0) && (board_state[45] > 0) &&
    (board_state[46] > 0) && (board_state[47] > 0) && (board_state[48] > 0)) {
      return 3; // board is full, tie game
  }
  return 0; // no winner yet
}

void RunWinnerAnimation(uint8_t player) {
  matrix.setTextWrap(false);
  matrix.setTextSize(1);
  matrix.setRotation(1);
  if (player == 1) {
    matrix.setTextColor(color_p1);
    for (int8_t x=7; x>=-43; x--) {
      matrix.clear();
      matrix.setCursor(x,0);
      matrix.print("player1");
      matrix.show();
      delay(50);
    }
  }
  else if (player == 2) {
    matrix.setTextColor(color_p2);
    for (int8_t x=7; x>=-36; x--) {
      matrix.clear();
      matrix.setCursor(x,0);
      matrix.print("player2");
      matrix.show();
      delay(50);
    }
  }
  else {
    matrix.setTextColor(color_win);
    for (int8_t x=7; x>=-29; x--) {
      matrix.clear();
      matrix.setCursor(x,0);
      matrix.print("draw");
      matrix.show();
      delay(50);
    }
  }
}

/////////////////////////////////////////////////////////////////
//////////////// AI Functions
/////////////////////////////////////////////////////////////////

#define n_MCTS_runs 10000

// InitBoardAI: resets simulation board and column win/lose stats for MCTS
void InitBoardAI() {
  for (int i = 0; i < 49; i++) {
    board_stateAI[i] = board_state[i];
  }
}

// checks if board is in winning state, return who won
uint8_t CheckforWinAI()
{
  uint8_t p;
  for (int c = 0; c < 4; c++) {
    for (int r = 0; r < 6; r++) {
      p = board_stateAI[7*r+c];
      if (p == 0) {
        continue;
      }
      if (board_stateAI[7*r+c+1] == p && board_stateAI[7*r+c+2] == p && 
        board_stateAI[7*r+c+3] == p) { // horizontal line
        return p;
      }
    }
  }
  for (int c = 0; c < 7; c++) {
    for (int r = 0; r < 3; r++) {
      p = board_stateAI[7*r+c];
      if (p == 0) {
        continue;
      }
      if (board_stateAI[7*(r+1)+c] == p && board_stateAI[7*(r+2)+c] == p && 
        board_stateAI[7*(r+3)+c] == p) { // vertical line
        return p;
      }
    }
  }
  for (int c = 0; c < 4; c++) {
    for (int r = 0; r < 3; r++) {
      p = board_stateAI[7*r+c];
      if (p == 0) {
        continue;
      }
      if (board_stateAI[7*(r+1)+c+1] == p && board_stateAI[7*(r+2)+c+2] == p && 
        board_stateAI[7*(r+3)+c+3] == p) { // diagonal line
        return p;
      }
      p = board_stateAI[7*r+6-c];
      if (p == 0) {
        continue;
      }
      if (board_stateAI[7*(r+1)+6-c-1] == p && board_stateAI[7*(r+2)+6-c-2] == p && 
        board_stateAI[7*(r+3)+6-c-3] == p) { // diagonal line (opposite direction)
        return p;
      }
    }
  }
  // check for tie
  if ((board_stateAI[35] > 0) && (board_stateAI[36] > 0) && (board_stateAI[37] > 0) && (board_stateAI[38] > 0) &&
    (board_stateAI[39] > 0) && (board_stateAI[40] > 0) && (board_stateAI[41] > 0)) {
      return 3; // board is full, tie game
  }
  return 0; // no winner yet
}

// PlaceTokenAI: updates simulation game board, if possible, returns 0 otherwise
uint8_t PlaceTokenAI(uint8_t col, uint8_t player)
{
  if (board_stateAI[7*6 + col] > 0) { // can't place token here, already full on this column
    return(0);
  }
  uint8_t row = 0;
  while (board_stateAI[7*row + col] > 0) {
    row++;
  }
  board_stateAI[7*row + col] = player;
  return(1);
}

void DrawBoardAI() {
  Serial.print("\n");
  for (int r = 6; r >= 0; r--) {
    for (int c = 0; c < 7; c++) {
      Serial.print(board_stateAI[7*r+c]);
      Serial.print(" ");
    }
    Serial.print("\n");
  }
}

// AIPlay: function runs AI moves
uint8_t AIPlay(uint8_t player_ai) {

  uint8_t valid;
  uint8_t pwin;
  uint8_t first;
  uint8_t col = 0;
  uint8_t playcol;
  uint8_t simplayer;

  long score, best_score;
  uint8_t best_col;
  uint8_t moves_left;
  uint8_t moves_left2;
  
  uint8_t opponent;

  // record player ID of opponent
  opponent = (player_ai + 1) % 2;
  
  // Start off by looking for immediate win
  for (int i = 0; i < 7; i++) {
    InitBoardAI();
    if (PlaceTokenAI(i, player_ai)) {
      pwin = CheckforWinAI();
      if (pwin == player_ai) {
        return i;
      }
    }
  }

  // Next look for blocks necessary to stop player immediate win
  for (int i = 0; i < 7; i++) {
    InitBoardAI();
    if(PlaceTokenAI(i, opponent)) {
      pwin = CheckforWinAI();
      if (pwin == opponent) {
        return i;
      }
    }
  }
  
  // Count remaining moves
  moves_left = 0;
  for (int i = 0; i < 42; i++) {
    if (board_state[i] == 0) {
      moves_left++;
    }
  }
  
  // no win next move, run simuations
  for (int i = 0; i < 7; i++) {
    col_wins[i] = 0;
    col_losses[i] = 0;
  }
  
  for (int i = 0; i < n_MCTS_runs; i++) {
    InitBoardAI();
    simplayer = player_ai;

    // play first move
    while (!PlaceTokenAI(col, simplayer)) {
      col++;
      if (col == 7) {
        col = 0;
      }
    }
    simplayer++;
    if (simplayer == 3) {
      simplayer = 1;
    }
    pwin = CheckforWinAI();
    if (pwin == 3) { // check for tie
      continue; // move to next random sample
    }

    // loop through remaining moves until result
    moves_left2 = moves_left - 1;
    while (1) {
      playcol = random(0, 6);
      while (!PlaceTokenAI(playcol, simplayer)) {
        playcol++;
        if (playcol == 7) {
          playcol = 0;
        }
      }

      pwin = CheckforWinAI();
      if (pwin == player_ai) { // game resulted in win
        // record victory against this first played column
        col_wins[col] = col_wins[col] + moves_left2;
        //col_wins[col]++;
        break; // move to next random sample
      }
      else if (pwin == opponent) { // game resulted in loss
        // record defeat against this first played column
        col_losses[col] = col_losses[col] + moves_left2;
        //col_losses[col]++;
        //DrawBoardAI();
        break; // move to next random sample
      }
      else if (pwin == 3) { // resulted in draw, move onto next sim
        break;
      }
      simplayer++;
      if (simplayer == 3) {
        simplayer = 1;
      }
      moves_left2--;
    }

    // Cycle through first-played cols
    col++;
    if (col == 7) {
      col = 0;
    }
    
  } // MCTS loop

  // evaluate win/loss stats to determine best move
  best_col = 0;
  best_score = col_wins[0]-col_losses[0];
  if (board_state[7*6+0] > 0) {
    best_score = -49*n_MCTS_runs;
  }
  score_rec[0] = best_score;
  for (int i = 1; i < 7; i++) {
    score = col_wins[i] - col_losses[i];
    score_rec[i] = score;
    if ( (score > best_score) && (board_state[7*6 + i] == 0) ) {
      best_col = i;
      best_score = score;
    }
  }
  
  for (int i = 0; i < 7; i++) {
    Serial.print(col_wins[i]);
    Serial.print("/");
    Serial.print(col_losses[i]);
    Serial.print("/");
    Serial.print(score_rec[i]);
    Serial.print(", ");
  }
  Serial.print("\n\r");
  
  return best_col;
  
}

uint8_t players;
void setup() {

  Serial.begin(9600);
  randomSeed(analogRead(3));

  pinMode(rot1, INPUT_PULLUP);
  pinMode(rot2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(rot1), doEncoder, RISING);
  
  pinMode(Button_drop, INPUT_PULLUP);
  pinMode(13, OUTPUT);

  matrix.begin(); // start up the bi-colour LED matrix
  matrix.setBrightness(10);
  matrix.clear();

  players = getPlayers();
}


int valRotary = 0;
int lastValRotary = 0;
unsigned long Time;
unsigned long lastTime = 0;

void doEncoder() {
  // Digitally debounce a bit
  // Make sure there are 12 consecutive 1's on rot1
  // before reading the state of rot2
  uint16_t state = 0;
  do
    state = ((state << 1) | digitalRead(rot1)) & 0x2000-1;
  while (state != 0x2000-1);
  if (digitalRead(rot2)) 
    valRotary--;
  else
    valRotary++;
}

uint8_t getMove(uint8_t col, uint16_t color) {
    uint8_t button_r = 0;
    uint8_t button_l = 0;
    uint8_t button_d;

    matrix.drawPixel(7, col, color);
    matrix.show();

    // Read column/drop loop
    while(1) {
      if(valRotary > lastValRotary)
        button_r = 1;
      if(valRotary < lastValRotary)
        button_l = 1;
      lastValRotary = valRotary;
      button_d = digitalRead(Button_drop);

      if (button_r == 1) { // check for button right
        button_r = 0;
        matrix.drawPixel(7, col, 0);
        matrix.show();
        if (col < 6) {
          col++;
        }
        matrix.drawPixel(7, col, color);
        matrix.show();
      }
      if (button_l == 1) { // check for button left
        button_l = 0;
        matrix.drawPixel(7, col, 0);
        matrix.show();
        if (col != 0) {
          col--;
        }
        matrix.drawPixel(7, col, color);
        matrix.show();
      }

      if (button_d == 0) {
        break;
      }
      delay(100);
    }
    
    matrix.drawPixel(7, col, 0);
    matrix.show();
    return col;
}

// Device if this is one or two player game
uint8_t getPlayers() {
    uint8_t button_d;
    uint8_t players = 2;

    //matrix.setTextWrap(false);
    matrix.setTextSize(1);
    matrix.setRotation(1);
    matrix.setTextColor(color_p1);

    // Create condition for first print
    lastValRotary = valRotary - 1;
    
    // Read players loop
    while(1) {
      button_d = digitalRead(Button_drop);
      if (button_d == 0) {
        break;
      }
      if(valRotary != lastValRotary) {
        players = 2 - ((players - 1) % 2);
        lastValRotary = valRotary;
  
        matrix.clear();
        matrix.setCursor(2, 1);
        matrix.print(players);
        matrix.show();
      }  
    }
    delay(100);
    return players;
}

void drawScore() {
  //Serial.println("drawScore");
  matrix.clear();
  
  // score p1
  for (uint8_t i = 0; i < score_p1; i++)
    matrix.drawPixel(i, 7, color_p1);

  // score p1
  for (uint8_t i = 0; i < score_p2; i++)
    matrix.drawPixel(7-i, 7, color_p2);

  matrix.show();
}

void loop() {

  uint8_t player_win = 0;
  uint8_t valid = 0;
  uint8_t col = 3;
  uint8_t last_col_p1 = 3;
  uint8_t last_col_p2 = 3;
  
  // initialise board
  matrix.clear();
  matrix.setRotation(0);
  ClearBoard();

  drawScore();
  
  // start game loop
  while (1) {
    col = last_col_p1;

    // human player move
    col = getMove(col, color_p1);
    last_col_p1 = col;

    if (! PlaceToken(col, 1, color_p1))
      continue;
    player_win = CheckforWin();
    if (player_win > 0) {
      if (++score_p1 > 4) {
        score_p1 = 0;
        score_p2 = 0;
        RunWinnerAnimation(player_win);
      }
      break;
    }
    // AI player move
    col = last_col_p2;
    do
      if (players == 1)
        col = AIPlay(2);
      else
        col = getMove(col, color_p2);
    while (!PlaceToken(col, 2, color_p2));
    last_col_p2 = col;

    player_win = CheckforWin();
    if (player_win > 0) {
      if (++score_p2 > 4) {
        score_p1 = 0;
        score_p2 = 0;
        RunWinnerAnimation(player_win);
      }
      break;
    }
  }
}
