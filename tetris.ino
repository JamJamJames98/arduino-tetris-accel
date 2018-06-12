#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Wire.h>

#define MAX_DEVICES 4
#define CLK_PIN   ICSP-3
#define DATA_PIN  ICSP-4
#define CS_PIN    10
 
MD_MAX72XX mx = MD_MAX72XX(CS_PIN, MAX_DEVICES);                      // SPI hardware interface
 
//ACCELEROMETER CODE
// Declaration of the adress of the module
#define ADXL345_Adresse 0x53 // ADXL345 adress
#define POWER_CTL 0x2D // Power Control register
#define DATA_FORMAT 0x31 // Data Format register
#define DATAX0 0x32 // LSB axe X
#define DATAX1 0x33 // MSB axe X
#define DATAY0 0x34 // LSB axe Y
#define DATAY1 0x35 // MSB Y
#define DATAZ0 0x36 // LSB axe Z
#define DATAZ1 0x37 // bMSB Z
 
// Configuration of the module
#define ADXL345_Precision2G 0x00
#define ADXL345_Precision4G 0x01
#define ADXL345_Precision8G 0x02
#define ADXL345_Precision16G 0x03
#define ADXL345_ModeMesure 0x08
 
byte buffer[6]; // storage of data of the module
int i = 0;
double xValue;
double yValue;

//node struct for each block
struct node {
  int height;
  int width;
  struct node* next;
  struct node* prev;
  struct orientation* shape; // current shape of the block
};

//orientation struct, allowes each node to rotate
struct orientation {
  int arr[10][10]; // this is the new shape->arr, this is the array to render so: curr->shape->arr[][] instead of shape->arr[][]
  int arr_height; // height of current shape
  int arr_width; // width of current shape
  struct orientation* rotate; // this is a next, so to go to next shape we call: curr->shape->rotate (THIS IS ALREADY DONE IN FORWARD OPERATION)
  struct orientation* reverse; // reverse rotation
  struct orientation* default_shape; // spawning orientation
};

//all our blocks
struct node* block_4 = (node*)malloc(sizeof *block_4);
struct node* block_2_2 = (node*)malloc(sizeof *block_2_2);
struct node* block_3_1 = (node*)malloc(sizeof *block_3_1);
struct node* block_Z = (node*)malloc(sizeof *block_Z);
struct node* block_S = (node*)malloc(sizeof *block_S);
struct node* block_L = (node*)malloc(sizeof *block_L);
struct node* block_J = (node*)malloc(sizeof *block_J);

