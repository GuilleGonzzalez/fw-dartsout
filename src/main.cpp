

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"

#include "leds.h"

#include "dartsout_boardv2.h"
#include "decathlon_dartboard.h"

#define AP_STR   "Dartsout Dartboard"
#define NOT_DET  -1
#define ERR_READ -2

/** Global variables **********************************************************/

extern int map_numbers[N_ROWS][N_COLS];
extern int map_zones[N_ROWS][N_COLS];

// TODO: move to boards

// Dartsout dartboard V1
// static int pin_rows[] = {10, 9, 3, 8, 18, 17, 7, 6, 5, 4};
// static int pin_cols[] = {37, 36, 35, 34, 33, 21, 14, 13, 12, 11};

// Dartsout dartboard V2
static int pin_rows[] = {-1, 1, 2, 42, 41, 40, 39, 38, -1, -1};
static int pin_cols[] = {37, 36, 35, 34, 33, 21, 14, 13, 12, 11};

static volatile int row_pressed = NOT_DET;
static volatile int col_pressed = NOT_DET;

static const char* param_ssid = "ssid";
static const char* param_pass = "pass";
static const char* param_server = "server";
static const char* ssid_path = "/ssid.txt";
static const char* pass_path = "/pass.txt";
static const char* server_path = "/server.txt";

static String ssid;
static String pass;
static String server_url;

static unsigned long previousMillis = 0;
static const long interval = 10000; // Interval to wait for Wi-Fi conn (ms)

static bool flag_next_player = false;

static int dartboard_id = 0;

// TODO: move to LEDs
static int leds_num[20][3] = {
	{27, 28, 29}, // 1
	{4, 5, 6}, // 2
	{64, 65, 66}, // 3
	{20, 21, 22}, // 4
	{34, 35, 35}, // 5
	{14, 15, 16}, // 6
	{57, 58, 59}, // 7
	{50, 51, 52}, // 8
	{40, 41, 42}, // 9
	{10, 11, 12}, // 10
	{47, 48, 49}, // 11
	{37, 38, 39}, // 12
	{17, 18, 19}, // 13
	{44, 45, 46}, // 14
	{7, 8, 9}, // 15
	{54, 55, 56}, // 16
	{0, 1, 2}, // 17
	{24, 25, 26}, // 18
	{60, 61, 62}, // 19
	{30, 31, 32}, // 20
};


AsyncWebServer server(80);

/** Function prototypes *******************************************************/

static void IRAM_ATTR dart_cb(void* arg);
static void IRAM_ATTR btn_player_cb(void* arg);

static void set_rows_low();
static void set_rows_high();
static void print_value(int i, int j);
static void keypad_setup();
static void buttons_setup();
static void send_next_player();
static void send_dart(int num, int zone);
static void keypad_read();
static void read_mac_addr();
static int get_dartboard_id();
static void init_littlefs();
static String read_file(fs::FS &fs, const char* path);
static void write_file(fs::FS &fs, const char* path, const char* message);
static bool init_wifi();
static String processor(const String &var);
static void setAP();
static void server_config();

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

