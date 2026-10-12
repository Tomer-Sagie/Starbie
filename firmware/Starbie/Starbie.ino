#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <DHT.h>
#include <Preferences.h>
#include <math.h>
#include <Adafruit_NeoPixel.h>

const int I2C_SDA_PIN = 7;
const int I2C_SCL_PIN = 6;
const int DHT_PIN = 2;
const int BUTTON_ONE_PIN = 4;
const int BUTTON_TWO_PIN = 5;

const int BUZZER_PIN = 10;
const int HAPTIC_PIN = 20;
const int LIGHT_PIN  = 3;
const int LED_PIN    = 21;

const bool USE_DHT11 = true;
const bool USE_SOUND        = true;
const bool USE_LIGHT_SENSOR = true;
const bool USE_HAPTIC       = true;
const bool USE_LEDS         = true;

const uint8_t OLED_ADDRESS = 0x3C;
const uint8_t MPU6050_ADDRESS = 0x68;

const int STARTING_JOY = 70;
const int STARTING_ENERGY = 75;
const int STARTING_FULLNESS = 65;

const bool RESET_SAVED_PET_ON_BOOT = false;

enum PetReaction {
  NAP_REACTION,
  JUMP_REACTION,
  HEART_REACTION,
  RUN_REACTION,
};

struct MenuItem {
  const char *label;
  int joyChange;
  int energyChange;
  int fullnessChange;
  PetReaction reaction;
};

const MenuItem MENU_ITEMS[] = {
  {"NAP",   1,  18, -4, NAP_REACTION},
  {"PLAY", 12, -9, -5, RUN_REACTION},
  {"FEED",  3,  2,  18, JUMP_REACTION},
  {"PET",   7,  0,  0, HEART_REACTION},
};
const int MENU_ITEM_COUNT = sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]);

const float MENU_TILT_LIMIT = 6.0f;

const bool SWAP_MPU_AXES = false;
const float MENU_X_DIRECTION = 1.0f;
const float MENU_Y_DIRECTION = -1.0f;

const float MENU_CENTER_DEADZONE = 0.8f;
const float SHAKE_THRESHOLD = 7.0f;

const int SHAKE_JOY_CHANGE = 5;
const int SHAKE_ENERGY_CHANGE = -2;
const int SHAKE_FULLNESS_CHANGE = -1;

const int PET_SPRITE_WIDTH = 32;
const int PET_SPRITE_HEIGHT = 32;

const uint16_t PET_WALK_PIXEL_MS = 70;
const uint16_t PET_PRE_JUMP_MS = 230;
const uint16_t PET_JUMP_MS = 430;
const int PET_JUMP_HEIGHT = 16;
const uint32_t NAP_DURATION_MS = 48000;
const uint16_t HEARTS_DURATION_MS = 1600;
const uint16_t PLAY_LAP_MS = 800;
const uint8_t PLAY_LAP_COUNT = 2;

const uint8_t PROGMEM PET_SPRITE[] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x70, 0x0a, 0x00,
  0x00, 0xf8, 0x1f, 0x00,
  0x00, 0x8c, 0x3f, 0x80,
  0x00, 0x43, 0xff, 0x00,
  0x00, 0x43, 0xff, 0x00,
  0x00, 0x30, 0x7e, 0x00,
  0x00, 0x98, 0x01, 0x00,
  0x01, 0x98, 0x01, 0x80,
  0x03, 0x00, 0x10, 0xc0,
  0x03, 0x04, 0x00, 0xc0,
  0x01, 0x8b, 0x01, 0x80,
  0x01, 0xcb, 0x03, 0x80,
  0x03, 0xc0, 0x03, 0xc0,
  0x03, 0xc0, 0x03, 0xc0,
  0x01, 0xc0, 0x03, 0x80,
  0x01, 0x80, 0x01, 0x80,
  0x00, 0x60, 0x06, 0x00,
  0x00, 0x3f, 0xfc, 0x00,
  0x00, 0x7f, 0xfe, 0x00,
  0x00, 0x70, 0x0e, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
};

