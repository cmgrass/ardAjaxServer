#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <string.h>
#include <stdlib.h>

#define CMG_DBG

#ifdef CMG_DBG
#define CMG_PRINT(x) Serial.println x
#else
#define CMG_PRINT(x) do {} while (0)
#endif

#define MAX_STR_C 512
#define HTML_STDREQ_C 111
#define HTML_GETTEST_C 115
#define SUCCESS 0
#define ERROR -1

typedef struct eth_shield_t {
  int spi_sck;
  int spi_miso;
  int spi_mosi;
  int spi_et_sel;
  int spi_sd_sel;
} eth_shield_t;

/*-------------------------------------------------
           L O C A L   F U N C T I O N S
-------------------------------------------------*/
int alloc_req(char *req_p)
{
  int idx = 0;

  if (req_p != NULL) {
    /* Destroy first */
    free(req_p);
  } else {
    req_p = malloc(MAX_STR_C + 1);
    for (idx = 0; idx >= MAX_STR_C; idx++) {
      req_p[idx] = '\0';
    }
  }
  return SUCCESS;
} /* alloc_req */

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
} /* init_spi */

int proc_req(char *req_p, int *requestType_p)
{
  const char *getStr_p = "GET /test.txt";
  char *retPtr_p = NULL;

  retPtr_p = strstr(req_p, getStr_p);
  if (retPtr_p == NULL) {
    *requestType_p = HTML_STDREQ_C;
    CMG_PRINT(("GET TEST NOT FOUND"));
  } else {
    *requestType_p = HTML_GETTEST_C;
    CMG_PRINT(("GET TEST FOUND"));
  }
  return SUCCESS;
} /* proc_req */

int write_http_data(EthernetClient *client_p, File *file_p)
{
  int status = SUCCESS;

  if (file_p) {
    while (file_p->available()) {
      client_p->write(file_p->read());
    }
    file_p->close();
  } else {
    status = ERROR;
  }
  return status; 
} /* write_http_data */

int send_http(EthernetClient *client_p, char *req_p, File *file_p)
{
  int status = SUCCESS;
  int requestType = 0;

  /* Process Request */
  status = proc_req(req_p, &requestType);

  /* Send standard http resopnse header */
  client_p->println("HTTP/1.1 200 OK");
  client_p->println("Content-Type: text/html");
  client_p->println("Connection: close");
  client_p->println();

  /* Send webpage */
  switch (requestType) {
    case HTML_STDREQ_C:
      CMG_PRINT(("DBG: HTML_STDREQ_C"));
      *file_p = SD.open("main.htm");
      status = write_http_data(client_p, file_p);
      break;

    case HTML_GETTEST_C:
      CMG_PRINT(("DBG: HTML_GETREQ_C"));
      *file_p = SD.open("test.txt");
      status = write_http_data(client_p, file_p);
      break;

    default:
      break;
  }

  CMG_PRINT((status));

  return status; 
} /* send_http */

/*-------------------------------------------------
          D E C L A R E   G L O B A L S
-------------------------------------------------*/
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

/*-------------------------------------------------
                    S E T U P
-------------------------------------------------*/
void setup() {
  status = init_spi(eth_shield_p);
#ifdef CMG_DBG
  Serial.begin(9600);
#endif
  Ethernet.begin(mac, ip);
  server.begin();
  CMG_PRINT(("sever is at: "));
  CMG_PRINT((Ethernet.localIP()));
  if (!SD.begin(eth_shield_p->spi_sd_sel)) {
    return;
  }
  CMG_PRINT(("SD Initialization SUCCESS"));
  if (!SD.exists("main.htm")) {
    return;
  }
}

/*-------------------------------------------------
                     L O O P
-------------------------------------------------*/
void loop() {
  boolean currentLineBlank = true;
  int idx = 0;
  int status = 0;
  char chr = '\0';
  char *httpReq_p = NULL; 
  EthernetClient client;

  while (1) {
    /* check if client available */
    client = server.available();

    if (client) {
      CMG_PRINT(("Client available"));
      currentLineBlank = true;
      chr = '\0';
      idx = 0;

      /* Allocate string */
      status = alloc_req(httpReq_p);

      while (client.connected()) {
        if (client.available()) {
          /* Process client request */
          chr = client.read();
          if (idx >= MAX_STR_C) {
            break;
          }
          #ifdef CMG_DBG
          Serial.write(chr);
          #endif
          /* http request ends with '\n', then we can reply */
          if (chr == '\n' && currentLineBlank) {
            CMG_PRINT(("We can reply"));
            status = send_http(&client, httpReq_p, &webFile);
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
      free(httpReq_p);
      CMG_PRINT(("Disconnected from client\n\n"));
    }
  }
}
