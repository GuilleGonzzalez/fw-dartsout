#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define WIFI_SSID "TycheTools"
#define WIFI_PASSWORD "DanilleX2023"
// #define WIFI_SSID "Gonmar-Livebox"
// #define WIFI_PASSWORD "618995151609549464"

#define PIN_LED1 1
#define PIN_LED2 2

#define BOARD_ID 0

typedef enum {
	N_INVALID = -1,
	BULL = 0,
	N1 = 1,
	N2 = 2,
	N3 = 3,
	N4 = 4,
	N5 = 5,
	N6 = 6,
	N7 = 7,
	N8 = 8,
	N9 = 9,
	N10 = 10,
	N11 = 11,
	N12 = 12,
	N13 = 13,
	N14 = 14,
	N15 = 15,
	N16 = 16,
	N17 = 17,
	N18 = 18,
	N19 = 19,
	N20 = 20,
} numbers_t;

typedef enum {
	TRIPLE,
	DOUBLE,
	SINGLE_INT,
	SINGLE_EXT,
	Z_INVALID,
} zones_t;

/** Global variables **********************************************************/

#define NOT_DET  -1
#define ERR_READ -2

// Decathlon dartboard
#define N_ROWS   7
#define N_COLS   10
#define ROWS_OFF 2 // First row pin
#define COLS_OFF 0 // First col pin

static int map_numbers[N_ROWS][N_COLS] = {
	{N7, N19, N3, N17, N2, N15, N16, N8, N11, N14},
	{N7, N19, N3, N17, N2, N15, N16, N8, N11, N14},
	{N7, N19, N3, N17, N2, N15, N16, N8, N11, N14},
	{N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, BULL, BULL},
	{N1, N18, N4, N13, N6, N10, N20, N5, N12, N9},
	{N1, N18, N4, N13, N6, N10, N20, N5, N12, N9},
	{N1, N18, N4, N13, N6, N10, N20, N5, N12, N9},
};

static int map_zones[N_ROWS][N_COLS] = {
	{TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE},
	{DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE},
	{SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT},
	{Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, SINGLE_EXT, DOUBLE},
	{SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT},
	{DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE},
	{TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE},
};

// Chino dartboard
// #define N_ROWS   8
// #define N_COLS   8
// #define ROWS_OFF 2 // First row pin
// #define COLS_OFF 0 // First col pin
// static int map_numbers[N_ROWS][N_COLS] = {
// 	{N19, N7, N16, N8, N11, N14, N9, N12},
// 	{N19, N7, N16, N8, N11, N14, N9, N12},
// 	{N19, N7, N16, N8, N11, N14, N9, N12},
// 	{N3, N17, N2, N15, N10, N6, N13, N4},
// 	{N3, N17, N2, N15, N10, N6, N13, N4},
// 	{N3, N17, N2, N15, N10, N6, N13, N4},
// 	{Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, BULL, Z_INVALID},
// 	{Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, BULL, Z_INVALID},
// };

// static int map_zones[N_ROWS][N_COLS] = {
// 	{DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE},
// 	{SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT},
// 	{TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE},
// 	{TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE},
// 	{SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT},
// 	{DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE},
// 	{Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, SINGLE_EXT, Z_INVALID},
// 	{Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, DOUBLE, Z_INVALID},
// };

static int pin_rows[] = {10, 9, 3, 8, 18, 17, 7, 6, 5, 4};
static int pin_cols[] = {37, 36, 35, 34, 33, 21, 14, 13, 12, 11};

static volatile int row_pressed = NOT_DET;
static volatile int col_pressed = NOT_DET;

static String server = "http://192.168.0.39:8000/";

/** Function prototypes *******************************************************/

static void IRAM_ATTR dart_cb(void* arg);

static void keypad_read();
static void set_rows_high();
static void set_rows_low();
static void print_value(int i, int j);
static void keypad_setup();

/** Callbacks *****************************************************************/

static void IRAM_ATTR dart_cb(void* arg)
{
	if (col_pressed != NOT_DET) {
		return;
	}
	col_pressed = (int)arg;
	for (int i = 0; i < N_ROWS; i++) {
		digitalWrite(pin_rows[i+ROWS_OFF], HIGH);
		if (digitalRead(pin_cols[col_pressed]) == HIGH) {
			if (row_pressed != NOT_DET) {
				row_pressed = ERR_READ; // Varios pulsados / pulsación rápida
				break;
			}
			row_pressed = i;
		}
		digitalWrite(pin_rows[i+ROWS_OFF], LOW);
	}
}

