#include <SPI.h>
#include <Ethernet.h>

typedef struct eth_shield_t {
  int spi_sck;
  int spi_miso;
  int spi_mosi;
  int spi_et_sel;
  int spi_sd_sel;
} ETH_SHIELD_T;

int init_spi(eth_shield_t *eth_shield_p) {
  /* Assignments */
  eth_shield_p->spi_sck = 13;
  eth_shield_p->spi_miso = 12;
  eth_shield_p->spi_mosi = 11;
  eth_shield_p->spi_et_sel = 10;
  eth_shield_p->spi_sd_sel = 4;

  /* Disable SD SPI SS (Active LOW) */
  pinMode(eth_shield_p->spi_sd_sel, OUTPUT);
  digitalWrite(eth_shield_p->spi_sd_sel, HIGH);

  return 0;
} 

/* Declare Globals */
int status = 0;
eth_shield_t eth_shield_d;
eth_shield_t *eth_shield_p = &eth_shield_d;
byte mac[] = {
  0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF
};
IPAddress ip(192, 168, 2, 125);
IPAddress subnet(255, 255, 255, 0);
/*IPAddress gateway(192, 168, 0, 1);*/
EthernetServer server(80);

void setup() {
  status = init_spi(eth_shield_p);
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Bacon11");
  }
  /*
  Serial.println("Corndog");
  Serial.println(eth_shield_p->spi_sd_sel, HEX);
  delay(1000);
  */
}