const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int MENU_CENTER_X = 64;
const int MENU_CENTER_Y = 32;
const int MENU_ITEM_X[] = {64, 108, 64, 20};
const int MENU_ITEM_Y[] = {12, 32, 53, 32};
const int MENU_BALL_X_RANGE = 37;
const int MENU_BALL_Y_RANGE = 22;
const uint16_t BUTTON_DEBOUNCE_MS = 30;
const uint16_t MPU_READ_INTERVAL_MS = 30;
const uint16_t DHT_READ_INTERVAL_MS = 2200;
const uint16_t SHAKE_COOLDOWN_MS = 650;
const float STANDARD_GRAVITY = 9.80665f;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_MPU6050 mpu;
DHT dht(DHT_PIN, DHT11);
Preferences preferences;

struct PetState {
  int joy;
  int energy;
  int fullness;
};

struct ButtonState {
  int pin;
  bool stableState;
  bool lastRawState;
  uint32_t lastChangedAt;
};

enum View {
  PET_VIEW,
  MENU_VIEW,
  STATS_VIEW,
};

PetState pet = {STARTING_JOY, STARTING_ENERGY, STARTING_FULLNESS};
ButtonState buttonOne = {BUTTON_ONE_PIN, HIGH, HIGH, 0};
ButtonState buttonTwo = {BUTTON_TWO_PIN, HIGH, HIGH, 0};
View currentView = PET_VIEW;

bool mpuFound = false;
bool dhtFound = false;
float accelerationX = 0.0f;
float accelerationY = 0.0f;
float accelerationZ = STANDARD_GRAVITY;
float temperatureC = NAN;
float humidity = NAN;
float menuBallX = MENU_CENTER_X;
float menuBallY = MENU_CENTER_Y;
float menuCenterAccelerationX = 0.0f;
float menuCenterAccelerationY = 0.0f;
int selectedMenuItem = -1;
uint32_t lastMpuReadAt = 0;
uint32_t lastDhtReadAt = 0;
uint32_t lastShakeAt = 0;
uint32_t shakeAnimationEndsAt = 0;
uint32_t petJumpStartedAt = 0;
uint32_t nappingUntil = 0;
uint32_t heartAnimationEndsAt = 0;
uint32_t playRunStartedAt = 0;
int nappingPetX = 0;

int petWalkingX(uint32_t now);

struct Note {
  uint16_t freq;
  uint16_t ms;
};

struct Pulse {
  uint8_t  percent;
  uint16_t onMs;
  uint16_t offMs;
};

enum {
  N_REST = 0,
  N_C4 = 262, N_D4 = 294, N_E4 = 330, N_F4 = 349,
  N_G4 = 392, N_A4 = 440, N_B4 = 494,
  N_C5 = 523, N_D5 = 587, N_E5 = 659, N_F5 = 698,
  N_G5 = 784, N_A5 = 880, N_C6 = 1047
};

enum {
  TUNE_BLIP = 0, TUNE_HAPPY, TUNE_SAD, TUNE_FEED,
  TUNE_SLEEP, TUNE_ALERT, TUNE_NIGHT, TUNE_COUNT
};

enum {
  HAP_TICK = 0, HAP_SELECT, HAP_FEED,
  HAP_PET, HAP_ALERT, HAP_NIGHT, HAP_COUNT
};

Adafruit_NeoPixel pixels(2, LED_PIN, NEO_GRB + NEO_KHZ800);

const uint8_t TUNE_MAX_NOTES = 6;
const Note TUNES[TUNE_COUNT][TUNE_MAX_NOTES] = {
   {{N_C6, 30}},
   {{N_C5, 100}, {N_E5, 100}, {N_G5, 100}, {N_C6, 170}},
   {{N_G4, 150}, {N_E4, 150}, {N_C4, 260}},
   {{N_E5, 90},  {N_G5, 160}},
   {{N_G4, 180}, {N_C4, 320}},
   {{N_A5, 60},  {N_REST, 40}, {N_A5, 60}},
   {{N_D4, 200}, {N_C4, 300}},
};
const uint8_t TUNE_LENGTHS[TUNE_COUNT] = {1, 4, 3, 2, 2, 3, 2};

