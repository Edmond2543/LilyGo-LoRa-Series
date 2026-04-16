# RadioLibExamples - Télémétrie LoRa Intelligente

Ce dossier contient une implémentation complète d'un système de télémétrie LoRa basé sur la bibliothèque **RadioLib**, conçu pour les cartes **LilyGo T-Beam Supreme (ESP32-S3)**.

Le système est divisé en deux rôles distincts communicant via le protocole LoRa.

## Architecture du Système

Le projet fonctionne selon un modèle maître-esclave :

1.  **Nœud Capteur (`Transmit_Interrupt-Exemple`)** :
    *   Lit une valeur analogique (ex: potentiomètre).
    *   Envoie cette donnée via LoRa à intervalle régulier.
    *   Attend une réponse de la passerelle.
    *   Réagit en fonction de la décision reçue (ex: allumage d'une LED).

2.  **Passerelle Intelligente (`Receive_Interrupt-Exemple`)** :
    *   Écoute les messages LoRa.
    *   Lorsqu'une donnée est reçue, elle interroge une API LLM (via Wi-Fi) pour obtenir une "décision" intelligente basée sur la valeur reçue.
    *   Publie le résultat complet sur un broker MQTT (via WebSockets).
    *   Renvoie une commande simplifiée au nœud capteur.

---

## 1. Transmit_Interrupt-Exemple (Nœud Capteur)

Ce programme se concentre sur l'efficacité énergétique et la fiabilité de la transmission par interruptions.

### Fonctionnement
*   **Lecture** : Effectue une moyenne glissante sur 20 lectures pour stabiliser la valeur du potentiomètre.
*   **Transmission** :
    *   Sérialise les données en JSON (`ArduinoJson`).
    *   Utilise `radio.startTransmit()` pour envoyer les données.
    *   La transmission est confirmée par une interruption (`setFlag`).
*   **Réception de commande** :
    *   Passe immédiatement en mode réception après l'envoi.
    *   Attend une réponse de la passerelle pendant un maximum de 4 secondes.
    *   Si une décision (`ON` ou `OFF`) est reçue, elle met à jour l'état d'une LED locale.

---

## 2. Receive_Interrupt-Exemple (Passerelle)

Ce programme gère la logique complexe et l'interface avec le monde extérieur.

### Fonctionnement
*   **Réception LoRa** : Utilise les interruptions pour une écoute permanente des messages entrants.
*   **Intelligence (LLM)** :
    *   Vérifie si la valeur reçue est différente de la dernière traitée (système de cache pour économiser les appels API).
    *   Envoie une requête HTTP POST à un LLM (ex: Groq) avec une consigne système prédéfinie.
    *   Analyse la réponse JSON du LLM pour extraire la décision.
*   **Communication Web** :
    *   **MQTT** : Publie la réponse complète du LLM sur un broker sécurisé via WebSockets (WSS).
    *   **Wi-Fi** : Supporte à la fois les connexions domestiques et WPA2 Enterprise (campus/entreprise).
*   **Feedback LoRa** : Renvoie une commande optimisée (format JSON compact) au nœud émetteur.

---

## Configuration Requise

Pour compiler ces exemples, assurez-vous d'avoir :

1.  **Bibliothèques Arduino** :
    *   `RadioLib`
    *   `ArduinoJson`
    *   `u8g2` (pour l'affichage OLED)
2.  **Configuration** :
    *   Modifiez `Receive_Interrupt-Exemple.ino` pour renseigner vos identifiants Wi-Fi, votre clé API LLM et les paramètres de votre broker MQTT.
    *   Le `LORA_SYNC_WORD` doit être identique sur les deux appareils pour qu'ils puissent communiquer.

---
*Ce projet est une adaptation pour les modules LilyGo LoRa Series utilisant RadioLib.*
