#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include <AFMotor.h>
#include <Servo.h> 
#include <SoftwareSerial.h>

#define ROW 4
#define COL 4

// ---- Motor and MPU ----
AF_DCMotor WBL(1);
AF_DCMotor WBR(2);
AF_DCMotor WFR(3);
AF_DCMotor WFL(4);
MPU6050 mpu(Wire);

Servo servo1;
int trig = A3;
int echo_f = 9;
int echo_l = 10;
int echo_r = A2;
int distance;
int distance_f;
int distance_l;
int distance_r;
int vicinity[3];

int motorSpeed = 230;
unsigned long timer = 0;
int measured_angle = 0;
int target_angle = 0;
int delta_angle;
int carry_angle = 0;
float accY, accX;
int SFR = motorSpeed;
int SFL = motorSpeed;
int SBR = motorSpeed;
int SBL = motorSpeed;
String currentDirection = "forward" ; 
int count = 0;
int enCount = 0;

int ninety = 89;

// ---- grids ----
int grid_explore[ROW][COL] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

// int grid_explore[ROW][COL] = {
//   {0, 0, 0, 0},
//   {0, 0, 0, 0},
//   {0, 3, 1, 1},
//   {1, 1, 2, 1}
// };

bool visited[ROW][COL];
struct Cell {
  int row;
  int col;
  int dist;
  String path;
};

int obstacle_num = 2;
int current_location_num = 3;
int explored_num = 1;
int metal_num = 4;

String serializedData;

//process path
String tempWord;
int spaceIndex;

// for arduino internal , not for grid
int current_row = ROW - 1;
int current_col = 0;
bool all_explored = false;

SoftwareSerial mySerial(A0,A1); //rx, tx, A0 connects to 1(tx)

// ---- setup ----
void setup(){
  servo1.attach(10);
  pinMode(trig, OUTPUT);
  pinMode(echo_f, INPUT);
  pinMode(echo_r, INPUT);
  pinMode(echo_l, INPUT);

  mySerial.begin(9600);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(2), counter, RISING);

  Wire.begin();

  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!= 0){
    Serial.print(".");
    delay(100);
    byte status = mpu.begin();
  }
  Serial.println("");
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(500);
  mpu.calcOffsets();
  Serial.println("Done Calibration!\n");

  // update server and port
  mySerial.println("3.15.231.142,8080");
  delay(100);
}

void loop(){

  while(!all_explored){
    Serial.println("------------------------");
    Serial.print("CURRENT POSITION: ");
    Serial.print(current_row);
    Serial.print("-");
    Serial.print(current_col);
    Serial.print("    ");
    Serial.print("CURRENT DIRECTION: ");
    Serial.println(currentDirection);

    String shortest_path = get_path_from_wemos();
    Serial.println(shortest_path);
    processPath(shortest_path);
    
    update_location(current_row, current_col);

    printGrid(grid_explore, ROW, COL);

    all_explored = check_all_explored();
  }
  
}


// ------------ END VOID LOOP ---------------


// ------------ SEND DATA ---------------------
void serialize_data(){
  serializedData = "";
  for (int i = 0; i < ROW; i++) {
    for (int j = 0; j < COL; j++) {
      serializedData += String(grid_explore[i][j]) + ",";
    }
  }
  serializedData += String(current_row) + ",";
  serializedData += String(current_col);
}

String get_path_from_wemos(){
  serialize_data();
  delay(10);
  // Serial.println(serializedData);
  mySerial.println(serializedData);
  delay(100);
  while(!mySerial.available()){
    Serial.print(".");
    delay(100);
  }
  if (mySerial.available()){
    String receivedString = "";
    Serial.println();
    while(mySerial.available()){
      char c = mySerial.read();
      receivedString += c;
    }
    return receivedString;
  }
}

void update_to_server(){
  serialize_data();
  String jsonData = "{";
  jsonData += "\"grid\":";
  jsonData += "\""+serializedData+"\",";
  jsonData += "\"post\":";
  jsonData += "\"post\"";
  jsonData += "}";

  delay(10);
  mySerial.println(jsonData);
  Serial.println("Updated to server.");

}

// ----------------- SERVO and Ultrasonic -------------------