const uint8_t PATTERN_MAX_PULSES = 3;
const Pulse PATTERNS[HAP_COUNT][PATTERN_MAX_PULSES] = {
   {{70, 25, 0}},
   {{100, 45, 55}, {100, 45, 0}},
   {{85, 35, 45},  {85, 35, 0}},
   {{55, 240, 0}},
   {{100, 70, 60}, {100, 70, 60}, {100, 70, 0}},
   {{60, 150, 0}},
};
const uint8_t PATTERN_LENGTHS[HAP_COUNT] = {1, 2, 2, 1, 3, 1};

uint8_t  activeTune = TUNE_COUNT;
uint8_t  activeNote = 0;
uint32_t nextNoteAt = 0;

const int LIGHT_DARK_MV   = 2200;
const int LIGHT_BRIGHT_MV = 1500;
const uint16_t LIGHT_READ_INTERVAL_MS = 1000;

bool nightMode = false;
int  ambientMilliVolts = 0;
uint32_t lastLightReadAt = 0;

uint8_t  activePattern = HAP_COUNT;
uint8_t  activePulse = 0;
bool     hapticIsOn = false;
uint32_t hapticNextAt = 0;
uint32_t hapticBusyUntil = 0;

void ledsApply(bool night) {
  if (!USE_LEDS) return;
  if (night) {
    pixels.setBrightness(20);
    pixels.setPixelColor(0, pixels.Color(40, 12, 0));
    pixels.setPixelColor(1, pixels.Color(8, 4, 20));
  } else {
    pixels.setBrightness(120);
    pixels.setPixelColor(0, pixels.Color(0, 90, 60));
    pixels.setPixelColor(1, pixels.Color(60, 0, 90));
  }
  pixels.show();
}

void ledsBlankForRead() {
  if (!USE_LEDS) return;
  pixels.clear();
  pixels.show();
}

void ledsFlash(uint8_t r, uint8_t g, uint8_t b, uint16_t ms) {
  if (!USE_LEDS) return;
  pixels.setBrightness(200);
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.setPixelColor(1, pixels.Color(r, g, b));
  pixels.show();
  delay(ms);
  ledsApply(nightMode);
}

void tunePlay(uint8_t tuneId) {
  if (!USE_SOUND || tuneId >= TUNE_COUNT) return;
  activeTune = tuneId;
  activeNote = 0;
  nextNoteAt = millis();
}

void tuneUpdate() {
  if (activeTune >= TUNE_COUNT) return;
  const uint32_t now = millis();
  if (now < nextNoteAt) return;

  if (activeNote >= TUNE_LENGTHS[activeTune]) {
    noTone(BUZZER_PIN);
    activeTune = TUNE_COUNT;
    return;
  }

  const Note &n = TUNES[activeTune][activeNote];
  if (n.freq == N_REST) {
    noTone(BUZZER_PIN);
  } else {
    tone(BUZZER_PIN, n.freq, n.ms);
  }
  nextNoteAt = now + n.ms + 25;
  activeNote++;
}

void updateLight() {
  if (!USE_LIGHT_SENSOR) return;
  const uint32_t now = millis();
  if (now - lastLightReadAt < LIGHT_READ_INTERVAL_MS) return;
  lastLightReadAt = now;

  ledsBlankForRead();
  delay(2);
  ambientMilliVolts = static_cast<int>(analogReadMilliVolts(LIGHT_PIN));
  ledsApply(nightMode);

  if (!nightMode && ambientMilliVolts > LIGHT_DARK_MV)   nightMode = true;
  if ( nightMode && ambientMilliVolts < LIGHT_BRIGHT_MV) nightMode = false;
}

void updateNightModeTransition() {
  static bool lastNightMode = nightMode;
  if (nightMode == lastNightMode) return;
  lastNightMode = nightMode;

  if (nightMode) {
    if (!isNapping()) {
      nappingUntil = millis() + NAP_DURATION_MS;
      nappingPetX = petWalkingX(millis());
    }
    tunePlay(TUNE_NIGHT);
    hapticPlay(HAP_NIGHT);
  } else {
    tunePlay(TUNE_HAPPY);
  }
  ledsApply(nightMode);
}

void hapticPlay(uint8_t patternId) {
  if (!USE_HAPTIC || patternId >= HAP_COUNT) return;
  activePattern = patternId;
  activePulse = 0;
  hapticIsOn = false;
  hapticNextAt = millis();
}

