/*

 Main project file
 Processes user input via pins and buttons or interrupt sensors converts input into menu action i.e. forward, back, left, right ect
 Displays selected pattern and color on neopixel strings
 Best practice guide at bottom

*/

// Gesture sensing setup
  #include <Wire.h>
  #include <SparkFun_APDS9960.h>
  
  #define APDS9960_INT 19 // Needs to be an interrupt pin
  #define INTURRUPT_PIN 4
  
  SparkFun_APDS9960 apds = SparkFun_APDS9960();
  int isr_flag = 0;

// Neopixel Setup
  #include <Adafruit_NeoPixel.h>
  #ifdef __AVR__
    #include <avr/power.h>
  #endif
  
  #define NUM_LEDS 17
  
  #define BRIGHTNESS 100
  
  #define LED_PIN 10 //23 for mega 10 for pro mini
  
  #define BRIGHTNESS_PIN 11
  
  #define BUTTON_A_PIN 12 // 9 is an interrupt 1/2
  
  #define BUTTON_B_PIN 13 // 10 is an interrupt 2/2
  

//  
//Static Variables
//--------------------
//

    Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

    byte wheel_pos = 0; //Color wheel position
  

//Menu Position 
//--------------------
//
// Unused menu position labels for screens
//
//    String effect_labels[][9] = {
//      {"White", "Rainbow",  "Wipe",     "Chase",    "Spark",      "Starlight",  "Strobe",   "Fire",     "Dissolve",  },
//      {"Temp",  "Speed",    "Speed",    "Speed",    "Speed",      "Speed",      "Speed",    "Speed",    "Speed",},
//      {"Tint",  "Style",    "Variance", "Variance", "Variance",   "Variance",   "Variance", "Variance", "Variance", },
//      {"Lux",   "Lux",      "Color",    "Color",    "Color",      "Color",      "Color",    "Color",    "Color", },
//      {"None",  "Gamma",    "Lux",      "Lux",      "Lux",        "Lux",        "Lux",      "Lux",      "Lux",},
//      {"None",  "None",     "Gamma",    "Gamma",    "Gamma",      "Gamma",      "Gamma",    "Gamma",    "Gamma",  } 
//    };
//    
//    String rainbow_labels[] = {
//      "Cycle", "Strip", "Center", "Random"
//    };
//  
//    String input_labels[] = {
//      "FFT", "Gestgure", "Color", "Moisture"
//    };
    
    byte menu_position[] = {
     1,1
    };
    
    byte max_level[] = {
     4,4
    };
    byte level = 0;
    
    byte last_pos[3];
  
  
  //Options
  //--------------------
    byte color[] = {
      255, 255, 255
    };
    
    byte tint[] = {
      255, 0, 255
    };
    
    byte color_temperature = 0; // 0 - Cold, 255 - Warm
    
    byte lux = 255; // 0 - min, 255 - max
    
    byte gammalvl = 0; // 0 - min, 255 - max
    
    byte animation_speed = 3; //higher is slower
  
    byte variance = 120; // 0 - min, 255 -max

    int c, c2 = 0;

    byte start = 0;
    
union byte2array {
  byte array[3];
 uint32_t integer;
};


//
// Main Program
//================================================================================
/*LOOP DESCRIPTION
 * Check menu positon array state
 * Check input if selected
 * Advance animation 1 frame
 * Set leds accordingly
 * Check controls - if changed set menu pos or setting
 * Repeat
 */

  void setup() {
    Serial.begin(9600);
    
    pinMode(APDS9960_INT, INPUT);
    attachInterrupt(INTURRUPT_PIN, interruptRoutine, FALLING);
     // Initialize APDS-9960 (configure I2C and initial values)
    if ( apds.init() ) {
      Serial.println(F("APDS-9960 initialization complete"));
    } else {
      Serial.println(F("Something went wrong during APDS-9960 init!"));
    }
    // Start running the APDS-9960 gesture sensor engine
    if ( apds.enableGestureSensor(true) ) {
      Serial.println(F("Gesture sensor is now running"));
    } else {
      Serial.println(F("Something went wrong during gesture sensor init!"));
    }
    
    strip.setBrightness(BRIGHTNESS);
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    rainbowCycle(animation_speed);
    animation_speed = 30;

    
}