void sense_surrounding(){
  for (int u=0; u<3;u++){
    vicinity[u]=0;
  }
  // servo1.write(0);
  // delay(500);
  distance = get_distance(echo_r);
  delay(100);
  if (distance < 25){
    set_obstacle(0);
    vicinity[0] = 1;
  }
  // servo1.write(90);
  // delay(500);
  distance = get_distance(echo_f);
  delay(100);
  if (distance < 25){
    set_obstacle(90);
    vicinity[1] = 1;
  }
  // servo1.write(180);
  // delay(500);
  distance = get_distance(echo_l);
  delay(100);
  if (distance < 25){
    set_obstacle(180);
    vicinity[2] = 1;
  }
  delay(500);
}

int get_distance(int echo_pin){
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  int duration = pulseIn(echo_pin, HIGH);
  int distance = duration/29/2;
  return distance;
}

void set_obstacle(int degree){
  if (currentDirection == "up"){
    if (degree == 0 && current_col < COL){
      update_obstacle(current_row, current_col + 1);
    } else if (degree == 90 && current_row > 0) {
      update_obstacle(current_row -1, current_col);
    } else if (degree == 180 && current_col > 0){
      update_obstacle(current_row, current_col - 1);
    }
  } else if (currentDirection == "down"){
    if (degree == 0 && current_col > 0){
      update_obstacle(current_row, current_col - 1);
    } else if (degree == 90 && current_row < ROW){
      update_obstacle(current_row + 1, current_col);
    } else if (degree == 180 && current_col < COL){
      update_obstacle(current_row, current_col + 1);
    }
  } else if (currentDirection == "forward"){
    if (degree == 0 && current_row < ROW){
      update_obstacle(current_row + 1, current_col);
    } else if (degree == 90 && current_col < COL){
      update_obstacle(current_row, current_col + 1);
    } else if (degree == 180 && current_row > 0){
      update_obstacle(current_row -1, current_col);
    }
  } else if (currentDirection == "backward"){
    if (degree == 0 && current_row > 0){
      update_obstacle(current_row - 1, current_col);
    } else if (degree == 90 && current_col > 0){
      update_obstacle(current_row, current_col - 1);
    } else if (degree == 180 && current_row < ROW){
      update_obstacle(current_row + 1, current_col);
    }
  }
}

int check_all_explored(){
  for (int i=0; i<ROW; i++){
    for (int j=0; j<COL;j++){
      if (grid_explore[i][j] == 0){
        return false;
      }
    }
  }
  return true;
}

// ---------------- Process the string from BFS -----------------
void processPath(String pathString){
  pathString.trim();
  while (pathString.length() != 0){
    spaceIndex = pathString.indexOf(" ");
    if (spaceIndex == -1){
      tempWord = pathString;
      pathString = "";
    } else {
      tempWord = pathString.substring(0, spaceIndex);
      pathString = pathString.substring(spaceIndex + 1);
    }
    tempWord.trim();
    sense_surrounding();
    delay(500);
    if (tempWord == "forward" && grid_explore[current_row][current_col + 1] == obstacle_num){Serial.println("SAW OBSTACLE"); break;}
    if (tempWord == "backward" && grid_explore[current_row][current_col - 1] == obstacle_num){Serial.println("SAW OBSTACLE"); break;}
    if (tempWord == "up" && grid_explore[current_row - 1][current_col] == obstacle_num){Serial.println("SAW OBSTACLE"); break;}
    if (tempWord == "down" && grid_explore[current_row + 1][current_col] == obstacle_num){Serial.println("SAW OBSTACLE"); break;}

    Serial.print("Moving: ");
    Serial.println(tempWord);
    decideTurn(tempWord); //  current position is changed here
    delay(100);

    moveOneFoot();
    update_location(current_row, current_col);
    delay(500);
    update_to_server();
    delay(500);
  }
}

void update_obstacle(int row, int col){
  grid_explore[row][col] = obstacle_num;
}

void update_location(int row, int col){ // use this in moveOneFoot
  grid_explore[row][col] = current_location_num;
}

void update_explored(int row, int col){ //use this in moveOneFoot function
  // if (currentDirection == "backward"){
  //   return; //change
  // }
  grid_explore[row][col] = explored_num;
}