void hapticUpdate() {
  if (activePattern >= HAP_COUNT) return;
  const uint32_t now = millis();
  if (now < hapticNextAt) return;

  if (hapticIsOn) {

    ledcWrite(HAPTIC_PIN, 0);
    hapticIsOn = false;
    const uint16_t gap = PATTERNS[activePattern][activePulse].offMs;
    activePulse++;
    if (activePulse >= PATTERN_LENGTHS[activePattern]) {
      activePattern = HAP_COUNT;

      hapticBusyUntil = now + 500;
      return;
    }
    hapticNextAt = now + gap;
    return;
  }

  const Pulse &p = PATTERNS[activePattern][activePulse];
  ledcWrite(HAPTIC_PIN, static_cast<uint32_t>(p.percent) * 255 / 100);
  hapticIsOn = true;
  hapticNextAt = now + p.onMs;
}

void expansionSetup() {
  if (USE_LEDS) {
    pixels.begin();
    pixels.setBrightness(120);
    pixels.clear();
    pixels.show();
  }
  if (USE_HAPTIC) {

    ledcAttach(HAPTIC_PIN, 20000, 8);
    ledcWrite(HAPTIC_PIN, 0);
  }
  if (USE_LIGHT_SENSOR) {

    delay(5);
    ambientMilliVolts = static_cast<int>(analogReadMilliVolts(LIGHT_PIN));
    nightMode = ambientMilliVolts > LIGHT_DARK_MV;
  }
  if (USE_SOUND) {
    tunePlay(TUNE_HAPPY);
  }
}

void expansionUpdate() {
  tuneUpdate();
  hapticUpdate();
  updateLight();
  updateNightModeTransition();
}

int keepInRange(int value, int smallest, int largest) {
  return constrain(value, smallest, largest);
}

void changePet(int joyChange, int energyChange, int fullnessChange) {
  pet.joy = keepInRange(pet.joy + joyChange, 0, 100);
  pet.energy = keepInRange(pet.energy + energyChange, 0, 100);
  pet.fullness = keepInRange(pet.fullness + fullnessChange, 0, 100);
}

void loadPet() {
  preferences.begin("starbie", false);
  if (RESET_SAVED_PET_ON_BOOT) {
    preferences.clear();
  }
  pet.joy = preferences.getInt("joy", STARTING_JOY);
  pet.energy = preferences.getInt("energy", STARTING_ENERGY);
  pet.fullness = preferences.getInt("full", STARTING_FULLNESS);
}

void savePet() {
  preferences.putInt("joy", pet.joy);
  preferences.putInt("energy", pet.energy);
  preferences.putInt("full", pet.fullness);
}

void setUpButton(ButtonState &button) {
  pinMode(button.pin, INPUT_PULLUP);
  button.stableState = digitalRead(button.pin);
  button.lastRawState = button.stableState;
  button.lastChangedAt = millis();
}

bool wasPressed(ButtonState &button) {
  const bool rawState = digitalRead(button.pin);
  const uint32_t now = millis();

  if (rawState != button.lastRawState) {
    button.lastRawState = rawState;
    button.lastChangedAt = now;
  }

  if (rawState != button.stableState &&
      now - button.lastChangedAt >= BUTTON_DEBOUNCE_MS) {
    button.stableState = rawState;
    return button.stableState == LOW;
  }

  return false;
}

void updateMpu() {
  if (!mpuFound || millis() - lastMpuReadAt < MPU_READ_INTERVAL_MS) {
    return;
  }
  lastMpuReadAt = millis();

  sensors_event_t acceleration;
  sensors_event_t gyro;
  sensors_event_t sensorTemperature;
  mpu.getEvent(&acceleration, &gyro, &sensorTemperature);
  accelerationX = acceleration.acceleration.x;
  accelerationY = acceleration.acceleration.y;
  accelerationZ = acceleration.acceleration.z;
}

void updateDht() {
  if (!dhtFound || millis() - lastDhtReadAt < DHT_READ_INTERVAL_MS) {
    return;
  }
  lastDhtReadAt = millis();

  const float newHumidity = dht.readHumidity();
  const float newTemperatureC = dht.readTemperature();
  if (!isnan(newHumidity)) {
    humidity = newHumidity;
  }
  if (!isnan(newTemperatureC)) {
    temperatureC = newTemperatureC;
  }
}