static void IRAM_ATTR btn_player_cb(void* arg)
{
	flag_next_player = true;
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
	Serial.printf("Num: %d, zone: ", map_numbers[i][j]);
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

static void buttons_setup()
{
	// TODO: iterate buttons
	for (int i = 0; i < 1; i++) {
		//TODO: define buttons SW5(PLAYER) -> GPIO6
		pinMode(BTN_PLAYER, INPUT);
		attachInterruptArg(BTN_PLAYER, btn_player_cb, NULL, FALLING);
	}
}

static void send_next_player()
{
	if (WiFi.status() == WL_CONNECTED) {
		WiFiClient client;
		HTTPClient http;
		//TODO: send board_id instead hardcoded id=0
		String serverPath = server_url + "next-player?id=0";
		Serial.println(serverPath);
		http.begin(client, serverPath.c_str());
		int httpResponseCode = http.GET();
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
static void send_dart(int num, int zone)
{
	if (WiFi.status() == WL_CONNECTED) {
		WiFiClient client;
		HTTPClient http;
		String serverPath = server_url + "new-dart";
		http.begin(client, serverPath.c_str());
		http.addHeader("Content-Type", "application/json");
		JsonDocument json;
		json["board_id"] = dartboard_id;
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

static void keypad_read()
{
	for (int i = 0; i < N_ROWS; i++) {
		set_rows_high();
		digitalWrite(pin_rows[i+ROWS_OFF], LOW);
		for (int j = 0; j < N_COLS; j++) {
			if (digitalRead(pin_cols[j+COLS_OFF]) == 0) {
				// i: starts in 1 and no first cable, j: starts in 1
				Serial.printf("Pressed: %d, %d\n", i-1, j+1);
				delay(50);
				return;
			}
		}
	}
}

static void read_mac_addr()
{
	uint8_t mac[6];
	esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, mac);
	if (ret == ESP_OK) {
		Serial.printf("MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
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

static String read_file(fs::FS &fs, const char* path)
{
	Serial.printf("Reading file: %s\r\n", path);

	File file = fs.open(path);
	if (!file || file.isDirectory()) {
		Serial.println("Failed to open file for reading");
		return String();
	}
	
	String fileContent;
	while (file.available()) {
		fileContent = file.readStringUntil('\n');
		break;
	}
	return fileContent;
}

static void write_file(fs::FS &fs, const char* path, const char* message)
{
	Serial.printf("Writing file: %s\r\n", path);

	File file = fs.open(path, FILE_WRITE);
	if (!file){
		Serial.println("Failed to open file for writing");
		return;
	}
	if (file.print(message)){
		Serial.println("File written");
	} else {
		Serial.println("Write failed");
	}
}

static void delete_file(fs::FS &fs, const char* path)
{
	Serial.printf("Deleting file: %s\r\n", path);

	if (fs.remove(path)) {
		Serial.println("File deleted");
	} else {
		Serial.println("Delete failed");
	}
}

static bool init_wifi()
{
	if (ssid == "") {
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

static String processor(const String &var)
{
	if (var == "IP") {
		return WiFi.localIP().toString();
	} else if (var == "SERVER") {
		return server_url;
	} else if (var == "SSID") {
		return ssid;
	}
	return String();
}

static void setAP()
{
	Serial.println("Setting AP");
	WiFi.softAP(AP_STR, NULL);

	IPAddress ip = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(ip);

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(LittleFS, "/wifi_manager.html", "text/html");
	});
}

static void server_config()
{
	ssid = read_file(LittleFS, ssid_path);
	pass = read_file(LittleFS, pass_path);
	server_url = read_file(LittleFS, server_path);
	Serial.print("SSID: ");
	Serial.println(ssid);
	Serial.print("Password: ");
	Serial.println(pass);
	Serial.print("Server: ");
	Serial.println(server_url);

	if (init_wifi()) {
		digitalWrite(PIN_LED1, HIGH);
		server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
			request->send(LittleFS, "/index.html", "text/html", false,
					processor);
		});
	} else {
		//TODO: when AP mode, led blinking
		setAP();
		digitalWrite(PIN_LED1, LOW);
	}
	server.serveStatic("/", LittleFS, "/");
	server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
		int params = request->params();
		for (int i = 0; i < params; i++) {
			const AsyncWebParameter* p = request->getParam(i);
			if (p->isPost()) {
				if (p->name() == param_ssid) {
					ssid = p->value().c_str();
					Serial.print("SSID set to: ");
					Serial.println(ssid);
					write_file(LittleFS, ssid_path, ssid.c_str());
				} else if (p->name() == param_pass) {
					pass = p->value().c_str();
					Serial.print("Password set to: ");
					Serial.println(pass);
					write_file(LittleFS, pass_path, pass.c_str());
				} else if (p->name() == param_server) {
					server_url = p->value().c_str();
					Serial.print("Server URL set to: ");
					Serial.println(server_url);
					write_file(LittleFS, server_path, server_url.c_str());
				}
			}
		}
		request->send(200, "text/plain", "Done. ESP will restart");
		delay(2000);
		ESP.restart();
	});
	server.begin();
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
	buttons_setup();
	set_rows_low();

	server_config();

	leds_init();

	read_mac_addr();
	dartboard_id = get_dartboard_id();
	Serial.printf("Dartboard ID: %X\n", dartboard_id);
}

static void rainbow_loop_leds()
{
	static long last_time = 0;
	static uint32_t first_pixel_hue = 0;
	long now = millis();

	if (now - last_time > 10) {
		leds_rainbow(first_pixel_hue);
		first_pixel_hue += 200;
		last_time = now;
		leds_show();
	}
}

static void set_sector_led(int num, int color)
{
	leds_set_color(0);
	for (int i = 0; i < 3; i++) {
		leds_set_pixel_color(leds_num[num][i], color);
	}
	leds_show();
}

static void show_leds(int num, int zone)
{
	if (num == 0) {
		if (zone == DOUBLE) {
			leds_color_wipe(leds_color(0, 255, 0, 0), 10);
		} else {
			leds_color_wipe(leds_color(0, 0, 0, 255), 10);
		}
	} else {
		switch (zone) {
		case SINGLE_EXT:
		case SINGLE_INT:
			set_sector_led(num-1, leds_color(0, 0, 0, 255));
			break;
		case DOUBLE:
			set_sector_led(num-1, leds_color(0, 0, 0, 255));
			delay(100);
			set_sector_led(num-1, 0);
			delay(200);
			set_sector_led(num-1, leds_color(0, 0, 0, 255));
			break;
		case TRIPLE:
			set_sector_led(num-1, leds_color(0, 0, 0, 255));
			delay(100);
			set_sector_led(num-1, 0);
			delay(200);
			set_sector_led(num-1, leds_color(0, 0, 0, 255));
			delay(100);
			set_sector_led(num-1, 0);
			delay(200);
			set_sector_led(num-1, leds_color(0, 0, 0, 255));
			break;
		}
	}
	delay(1000);
}

/* Loop ***********************************************************************/


void loop()
{
	rainbow_loop_leds();

	if (flag_next_player) {
		Serial.println("Next player!");
		send_next_player();
		delay(1000);
		flag_next_player = false;
	}


	if (digitalRead(BTN_POWER) == 0) {
		Serial.println("Power button");
		int set_factory_reset_flag = 1;
		for (int i = 0; i < 5000; i++) {
			delay(1);
			if (digitalRead(BTN_POWER) == 1) {
				set_factory_reset_flag = 0;
				break;
			}
		}
		if (set_factory_reset_flag) {
			Serial.println("Factory reset");
			//TODO: mejor un factory reset. Si está disponible el wifi, se va a conectar de nuevo y no se va a poder configurar
			// Factory reset sería borrar los files y hacer un restart
			delete_file(LittleFS, ssid_path);
			delete_file(LittleFS, pass_path);
			delete_file(LittleFS, server_path);
			delay(2000);
			ESP.restart();
			// setAP();
		}
	}

	if (row_pressed != NOT_DET || col_pressed != NOT_DET) {
		digitalWrite(PIN_LED2, HIGH);
		// TODO: Lectura lenta para ver si hay más de un botón pulsado
		Serial.printf("Row: %d, col: %d\n", row_pressed + 1, col_pressed + 1);
		print_value(row_pressed, col_pressed);
		delay(100);
		set_rows_low();
		if (row_pressed >= 0 && col_pressed >= 0) {
			int num = map_numbers[row_pressed][col_pressed];
			int zone = map_zones[row_pressed][col_pressed];
			send_dart(num, zone);
			show_leds(num, zone);

		}
		row_pressed = NOT_DET;
		col_pressed = NOT_DET;
		Serial.println("Ready");
		digitalWrite(PIN_LED2, LOW);
	}
	// keypad_read();
}