void loop() {
  byte2array rgb_converter;
  
  // Main Menu 
  //==================================================================================================================
    
    //Battery level check
    //Brightness
      strip.setBrightness(BRIGHTNESS);

    switch (menu_position[0]){ 
      case 1: // Option 1: White - Adjustable color temperature & tint(todo)
        fullWhite(color_temperature);
        color_temperature = incrementer(color_temperature);
      break;
         
      case 2: // Option 2: Rainbow white - Casts a white light from rainbow colored pixels
        wheel_pos = rainbowCycleTwo(animation_speed, wheel_pos);
      break;

      case 3: // Option 3: Color Selector - Allows the user to select a static color
        wheel_pos = incrementer(wheel_pos);
        rgb_converter.integer = Wheel(wheel_pos);
        colorWipe(strip.Color(rgb_converter.array[0],rgb_converter.array[1],rgb_converter.array[2]), 50);
        Serial.println(wheel_pos);
      break;
    };
    //Option 4 - Features - Allows the user to overlay features with compatible colors i.e white, rainbow, color picker
    // - Sound to light VU
    // - Sound to light FFT
    // - Motion to light PIR
    // - Gesture to light

    //Menu Navigation
    //================================
    copy(menu_position, last_pos, 3);
    //Gesture Menu
    gestureSense();
    //Button Menu
    checkButtons();
    //
}


// Function to copy 'len' elements from 'src' to 'dst'
void copy(byte* src, byte* dst, int len) {
    memcpy(dst, src, sizeof(src[0])*len);
}

    
    // Some example procedures showing how to display to the pixels:
    //  colorWipe(strip.Color(255, 0, 0), 50); // Red
    //  colorWipe(strip.Color(0, 255, 0), 50); // Green
    //  colorWipe(strip.Color(0, 0, 255), 50); // Blue
    //  colorWipe(strip.Color(255, 255, 255, 255), 50); // White
    //
    //  whiteOverRainbow(20,75,5);  
    //
    //  pulseWhite(5); 
    // delay(2000);

/*
 * NEOPIXEL CONNECTION BEST PRACTICES (From: Adafruit Neopixel Uber Guide)
 * Before connecting NeoPixels to any large power source (DC “wall wart” or even a large battery), 
 * add a capacitor (1000 µF, 6.3V or higher) across the + and – terminals as shown above. 
 * The capacitor buffers sudden changes in the current drawn by the strip.
 * 
 * Place a 300 to 500 Ohm resistor between the Arduino data output pin and the input to the first NeoPixel. 
 * The resistor should be at the end of the wire closest to the NeoPixel(s), not the microcontroller. 
 * Some products already incorporate this resistor…if you’re not sure, add one…there’s no harm in doubling up!
 * 
 * Try to minimize the distance between the Arduino and first pixel, so the signal is clear.  
 * A meter or two is usually no problem. Much longer and things can become unreliable.
 * 
 * Avoid connecting NeoPixels to a live circuit. If you simply must, always connect ground first, 
 * then +5V, then data. Disconnect in the reverse order.
 * 
 * If powering the pixels with a separate supply, apply power to the pixels before applying 
 * power to the microcontroller.
 * 
 * Observe the same precautions as you would for any static-sensitive part; ground yourself before handling, etc.
 * 
 * NeoPixels powered by 5v require a 5V data signal. If using a 3.3V microcontroller you must use a logic level 
 * shifter such as a 74AHCT125 or 74HCT245. (If you are powering your NeoPixels with 3.7v like from a LiPoly, 
 * a 3.3v data signal is OK).
 * 
 * If your microcontroller and NeoPixels are powered from two different sources 
 * (e.g. separate batteries for each), there must be a ground connection between the two.
 */