int petWalkingX(uint32_t now) {
  const int farthestX = SCREEN_WIDTH - PET_SPRITE_WIDTH;
  const uint32_t roundTrip = static_cast<uint32_t>(farthestX) * 2;
  const uint32_t step = (now / PET_WALK_PIXEL_MS) % roundTrip;
  return step <= farthestX ? step : roundTrip - step;
}

uint32_t playRunDuration() {
  return static_cast<uint32_t>(PLAY_LAP_MS) * PLAY_LAP_COUNT;
}

bool isNapping() {
  return nappingUntil != 0 && millis() < nappingUntil;
}

bool isPlaying() {
  return playRunStartedAt != 0 && millis() - playRunStartedAt < playRunDuration();
}

int playRunX(uint32_t now) {
  const int farthestX = SCREEN_WIDTH - PET_SPRITE_WIDTH;
  const uint32_t lapAge = (now - playRunStartedAt) % PLAY_LAP_MS;
  const float progress = static_cast<float>(lapAge) / PLAY_LAP_MS;
  return progress < 0.5f ? static_cast<int>(progress * 2.0f * farthestX)
                         : static_cast<int>((1.0f - progress) * 2.0f * farthestX);
}

void updatePetTimers() {
  if (nappingUntil != 0 && !isNapping()) {
    nappingUntil = 0;
  }
  if (playRunStartedAt != 0 && !isPlaying()) {
    playRunStartedAt = 0;
  }
}

void updateMenuBall() {
  float xTilt = accelerationX - menuCenterAccelerationX;
  float yTilt = accelerationY - menuCenterAccelerationY;

  if (SWAP_MPU_AXES) {
    const float oldXTilt = xTilt;
    xTilt = yTilt;
    yTilt = oldXTilt;
  }
  xTilt = constrain(xTilt * MENU_X_DIRECTION, -MENU_TILT_LIMIT, MENU_TILT_LIMIT);
  yTilt = constrain(yTilt * MENU_Y_DIRECTION, -MENU_TILT_LIMIT, MENU_TILT_LIMIT);
  const float targetX = MENU_CENTER_X + (xTilt / MENU_TILT_LIMIT) * MENU_BALL_X_RANGE;
  const float targetY = MENU_CENTER_Y + (yTilt / MENU_TILT_LIMIT) * MENU_BALL_Y_RANGE;

  menuBallX += (targetX - menuBallX) * 0.20f;
  menuBallY += (targetY - menuBallY) * 0.20f;

  if (fabsf(xTilt) < MENU_CENTER_DEADZONE && fabsf(yTilt) < MENU_CENTER_DEADZONE) {
    selectedMenuItem = -1;
  } else if (fabsf(xTilt) > fabsf(yTilt)) {
    selectedMenuItem = xTilt > 0.0f ? 1 : 3;
  } else {
    selectedMenuItem = yTilt > 0.0f ? 2 : 0;
  }
}

void checkForShake() {
  if (!mpuFound || currentView != PET_VIEW) {
    return;
  }
  if (millis() < hapticBusyUntil) {
    return;
  }

  const float magnitude = sqrtf(accelerationX * accelerationX +
                                accelerationY * accelerationY +
                                accelerationZ * accelerationZ);
  const uint32_t now = millis();
  if (fabsf(magnitude - STANDARD_GRAVITY) >= SHAKE_THRESHOLD &&
      now - lastShakeAt >= SHAKE_COOLDOWN_MS) {
    lastShakeAt = now;
    nappingUntil = 0;
    shakeAnimationEndsAt = now + 350;
    changePet(SHAKE_JOY_CHANGE, SHAKE_ENERGY_CHANGE, SHAKE_FULLNESS_CHANGE);
    savePet();
    tunePlay(TUNE_ALERT);
    hapticPlay(HAP_ALERT);
  }
}

