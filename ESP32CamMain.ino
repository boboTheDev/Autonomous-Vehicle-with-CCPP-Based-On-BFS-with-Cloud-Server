#include <WiFi.h>
#include <HTTPClient.h>

#define ROW 4
#define COL 4

int grid_explore[ROW][COL];
bool visited[ROW][COL];
struct Cell {
  int row;
  int col;
  int dist;
  String path;
};

int current_row;
int current_col;
int target_row;
int target_col;

int obstacle_num = 2;
int metal_num = 4;

const char* ssid = "XXX"; //wifi name
const char* password = "XXXX"; //wifi password

String serverIP;
String serverPort;
String serverName;


WiFiClient client;
String serverUrl = "/updategrid/";

void setup() {
  pinMode(4, OUTPUT); //built-in LED
  digitalWrite(4, LOW);
  Serial.begin(9600);
  Serial.println("PROGRAM STARTS.");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
  }
  flash_LED(3);
}

void loop() {
  if (Serial.available()){
    String receivedString = Serial.readStringUntil('\n');

    if (receivedString.indexOf("post") != -1){
      post_grid(receivedString);
      flash_LED(1);
    } else if (receivedString.indexOf("8080") != -1) {
      int commaIndex = receivedString.indexOf(",");
      serverIP = receivedString.substring(0, commaIndex);
      serverPort = receivedString.substring(commaIndex + 1);
      serverName = "http://"+serverIP+":"+serverPort+"/";
    } else {
      get_all_data(receivedString);
      flash_LED(2);
    }
  }
}

void post_grid(String strBody){
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverName+"updategrid/");

  http.addHeader("Content-Type","application/json");
  http.addHeader("Content-Length",String(strBody.length()+6));

  int httpResponseCode = http.POST(strBody);

  // if(client.connect(serverName.c_str(), serverPort)){
  //   client.println("POST " + serverUrl + " HTTP/1.1");
  //   client.println("Host: " + serverName+":"+serverPort);
  //   client.println("Content-Length: " + String(strBody.length()+6));
  //   client.println("Content-Type: application/json");
  // }
}



void flash_LED(int counter){
  for (int i = 0; i<counter; i++){
    digitalWrite(4, HIGH);
    delay(50);
    digitalWrite(4, LOW);
    delay(50);
  }
  
}


void get_all_data(String receivedString){
  int index = 0;
  for (int i = 0; i < ROW; i++) {
    for (int j = 0; j < COL; j++) {
      int commaPos = receivedString.indexOf(',', index);
      String valueString = receivedString.substring(index, commaPos);
      grid_explore[i][j] = valueString.toInt();
      index = commaPos + 1;
    }
  }
  int commaPos = receivedString.indexOf(',', index);
  current_row = receivedString.substring(index, commaPos).toInt();
  index = commaPos + 1;
  current_col = receivedString.substring(index).toInt();

  String shortest_path = find_shortest_path(current_row, current_col);
  Serial.println(shortest_path);
  
}

/// --------------------------- BFS -------------------------------
String find_shortest_path(int current_row, int current_col){
  for (int i = ROW - 1; i >= 0; i-- ){
    int zeros_in_row[COL] = {9,9,9,9};
    int temp_index = 0;
    for (int j = 0; j < COL; j++){
      if (grid_explore[i][j] == 0){
        zeros_in_row[temp_index] = j;
        temp_index++;
      } 
    }
    if (sizeof(zeros_in_row) > 0){
      int closest_diff = sizeof(grid_explore[i]);
      int closest_zero = -1;
      for (int k = 0; k < COL; k++){
        if (zeros_in_row[k] != 9){
          int diff = abs(zeros_in_row[k] - current_col);
          if (diff < closest_diff){
            closest_zero = zeros_in_row[k];
            closest_diff = diff;
          }
        }
      }
      if (closest_zero != -1){
        int target_row = i;
        int target_col = closest_zero;
        String shortest_path = bfs(current_row, current_col, target_row, target_col);
        // Serial.print("TARGET PATH: ");
        // Serial.println(shortest_path);
        if (shortest_path.length() > 0){
          
          return shortest_path;
        } else{
          update_obstacle(target_row, target_col);
          continue; 
        }
      }      
    } else {
      continue;
    }
  }
  return "";
}

void update_obstacle(int row, int col){
  grid_explore[row][col] = obstacle_num;
}

bool isValid(int row, int col) {
  return (row >= 0 && row < ROW && col >= 0 && col < COL && grid_explore[row][col] != 2 && grid_explore[row][col] != 4 && !visited[row][col]);
}

String bfs(int s_row, int s_col, int e_row, int e_col) {
  memset(visited, false, sizeof(visited));
  visited[s_row][s_col] = true;

  Cell start = {s_row, s_col, 0, ""};
  Cell queue[ROW * COL];
  int front = 0;
  int rear = 0;
  queue[rear++] = start;

  int rowDir[] = {-1, 0, 0, 1};
  int colDir[] = {0, -1, 1, 0};
  String dir[] = {"up", "backward", "forward", "down"};

  while (front < rear) {
    Cell curr = queue[front++];
    if (curr.row == e_row && curr.col == e_col) {
      return curr.path;
    }

    for (int i = 0; i < 4; i++) {
      int row = curr.row + rowDir[i];
      int col = curr.col + colDir[i];
      if (isValid(row, col)) {
        visited[row][col] = true;
        // Serial.print("Next: ");
        // Serial.println(curr.path + " " + dir[i]);
        Cell next = {row, col, curr.dist + 1, curr.path + " " + dir[i]};
        queue[rear++] = next;
      }
    }
  }

  return "";
}

/// ----------------------- END BFS ---------------------------
