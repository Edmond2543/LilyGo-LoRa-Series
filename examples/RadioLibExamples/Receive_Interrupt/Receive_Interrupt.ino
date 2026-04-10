/*
   RadioLib Receive with Interrupts Example
   Modifié pour : SX1262 (T-Beam Supreme), 915 MHz, WiFi WPA2-Enterprise, API LLM 
   et INTERFACE OLED AVANCÉE.
*/

#include <RadioLib.h>
#include "LoRaBoards.h"
#include <WiFi.h>
#include <HTTPClient.h>

// =============================================
// CONFIGURATION WIFI
// =============================================
#define USE_WPA2_ENTERPRISE  true

// --- WPA2 Personnel ---
const char* WIFI_SSID     = "climoilou";
const char* WIFI_PASSWORD = "MonMotDePasse";

// --- WPA2 Entreprise (EAP-PEAP) ---
const char* EAP_IDENTITY  = "2440312";
const char* EAP_USERNAME  = "2440312";
const char* EAP_PASSWORD  = "Teladmin2026!";

// =============================================
// CONFIGURATION LLM
// =============================================
const char* OPENWEBUI_URL = "https://chat.ve2fpd.com/api/chat/completions";
const char* API_KEY       = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6ImUwOWZhNjRhLTdhMzctNDRhNi05NWU4LTAxMzY0MWFjNDhkNiIsImV4cCI6MTc3Njk5NDg5MSwianRpIjoiODJhNTE1MWQtOWM3ZC00N2E4LWJmZDEtYzNjNzA0MWU5YzlhIn0.ig3_rGHIos4znA2_M27_x0Jf1KgeAKJoWL4k6XjfmuA";
const char* MODEL_NAME    = "assistant-iot-v2";

// =============================================
// VARIABLES GLOBALES D'INTERFACE (NOUVEAU)
// =============================================
static volatile bool receivedFlag = false;
static String rssi = "--";
static String snr = "--";
static String payload = "--";

static String wifiStatus = "WIFI: WAIT...";
static String sysStatus  = "LORA: WAIT...";

void drawMain(); // Déclaration préalable

#if     defined(USING_SX1276)
#ifndef CONFIG_RADIO_FREQ
#define CONFIG_RADIO_FREQ           868.0
#endif
#ifndef CONFIG_RADIO_OUTPUT_POWER
#define CONFIG_RADIO_OUTPUT_POWER   17
#endif
#ifndef CONFIG_RADIO_BW
#define CONFIG_RADIO_BW             125.0
#endif
SX1276 radio = new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN);

#elif   defined(USING_SX1278)
#ifndef CONFIG_RADIO_FREQ
#define CONFIG_RADIO_FREQ           433.0
#endif
#ifndef CONFIG_RADIO_OUTPUT_POWER
#define CONFIG_RADIO_OUTPUT_POWER   17
#endif
#ifndef CONFIG_RADIO_BW
#define CONFIG_RADIO_BW             125.0
#endif
SX1278 radio = new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN);

#elif   defined(USING_SX1262) // --- T-BEAM SUPREME ---
#ifndef CONFIG_RADIO_FREQ
#define CONFIG_RADIO_FREQ           915.0 
#endif
#ifndef CONFIG_RADIO_OUTPUT_POWER
#define CONFIG_RADIO_OUTPUT_POWER   22
#endif
#ifndef CONFIG_RADIO_BW
#define CONFIG_RADIO_BW             125.0
#endif
SX1262 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

#elif   defined(USING_SX1280)
#ifndef CONFIG_RADIO_FREQ
#define CONFIG_RADIO_FREQ           2400.0
#endif
#ifndef CONFIG_RADIO_OUTPUT_POWER
#define CONFIG_RADIO_OUTPUT_POWER   13
#endif
#ifndef CONFIG_RADIO_BW
#define CONFIG_RADIO_BW             203.125
#endif
SX1280 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

#elif  defined(USING_SX1280PA)
#ifndef CONFIG_RADIO_FREQ
#define CONFIG_RADIO_FREQ           2400.0
#endif
#ifndef CONFIG_RADIO_OUTPUT_POWER
#define CONFIG_RADIO_OUTPUT_POWER   3           
#endif
#ifndef CONFIG_RADIO_BW
#define CONFIG_RADIO_BW             203.125
#endif
SX1280 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

#elif   defined(USING_SX1268)
#ifndef CONFIG_RADIO_FREQ
#define CONFIG_RADIO_FREQ           433.0
#endif
#ifndef CONFIG_RADIO_OUTPUT_POWER
#define CONFIG_RADIO_OUTPUT_POWER   22
#endif
#ifndef CONFIG_RADIO_BW
#define CONFIG_RADIO_BW             125.0
#endif
SX1268 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

