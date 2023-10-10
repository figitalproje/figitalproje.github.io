#include <EEPROM.h>
#include <Ed25519.h>
#include <ESP32Servo.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

const int PRIVATE_KEY_SIZE = 32;
const int PUBLIC_KEY_SIZE = 32;
const int PRIVATE_KEY_ADDR = 0;
const int PUBLIC_KEY_ADDR = PRIVATE_KEY_ADDR + PRIVATE_KEY_SIZE;
const int SERVO_ADDR=PUBLIC_KEY_ADDR+PUBLIC_KEY_SIZE; 

uint8_t privateKey[PRIVATE_KEY_SIZE];
uint8_t publicKey[PUBLIC_KEY_SIZE];
String barkod;
Servo myservo;


BLEServer* pServer = NULL;
BLECharacteristic* pHandshakeCharacteristic = NULL;
BLECharacteristic* pSignatureCharacteristic = NULL;
BLECharacteristic* pServoCharacteristic = NULL;
bool deviceConnected = false;

const char* SERVICE_UUID = "2b5dc6e0-2727-453f-b69d-7fa0f50c7705";
const char* HANDSHAKE_CHARACTERISTIC_UUID = "0abd0e1d-b033-486d-bd6f-a9423c091146";
const char* SIGNATURE_CHARACTERISTIC_UUID ="c029a5f5-03fe-40ff-a379-896dfa677240";
const char* SERVO_CHARACTERISTIC_UUID ="ca9d304b-1afc-4006-94fc-89b37d127c6c";


void HandShake() {
  String elsalla;
  for (int i = 0; i < PUBLIC_KEY_SIZE; i++) {
    if (publicKey[i] < 16) {
      elsalla += '0';
    }
    elsalla += String(publicKey[i], HEX);
  }
  pHandshakeCharacteristic->setValue(elsalla.c_str());
  pHandshakeCharacteristic->notify();
}


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      HandShake();  // Cihaz bağlandığında HandShake fonksiyonunu otomatik olarak çağır
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
       pServer->getAdvertising()->start();
    }
};

class SignatureCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        // Gelen veriyi imzala
        uint8_t signature[64];
        Ed25519::sign(signature, privateKey, publicKey, (uint8_t*)value.c_str(), value.length());
        String imza;
        for (int i = 0; i < 64; i++) {
        if (signature[i] < 16) {
        imza += '0';
        }
        imza += String(signature[i], HEX);
}

        // İmzayı karakteristiğin değeri olarak ayarla
        pCharacteristic->setValue(imza.c_str());
        
        // İmzayı istemciye bildir
        pCharacteristic->notify();
      }
    }
};
class ServoCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        int pos = atoi(value.c_str());  // String değeri tam sayıya dönüştür
        if (pos >= 0 && pos <= 180) {    // Servo motor için güvenli bir aralık kontrolü
          myservo.write(pos);
          EEPROM.write(SERVO_ADDR, pos);  // Yeni pozisyonu EEPROM'a kaydet
          EEPROM.commit();  // Değişiklikleri EEPROM'a yaz
        }
      }
    }
};


String publicKeyToString(uint8_t* publicKey) {
  String result;
  for (int i = 0; i < PUBLIC_KEY_SIZE/8; i++) {
    if (publicKey[i] < 16) {
      result += '0';
    }
    result += String(publicKey[i], HEX);
  }
  return result;
}




void setup() {
  EEPROM.begin(512);
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
      EEPROM.put(PRIVATE_KEY_ADDR + i, privateKey[i]);
      EEPROM.commit();
      
    }
    for (int i = 0; i < PUBLIC_KEY_SIZE; i++) {
      EEPROM.put(PUBLIC_KEY_ADDR + i, publicKey[i]);
      EEPROM.commit();
    }
    
  } else {
    for (int i = 0; i < PRIVATE_KEY_SIZE; i++) {
      privateKey[i] = EEPROM.read(PRIVATE_KEY_ADDR + i);
    }
    for (int i = 0; i < PUBLIC_KEY_SIZE; i++) {
      publicKey[i] = EEPROM.read(PUBLIC_KEY_ADDR + i);
      
      
    }
    
  }

  myservo.attach(13);
  if(EEPROM.read(SERVO_ADDR)==0xFF){
    EEPROM.write(SERVO_ADDR,0);
  }
    int servoson = EEPROM.read(SERVO_ADDR);  // EEPROM'dan servoson değerini oku
  myservo.write(servoson);  // Servo motorun pozisyonunu ayarla
  Serial.begin(9600);
  delay(1000);

String deviceName = "BARKOD" + publicKeyToString(publicKey);
BLEDevice::init(deviceName.c_str());
Serial.println(deviceName);


  // BLE aygıtını başlat
 
 
  //BLEDevice::init(stddeviceName);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());  // Callback fonksiyonlarını ayarla
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pHandshakeCharacteristic = pService->createCharacteristic(
                     HANDSHAKE_CHARACTERISTIC_UUID,
                     BLECharacteristic::PROPERTY_READ   |
                     BLECharacteristic::PROPERTY_WRITE  |
                     BLECharacteristic::PROPERTY_NOTIFY |
                     BLECharacteristic::PROPERTY_INDICATE
                   );
  pHandshakeCharacteristic->addDescriptor(new BLE2902());

pSignatureCharacteristic = pService->createCharacteristic(
                     SIGNATURE_CHARACTERISTIC_UUID,
                     BLECharacteristic::PROPERTY_READ   |
                     BLECharacteristic::PROPERTY_WRITE  |
                     BLECharacteristic::PROPERTY_NOTIFY
                   );
pSignatureCharacteristic->setCallbacks(new SignatureCallbacks());
pSignatureCharacteristic->addDescriptor(new BLE2902());


pServoCharacteristic =pService-> createCharacteristic(
                      SERVO_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                     BLECharacteristic::PROPERTY_WRITE  
);
pServoCharacteristic->setCallbacks(new ServoCallbacks());
pServoCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}



void loop() {
 
}
