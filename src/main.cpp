#include <Arduino.h>
#include <Ethernet.h>
#include <Mudbus.h>

uint8_t mac[6];

Mudbus Mb;

// ===== Hardware Pin Constants ===== //
const int SCS = 10, ETHLINK = 14, ETHRX = 15, ETHTX = 16;
const int TRST = 17, TPGM = 20, ETHRST = 21;

void resetEthernet()
{
  digitalWrite(ETHRST, LOW);
  delay(5);
  digitalWrite(ETHRST, HIGH);
  delay(50);
}

void setup()
{
  mac[0] = 04;
  mac[1] = 111;
  mac[2] = 90;
  mac[3] = 88;
  mac[4] = 57;
  mac[5] = 67;

  Serial.begin(9600);
  Serial.println("Connecting to ethernet");

  pinMode(SCS, OUTPUT);
  digitalWrite(SCS, HIGH);
  pinMode(ETHLINK, INPUT);
  pinMode(ETHRX, INPUT);
  pinMode(ETHTX, INPUT);
  pinMode(TRST, OUTPUT);
  digitalWrite(TRST, LOW);
  pinMode(TPGM, OUTPUT);
  digitalWrite(TPGM, LOW);
  pinMode(ETHRST, OUTPUT);
  digitalWrite(ETHRST, HIGH);

  delay(1000);

  resetEthernet();

  IPAddress deviceIP = IPAddress(10, 30, 128, 81);
  IPAddress gateway = IPAddress(10, 30, 1, 1);
  IPAddress subnet = IPAddress(255, 255, 0, 0);

  Ethernet.begin(mac, deviceIP, gateway, gateway, subnet);

  delay(2000);
  Serial.println(Ethernet.localIP()[0]);
  Serial.println(Ethernet.localIP()[1]);
  Serial.println(Ethernet.localIP()[2]);
  Serial.println(Ethernet.localIP()[3]);
}

// void runModbusServer() {
//   Serial.println(digitalRead(22));
// }

void loop()
{
  Mb.C[1] = digitalRead(22);
  Mb.Run();
  delay(100);
}