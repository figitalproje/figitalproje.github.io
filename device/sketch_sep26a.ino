#include <EEPROM.h>
#include <Ed25519.h>
#include <Servo.h>

const int PRIVATE_KEY_SIZE = 32;
const int PUBLIC_KEY_SIZE = 32;
const int PRIVATE_KEY_ADDR = 0;
const int PUBLIC_KEY_ADDR = PRIVATE_KEY_ADDR + PRIVATE_KEY_SIZE;

uint8_t privateKey[PRIVATE_KEY_SIZE];
uint8_t publicKey[PUBLIC_KEY_SIZE];
Servo myservo;

void HandShake() {
  String elsalla ;
  for (int i = 0; i < PUBLIC_KEY_SIZE; i++) {
    if (publicKey[i] < 16) {
      elsalla += '0';
    }
    elsalla += String(publicKey[i], HEX);
  }
  Serial.println(elsalla);
}
void HandShake0() {
  String elsalla = "Private Key: ";
  for (int i = 0; i < PRIVATE_KEY_SIZE; i++) {
    if (privateKey[i] < 16) {
      elsalla += '0';
    }
    elsalla += String(privateKey[i], HEX);
  }
  Serial.println(elsalla);
}

void setup() {
  bool keyExists = true;
  for (int i = 0; i < PRIVATE_KEY_SIZE; i++) {
    if (EEPROM.read(PRIVATE_KEY_ADDR + i) == 0xFF) {
      keyExists = false;
      break;
    }
  }

  if (!keyExists) {
    Ed25519::generatePrivateKey(privateKey);
    Ed25519::derivePublicKey(publicKey, privateKey);
    for (int i = 0; i < PRIVATE_KEY_SIZE; i++) {
      EEPROM.write(PRIVATE_KEY_ADDR + i, privateKey[i]);
    }
    for (int i = 0; i < PUBLIC_KEY_SIZE; i++) {
      EEPROM.write(PUBLIC_KEY_ADDR + i, publicKey[i]);
    }
  } else {
    for (int i = 0; i < PRIVATE_KEY_SIZE; i++) {
      privateKey[i] = EEPROM.read(PRIVATE_KEY_ADDR + i);
    }
    for (int i = 0; i < PUBLIC_KEY_SIZE; i++) {
      publicKey[i] = EEPROM.read(PUBLIC_KEY_ADDR + i);
    }
  }

  myservo.attach(5);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'H') {
      HandShake();
    }
    if (command == 'P') {
      HandShake0();
    }
    if (command == 'S') {
      delay(50);
      String posStr = "";
      while (Serial.available()) {
        char c = Serial.read();
        if (c >= '0' && c <= '9') {
            posStr += c;
        }
        delay(10);
      }
      int pos = posStr.toInt();
      myservo.write(pos);
    } 
    if (command == 'M') {
      String mesaj;
      delay(50);
      while (Serial.available()) {
        char c = Serial.read();
        mesaj += c;
        delay(10);
      }
      uint8_t signature[64];
      Ed25519::sign(signature, privateKey, publicKey, mesaj.c_str(), mesaj.length());
      String signatureString;
      for (int i = 0; i < 64; i++) {
        if (signature[i] < 16) {
          signatureString += '0';
        }
        signatureString += String(signature[i], HEX);
      }
      
      Serial.print(signatureString);

     
    }
  }
}