#elif   defined(USING_LR1121)
#define CONFIG_RADIO_FREQ           2450.0
#define CONFIG_RADIO_OUTPUT_POWER   LILYGO_RADIO_2G4_TX_POWER_LIMIT
#define CONFIG_RADIO_BW             125.0
LR1121 radio = new Module(RADIO_CS_PIN, RADIO_DIO9_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

#ifdef USING_LR1121PA
static const uint32_t pa_version_rf_switch_dio_pins[] = {
    RADIOLIB_LR11X0_DIO5, RADIOLIB_LR11X0_DIO6, RADIOLIB_LR11X0_DIO7, RADIOLIB_LR11X0_DIO8, RADIOLIB_NC
};
static const Module::RfSwitchMode_t high_freq_switch_table[] = {
    { LR11x0::MODE_STBY,   { LOW,  LOW, LOW, LOW} },
    { LR11x0::MODE_TX,     { LOW,  LOW, LOW, HIGH} },
    { LR11x0::MODE_RX,     { LOW,  LOW, HIGH, LOW} },
    { LR11x0::MODE_TX_HP,  { LOW,  LOW, HIGH, LOW} },
    { LR11x0::MODE_TX_HF,  { LOW,  LOW, HIGH, LOW} },
    { LR11x0::MODE_GNSS,   { LOW,  LOW, LOW, HIGH} },
    { LR11x0::MODE_WIFI,   { LOW,  LOW, LOW, HIGH} },
    END_OF_MODE_TABLE,
};
static const Module::RfSwitchMode_t low_freq_switch_table[] = {
    { LR11x0::MODE_STBY,   { LOW,  LOW, LOW, LOW} },
    { LR11x0::MODE_TX,     { LOW,  HIGH, LOW, LOW} },
    { LR11x0::MODE_RX,     { HIGH, LOW, LOW, LOW} },
    { LR11x0::MODE_TX_HP,  { LOW,  HIGH, LOW, LOW} },
    { LR11x0::MODE_TX_HF,  { LOW,  LOW, LOW, LOW} },
    { LR11x0::MODE_GNSS,   { LOW,  LOW, LOW, LOW} },
    { LR11x0::MODE_WIFI,   { LOW,  LOW, LOW, LOW} },
    END_OF_MODE_TABLE,
};
#endif 
#endif 

// Interruption LoRa
void setFlag(void)
{
    receivedFlag = true;
}

// =============================================
// FONCTIONS WIFI ET LLM
// =============================================
void setupWiFi() {
    Serial.println("\n--- Initialisation du WiFi ---");
    wifiStatus = "WIFI: CONNEXION...";
    drawMain(); // Mise à jour de l'écran

    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    if (USE_WPA2_ENTERPRISE) {
        WiFi.begin(WIFI_SSID, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);
    } else {
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        wifiStatus = "IP: " + WiFi.localIP().toString();
        Serial.println("\nWiFi Connecté ! " + wifiStatus);
    } else {
        wifiStatus = "WIFI: ERREUR";
        Serial.println("\nÉchec de la connexion WiFi.");
    }
    drawMain(); // Mise à jour de l'écran avec le résultat
}

void sendDataToLLM(String loraData) {
    if (WiFi.status() == WL_CONNECTED) {
        
        sysStatus = "API SENDING...";
        drawMain(); // Indiquer qu'on parle à l'API
        
        Serial.println("\n--- Envoi des données au LLM ---");
        HTTPClient http;
        http.begin(OPENWEBUI_URL);
        
        http.addHeader("Content-Type", "application/json");
        String authHeader = "Bearer ";
        authHeader += API_KEY;
        http.addHeader("Authorization", authHeader);

        String jsonPayload = "{\"model\": \"" + String(MODEL_NAME) + "\", \"messages\": [{\"role\": \"user\", \"content\": \"Nouvelle valeur de potentiomètre lue via LoRa : " + loraData + "\"}]}";

        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
            sysStatus = "API: HTTP " + String(httpResponseCode); // Ex: API: HTTP 200
            Serial.printf("Réponse HTTP : %d\n", httpResponseCode);
            String response = http.getString();
            Serial.println(response);
        } else {
            sysStatus = "API: ERR TIMEOUT";
            Serial.printf("Erreur d'envoi HTTP : %s\n", http.errorToString(httpResponseCode).c_str());
        }
        
        http.end();
    } else {
        sysStatus = "API: NO WIFI";
        Serial.println("Erreur: Impossible d'envoyer au LLM, WiFi déconnecté.");
        setupWiFi(); // Tente une reconnexion
    }
    drawMain();
}