void setup() {
  //start up the combined led matrix controls
  mx.begin();
  mx.update(true);
  draw_end_line();
 
  //ACCELEROMETER SETUP
  Serial.begin(9600); // initialization of serial communication
  Wire.begin (); // initialization of I2C communication
  Wire.beginTransmission (ADXL345_Adresse); // configuration of the module
  Wire.write (DATA_FORMAT);
  Wire.write (ADXL345_Precision4G);
  Wire.endTransmission ();
  Wire.beginTransmission (ADXL345_Adresse);
  Wire.write (POWER_CTL);
  Wire.write (ADXL345_ModeMesure);
  Wire.endTransmission ();
  
   /*
    * 
    * NEW BLOCK
    * 
    */
    
  // block 4
  block_4->height = 4;
  block_4->width = 1;
  block_4->next = block_2_2;
  block_4->prev = block_J;

  // 2 different orientations of block_4
  /*
   * [[1],
   *  [1],
   *  [1],
   *  [1]]
   */
  struct orientation* block_4_shape_1 = (orientation*)malloc(sizeof(orientation));
  block_4_shape_1->arr_height = 4;
  block_4_shape_1->arr_width = 1;
  for (int i = 0; i < block_4_shape_1->arr_height; i++) {          
    for (int j = 0; j < block_4_shape_1->arr_width; j++) {
      block_4_shape_1->arr[i][j] = 1;
    }
  }
 
  /*
   * [[1, 1, 1, 1]]
   */
  struct orientation* block_4_shape_2 = (orientation*)malloc(sizeof(orientation));
  block_4_shape_2->arr_height = 1;
  block_4_shape_2->arr_width = 4;
  for (int i = 0; i < block_4_shape_2->arr_height; i++) {          
    for (int j = 0; j < block_4_shape_2->arr_width; j++) {
      block_4_shape_2->arr[i][j] = 1;
    }
  }
 
  // assigning circular dependency
  block_4_shape_1->default_shape = block_4_shape_1;
  block_4_shape_2->default_shape = block_4_shape_1;
  block_4_shape_1->rotate = block_4_shape_2;
  block_4_shape_1->reverse = block_4_shape_2;
  block_4_shape_2->rotate = block_4_shape_1;
  block_4_shape_2->reverse = block_4_shape_1;
  block_4->shape = block_4_shape_1;

  /*
   * 
   * NEW BLOCK
   * 
   */

  // block 2_2
  block_2_2->height = 2;
  block_2_2->width = 2;
  block_2_2->next = block_3_1;
  block_2_2->prev = block_4;

  // 1 orientation of block_2_2
  /*
   * [[1, 1],
   *  [1, 1]]
   */
  struct orientation* block_2_2_shape_1 = (orientation*)malloc(sizeof(orientation));
  block_2_2_shape_1->arr_height = 2;
  block_2_2_shape_1->arr_width = 2;
  for (int i = 0; i < block_2_2_shape_1->arr_height; i++) {          
    for (int j = 0; j < block_2_2_shape_1->arr_width; j++) {
      block_2_2_shape_1->arr[i][j] = 1;
    }
  }
 
  // assigning circular dependency
  block_2_2_shape_1->default_shape = block_2_2_shape_1;
  block_2_2_shape_1->rotate = block_2_2_shape_1;
  block_2_2_shape_1->reverse = block_2_2_shape_1;
  block_2_2->shape = block_2_2_shape_1;

  /*
   * 
   * NEW BLOCK
   * 
   */

  // block 3_1
  block_3_1->height = 2;
  block_3_1->width = 3;
  block_3_1->next = block_Z;
  block_3_1->prev = block_2_2;

  // 4 different orientations of block_3_1
  /*
   * [[0, 1, 0],
   *  [1, 1, 1]]
   */
  struct orientation* block_3_1_shape_1 = (orientation*)malloc(sizeof(orientation));
  block_3_1_shape_1->arr_height = 2;
  block_3_1_shape_1->arr_width = 3;
  for (int i = 0; i < block_3_1_shape_1->arr_height; i++) {          
    for (int j = 0; j < block_3_1_shape_1->arr_width; j++) {
      if (i == 0 && j == 0 || i == 0 && j == 2) {
        block_3_1_shape_1->arr[i][j] = 0;
      } else {
        block_3_1_shape_1->arr[i][j] = 1;
      }
    }
  }
 
  /*
   * [[1, 0],
   *  [1, 1],
   *  [1, 0]]
   */
  struct orientation* block_3_1_shape_2 = (orientation*)malloc(sizeof(orientation));
  block_3_1_shape_2->arr_height = 3;
  block_3_1_shape_2->arr_width = 2;
  for (int i = 0; i < block_3_1_shape_2->arr_height; i++) {          
    for (int j = 0; j < block_3_1_shape_2->arr_width; j++) {
      if (i == 0 && j == 1 || i == 2 && j == 1) {
        block_3_1_shape_2->arr[i][j] = 0;
      } else {
        block_3_1_shape_2->arr[i][j] = 1;
      }
    }
  }
 
  /*
   * [[1, 1, 1],
   *  [0, 1, 0]]
   */
  struct orientation* block_3_1_shape_3 = (orientation*)malloc(sizeof(orientation));
  block_3_1_shape_3->arr_height = 2;
  block_3_1_shape_3->arr_width = 3;
  for (int i = 0; i < block_3_1_shape_3->arr_height; i++) {          
    for (int j = 0; j < block_3_1_shape_3->arr_width; j++) {
      if (i == 1 && j == 0 || i == 1 && j == 2) {
        block_3_1_shape_3->arr[i][j] = 0;
      } else {
        block_3_1_shape_3->arr[i][j] = 1;
      }
    }
  }
 
  /*
   * [[0, 1],
   *  [1, 1],
   *  [0, 1]]
   */
  struct orientation* block_3_1_shape_4 = (orientation*)malloc(sizeof(orientation));
  block_3_1_shape_4->arr_height = 3;
  block_3_1_shape_4->arr_width = 2;
  for (int i = 0; i < block_3_1_shape_4->arr_height; i++) {          
    for (int j = 0; j < block_3_1_shape_4->arr_width; j++) {
      if (i == 0 && j == 0 || i == 2 && j == 0) {
        block_3_1_shape_4->arr[i][j] = 0;
      } else {
        block_3_1_shape_4->arr[i][j] = 1;
      }
    }
  }
 
  // assigning circular dependency
  block_3_1_shape_1->default_shape = block_3_1_shape_1;
  block_3_1_shape_2->default_shape = block_3_1_shape_1;
  block_3_1_shape_3->default_shape = block_3_1_shape_1;
  block_3_1_shape_4->default_shape = block_3_1_shape_1;
  block_3_1_shape_1->rotate = block_3_1_shape_2;
  block_3_1_shape_1->reverse = block_3_1_shape_4;
  block_3_1_shape_2->rotate = block_3_1_shape_3;
  block_3_1_shape_2->reverse = block_3_1_shape_1;
  block_3_1_shape_3->rotate = block_3_1_shape_4;
  block_3_1_shape_3->reverse = block_3_1_shape_2;
  block_3_1_shape_4->rotate = block_3_1_shape_1;
  block_3_1_shape_4->reverse = block_3_1_shape_3;
  block_3_1->shape = block_3_1_shape_1;

  /*
   * 
   * NEW BLOCK
   * 
   */
 
  // block Z
  block_Z->height = 2;
  block_Z->width = 3;
  block_Z->next = block_S;
  block_Z->prev = block_3_1;

  // 2 different orientations of block_Z
  /*
   *    [[1, 1, 0],
   *     [0, 1, 1]]
   */
  struct orientation* block_Z_shape_1 = (orientation*)malloc(sizeof(orientation));
  block_Z_shape_1->arr_height = 2;
  block_Z_shape_1->arr_width = 3;
  for (int i = 0; i < block_Z_shape_1->arr_height; i++) {          
    for (int j = 0; j < block_Z_shape_1->arr_width; j++) {
      if (i == 0 && j == 2 || i == 1 && j == 0) {
        block_Z_shape_1->arr[i][j] = 0;
      } else {
        block_Z_shape_1->arr[i][j] = 1;
      }
    }
  }
 
  /*
   *    [[0, 1],
   *     [1, 1],
   *     [1, 0]]
   */
  struct orientation* block_Z_shape_2 = (orientation*)malloc(sizeof(orientation));
  block_Z_shape_2->arr_height = 3;
  block_Z_shape_2->arr_width = 2;
  for (int i = 0; i < block_Z_shape_2->arr_height; i++) {          
    for (int j = 0; j < block_Z_shape_2->arr_width; j++) {
      if (i == 0 && j == 0 || i == 2 && j == 1) {
        block_Z_shape_2->arr[i][j] = 0;
      } else {
        block_Z_shape_2->arr[i][j] = 1;
      }
    }
  }
 
  // assigning circular dependency
  block_Z_shape_1->default_shape = block_Z_shape_1;
  block_Z_shape_2->default_shape = block_Z_shape_1;
  block_Z_shape_1->rotate = block_Z_shape_2;
  block_Z_shape_1->reverse = block_Z_shape_2;
  block_Z_shape_2->rotate = block_Z_shape_1;
  block_Z_shape_2->reverse = block_Z_shape_1;
  block_Z->shape = block_Z_shape_1;

  /*
   * 
   * NEW BLOCK
   * 
   */
 
  // block S
  block_S->height = 2;
  block_S->width = 3;
  block_S->next = block_L;
  block_S->prev = block_Z;

  // 2 different orientations of block_S
  /*
   *    [[0, 1, 1],
   *     [1, 1, 0]]
   */
  struct orientation* block_S_shape_1 = (orientation*)malloc(sizeof(orientation));
  block_S_shape_1->arr_height = 2;
  block_S_shape_1->arr_width = 3;
  for (int i = 0; i < block_S_shape_1->arr_height; i++) {          
    for (int j = 0; j < block_S_shape_1->arr_width; j++) {
      if (i == 0 && j == 0 || i == 1 && j == 2) {
        block_S_shape_1->arr[i][j] = 0;
      } else {
        block_S_shape_1->arr[i][j] = 1;
      }
    }
  }
 
  /*
   *    [[1, 0],
   *     [1, 1],
   *     [0, 1]]
   */
  struct orientation* block_S_shape_2 = (orientation*)malloc(sizeof(orientation));
  block_S_shape_2->arr_height = 3;
  block_S_shape_2->arr_width = 2;
  for (int i = 0; i < block_S_shape_2->arr_height; i++) {          
    for (int j = 0; j < block_S_shape_2->arr_width; j++) {
      if (i == 0 && j == 1 || i == 2 && j == 0) {
        block_S_shape_2->arr[i][j] = 0;
      } else {
        block_S_shape_2->arr[i][j] = 1;
      }
    }
  }
 
  // assigning circular dependency
  block_S_shape_1->default_shape = block_S_shape_1;
  block_S_shape_2->default_shape = block_S_shape_1;
  block_S_shape_1->rotate = block_S_shape_2;
  block_S_shape_1->reverse = block_S_shape_2;
  block_S_shape_2->rotate = block_S_shape_1;
  block_S_shape_2->reverse = block_S_shape_1;
  block_S->shape = block_S_shape_1;

  /*
   * 
   * NEW BLOCK
   * 
   */
 
  // block L
  block_L->height = 2;
  block_L->width = 3;
  block_L->next = block_J;
  block_L->prev = block_S;

  // 4 different orientations of block_L
  /*
   * [[0, 0, 1],
   *  [1, 1, 1]]
   */
  struct orientation* block_L_shape_1 = (orientation*)malloc(sizeof(orientation));
  block_L_shape_1->arr_height = 2;
  block_L_shape_1->arr_width = 3;
  for (int i = 0; i < block_L_shape_1->arr_height; i++) {          
    for (int j = 0; j < block_L_shape_1->arr_width; j++) {
      if (i == 0 && j == 0 || i == 0 && j == 1) {
        block_L_shape_1->arr[i][j] = 0;
      } else {
        block_L_shape_1->arr[i][j] = 1;
      }
    }
  }
 
  /*
   * [[1, 0],
   *  [1, 0],
   *  [1, 1]]
   */
  struct orientation* block_L_shape_2 = (orientation*)malloc(sizeof(orientation));
  block_L_shape_2->arr_height = 3;
  block_L_shape_2->arr_width = 2;
  for (int i = 0; i < block_L_shape_2->arr_height; i++) {          
    for (int j = 0; j < block_L_shape_2->arr_width; j++) {
      if (i == 0 && j == 1 || i == 1 && j == 1) {
        block_L_shape_2->arr[i][j] = 0;
      } else {
        block_L_shape_2->arr[i][j] = 1;
      }
    }
  }
 
  /*
   * [[1, 1, 1],
   *  [1, 0, 0]]
   */
  struct orientation* block_L_shape_3 = (orientation*)malloc(sizeof(orientation));
  block_L_shape_3->arr_height = 2;
  block_L_shape_3->arr_width = 3;
  for (int i = 0; i < block_L_shape_3->arr_height; i++) {          
    for (int j = 0; j < block_L_shape_3->arr_width; j++) {
      if (i == 1 && j == 1 || i == 1 && j == 2) {
        block_L_shape_3->arr[i][j] = 0;
      } else {
        block_L_shape_3->arr[i][j] = 1;
      }
    }
  }
 
  /*
   * [[1, 1],
   *  [0, 1],
   *  [0, 1]]
   */
  struct orientation* block_L_shape_4 = (orientation*)malloc(sizeof(orientation));
  block_L_shape_4->arr_height = 3;
  block_L_shape_4->arr_width = 2;
  for (int i = 0; i < block_L_shape_4->arr_height; i++) {          
    for (int j = 0; j < block_L_shape_4->arr_width; j++) {
      if (i == 1 && j == 0 || i == 2 && j == 0) {
        block_L_shape_4->arr[i][j] = 0;
      } else {
        block_L_shape_4->arr[i][j] = 1;
      }
    }
  }
 
  // assigning circular dependency
  block_L_shape_1->default_shape = block_L_shape_1;
  block_L_shape_2->default_shape = block_L_shape_1;
  block_L_shape_3->default_shape = block_L_shape_1;
  block_L_shape_4->default_shape = block_L_shape_1;
  block_L_shape_1->rotate = block_L_shape_2;
  block_L_shape_1->reverse = block_L_shape_4;
  block_L_shape_2->rotate = block_L_shape_3;
  block_L_shape_2->reverse = block_L_shape_1;
  block_L_shape_3->rotate = block_L_shape_4;
  block_L_shape_3->reverse = block_L_shape_2;
  block_L_shape_4->rotate = block_L_shape_1;
  block_L_shape_4->reverse = block_L_shape_3;
  block_L->shape = block_L_shape_1;

  /*
   * 
   * NEW BLOCK
   * 
   */
 
  // block J
  block_J->height = 2;
  block_J->width = 3;
  block_J->next = block_4;
  block_J->prev = block_L;
 
  // 4 different orientations of block_J
  /*
   * [[1, 0, 0],
   *  [1, 1, 1]]
   */
  struct orientation* block_J_shape_1 = (orientation*)malloc(sizeof(orientation));
  block_J_shape_1->arr_height = 2;
  block_J_shape_1->arr_width = 3;
  for (int i = 0; i < block_J_shape_1->arr_height; i++) {          
    for (int j = 0; j < block_J_shape_1->arr_width; j++) {
      if (i == 0 && j == 1 || i == 0 && j == 2) {
        block_J_shape_1->arr[i][j] = 0;
      } else {
        block_J_shape_1->arr[i][j] = 1;
      }
    }
  }
 
  /*
   * [[1, 1],
   *  [1, 0],
   *  [1, 0]]
   */
  struct orientation* block_J_shape_2 = (orientation*)malloc(sizeof(orientation));
  block_J_shape_2->arr_height = 3;
  block_J_shape_2->arr_width = 2;
  for (int i = 0; i < block_J_shape_2->arr_height; i++) {          
    for (int j = 0; j < block_J_shape_2->arr_width; j++) {
      if (i == 1 && j == 1 || i == 2 && j == 1) {
        block_J_shape_2->arr[i][j] = 0;
      } else {
        block_J_shape_2->arr[i][j] = 1;
      }
    }
  }
 
  /*
   * [[1, 1, 1],
   *  [0, 0, 1]]
   */
  struct orientation* block_J_shape_3 = (orientation*)malloc(sizeof(orientation));
  block_J_shape_3->arr_height = 2;
  block_J_shape_3->arr_width = 3;
  for (int i = 0; i < block_J_shape_3->arr_height; i++) {          
    for (int j = 0; j < block_J_shape_3->arr_width; j++) {
      if (i == 1 && j == 0 || i == 1 && j == 1) {
        block_J_shape_3->arr[i][j] = 0;
      } else {
        block_J_shape_3->arr[i][j] = 1;
      }
    }
  }
 
  /*
   * [[0, 1],
   *  [0, 1],
   *  [1, 1]]
   */
  struct orientation* block_J_shape_4 = (orientation*)malloc(sizeof(orientation));
  block_J_shape_4->arr_height = 3;
  block_J_shape_4->arr_width = 2;
  for (int i = 0; i < block_J_shape_4->arr_height; i++) {          
    for (int j = 0; j < block_J_shape_4->arr_width; j++) {
      if (i == 0 && j == 0 || i == 1 && j == 0) {
        block_J_shape_4->arr[i][j] = 0;
      } else {
        block_J_shape_4->arr[i][j] = 1;
      }
    }
  }
 
  // assigning circular dependency
  block_J_shape_1->default_shape = block_J_shape_1;
  block_J_shape_2->default_shape = block_J_shape_1;
  block_J_shape_3->default_shape = block_J_shape_1;
  block_J_shape_4->default_shape = block_J_shape_1;
  block_J_shape_1->rotate = block_J_shape_2;
  block_J_shape_1->reverse = block_J_shape_4;
  block_J_shape_2->rotate = block_J_shape_3;
  block_J_shape_2->reverse = block_J_shape_1;
  block_J_shape_3->rotate = block_J_shape_4;
  block_J_shape_3->reverse = block_J_shape_2;
  block_J_shape_4->rotate = block_J_shape_1;
  block_J_shape_4->reverse = block_J_shape_3;
  block_J->shape = block_J_shape_1;

  //seed for random function later on
  randomSeed(analogRead(9));
}
 
