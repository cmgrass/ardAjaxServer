#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

typedef struct eth_shield_t {
  int spi_sck;
  int spi_miso;
  int spi_mosi;
  int spi_et_sel;
  int spi_sd_sel;
} eth_shield_t;

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

int send_http(EthernetClient *client_p, File *file_p) {
  /* Send standard http resopnse header */
  client_p->println("HTTP/1.1 200 OK");
  client_p->println("Content-Type: text/html");
  client_p->println("Connection: close");
  client_p->println();

  /* Send webpage */
  *file_p = SD.open("main.htm");
  if (file_p) {
    while (file_p->available()) {
      client_p->write(file_p->read());
    }
    file_p->close();
  }
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
File webFile;

void setup() {
  status = init_spi(eth_shield_p);
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at: ");
  Serial.println(Ethernet.localIP());
  Serial.println("Initializing sd card...");
  if (!SD.begin(eth_shield_p->spi_sd_sel)) {
    Serial.println("SD Initialization FAILED");
    return;
  }
  Serial.println("SD Initialization SUCCESS");
  if (!SD.exists("main.htm")) {
    Serial.println("ERROR: Can't find main.htm on SD");
    return;
  }
  Serial.println("Success, found SD card");
}

void loop() {
  boolean currentLineBlank = true;
  char chr = '\0';

  EthernetClient client = server.available();

  if (client) {
    Serial.println("Client available");
    currentLineBlank = true;
    while (client.connected()) {
      if (client.available()) {
        /* Process client request */
        chr = client.read();
        Serial.write(chr);
        
        /* http request ends with '\n', then we can reply */
        if (chr == '\n' && currentLineBlank) {
          Serial.println("We can reply");
          status = send_http(&client, &webFile);
          break;
        }
        if (chr == '\n') {
          // Newline
          currentLineBlank = true;
        } else if (chr != '\r') {
          // Character on current line
          currentLineBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
    Serial.println("Disconnected from client");
  }
}