void setup()
{
    setupBoards();
    delay(1500);
    
    // On dessine l'interface initiale
    drawMain();
    
    // Initialiser le Wi-Fi (l'écran se mettra à jour dedans)
    setupWiFi();

#ifdef  RADIO_TCXO_ENABLE
    pinMode(RADIO_TCXO_ENABLE, OUTPUT);
    digitalWrite(RADIO_TCXO_ENABLE, HIGH);
#endif

    int state = radio.begin();
    printResult(state == RADIOLIB_ERR_NONE);

    if (state != RADIOLIB_ERR_NONE) {
        sysStatus = "LORA: INIT ERR " + String(state);
        drawMain();
        while (true);
    }

    radio.setPacketReceivedAction(setFlag);
    radio.setFrequency(CONFIG_RADIO_FREQ);
    radio.setBandwidth(CONFIG_RADIO_BW);
    radio.setSpreadingFactor(12);
    radio.setCodingRate(6);
    radio.setSyncWord(0xAB);
    radio.setOutputPower(CONFIG_RADIO_OUTPUT_POWER);

#if !defined(USING_SX1280) && !defined(USING_LR1121) && !defined(USING_SX1280PA)
    radio.setCurrentLimit(140);
#endif
    radio.setPreambleLength(16);
    radio.setCRC(false);

#if  defined(USING_LR1121)
#if defined(USING_LR1121PA)
    if (CONFIG_RADIO_FREQ < 2400) {
        radio.setRfSwitchTable(pa_version_rf_switch_dio_pins, low_freq_switch_table);
    } else {
        radio.setRfSwitchTable(pa_version_rf_switch_dio_pins, high_freq_switch_table);
    }
#else  
    static const uint32_t rfswitch_dio_pins[] = { RADIOLIB_LR11X0_DIO5, RADIOLIB_LR11X0_DIO6, RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC };
    static const Module::RfSwitchMode_t rfswitch_table[] = {
        { LR11x0::MODE_STBY,   { LOW,  LOW  } },
        { LR11x0::MODE_RX,     { HIGH, LOW  } },
        { LR11x0::MODE_TX,     { LOW,  HIGH } },
        { LR11x0::MODE_TX_HP,  { LOW,  HIGH } },
        { LR11x0::MODE_TX_HF,  { LOW,  LOW  } },
        { LR11x0::MODE_GNSS,   { LOW,  LOW  } },
        { LR11x0::MODE_WIFI,   { LOW,  LOW  } },
        END_OF_MODE_TABLE,
    };
    radio.setRfSwitchTable(rfswitch_dio_pins, rfswitch_table);
#endif 
    radio.setTCXO(3.0);
#endif 

#ifdef USING_DIO2_AS_RF_SWITCH
#ifdef USING_SX1262
    radio.setDio2AsRfSwitch();
#endif 
#endif 

#ifdef RADIO_RX_PIN
    radio.setRfSwitchPins(RADIO_RX_PIN, RADIO_TX_PIN);
#endif

#ifdef RADIO_SWITCH_PIN
    const uint32_t pins[] = { RADIO_SWITCH_PIN, RADIO_SWITCH_PIN, RADIOLIB_NC };
    static const Module::RfSwitchMode_t table[] = {
        {Module::MODE_IDLE,  {0,  0} },
        {Module::MODE_RX,    {1, 0} },
        {Module::MODE_TX,    {0, 1} },
        END_OF_MODE_TABLE,
    };
    radio.setRfSwitchTable(pins, table);
#endif

#ifdef RADIO_CTRL
    digitalWrite(RADIO_CTRL, HIGH);
#endif 

    delay(1000);

    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
        sysStatus = "LORA: LISTENING...";
    } else {
        sysStatus = "LORA: RX ERR " + String(state);
    }
    drawMain();
}

void loop()
{
    if (receivedFlag) {
        receivedFlag = false;
        
        sysStatus = "LORA: RX OK!";
        drawMain(); // Met à jour l'écran pour montrer qu'on a capté un truc

        int state = radio.readData(payload);
        flashLed();

        if (state == RADIOLIB_ERR_NONE) {
            rssi = String(radio.getRSSI()) + "dBm";
            snr = String(radio.getSNR()) + "dB";

            drawMain(); // Affiche la valeur reçue

            Serial.println(F("\n[Radio] Paquet reçu !"));
            
            // --- APPEL API LLM ---
            sendDataToLLM(payload);

        } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
            sysStatus = "LORA: CRC ERROR";
        } else {
            sysStatus = "LORA: ERR " + String(state);
        }

        // Remettre le module en écoute après traitement
        radio.startReceive();
        if(sysStatus.indexOf("API") != -1) {
            delay(1500); // Laisse le temps de lire le code HTTP sur l'écran
            sysStatus = "LORA: LISTENING...";
            drawMain();
        }
    }
}

// --- NOUVELLE INTERFACE GRAPHIQUE ---
void drawMain()
{
    if (disp) {
        disp->clearBuffer();
        disp->drawRFrame(0, 0, 128, 64, 3);
        
        // Utilisation d'une police petite mais lisible pour faire tenir 4 lignes
        disp->setFont(u8g2_font_pxplusibmvga8_mr); 
        
        // Ligne 1 : Statut WiFi (ex: IP: 192.168.x.x)
        disp->setCursor(5, 15);
        disp->print(wifiStatus);

        // Ligne 2 : Statut Système (ex: LORA: LISTENING... ou API SENDING...)
        disp->setCursor(5, 30);
        disp->print(sysStatus);

        // Ligne 3 : Message Reçu (ex: MSG: Pot: 2048 #1)
        disp->setCursor(5, 45);
        disp->print("MSG: " + payload);

        // Ligne 4 : Qualité du signal
        disp->setCursor(5, 60);
        disp->print("Sig: " + snr + " | " + rssi);
        
        disp->sendBuffer();
    }
}