//// ---------- MOTOR -----------
void decideTurn(String dir){
  update_explored(current_row, current_col);
  int turn_state = 0;
  if (dir == "forward"){
    enCount = 0;
    current_col += 1;

    if (currentDirection == "forward"){
      return;
    } else if (currentDirection == "backward"){
      while(turn_state != 1){
        turn_state = turnCCW(ninety);
      }
      turn_state = 0;
      while(turn_state != 1){
        turn_state = turnCCW(ninety);
      }
      currentDirection = "forward";
      return;
    } else if (currentDirection == "up"){
      while(turn_state != 1){
        turn_state = turnCW(ninety);
      }
      currentDirection = "forward";
      return;
    } else if (currentDirection == "down"){
      while(turn_state != 1){
        turn_state = turnCCW(ninety);
      }
      currentDirection = "forward";
      return;
    }
  }

  // backward
  if (dir == "backward"){
    enCount = 0;
    current_col -= 1;
    if (currentDirection == "forward"){
      while(turn_state != 1){
        turn_state = turnCCW(ninety);
      }
      turn_state = 0;
      while(turn_state != 1){
        turn_state = turnCCW(ninety);
      }
      currentDirection = "backward";
      return;
    } else if (currentDirection == "backward"){
      return;
    } else if (currentDirection == "up"){
      while(turn_state != 1){
        turn_state = turnCCW(ninety);
      }
      currentDirection = "backward";
      return;
    } else if (currentDirection == "down"){
      while(turn_state != 1){
        turn_state = turnCW(ninety);
      }
      currentDirection = "backward";
      return;
    }
  }

  // up
  if (dir == "up"){
    enCount = -1;
    current_row -= 1;
    if (currentDirection == "forward"){
      while(turn_state != 1){
        turn_state = turnCCW(ninety);
      }
      currentDirection = "up";
      return;
    } else if (currentDirection == "backward"){
      while(turn_state != 1){
        turn_state = turnCW(ninety);
      }
      currentDirection = "up";
      return;
    } else if (currentDirection == "up"){
      return;
    } else if (currentDirection == "down"){
      while(turn_state != 1){
        turn_state = turnCCW(ninety);
      }
      turn_state = 0;
      while(turn_state != 1){
        turn_state = turnCCW(ninety);
      }
      currentDirection = "up";
      return;
    }
  }

  //down
  if (dir == "down"){
    enCount = -1;
    current_row += 1;
    if (currentDirection == "forward"){
      while(turn_state != 1){
        turn_state = turnCW(ninety);
      }
      currentDirection = "down";
      return;
    } else if (currentDirection == "backward"){
      while(turn_state != 1){
        turn_state = turnCCW(ninety);
      }
      currentDirection = "down";
      return;
    } else if (currentDirection == "up"){
      while(turn_state != 1){
         turn_state = turnCCW(ninety);     
      }
      turn_state = 0;
      while(turn_state != 1){
         turn_state = turnCCW(ninety);     
      }
      currentDirection = "down";
      return;
    } else if (currentDirection == "down"){
      return;
    }
  }
  Serial.println("failed path processing");
  return;
}

void moveOneFoot(){
  count = 0;
  // MPU6050 mpu(Wire);
  int status = mpu.begin();
  mpu.calcOffsets();
  mpu.resetAngleZ();
  
  if (enCount > 0){
    enCount = 0;
  } else if (enCount < 0){
    enCount = enCount;
  } else{
    enCount = 1;
  }
  while(count < (25 + enCount)){
    mpu.update();
    measured_angle = round(mpu.getAngleZ());
    if (measured_angle > 0){
      runMotor(0,255,0,255,1,1); //clockwise
    } else if (measured_angle < 0) {
      runMotor(255,0,255,0,1,1); //counter clockwise
    } else if (measured_angle == 0){
      runMotor(225,225,225,225,1,1);
    }

  }
  count = 0;
  Serial.println("moved one foot");
  releaseMotor();
}


int turnCCW(int targetAngle){
  Serial.println("CCW");
  targetAngle -= carry_angle;
  // MPU6050 mpu(Wire);
  byte status = mpu.begin();
  Serial.println(status);
  mpu.calcOffsets();
  mpu.resetAngleZ();
  SFR = motorSpeed;
  SFL = motorSpeed;
  SBR = motorSpeed;
  SBL = motorSpeed;
  mpu.update();
  measured_angle = round(mpu.getAngleZ()) % 360;
  delta_angle = targetAngle - measured_angle;
  releaseMotor();
  delay(500);
  while(delta_angle != 0){
    mpu.update();
    measured_angle = round(mpu.getAngleZ());
    delta_angle = targetAngle - measured_angle;
    adjustMotorSpeedWhileTurn(delta_angle);
    if (delta_angle  > 0){
      runMotor(SFR,SFL,SBR,SBL,1,0); //counter clockwise
    } else if (delta_angle < 0){
      runMotor(SFR,SFL,SBR,SBL,0,1); //clockwise
    } else if (delta_angle == 0 ){
      releaseMotor();
      break;
    }
  }
  releaseMotor();
  for (int i = 0; i < 100; i++){
    mpu.update();
    measured_angle = round(mpu.getAngleZ());
    if (targetAngle ==0){
      targetAngle = 360;
    }
    delta_angle = targetAngle - measured_angle;
    adjustMotorSpeedWhileTurn(delta_angle);
    if (delta_angle  > 0){
      runMotor(SFR,SFL,SBR,SBL,1,0); //counter clockwise
    } else if (delta_angle < 0){
      runMotor(SFR,SFL,SBR,SBL,0,1); //clockwise
    } else if (delta_angle == 0){
      releaseMotor();
    }
  }
  releaseMotor();
  mpu.update();
  measured_angle = round(mpu.getAngleZ());
  carry_angle = measured_angle - targetAngle;
  count = 0;
  return 1;
}


