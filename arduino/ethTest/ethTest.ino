#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

#define CMG_DBG

#ifdef CMG_DBG
#define CMG_PRINT(x) Serial.println x
#else
#define CMG_PRINT(x) do {} while (0)
#endif

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
#ifdef CMG_DBG
  Serial.begin(9600);
#endif
  Ethernet.begin(mac, ip);
  server.begin();
  CMG_PRINT(("sever is at: "));
  CMG_PRINT((Ethernet.localIP()));
  CMG_PRINT(("Initializing sd card..."));
  if (!SD.begin(eth_shield_p->spi_sd_sel)) {
    CMG_PRINT(("SD Initialization FAILED"));
    return;
  }
  CMG_PRINT(("SD Initialization SUCCESS"));
  if (!SD.exists("main.htm")) {
    CMG_PRINT(("ERROR: Can't find main.htm on SD"));
    return;
  }
  CMG_PRINT(("Success, found SD card"));
}

void loop() {
  boolean currentLineBlank = true;
  char chr = '\0';

  EthernetClient client = server.available();

  if (client) {
    CMG_PRINT(("Client available"));
    currentLineBlank = true;
    while (client.connected()) {
      if (client.available()) {
        /* Process client request */
        chr = client.read();
        #ifdef CMG_DBG
        Serial.write(chr);
        #endif

        /* http request ends with '\n', then we can reply */
        if (chr == '\n' && currentLineBlank) {
          CMG_PRINT(("We can reply"));
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
    CMG_PRINT(("Disconnected from client"));
  }
}
