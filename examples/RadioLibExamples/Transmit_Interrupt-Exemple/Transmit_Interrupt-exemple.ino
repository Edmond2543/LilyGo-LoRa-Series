/*
   PROJET : Télémétrie LoRa / MQTT / LLM
   FICHIER : Emetteur_Capteur_LoRa.ino
   RÔLE : Nœud capteur distant. Lit un potentiomètre, lisse la valeur, 
          l'envoie par LoRa toutes les 5 secondes et affiche la décision (ON/OFF)
          renvoyée par la passerelle intelligente.
   MATÉRIEL : LilyGo T-Beam Supreme (ESP32-S3)
*/

#include "LoRaBoards.h"
#include <RadioLib.h>
#include <ArduinoJson.h>

// --- CÂBLAGE ---
#define POT_PIN 3         // Broche du potentiomètre
#define LED_ACTION_PIN 6  // Broche de la DEL d'action

// --- CONFIGURATION RADIO LORA ---
#define CONFIG_RADIO_FREQ           915.0 
#define CONFIG_RADIO_OUTPUT_POWER   22
#define CONFIG_RADIO_BW             125.0
#define LORA_SYNC_WORD              0x77 // Canal privé pour isoler le réseau

SX1262 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

static volatile bool actionDoneFlag = false;
String displayStatus = "INIT";
String decisionRecue = "--";
int smoothedPotValue = 0;
unsigned long dernierEnvoi = 0;

void setFlag(void) { actionDoneFlag = true; }

void drawMain() {
    if (disp) {
        disp->clearBuffer();
        disp->drawRFrame(0, 0, 128, 64, 3);
        disp->setFont(u8g2_font_pxplusibmvga8_mr); 
        disp->setCursor(5, 15); disp->print("POT: " + String(smoothedPotValue)); 
        disp->setCursor(5, 35); disp->print("STAT: " + displayStatus);
        disp->setCursor(5, 55); disp->print("LLM : " + decisionRecue);
        disp->sendBuffer();
    }
}

void setup() {
    setupBoards();
    delay(1500);
    pinMode(LED_ACTION_PIN, OUTPUT);
    digitalWrite(LED_ACTION_PIN, LOW); 

    radio.begin();
    radio.setDio1Action(setFlag);
    radio.setFrequency(CONFIG_RADIO_FREQ);
    radio.setSyncWord(LORA_SYNC_WORD); 
    radio.startReceive();
}

void loop() {
    // --- Lissage du potentiomètre (20 lectures) ---
    long sommePot = 0;
    for(int i = 0; i < 20; i++) {
        sommePot += analogRead(POT_PIN);
        delay(5); 
    }
    smoothedPotValue = sommePot / 20;
    
    drawMain(); 

    // --- Routine d'envoi périodique (5 sec) ---
    if (millis() - dernierEnvoi >= 5000) {
        dernierEnvoi = millis();
        
        JsonDocument doc;
        doc["valeur_pot"] = smoothedPotValue;
        String msg;
        serializeJson(doc, msg);
        
        displayStatus = "TX EN COURS...";
        drawMain();
        actionDoneFlag = false;
        radio.startTransmit(msg);
        
        unsigned long t0 = millis();
        while(!actionDoneFlag && millis() - t0 < 1000); 
        
        displayStatus = "ATTENTE LLM";
        drawMain();
        radio.startReceive();
        
        unsigned long t1 = millis();
        bool recu = false;
        while(millis() - t1 < 4000) { 
            if (actionDoneFlag) {
                String rep;
                if (radio.readData(rep) == RADIOLIB_ERR_NONE) {
                    JsonDocument rxDoc;
                    if (!deserializeJson(rxDoc, rep)) {
                        decisionRecue = rxDoc["decision"].as<String>();
                        decisionRecue.trim();
                        digitalWrite(LED_ACTION_PIN, (decisionRecue.indexOf("ON") >= 0));
                        recu = true;
                        break;
                    }
                }
            }
        }
        if(!recu) decisionRecue = "TIMEOUT";
        displayStatus = "PRET";
    }
}