int turnCW(int targetAngle){
  Serial.println("CW");
  targetAngle += carry_angle;
  // MPU6050 mpu(Wire);
  int status = mpu.begin();
  mpu.calcOffsets();
  mpu.resetAngleZ();
  SFR = motorSpeed;
  SFL = motorSpeed;
  SBR = motorSpeed;
  SBL = motorSpeed;
  mpu.update();
  measured_angle = round(mpu.getAngleZ());
  delta_angle = targetAngle + measured_angle;
  releaseMotor();
  delay(500);
  while(delta_angle != 0){
    mpu.update();
    measured_angle = round(mpu.getAngleZ());
    delta_angle = measured_angle + targetAngle ;
    adjustMotorSpeedWhileTurn(delta_angle);
    if (delta_angle  < 0){
      runMotor(SFR,SFL,SBR,SBL,1,0); //counter clockwise
    } else if (delta_angle > 0){
      runMotor(SFR,SFL,SBR,SBL,0,1); //clockwise
    } else if (delta_angle == 0 ){
      releaseMotor();
      break;
    }
  }
  releaseMotor();
  for (int i = 0; i < 100; i++){
    mpu.update();
    measured_angle = round(mpu.getAngleZ()) % 360;
    if (targetAngle ==0){
      targetAngle = 360;
    }
    delta_angle = measured_angle + targetAngle ;
    adjustMotorSpeedWhileTurn(delta_angle);
    if (delta_angle  < 0){
      runMotor(SFR,SFL,SBR,SBL,1,0); //counter clockwise
    } else if (delta_angle > 0){
      runMotor(SFR,SFL,SBR,SBL,0,1); //clockwise
    } else if (delta_angle == 0){
      releaseMotor();
    }
  }
  releaseMotor();
  mpu.update();
  measured_angle = round(mpu.getAngleZ());
  carry_angle = measured_angle + targetAngle;
  count = 0;
  return 1;
}


void adjustMotorSpeedWhileTurn(int delta_angle){
    int minimum_turn_speed = 220;
    if (delta_angle < 90){
      SFR = 255;
      SFL = 255;
      SBR = 255;
      SBL = 255;
    }
    if (delta_angle < 50){
      SFR = 235;
      SFL = 235;
      SBR = 235;
      SBL = 235;
    }

    if (delta_angle < 5){
      SFR = minimum_turn_speed - 30;
      SFL = minimum_turn_speed - 30;
      SBR = minimum_turn_speed - 30;
      SBL = minimum_turn_speed - 30;
    }   
   
}


void runMotor(int SFR, int SFL, int SBR, int SBL, int dirR, int dirL){
  WBL.setSpeed(SBL);
  WBR.setSpeed(SBR);
  WFL.setSpeed(SFL);
  WFR.setSpeed(SFR);

  if (dirR == 1){
    WFR.run(FORWARD);
    WBR.run(FORWARD);
  } else if (dirR == 0){
    WFR.run(BACKWARD);
    WBR.run(BACKWARD);
  }

  if (dirL == 1){
    WFL.run(FORWARD);
    WBL.run(FORWARD);
  } else if (dirL == 0){
    WFL.run(BACKWARD);
    WBL.run(BACKWARD);
  }
}

void releaseMotor(){
  WBL.run(RELEASE);
  WBR.run(RELEASE);
  WFL.run(RELEASE);
  WFR.run(RELEASE);
}

/// ------------- END MOTOR ---------------

void counter(){
  count++;
}

/// ---------- print grid -------------
void printGrid(int grid[][COL], int numRows, int numCols) {
  for (int i = 0; i < numRows; i++) {
    for (int j = 0; j < numCols; j++) {
      Serial.print(grid[i][j]);
      Serial.print(" ");
    }
    Serial.println(); // Move to the next line after printing each row
  }
}