void openMenu() {
  currentView = MENU_VIEW;
  selectedMenuItem = -1;
  menuBallX = MENU_CENTER_X;
  menuBallY = MENU_CENTER_Y;

  menuCenterAccelerationX = accelerationX;
  menuCenterAccelerationY = accelerationY;
  tunePlay(TUNE_BLIP);
  hapticPlay(HAP_TICK);
}

void chooseMenuItem() {
  if (selectedMenuItem == -1) {
    return;
  }

  const MenuItem &item = MENU_ITEMS[selectedMenuItem];
  changePet(item.joyChange, item.energyChange, item.fullnessChange);
  savePet();
  const uint32_t now = millis();
  nappingUntil = 0;
  heartAnimationEndsAt = 0;
  playRunStartedAt = 0;

  if (item.reaction == NAP_REACTION) {
    petJumpStartedAt = 0;
    nappingPetX = petWalkingX(now);
    nappingUntil = now + NAP_DURATION_MS;
    tunePlay(TUNE_SLEEP);
    hapticPlay(HAP_PET);
  } else if (item.reaction == RUN_REACTION) {
    petJumpStartedAt = 0;
    playRunStartedAt = now;
    heartAnimationEndsAt = now + playRunDuration();
    tunePlay(TUNE_HAPPY);
    hapticPlay(HAP_SELECT);
    ledsFlash(0, 60, 90, 120);
  } else {

    petJumpStartedAt = now;
    if (item.reaction == HEART_REACTION) {
      heartAnimationEndsAt = now + HEARTS_DURATION_MS;
      tunePlay(TUNE_HAPPY);
      hapticPlay(HAP_PET);
    } else {
      tunePlay(TUNE_FEED);
      hapticPlay(HAP_FEED);
    }
  }
  currentView = PET_VIEW;
}

void handleButtons() {
  if (wasPressed(buttonOne)) {
    hapticPlay(HAP_TICK);
    if (currentView == MENU_VIEW) {
      chooseMenuItem();
    } else {
      openMenu();
    }
  }

  if (wasPressed(buttonTwo)) {

    hapticPlay(HAP_TICK);
    currentView = currentView == STATS_VIEW ? PET_VIEW : STATS_VIEW;
  }
}

void drawHeart(int x, int y) {

  display.fillRect(x - 2, y, 2, 2, SSD1306_WHITE);
  display.fillRect(x + 1, y, 2, 2, SSD1306_WHITE);
  display.fillRect(x - 3, y + 2, 7, 2, SSD1306_WHITE);
  display.fillRect(x - 2, y + 4, 5, 1, SSD1306_WHITE);
  display.fillRect(x - 1, y + 5, 3, 1, SSD1306_WHITE);
  display.drawPixel(x, y + 6, SSD1306_WHITE);
}

void drawHearts(uint32_t now, int petX, int petY) {
  if (now >= heartAnimationEndsAt) {
    return;
  }

  const float progress =
      1.0f - static_cast<float>(heartAnimationEndsAt - now) / HEARTS_DURATION_MS;
  const int rise = static_cast<int>(progress * 18.0f);
  drawHeart(petX + 10, petY - 3 - rise);
  drawHeart(petX + 22, petY - 9 - rise / 2);
}

void drawSleepZs(uint32_t now, int petX, int petY) {

  const int rise = static_cast<int>((now / 300UL) % 15UL);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(petX + 23, petY - 2 - rise);
  display.print("z");
  display.setCursor(petX + 28, petY - 8 - rise / 2);
  display.print("z");
}

void drawPet() {
  display.clearDisplay();

  const uint32_t now = millis();
  int petX = isNapping() ? nappingPetX : petWalkingX(now);
  if (isPlaying()) {
    petX = playRunX(now);
  }
  int petY = SCREEN_HEIGHT - PET_SPRITE_HEIGHT;

  if (now < shakeAnimationEndsAt) {
    petX += static_cast<int>(sinf(now / 18.0f) * 3.0f);
  }

  if (!isNapping() && petJumpStartedAt != 0) {
    const uint32_t animationAge = now - petJumpStartedAt;

    if (animationAge < PET_PRE_JUMP_MS) {
      petX += static_cast<int>(sinf(now / 16.0f) * 3.0f);
    } else if (animationAge < PET_PRE_JUMP_MS + PET_JUMP_MS) {
      const float jumpProgress =
          static_cast<float>(animationAge - PET_PRE_JUMP_MS) / PET_JUMP_MS;
      petY -= static_cast<int>(sinf(jumpProgress * PI) * PET_JUMP_HEIGHT);
    } else {
      petJumpStartedAt = 0;
    }
  }

  petX = constrain(petX, 0, SCREEN_WIDTH - PET_SPRITE_WIDTH);
  display.drawBitmap(petX, petY, PET_SPRITE, PET_SPRITE_WIDTH, PET_SPRITE_HEIGHT,
                     SSD1306_WHITE);

  if (isNapping()) {
    drawSleepZs(now, petX, petY);
  }
  drawHearts(now, petX, petY);
}