/*
 * GLOBAL VARIABLES
 */
 
struct node* curr = (node*)malloc(sizeof *curr);
struct node* next_block = (node*)malloc(sizeof *next_block);
int delay_value = 500;
int first_run = 0;
int lines_finished = 0;
 
void loop(void) {
  check_game_end();
 
  if (first_run == 0) {
    create_first_block();
  }

  //set the new block that is dropping to be the old next_block
  curr = next_block;

  //randomise next_block to be a new block
  randomise_next_block();

  //the next_block is always displayed in the top left
  //of the screen (the 5x4 section of pixels)
  //clear this to prepare it for the new next_block
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 4; j ++) {
      mx.setPoint(j, i, false);
    }
  }

  //display the next_block
  for (int i = 0; i < next_block->height; i++) {
    for (int j = 0; j < next_block->width; j++) {
      if (next_block->shape->arr[i][j] == 1) {
        mx.setPoint(j+1, i+1, true);
      }
    }
  }

  bool hit = false;
  int block_dropping_column = 3;

  //this loop will drop the block from the top (row 8) to the bottom (row 32)
  //unless there is a collision (which will be explored later)
  for (int i = 8; i < 32; i++) {
   //check if any lines need to be cleareds
   if (i == 8) {
     check_cleared_lines();
   }
   
   //setup accelerometer value for this row
   String control_direction = check_control();

   //setup booleans which are needed for later
   bool able_move_right = false;
   bool able_move_left = false;
   bool valid_move = true;
   bool change_delay = false;
   bool rotate_block = false;

   //if accelerometer value was "RIGHT"   
   if (control_direction == "RIGHT") {
     //need to check if every right-most block can move right one pixel
     for (int h = 0; h < curr->height; h++) {
       //i-h > 7 -> ensuring the block has finished spawning AND
       //curr->shape->arr[h][curr->width - 1] == 1 -> ensure that the pixel we are dealing with is a '1' in the shape array
       //mx.getPoint(block_dropping_column+curr->width, i-h) == true -> if the pixel to the right of the above ^ is lit up (therefore it cannot move right)
       //block_dropping_column + curr->width > 7 -> this will be off the screen
       if (i-h > 7 && (curr->shape->arr[h][curr->width - 1] == 1) && (mx.getPoint(block_dropping_column + curr->width, i-h) == true) || block_dropping_column + curr->width > 7) {
         //ensure this move is not allowed
         valid_move = false;
         break;
       }
     }
     //if the move was allowed, allow the right movement
     //increment the column of dropping (moves the block right)
     if (valid_move == true) {
       able_move_right = true;
       block_dropping_column++;
     }
   } else if (control_direction == "LEFT") {
     //need to check if every left-most block can move left one pixel
     for (int h = 0; h < curr->height; h++) {
       //i-h > 7 -> ensuring the block has finished spawning AND
       //curr->shape->arr[h][0] == 1 -> ensure that the pixel we are dealing with is a '1' in the shape array
       //mx.getPoint(block_dropping_column - 1, i-h) == true -> if the pixel to the left of the above ^ is lit up (therefore it cannot move left)
       //block_dropping_column - 1 < 0 -> this will be off the screen
       if (i-h > 7 && (curr->shape->arr[h][0] == 1) && (mx.getPoint(block_dropping_column - 1, i-h) == true || block_dropping_column - 1 < 0)) {
         //ensure this move is not allowed
         valid_move = false;
         break;
       }
     }
     //if the move was allowed, allow the left movement
     //decrement the column of dropping (moves the block left)
     if (valid_move == true) {
       able_move_left = true;
       block_dropping_column--;
     }
    } else if (control_direction == "BACK") {
      //inform the program to reduce the delay amount to drop the block faster
      change_delay = true;
    } else if (control_direction == "FORWARD") {
      bool can_rotate = true;
      //remove block from the board, it will trigger this check which we dont want
      int row_counter = 0;
      for (int k = curr->height-1; k >= 0; k--) {
        for (int j = 0; j < curr->width; j++) {
          //if value in the shape array is '1'
          if (curr->shape->arr[k][j] == 1) {
            //ensure we are out of spawn
            if (i-row_counter > 7) {
              //turn off pixel
              //k is decrementing so we cannot use it for height values
              //we use row_counter instead
              mx.setPoint(block_dropping_column + j, i-row_counter-1, false);
            }
          }
        }
        row_counter = row_counter + 1;
      }

      //change the orientation of the block (rotate it)
      curr->shape = curr->shape->rotate;
      curr->width = curr->shape->arr_width;
      curr->height = curr->shape->arr_height;

      //now attempt to redraw the block in new orientation
      row_counter = 0;
      for (int k = curr->height-1; k >= 0; k--) {
        for (int j = 0; j < curr->width; j++) {
          if (curr->shape->arr[k][j] == 1) {
            if (i-row_counter > 7) {
              //mx.getPoint(block_dropping_column + j, i-row_counter-1) == true -> if any of the pixels we try to draw to are already lit
              //or if we go out of bounds
              if (mx.getPoint(block_dropping_column + j, i-row_counter-1) == true || block_dropping_column + j > 7 || block_dropping_column + j < 0) {
                can_rotate = false;
                break;
              }
            }
          }
        }
        if (can_rotate == false) {
          break;
        }
        row_counter = row_counter + 1;
      }
      //if we could not redraw the block in new orientation, then reverse the rotation
      if (can_rotate == false) {
        curr->shape = curr->shape->reverse;
        curr->width = curr->shape->arr_width;
        curr->height = curr->shape->arr_height;
      }

      //now redraw the block
      int row_counter = 0;
      for (int k = curr->height-1; k >= 0; k--) {
        for (int j = 0; j < curr->width; j++) {
          //if value in the shape array is '1'
          if (curr->shape->arr[k][j] == 1) {
            //ensure we are out of spawn
            if (i-row_counter > 7) {
              //turn off pixel
              //k is decrementing so we cannot use it for height values
              //we use row_counter instead
              mx.setPoint(block_dropping_column + j, i-row_counter-1, true);
            }
          }
        }
        row_counter = row_counter + 1;
      }
    }
    
    //check for a downward collsion
    //iterate through the width of the block
    for (int j = 0; j < curr->width; j++) {
      //counter, this finds the lowest most lit pixel for every row in the block
      int counter = 0;
      while(curr->shape->arr[curr->height-counter-1][j] == 0) {
        counter = counter + 1;
      }
      //if the next block is filled i.e. there will be a collision
      //(curr->shape->arr[curr->height-counter-1][j] == 1) -> the index of the block is filled
      //mx.getPoint(block_dropping_column + j, i-counter) -> the next pixel below it
      //i-counter > 7 -> ensuring the block is out of the spawn point
      if ((curr->shape->arr[curr->height-counter-1][j] == 1) && mx.getPoint(block_dropping_column + j, i-counter) == true && i-counter > 7) {
        hit = true;
        break;
      }
    }
    //check if a collision has occured
    if (hit == true) {
      delay(1000);
      break;
    }
    //change the delay value to be 20 when the user called move "BACK"
    //else reset it
    if (change_delay == true) {
      delay_value = 20;
    } else {
      delay_value = 500;
    }
    
    //move the whole block down now
    //all collisions/rotations have been checked and cleared by now
    //no need to check them again
    int row_counter = 0;
    for (int k = curr->height-1; k >= 0; k--) {
      for (int j = 0; j < curr->width; j++) {
        if (curr->shape->arr[k][j] == 1) {
          if (i+row_counter > 7) {
            mx.setPoint(block_dropping_column + j, i-row_counter, true);
            if (control_direction == "LEFT" && able_move_left) {
              mx.setPoint(block_dropping_column + j+1, i-1-row_counter, false);
            } else if (control_direction == "RIGHT" && able_move_right) {
              mx.setPoint(block_dropping_column + j-1, i-1-row_counter, false);
            } else {
              mx.setPoint(block_dropping_column + j, i-1-row_counter, false);
            }
          }
        }
      }
      row_counter = row_counter + 1;
    }  
    draw_end_line();
    if (i == 31 && control_direction == "BACK") {
      delay(1000);
    } else {
      delay(delay_value-(lines_finished*10));
    }
    mx.update();
  }
}

