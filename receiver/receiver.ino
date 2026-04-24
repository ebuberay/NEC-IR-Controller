/* By Ebubechukwu Raymond Anieke, DeHardwareGuy, 24th of April, 2026
*/
//  NEC IR Receiver — PWM Brightness Control
//  Arduino Nano (ATmega328P)
//
//  IR receiver module OUT → pin 2
//  LED                    → pin 5 (PWM pin, through 220Ω)
//
//  Behavior:
//  Every valid NEC press increases brightness by 17
//  0 → 17 → 34 → 51 ... → 238 → 255 (maximum)
//  At maximum → blinks 4 times (0.5s interval)
//  Next press after maximum → LED OFF
//  Then cycle repeats from 0
//
//  255 / 17 = 15 steps to reach maximum
// ============================================================

#define DECODE_NEC
#define DECODE_NEC2

#include <IRremote.hpp>

// ── Pin definitions ──────────────────────────────────────────
const uint8_t IR_PIN  = 2;   // IR receiver module OUT pin
const uint8_t LED_PIN = 5;   // LED PWM output — uses Timer0
                              // pin 5 avoids Timer2 conflict
                              // with IRremote library

// ── Expected NEC values ──────────────────────────────────────
// Must match transmitter exactly
const uint8_t EXPECTED_ADDRESS = 0x00;
const uint8_t EXPECTED_COMMAND = 0x45;

// ── Brightness control ───────────────────────────────────────
// analogWrite() accepts 0 (off) to 255 (full brightness)
// We increment by 17 each press
// 255 / 17 = 15 steps to go from 0 to 255
// Step 0  = 0   (off)
// Step 1  = 17  (very dim)
// Step 2  = 34
// Step 3  = 51
// ...
// Step 15 = 255 (maximum)
const uint8_t BRIGHTNESS_STEP = 17;   // increment per button press
const uint8_t MAX_BRIGHTNESS   = 255;  // analogWrite maximum

// This variable tracks the current brightness level
// It starts at 0 meaning LED is off at power on
// uint16_t is used instead of uint8_t because during the
// increment check (currentBrightness + BRIGHTNESS_STEP)
// the value could temporarily exceed 255 before we clamp it
// and a uint8_t would overflow and wrap around silently
uint16_t currentBrightness = 0;

// This flag tracks whether we are in the OFF state after
// the maximum brightness blink confirmation
// false = normal incrementing mode
// true  = waiting for next press to turn LED off
bool waitingToTurnOff = false;

// ── Blink confirmation timing ────────────────────────────────
const uint8_t  BLINK_COUNT   = 4;    // blink 4 times at maximum
const uint16_t BLINK_ON_MS   = 500;  // 0.5 seconds ON per blink
const uint16_t BLINK_OFF_MS  = 500;  // 0.5 seconds OFF per blink

// ============================================================
//  blinkMaxConfirmation()
// ============================================================
//
//  Called when brightness reaches 255.
//  Blinks the LED 4 times at 0.5 second intervals to signal
//  that maximum brightness has been reached.
//  After blinking it restores the LED to full brightness
//  so the user knows it is still at maximum and waiting
//  for the next press to turn off.

void blinkMaxConfirmation() {

  Serial.println("Maximum brightness — blinking 4 times");

  // Loop 4 times for 4 blinks
  for (uint8_t i = 0; i < BLINK_COUNT; i++) {

    // Turn LED off for the blink
    analogWrite(LED_PIN, 0);
    delay(BLINK_OFF_MS);          // wait 0.5 seconds off

    // Turn LED back on at full brightness
    analogWrite(LED_PIN, 255);
    delay(BLINK_ON_MS);           // wait 0.5 seconds on

    Serial.print("Blink ");
    Serial.print(i + 1);
    Serial.println(" of 4");
  }

  // After all 4 blinks restore to full brightness
  // LED stays at maximum, waiting for the next press
  analogWrite(LED_PIN, MAX_BRIGHTNESS);

  Serial.println("Blink confirmation done");
  Serial.println("Next press will turn LED OFF");
}

// ============================================================
//  handleBrightness()
// ============================================================
//
//  Core brightness logic called on every valid NEC press.
//
//  Three possible states when this function is called:
//
//  State 1 — waitingToTurnOff is true
//            Maximum was already reached and confirmed
//            This press turns the LED completely off
//            and resets everything back to the start
//
//  State 2 — currentBrightness + BRIGHTNESS_STEP >= 255
//            This press will reach or exceed maximum
//            We clamp to exactly 255, apply it, then
//            run the 4 blink confirmation and set the
//            waitingToTurnOff flag for the next press
//
//  State 3 — Normal increment
//            Just add 17 to current brightness and apply it

