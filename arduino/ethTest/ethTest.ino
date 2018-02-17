#include <SPI.h>
#include <Ethernet.h>

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
          /* Send standard http resopnse header */
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();

          /* Send webpage */
          client.println("<!DOCTYPE html>");
          client.println("<html>");
          client.println("<head>");
          client.println("<title>Chris Arduino Webpage</title>");
          client.println("</head>");
          client.println("<body>");
          client.println("<h1>My Arduino sent this!! cool!!!</h1>");
          client.println("<p>My paragraph is even cooler</p>");
          client.println("</body>");
          client.println("</html>");
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