void create_first_block() {
  //increment first run to ensure it doesnt run again
  first_run = first_run + 1;

  //initialise next_block as a block, it doesnt matter which one
  next_block = block_S;
  randomise_next_block();
}

void randomise_next_block() {
  //the arr in each shape is declared to be arr[10][10]
  //we only ever use the first 1-4 of each index, meaning
  //the remaining 5-9 are 'junk' and will spew out huge random
  //integers when called, which is VERY useful for a truly random function

  //do ->next a random amount of times to get a random block
  for (int i = 0; i < random(abs(next_block->shape->arr[random(0, 9)][random(0, 9)])); i++) {
    next_block = next_block->next;
  }
  next_block->shape = next_block->shape->default_shape;
  next_block->width = next_block->shape->arr_width;
  next_block->height = next_block->shape->arr_height;
}

void check_game_end() {
  //check if any of the LED's in the 'end game row' are lit up
  for (int i = 0; i < 8; i++) {
    //if (above comment) is lit up
    if (mx.getPoint(i, 8) == true) {
      //clear the board and score
      mx.clear();
      clear_score();
      display_score();

      //print score to console
      Serial.println("Score:");
      Serial.println(lines_finished);

      //this shows the score for 7.5 seconds
      mx.update();
      delay(7500);

      //reset global variables for a new game
      lines_finished = 0;
      first_run = 0;

      //setup new game
      clear_score();
      draw_end_line();
    }
  }
}

