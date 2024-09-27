#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"


// TODO: server in config files
#define SERVER_STR "http://dartsout.duckdns.org/"

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

// Decathlon board CONN#1
// #define N_ROWS   7
// #define N_COLS   10
// #define ROWS_OFF 2 // First row pin
// #define COLS_OFF 0 // First col pin

// static int map_numbers[N_ROWS][N_COLS] = {
// 	{N7, N19, N3, N17, N2, N15, N16, N8, N11, N14},
// 	{N7, N19, N3, N17, N2, N15, N16, N8, N11, N14},
// 	{N7, N19, N3, N17, N2, N15, N16, N8, N11, N14},
// 	{N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, BULL, BULL},
// 	{N1, N18, N4, N13, N6, N10, N20, N5, N12, N9},
// 	{N1, N18, N4, N13, N6, N10, N20, N5, N12, N9},
// 	{N1, N18, N4, N13, N6, N10, N20, N5, N12, N9},
// };

// static int map_zones[N_ROWS][N_COLS] = {
// 	{TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE},
// 	{DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE},
// 	{SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT},
// 	{Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, SINGLE_EXT, DOUBLE},
// 	{SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT},
// 	{DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE},
// 	{TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE},
// };

// Decathlon board CONN#2
#define N_ROWS   7
#define N_COLS   10
#define ROWS_OFF 1 // First row pin
#define COLS_OFF 0 // First col pin

static int map_numbers[N_ROWS][N_COLS] = {
	{N9, N12, N5, N20, N10, N6, N13, N4, N18, N1},
	{N9, N12, N5, N20, N10, N6, N13, N4, N18, N1},
	{N9, N12, N5, N20, N10, N6, N13, N4, N18, N1},
	{BULL, BULL, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID, N_INVALID},
	{N14, N11, N8, N16, N15, N2, N17, N3, N19, N7},
	{N14, N11, N8, N16, N15, N2, N17, N3, N19, N7},
	{N14, N11, N8, N16, N15, N2, N17, N3, N19, N7},
};

static int map_zones[N_ROWS][N_COLS] = {
	{TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE, TRIPLE},
	{DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE},
	{SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT, SINGLE_EXT},
	{DOUBLE, SINGLE_EXT, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID, Z_INVALID},
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

static String server_str = SERVER_STR;

AsyncWebServer server(80);

static const char* param_input_1 = "ssid";
static const char* param_input_2 = "pass";
static const char* ssid_path = "/ssid.txt";
static const char* pass_path = "/pass.txt";

static String ssid;
static String pass;

static unsigned long previousMillis = 0;
static const long interval = 10000; // Interval to wait for Wi-Fi connection (milliseconds)


/** Function prototypes *******************************************************/

static void IRAM_ATTR dart_cb(void* arg);

static void keypad_read();
static void set_rows_high();
static void set_rows_low();
static void print_value(int i, int j);
static void keypad_setup();
static void read_mac_addr();
static int get_dartboard_id();
static void init_littlefs();
static String read_file(fs::FS &fs, const char * path);
static void write_file(fs::FS &fs, const char * path, const char * message);
static bool init_wifi();

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
	switch (map_zones[i][j]) {
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
		String serverPath = server_str + "new-dart?id=0";
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

static void read_mac_addr()
{
	uint8_t mac[6];
	esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, mac);
	if (ret == ESP_OK) {
		Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	} else {
		Serial.println("Failed to read MAC address");
	}
}

static int get_dartboard_id()
{
	uint8_t mac[6];
	if (esp_wifi_get_mac(WIFI_IF_STA, mac) == ESP_OK) {
		return (mac[3] << 16) | (mac[4] << 8) | (mac[5] << 0);
	} else {
		Serial.println("Failed to get dartboard ID");
		return 1;
	}
}

static void init_littlefs()
{
	if (!LittleFS.begin(true)) {
		Serial.println("An error has occurred while mounting LittleFS");
	}
	Serial.println("LittleFS mounted successfully");
}


static String read_file(fs::FS &fs, const char * path)
{
	Serial.printf("Reading file: %s\r\n", path);

	File file = fs.open(path);
	if (!file || file.isDirectory()) {
		Serial.println("- failed to open file for reading");
		return String();
	}
	
	String fileContent;
	while (file.available()) {
		fileContent = file.readStringUntil('\n');
		break;
	}
	return fileContent;
}

static void write_file(fs::FS &fs, const char * path, const char * message)
{
	Serial.printf("Writing file: %s\r\n", path);

	File file = fs.open(path, FILE_WRITE);
	if (!file){
		Serial.println("- failed to open file for writing");
		return;
	}
	if (file.print(message)){
		Serial.println("- file written");
	} else {
		Serial.println("- write failed");
	}
}

static bool init_wifi()
{
	if (ssid == ""){
		Serial.println("Undefined SSID");
		return false;
	}

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid.c_str(), pass.c_str());
	Serial.println("Connecting to WiFi...");

	unsigned long currentMillis = millis();
	previousMillis = currentMillis;

	while (WiFi.status() != WL_CONNECTED) {
		currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			Serial.println("Failed to connect.");
			return false;
		}
	}

	Serial.println(WiFi.localIP());

	return true;
}

/* Setup **********************************************************************/

void setup()
{
	Serial.begin(115200);

	init_littlefs();

	pinMode(PIN_LED1, OUTPUT);
	pinMode(PIN_LED2, OUTPUT);

	digitalWrite(PIN_LED1, LOW);
	digitalWrite(PIN_LED2, LOW);

	keypad_setup();
	set_rows_low();

	ssid = read_file(LittleFS, ssid_path);
	pass = read_file(LittleFS, pass_path);
	Serial.println(ssid);
	Serial.println(pass);

	// Serial.print("Connecting to WiFi");
	// WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	// while (WiFi.status() != WL_CONNECTED) {
	// 	Serial.print(".");
	// 	delay(100);
	// }

	// Serial.println("");
	// Serial.print("Connected to WiFi network with IP Address: ");
	// Serial.println(WiFi.localIP());

	if (init_wifi()) {
		digitalWrite(PIN_LED1, HIGH);
		server.begin();
	} else {
		digitalWrite(PIN_LED1, LOW);
		Serial.println("Setting AP (Access Point)");
		WiFi.softAP("Dartsout Dartboard", NULL);

		IPAddress IP = WiFi.softAPIP();
		Serial.print("AP IP address: ");
		Serial.println(IP); 

		server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
			request->send(LittleFS, "/wifi_manager.html", "text/html");
		});
		server.serveStatic("/", LittleFS, "/");
		server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
			int params = request->params();
			for (int i = 0; i < params; i++) {
				const AsyncWebParameter* p = request->getParam(i);
				if (p->isPost()) {
					if (p->name() == param_input_1) {
						ssid = p->value().c_str();
						Serial.print("SSID set to: ");
						Serial.println(ssid);
						write_file(LittleFS, ssid_path, ssid.c_str());
					}
					if (p->name() == param_input_2) {
						pass = p->value().c_str();
						Serial.print("Password set to: ");
						Serial.println(pass);
						write_file(LittleFS, pass_path, pass.c_str());
					}
					
					//Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
				}
			}
			request->send(200, "text/plain", "Done. ESP will restart");
			delay(3000);
			ESP.restart();
		});
		server.begin();
	}

	Serial.println("");
	Serial.print("MAC address: ");
	read_mac_addr();
	Serial.print("Dartboard ID: ");
	Serial.printf("%X\n", get_dartboard_id());
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
