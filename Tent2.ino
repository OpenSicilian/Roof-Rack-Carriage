// Motor pins
int motor1pin1 = 6;
int motor1pin2 = 7;
int motor1spin = 10;

int motor2pin1 = 4;

int motor2pin2 = 5;
int motor2spin = 9;

// Inputs
const int triggerDown = 2;    // INT0
const int triggerUp = 8;      // Polled
const int limitDown = 3;      // INT1
const int limitUp = 11;       // Polled

// Flags set by interrupts
volatile bool limitDownFlag = false;
volatile bool limitUpFlag = false;


// Debounce
unsigned long lastLimitDownTime = 0;
unsigned long lastLimitUpTime = 0;
const unsigned long DEBOUNCE_DELAY = 50; // ms

// State
bool stopMotors = false;
bool movementBlocked = false; // New flag
enum Movement { NONE, UP, DOWN };
Movement currentDirection = NONE;

void setup() {
  pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);
  pinMode(motor2pin1, OUTPUT);
  pinMode(motor2pin2, OUTPUT);
  pinMode(motor1spin, OUTPUT);
  pinMode(motor2spin, OUTPUT);

  pinMode(triggerDown, INPUT_PULLUP);
  pinMode(triggerUp, INPUT_PULLUP);
  pinMode(limitDown, INPUT_PULLUP);
  pinMode(limitUp, INPUT_PULLUP);

  analogWrite(motor1spin, 255);
  analogWrite(motor2spin, 255);

  attachInterrupt(digitalPinToInterrupt(triggerDown), dummyISR, CHANGE); // Required for pin 2 interrupt config
  attachInterrupt(digitalPinToInterrupt(limitDown), onLimitDown, FALLING);

}

void loop() {
  unsigned long currentTime = millis();

  // Debounce: Limit Down
  if (limitDownFlag && currentTime - lastLimitDownTime > DEBOUNCE_DELAY) {
    limitDownFlag = false;
    lastLimitDownTime = currentTime;
    if (currentDirection == DOWN) {
      stopAllMotors();
      movementBlocked = true;
    }
  }

  // Debounce: Limit Up
  if (limitUpFlag && currentTime - lastLimitUpTime > DEBOUNCE_DELAY) {
    limitUpFlag = false;
    lastLimitUpTime = currentTime;
    if (currentDirection == UP) {
      stopAllMotors();
      movementBlocked = true;
    }
  }

  // Read triggers (active LOW)
  bool downPressed = digitalRead(triggerDown) == LOW;
  bool upPressed = digitalRead(triggerUp) == LOW;

  // Unblock movement when both triggers released
  if (!downPressed && !upPressed) {
    movementBlocked = false;
  }

  // Evaluate input state only if not blocked
  if (!movementBlocked) {
    if (downPressed && !upPressed) {
      currentDirection = DOWN;
      moveDown();
    } else if (upPressed && !downPressed) {
      currentDirection = UP;
      moveUp();

      // Software debounce: limit up
      if (digitalRead(limitUp) == LOW && currentTime - lastLimitUpTime > DEBOUNCE_DELAY) {
        limitUpFlag = true;
      }
    } else {
      stopAllMotors();
      currentDirection = NONE;
    }
  } else {
    stopAllMotors();
    currentDirection = NONE;
  }
}

void moveDown() {
  digitalWrite(motor2pin1, HIGH);  
  digitalWrite(motor2pin2, LOW);
  digitalWrite(motor1pin1, HIGH);  
  digitalWrite(motor1pin2, LOW);
}

void moveUp() {
  digitalWrite(motor2pin1, LOW);  
  digitalWrite(motor2pin2, HIGH);
  digitalWrite(motor1pin1, LOW);  
  digitalWrite(motor1pin2, HIGH);
}

void stopAllMotors() {
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, LOW);
}

// ISR
void dummyISR() {}  // Not used, but required for INT0 setup
void onLimitDown() {
  limitDownFlag = true;
}
