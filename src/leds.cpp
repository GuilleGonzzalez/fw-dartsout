#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    38
#define LED_COUNT  66
#define BRIGHTNESS 125 //  0 (min) to 255 (max)

// Declare our NeoPixel strip object:
static Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

void leds_init()
{
	strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
	strip.show();            // Turn OFF all pixels ASAP
	strip.setBrightness(BRIGHTNESS);
}

void leds_set_pixel_color(int i, uint32_t color)
{
	strip.setPixelColor(i, color);
}

uint32_t leds_color(int r, int g, int b, int w)
{
	return strip.Color(r, g, b), w;
}

void leds_show()
{
	strip.show();
}

// Available functions
// colorWipe(strip.Color(255,   0,   0)     , 50); // Red
// colorWipe(strip.Color(  0, 255,   0)     , 50); // Green
// colorWipe(strip.Color(  0,   0, 255)     , 50); // Blue
// colorWipe(strip.Color(  0,   0,   0, 255), 50); // True white (not RGB white)
// whiteOverRainbow(75, 5);
// pulseWhite(5);
// rainbowFade2White(3, 3, 1);


// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void leds_set_color(uint32_t color)
{
	for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
		strip.setPixelColor(i, color);           //  Set pixel's color (in RAM)
	}
}
void leds_color_wipe(uint32_t color, int wait)
{
	for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
		strip.setPixelColor(i, color);           //  Set pixel's color (in RAM)
		strip.show();                            //  Update strip to match
		delay(wait);                             //  Pause for a moment
	}
}

void leds_rainbow(uint32_t firstPixelHue)
{
	for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
		int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
		strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
	}
}

void leds_white_over_rainbow(int white_speed, int white_len)
{
	if (white_len >= strip.numPixels()) {
		white_len = strip.numPixels() - 1;
	}

	int head = white_len - 1;
	int tail = 0;
	int loops = 3;
	int loopNum = 0;
	uint32_t lastTime = millis();
	uint32_t firstPixelHue = 0;

	for (;;) {
		for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
			if (((i >= tail) && (i <= head)) ||       //  If between head & tail...
					((tail > head) && ((i >= tail) || (i <= head)))) {
				strip.setPixelColor(i, strip.Color(0, 0, 0, 255)); // Set white
			} else {                                              // else set rainbow
				int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
				strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
			}
		}

		strip.show(); // Update strip with new contents
		// There's no delay here, it just runs full-tilt until the timer and
		// counter combination below runs out.
		firstPixelHue += 40; // Advance just a little along the color wheel
		if ((millis() - lastTime) > white_speed) { // Time to update head/tail?
			if (++head >= strip.numPixels()) {     // Advance head, wrap around
				head = 0;
				if (++loopNum >= loops) {
					return;
				}
			}
			if (++tail >= strip.numPixels()) { // Advance tail, wrap around
				tail = 0;
			}
			lastTime = millis(); // Save time of last movement
		}
	}
}

void leds_pulse_white(uint8_t wait)
{
	for (int j = 0; j < 256; j++) { // Ramp up from 0 to 255
		// Fill entire strip with white at gamma-corrected brightness level 'j':
		strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
		strip.show();
		delay(wait);
	}

	for(int j = 255; j >= 0; j--) { // Ramp down from 255 to 0
		strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
		strip.show();
		delay(wait);
	}
}

void leds_rainbow_fade_to_white(int wait, int rainbow_loops, int white_loops)
{
	int fadeVal = 0;
	int fadeMax = 100;

	// Hue of first pixel runs 'rainbow_loops' complete loops through the color
	// wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
	// just count from 0 to rainbow_loops*65536, using steps of 256 so we
	// advance around the wheel at a decent clip.
	for (uint32_t firstPixelHue = 0; firstPixelHue < rainbow_loops * 65536; firstPixelHue += 256) {
		for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
			// Offset pixel hue by an amount to make one full revolution of the
			// color wheel (range of 65536) along the length of the strip
			// (strip.numPixels() steps):
			uint32_t pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());

			// strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
			// optionally add saturation and value (brightness) (each 0 to 255).
			// Here we're using just the three-argument variant, though the
			// second value (saturation) is a constant 255.
			strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, 255,
				255 * fadeVal / fadeMax)));
		}

		strip.show();
		delay(wait);

		if (firstPixelHue < 65536) { // First loop,
			if (fadeVal < fadeMax) {
				fadeVal++; // Fade in
			}
		} else if (firstPixelHue >= ((rainbow_loops-1) * 65536)) { // Last loop,
			if(fadeVal > 0) {
				fadeVal--; // Fade out
			}
		} else {
			fadeVal = fadeMax; // Interim loop, make sure fade is at max
		}
	}

	for (int k = 0; k < white_loops; k++) {
		for (int j = 0; j < 256; j++) { // Ramp up 0 to 255
			// Fill entire strip with white at gamma-corrected brightness level 'j':
			strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
			strip.show();
		}
		delay(1000);
		for (int j = 255; j >= 0; j--) { // Ramp down 255 to 0
			strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
			strip.show();
		}
	}

	delay(500);
}