void drawMenuItem(int item) {
  const int boxWidth = 33;
  const int boxHeight = 12;
  const int boxX = MENU_ITEM_X[item] - boxWidth / 2;
  const int boxY = MENU_ITEM_Y[item] - boxHeight / 2;
  const bool isSelected = item == selectedMenuItem;

  if (isSelected) {
    display.fillRoundRect(boxX, boxY, boxWidth, boxHeight, 3, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  } else {
    display.drawRoundRect(boxX, boxY, boxWidth, boxHeight, 3, SSD1306_WHITE);
    display.setTextColor(SSD1306_WHITE);
  }

  display.setTextSize(1);
  const int labelLength = strlen(MENU_ITEMS[item].label);
  display.setCursor(MENU_ITEM_X[item] - labelLength * 3, MENU_ITEM_Y[item] - 3);
  display.print(MENU_ITEMS[item].label);
  display.setTextColor(SSD1306_WHITE);
}

void drawMenu() {
  display.clearDisplay();

  display.drawLine(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, 0, SSD1306_WHITE);
  for (int item = 0; item < MENU_ITEM_COUNT; item++) {
    drawMenuItem(item);
  }

  display.drawCircle(MENU_CENTER_X, MENU_CENTER_Y, 11, SSD1306_WHITE);
  display.fillCircle(static_cast<int>(menuBallX), static_cast<int>(menuBallY), 4,
                     SSD1306_WHITE);
}

void drawStatBar(int y, const char *label, int value) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4, y);
  display.print(label);
  display.drawRect(48, y, 57, 8, SSD1306_WHITE);
  display.fillRect(49, y + 1, map(value, 0, 100, 0, 55), 6, SSD1306_WHITE);
  display.setCursor(109, y);
  display.print(value);
}

void drawStats() {
  display.clearDisplay();
  drawStatBar(5, "JOY", pet.joy);
  drawStatBar(20, "ENERGY", pet.energy);
  drawStatBar(35, "FULL", pet.fullness);

  display.setCursor(4, 53);
  if (!isnan(temperatureC)) {
    display.print("TEMP ");
    display.print(static_cast<int>(temperatureC));
    display.print("C");
  } else {
    display.print("TEMP --");
  }

  display.setCursor(76, 53);
  if (!isnan(humidity)) {
    display.print("H ");
    display.print(static_cast<int>(humidity));
    display.print("%");
  } else {
    display.print("H --");
  }
}

void drawCurrentView() {
  if (currentView == MENU_VIEW) {
    drawMenu();
  } else if (currentView == STATS_VIEW) {
    drawStats();
  } else {
    drawPet();
  }
  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  setUpButton(buttonOne);
  setUpButton(buttonTwo);
  loadPet();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED not found. Check power, GND, SDA, SCL, and address.");
    while (true) {
      delay(10);
    }
  }

  mpuFound = mpu.begin(MPU6050_ADDRESS, &Wire);
  if (mpuFound) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  } else {
    Serial.println("MPU6050 not found. The menu will not move until it is connected.");
  }

  if (USE_DHT11) {
    dht.begin();
    dhtFound = true;
  }

  drawCurrentView();
  expansionSetup();
}

void loop() {
  updateMpu();
  expansionUpdate();
  updateDht();
  updatePetTimers();
  handleButtons();

  if (currentView == MENU_VIEW) {
    updateMenuBall();
  } else {
    checkForShake();
  }

  drawCurrentView();
  delay(16);
}