void clear_score() {
  for (int i = 0; i < 8; i++) {
      mx.setPoint(i, 5, false);
  }
}

void draw_end_line() {
  for (int i = 0; i < 8; i++) {
      mx.setPoint(i, 7, true);
  }
}
 
void check_cleared_lines() {
  int rows_cleared = 0;
  //check if any row is full
  for (int i = 8; i < 32; i++) {
    bool full_row = true;
    for (int j = 0; j < 8; j++) {
      if (mx.getPoint(j, i) == false) {
        full_row = false;
        break;
      }
    }

    //if a row is full, clear it and increment rows_cleared
    if (full_row == true) {
      for (int j = 0; j < 8; j++) {
        mx.setPoint(j, i, false);
      }
      rows_cleared = rows_cleared + 1;
    }
  }

  //increment global variable
  lines_finished = lines_finished + rows_cleared;

  //update screen to show new board
  delay(50);
  mx.update();

  //increment from bottom row to top of screen
  int starting_line = 31;
  for (int i = 31; i >= 8; i--) {
    bool empty_row = true;
    //check if a row is empty
    for (int j = 0; j < 8; j++) {
      if (mx.getPoint(j, i) == true) {
        empty_row = false;
        break;
      }
    }

    //if row was NOT empty
    if (empty_row == false) {
      //int starting_line starts at the bottom (line 31)
      //for each NON empty line, it is decremented
      //this variable is used to fill up the board from the bottom up
      //it ignores empty lines and hence clears the lines as required
      for (int j = 0; j < 8; j++) {
        if (mx.getPoint(j, i) == true) {
          mx.setPoint(j, starting_line, true);
        } else {
          mx.setPoint(j, starting_line, false);
        }
      }
      starting_line = starting_line - 1;
    }
  }

  //clear up everything else
  for (int i = starting_line; i >= 8; i--) {
    for (int j = 0; j < 8; j++) {
      mx.setPoint(j, i, false);
    }
  }
  clear_score();
  display_score();
  mx.update();
}

