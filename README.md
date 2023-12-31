# Autonomous-Vehicle-with-CCPP-Based-On-BFS-with-Cloud-Server
This is the final year project for the partial fulfillment of Bachelor in Electrical and Electronics Engineering Degree at Liverpool John Moores University. 

## Setup 
Follow each section underneath to properlly set up the Arduino Uno, ESP32Cam and the cloud server used in this project.

### ArduinoMain
This code is to be uploaded to the Arduino UNO which is mounted by the L293D shield. Inside the code, replace the ROW and COL in the variable definition section to configure the number of rows and columns you want in the actual process. Also modify the grid_explore 2D array with zeros respective to the modified number of rows and columns.

To update the server IP and server Port, update this line of code in the setup function with the new server IP and Port separated by a comma.
```C++
mySerial.println("13.59.52.41,8080");
```
This library is used to fetch data from the MPU6050 module: [MPU6050_light](https://github.com/rfetick/MPU6050_light)
The other dependencies can be downloaded directly from the Arduino IDE's library manager.

### ESP32CamMain
Upload this code to the ESP32Cam. As in ArduinoMain, replace the ROW and COL with the desired number of rows and columns to be used.

### Server
To initiate the server, upload the files and folder in /server/ to your desired hosting service, either local or on an AWS EC2 instance. Make sure the port is allowed to handle HTTP requests. The tutorial to share an EC2 instance to the internet can be found in the official document here (using Cloud9): [Share an application over the internet](https://docs.aws.amazon.com/cloud9/latest/user-guide/app-preview.html#app-preview-share)

Run this command to install dependencies
```bash
npm install
```

Run this command to start the server
```bash
node app.js
```

The html frontend page is served as a static page, which is located in the public folder. The default port used is 8080. To change the port, you need to make the changes in the codes for the microprocessors as well. 

<img src="https://github.com/boboTheDev/Autonomous-Vehicle-with-CCPP-Based-On-BFS-with-Cloud-Server/assets/91797672/1f0b03e8-6b37-4bc0-967d-96fa23c8b378" alt="drawing" width="500"/>