/** Function definitions ******************************************************/

static void set_rows_low()
{
	for (int i = 0; i < N_ROWS; i++) {
		digitalWrite(pin_rows[i+ROWS_OFF], LOW);
	}
}

static void set_rows_high()
{
	for (int i = 0; i < N_ROWS; i++) {
		digitalWrite(pin_rows[i+ROWS_OFF], HIGH);
	}
}

static void print_value(int i, int j)
{
	Serial.print("Num: ");
	Serial.print(map_numbers[i][j]);
	Serial.print(", zone: ");
	switch(map_zones[i][j]) {
	case SINGLE_EXT:
		Serial.print("SINGLE EXT");
		break;
	case SINGLE_INT:
		Serial.print("SINGLE INT");
		break;
	case DOUBLE:
		Serial.print("DOUBLE");
		break;
	case TRIPLE:
		Serial.print("TRIPLE");
		break;
	default:
		Serial.print("ERROR");
		break;
	}
	Serial.println();
}

static void keypad_setup()
{
	for (int i = 0; i < N_ROWS; i++) {
		pinMode(pin_rows[i+ROWS_OFF], OUTPUT);
		digitalWrite(pin_rows[i+ROWS_OFF], HIGH);
	}
	for (int i = 0; i < N_COLS; i++) {
		pinMode(pin_cols[i+COLS_OFF], INPUT_PULLUP);
		attachInterruptArg(pin_cols[i+COLS_OFF], dart_cb,
				(void*)(i+COLS_OFF), FALLING);
	}
}

static void send_dart(int num, int zone)
{
	if (WiFi.status() == WL_CONNECTED) {
		WiFiClient client;
		HTTPClient http;
		String serverPath = server + "new-dart";
		http.begin(client, serverPath.c_str());
		http.addHeader("Content-Type", "application/json");
		StaticJsonDocument<200> json;
		json["board_id"] = BOARD_ID;
		json["num"] = num;
		json["zone"] = zone;
		String json_str;
		serializeJson(json, json_str);
		int httpResponseCode = http.POST(json_str);
		if (httpResponseCode > 0) {
			Serial.print("HTTP Response code: ");
			Serial.println(httpResponseCode);
			String payload = http.getString();
			Serial.println(payload);
		} else {
			Serial.print("Error code: ");
			Serial.println(httpResponseCode);
		}
		http.end();
	} else {
		Serial.println("WiFi Disconnected");
	}
}

/* Setup **********************************************************************/

void setup()
{
	Serial.begin(115200);

	pinMode(PIN_LED1, OUTPUT);
	pinMode(PIN_LED2, OUTPUT);

	digitalWrite(PIN_LED1, LOW);
	digitalWrite(PIN_LED2, LOW);

	keypad_setup();
	set_rows_low();

	Serial.print("Connecting to WiFi");
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(100);
	}

	Serial.println("");
	Serial.print("Connected to WiFi network with IP Address: ");
	Serial.println(WiFi.localIP());

	digitalWrite(PIN_LED1, HIGH);
}

static void keypad_read()
{
	for (int i = 0; i < N_ROWS; i++) {
		set_rows_high();
		digitalWrite(pin_rows[i+ROWS_OFF], LOW);
		for (int j = 0; j < N_COLS; j++) {
			if (digitalRead(pin_cols[j+COLS_OFF]) == 0) {
				Serial.print("Pressed: ");
				Serial.print(i-1); // Starts in 1 and no first cable
				Serial.print(",");
				Serial.println(j+1); // Starts in 1
				delay(50);
				return;
			}
		}
	}
}

/* Loop ***********************************************************************/

void loop()
{	
	if (row_pressed != NOT_DET || col_pressed != NOT_DET) {
		digitalWrite(PIN_LED2, HIGH);
		// TODO: Lectura lenta para ver si hay más de un botón pulsado
		Serial.print("Row: ");
		Serial.print(row_pressed + 1);
		Serial.print(", col: ");
		Serial.println(col_pressed + 1);
		print_value(row_pressed, col_pressed);
		delay(100);
		set_rows_low();
		if (row_pressed >= 0 && col_pressed >= 0) {
			int num = map_numbers[row_pressed][col_pressed];
			int zone = map_zones[row_pressed][col_pressed];
			send_dart(num, zone);
		}
		row_pressed = NOT_DET;
		col_pressed = NOT_DET;
		Serial.println("Ready");
		digitalWrite(PIN_LED2, LOW);
	}
	// keypad_read();
}