void display_score() {
  //determine the score in binary
  int binaryBitsCount = lines_finished * 8;
  char str[binaryBitsCount + 1];
  itoa(lines_finished, str, 2);

  //display score on the blank screen
  int counter = 0;
  for (int i = binaryBitsCount; i >= 0; i--) {
    if (str[i] == '0') {
      mx.setPoint(7-counter, 5, false);
      counter = counter + 1;
    }
    if (str[i] == '1') {
      mx.setPoint(7-counter, 5, true);
      counter = counter + 1;
    }
  }
}
 
String check_control() {
  Wire.beginTransmission(ADXL345_Adresse);
  Wire.write(DATAX0);
  Wire.endTransmission();
  Wire.beginTransmission(ADXL345_Adresse);
  Wire.requestFrom(ADXL345_Adresse, 6); // Recovery of the 6 components
  i=0;
 
  while (Wire.available()) {
    buffer[i] = Wire.read();
    i++;
  }
  Wire.endTransmission();
  xValue=(buffer[1] << 8) | buffer[0]; // Development of the 3 components
  yValue=(buffer[3] << 8) | buffer[2];
    
  if ((xValue > 65500 || xValue < 100) && (yValue > 65400 && yValue < 65500)) {
    return "RIGHT";
  } else if ((xValue > 65500 || xValue < 100) && yValue < 100) {
    return "LEFT";
  } else if ((xValue < 120 && xValue > 50) && (yValue > 65500 || yValue < 150)) {
    return "FORWARD";
  } else if ((xValue > 65400 && xValue < 65520) && (yValue > 65500 || yValue < 100)) {
    return "BACK";
  } else {
    return "STILL";
  } 
}
