// Constants
const int PPM_PIN = 14; // Pin where the PPM signal is connected
const int NUM_CHANNELS = 6; // Number of channels
const int SYNC_GAP = 4000; // Microseconds threshold to detect sync pulse (adjust if necessary)


// Variables
volatile unsigned long lastTime = 0;
volatile unsigned long pulseWidths[NUM_CHANNELS];
volatile int currentChannel = 0;
volatile bool newData = false;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Initialize PPM pin
  pinMode(PPM_PIN, INPUT);
  
  // Attach interrupt to the PPM pin
  attachInterrupt(digitalPinToInterrupt(PPM_PIN), handlePPMInterrupt, RISING);
}

void loop() {
  //reciever();
  int roll = pulseWidths[0];
  int pitch = pulseWidths[1];
  int throttle = pulseWidths[2];
  int yaw = pulseWidths[3];
  Serial.print("throttle: "); Serial.print(throttle); Serial.print(" (us) ,");
  Serial.print("roll: "); Serial.print(roll); Serial.print(" (us) ,");
  Serial.print("pitch: "); Serial.print(pitch); Serial.print(" (us) ,");
  Serial.print("yaw: "); Serial.print(yaw); Serial.println(" (us)"); 
}
/*
void reciever(){
    if (newData) {
    newData = false;
    
    Serial.println("Channel pulse widths:");
    for (int i = 0; i < NUM_CHANNELS; i++) {
      Serial.print("Channel ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(pulseWidths[i]);
      Serial.println(" us");
    }
    Serial.println();
  }
}
*/
void handlePPMInterrupt() {
  // Get current time
  unsigned long currentTime = micros();
  // Calculate the time difference from the last interrupt
  unsigned long duration = currentTime - lastTime;
  lastTime = currentTime;
  
  // Check if this is a sync pulse
  if (duration > SYNC_GAP) {
    // Sync pulse detected, reset channel index
    currentChannel = 0;
  } else {
    // Store pulse width in the array
    if (currentChannel < NUM_CHANNELS) {
      pulseWidths[currentChannel] = duration;
      currentChannel++;
      
      // If all channels have been read, indicate that new data is available
      if (currentChannel == NUM_CHANNELS) {
        newData = true;
      }
    }
  }
}