void handleBrightness() {

  // ── State 1: Turn LED off after maximum was confirmed ────
  if (waitingToTurnOff) {
    currentBrightness = 0;        // reset brightness to zero
    waitingToTurnOff  = false;    // reset the flag
    analogWrite(LED_PIN, 0);      // turn LED completely off
    Serial.println("LED turned OFF — cycle reset");
    Serial.println("Press button to start increasing brightness");
    return;  // exit function, nothing else to do
  }

  // ── State 2: Next increment will reach maximum ───────────
  // Check if adding another step would reach or exceed 255
  // We use uint16_t arithmetic here to avoid overflow
  if ((uint16_t)currentBrightness + BRIGHTNESS_STEP >= MAX_BRIGHTNESS) {

    // Clamp to exactly 255 — do not go over
    currentBrightness = MAX_BRIGHTNESS;

    // Apply maximum brightness to the LED
    analogWrite(LED_PIN, currentBrightness);

    Serial.print("Brightness set to maximum: ");
    Serial.println(currentBrightness);

    // Run the 4 blink confirmation sequence
    blinkMaxConfirmation();

    // Set flag so the NEXT press turns the LED off
    // instead of trying to increment further
    waitingToTurnOff = true;

    return;  // exit function
  }

  // ── State 3: Normal brightness increment ─────────────────
  // Add 17 to current brightness
  currentBrightness += BRIGHTNESS_STEP;

  // Apply the new brightness to the LED via PWM
  // analogWrite sends a PWM signal to pin 5
  // the duty cycle is proportional to the value 0-255
  analogWrite(LED_PIN, currentBrightness);

  // Print current state to Serial Monitor
  Serial.print("Brightness: ");
  Serial.print(currentBrightness);
  Serial.print(" / 255  (step ");
  Serial.print(currentBrightness / BRIGHTNESS_STEP);
  Serial.println(" of 15)");
}

// ============================================================
//  setup()
// ============================================================

void setup() {
  Serial.begin(115200);

  // Configure LED pin as output and ensure it starts off
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0);

  // Start IR receiver on pin 2
  IrReceiver.begin(IR_PIN, DISABLE_LED_FEEDBACK);

  Serial.println("=========================================");
  Serial.println("   NEC IR PWM Brightness Control Ready   ");
  Serial.println("=========================================");
  Serial.print("Address        : 0x");
  Serial.println(EXPECTED_ADDRESS, HEX);
  Serial.print("Command        : 0x");
  Serial.println(EXPECTED_COMMAND, HEX);
  Serial.print("Brightness step: ");
  Serial.println(BRIGHTNESS_STEP);
  Serial.println("Press button to increase brightness");
  Serial.println("=========================================");
}

// ============================================================
//  loop()
// ============================================================

void loop() {

  if (IrReceiver.decode()) {

    // ── Gate 1: Must be NEC or NEC2 protocol ───────────────
    if (IrReceiver.decodedIRData.protocol != NEC &&
        IrReceiver.decodedIRData.protocol != NEC2) {
      Serial.println("Non-NEC signal — ignoring");
      IrReceiver.resume();
      return;
    }

    // ── Gate 2: Ignore repeat frames ───────────────────────
    // Without this, holding the button would rapidly
    // cycle through brightness levels uncontrollably
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
      IrReceiver.resume();
      return;
    }

    // ── Gate 3: Address must match ──────────────────────────
    if (IrReceiver.decodedIRData.address != EXPECTED_ADDRESS) {
      Serial.print("Wrong address 0x");
      Serial.print(IrReceiver.decodedIRData.address, HEX);
      Serial.println(" — ignoring");
      IrReceiver.resume();
      return;
    }

    // ── Gate 4: Command must match ──────────────────────────
    if (IrReceiver.decodedIRData.command != EXPECTED_COMMAND) {
      Serial.print("Wrong command 0x");
      Serial.print(IrReceiver.decodedIRData.command, HEX);
      Serial.println(" — ignoring");
      IrReceiver.resume();
      return;
    }

    // ── All gates passed ────────────────────────────────────
    // Resume receiver before handling brightness because
    // blinkMaxConfirmation() uses delay() and takes 4 seconds
    // We want the receiver ready before we go into that delay
    IrReceiver.resume();

    // Handle the brightness logic
    handleBrightness();
  }
}
