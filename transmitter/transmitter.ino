/* By Ebubechukwu Raymond Anieke, DeHardwareGuy, 24th of April 2026

*/



//  NEC IR Transmitter — Arduino Nano (ATmega328P)
//  IR LED        → pin 11 (Timer2 OC2A — must stay here)
//  Tactile switch → pin 2  (one leg to pin 2, other leg to GND)
// ============================================================

// ── Pin definitions ──────────────────────────────────────────
const uint8_t IR_PIN     = 11;  // OC2A — do NOT change this
const uint8_t BUTTON_PIN = 2;   // tactile switch input

// ── NEC timing constants (microseconds) ──────────────────────
const uint16_t NEC_AGC_BURST  = 9000;
const uint16_t NEC_AGC_SPACE  = 4500;
const uint16_t NEC_BIT_BURST  =  562;
const uint16_t NEC_ONE_SPACE  = 1687;
const uint16_t NEC_ZERO_SPACE =  562;
const uint16_t NEC_STOP_BURST =  562;

// ── NEC message ───────────────────────────────────────────────
const uint8_t NEC_ADDRESS = 0x00;
const uint8_t NEC_COMMAND = 0x45;

// ── Debounce timing (milliseconds) ───────────────────────────
// Mechanical switches bounce — they rapidly open and close
// several times in the first ~50 ms before settling. Without
// debouncing, one physical press could trigger many NEC frames.
const uint16_t DEBOUNCE_MS = 50;

// ============================================================
//  Timer2 helpers
// ============================================================

void carrierOn() {
  TCCR2A = _BV(COM2A0) | _BV(WGM21);
  TCCR2B = _BV(CS21);
  OCR2A  = 26;
  pinMode(IR_PIN, OUTPUT);
}

void carrierOff() {
  TCCR2A = 0;
  TCCR2B = 0;
  digitalWrite(IR_PIN, LOW);
}

// ============================================================
//  NEC waveform primitives
// ============================================================

void mark(uint16_t us) {
  carrierOn();
  delayMicroseconds(us);
}

void space(uint16_t us) {
  carrierOff();
  delayMicroseconds(us);
}

void sendBit(bool bit) {
  mark(NEC_BIT_BURST);
  if (bit) {
    space(NEC_ONE_SPACE);
  } else {
    space(NEC_ZERO_SPACE);
  }
}

void sendByte(uint8_t b) {
  for (uint8_t i = 0; i < 8; i++) {
    sendBit(b & 0x01);
    b >>= 1;
  }
}

// ============================================================
//  Complete NEC frame
// ============================================================

void sendNEC(uint8_t address, uint8_t command) {
  mark(NEC_AGC_BURST);
  space(NEC_AGC_SPACE);

  sendByte(address);
  sendByte(~address);
  sendByte(command);
  sendByte(~command);

  mark(NEC_STOP_BURST);
  space(NEC_STOP_BURST);
}

// ============================================================
//  setup()
// ============================================================

void setup() {
  // IR LED pin — start LOW (LED off)
  pinMode(IR_PIN, OUTPUT);
  digitalWrite(IR_PIN, LOW);

  // Button pin — INPUT_PULLUP activates the ATmega's internal
  // ~20–50 kΩ pull-up resistor, holding pin 2 HIGH by default.
  // When the switch closes it connects pin 2 to GND → reads LOW.
  // This means button pressed = LOW, button released = HIGH.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println("NEC IR Transmitter ready.");
  Serial.println("Press the button to transmit.");
}

// ============================================================
//  loop()
// ============================================================

// We track the last time a valid press was detected so we can
// ignore the switch bouncing in the milliseconds after a press.
uint32_t lastPressTime = 0;

// We also track the previous button state so we only trigger
// on the moment the button transitions from released → pressed,
// not continuously while it is held down.
bool lastButtonState = HIGH;  // HIGH = released (pull-up active)

void loop() {
  bool currentState = digitalRead(BUTTON_PIN);

  // Detect a falling edge — HIGH (released) → LOW (pressed)
  // This means we only react to the instant the button is pushed
  // down, not repeatedly while it stays pressed.
  if (lastButtonState == HIGH && currentState == LOW) {

    // Check enough time has passed since the last valid press.
    // millis() returns the number of milliseconds since power-on
    // as a uint32_t. If less than 50 ms has passed since the
    // last press, we treat this transition as bounce and ignore it.
    uint32_t now = millis();
    if (now - lastPressTime >= DEBOUNCE_MS) {

      lastPressTime = now;  // record time of this valid press

      Serial.println("Button pressed — sending NEC frame...");
      sendNEC(NEC_ADDRESS, NEC_COMMAND);
      Serial.println("Done.");
    }
  }

  // Save state for next loop iteration so we can detect the edge
  lastButtonState = currentState;
}

Add transmitter code
