
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-mesh-esp32-esp8266-painlessmesh/
  
  This is a simple example that uses the painlessMesh library: https://github.com/gmag11/painlessMesh/blob/master/examples/basic/basic.ino
*/

#include <Arduino.h>
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage(); // Prototype so PlatformIO doesn't complain
void measure();

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage);
Task taskMeasure(TASK_SECOND * 1, TASK_FOREVER, &measure);

String battery = "0";
String soil = "0";

void measure() {
  float analog_battery = map(analogRead(36), 0.0f, 4095.0f, 0, 100);
  battery = String(analog_battery);

  digitalWrite(23, HIGH);
  taskMeasure.delay(100);
  float analog_soil = map(analogRead(39), 0.0f, 4095.0f, 100, 0);
  soil = String(analog_soil);
  digitalWrite(23, LOW);
  taskMeasure.setInterval(TASK_SECOND * 1);
}

void sendMessage() {
  String msg = "Hi from N1 - id:";
  msg += mesh.getNodeId();
  msg += " | battery: " + battery + "%";
  msg += " | soil: " + soil + "%";
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 3 ));
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(23, OUTPUT);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  userScheduler.addTask( taskMeasure );
  taskSendMessage.enable();
  taskMeasure.enable();
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}
