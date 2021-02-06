#define STEPPER_OUT1 7
#define STEPPER_OUT2 2
#define STEPPER_OUT3 3
#define STEPPER_OUT4 4
#define ENCODER_KNOB 9
#define ENCODER_A 5
#define ENCODER_B 6

#define STEPPER_PERIOD 3 //msec
#define ENCODER_PERIOD 7 //msec
#define KNOB_PERIOD 20 //msec 
const bool PHASES_LAYOUT[4][4] = {
  {1,0,0,1},
  {1,1,0,0},
  {0,1,1,0},
  {0,0,1,1}
};
 //msec
unsigned long stepper_timer_ref, current_stepper_timer;
int stepper_destination; //steps relative to current position
int8_t phase = 0;

unsigned long encoder_timer_ref;
bool knob_pressed, knob_prev;
byte encoder_seqA;
byte encoder_seqB;
int8_t encoder_increment;

void setup() {
  pinMode(STEPPER_OUT1, OUTPUT);
  pinMode(STEPPER_OUT2, OUTPUT);
  pinMode(STEPPER_OUT3, OUTPUT);
  pinMode(STEPPER_OUT4, OUTPUT);
  pinMode(ENCODER_KNOB, INPUT_PULLUP);
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  stepper_timer_ref = millis();
  stepper_destination = 5000;
  
  encoder_timer_ref = millis();
  knob_pressed = false;
  knob_prev = false;
  encoder_increment = 0;
  Serial.begin(115200);
  Serial.println("Initiated:");
  Serial.println("Stepper pins: " + (String)STEPPER_OUT1 + "|" + (String)STEPPER_OUT2 + "|" + (String)STEPPER_OUT3 + "|" + (String)STEPPER_OUT4);
  Serial.println ("Encoder pins: " + (String)ENCODER_A + "|" + (String)ENCODER_B);
  Serial.println ("Knob pin: " + (String)ENCODER_KNOB);
}

void loop() {
  // process stepper 
  current_stepper_timer = millis(); 
  if ((millis() - stepper_timer_ref) >= STEPPER_PERIOD) {
    stepper_timer_ref = current_stepper_timer;
    if (stepper_destination < 0) { 
      if (--phase < 0) phase = 3;
      ++stepper_destination; 
    } 
    else if (stepper_destination > 0) {
      if (++phase > 3) phase = 0;
      --stepper_destination;
    }
/*    Serial.print("Writing ");
    for (int i = 0; i < 4; ++i) {
      Serial.print(" ");
      Serial.print(PHASES_LAYOUT[phase][i]);
    }
    Serial.println();
*/
    digitalWrite(STEPPER_OUT1, PHASES_LAYOUT[phase][0]);
    digitalWrite(STEPPER_OUT2, PHASES_LAYOUT[phase][1]); 
    digitalWrite(STEPPER_OUT3, PHASES_LAYOUT[phase][2]); 
    digitalWrite(STEPPER_OUT4, PHASES_LAYOUT[phase][3]);    
  }
  //process rotary knob
  if ((millis() - encoder_timer_ref) >= KNOB_PERIOD) {
    knob_prev = knob_pressed;
    knob_pressed = !digitalRead(ENCODER_KNOB);
    if (knob_pressed != knob_prev) 
      if (knob_pressed) {
        Serial.println("Pressed");
        stepper_destination = 0;
      }  
      else 
        Serial.println("Released");
  }
  if ((millis() - encoder_timer_ref) >= ENCODER_PERIOD) {
    boolean A_val = digitalRead(ENCODER_A);
    boolean B_val = digitalRead(ENCODER_B);
    
    if ((A_val != (encoder_seqA & 0b00000001)) || (B_val != (encoder_seqB & 0b00000001))) {
      Serial.println("Encoder signals: " + (String)A_val + "|" + (String)B_val);
      // Record the A and B signals in seperate sequences
      encoder_seqA <<= 1;
      encoder_seqA |= A_val;
      encoder_seqB <<= 1;
      encoder_seqB |= B_val;
      // Mask the MSB four bits
      encoder_seqA &= 0b00001111;
      encoder_seqB &= 0b00001111;
      // Compare the recorded sequence with the expected sequence
      if (encoder_seqA == 0b00001001 && encoder_seqB == 0b00000011) {
        ++encoder_increment;
        stepper_destination -= 102;
        Serial.println("Encoder: " + (String)encoder_increment + "; Stepper: " + (String)stepper_destination);
      }
      if (encoder_seqA == 0b00000011 && encoder_seqB == 0b00001001) {
        --encoder_increment;
        stepper_destination += 102;
        Serial.println("Encoder: " + (String)encoder_increment + "; Stepper: " + (String)stepper_destination);
      }
    }
  } 
}
