
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <DHT.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define DHTPIN 3
#define DHTTYPE DHT11   // DHT 11 

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);
IPAddress broadcastIP(0xFF, 0xFF, 0xFF, 0xFF);
IPAddress cloudIP(192, 168, 1, 8);
int packetSize;

uint32_t key[4] = {
		31231234, 412334, 12341, 657657
};

#define PIN_COFFEE 2

unsigned int localPort = 8888;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
char bufferCrypted[UDP_TX_PACKET_MAX_SIZE];
char ReplyBuffer[] = "acknowledged";

EthernetUDP Udp;

DHT dht(DHTPIN, DHTTYPE);

void encipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
    unsigned int i;
    uint32_t v0=v[0], v1=v[1], sum=0, delta=0x9E3779B9;
    for (i=0; i < num_rounds; i++) {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum>>11) & 3]);
    }
    v[0]=v0; v[1]=v1;
}

int encipherEvent(char *eventBuffer, char *bufferOutput) {

	char buffer[8];
	int len, numOfBlocks, i, j;
	char *pt;

	len = strlen(eventBuffer);
	numOfBlocks = (len / 8) * 8 + ( (len % 8) == 0 ? 0 : 1);

        Serial.print("len = ");
        Serial.println(len);

	pt = bufferOutput;

	memcpy(pt, &len, sizeof(len));
	pt += sizeof(len);

	memcpy(pt, &numOfBlocks, sizeof(numOfBlocks));
	pt += sizeof(numOfBlocks);

	//encipher(32, (uint32_t*) bufferOutput, key);

	for (i = 0; i < len; i += 8) {

		memset(buffer, 0, sizeof buffer);
		for (j = 0; j < 8; j++) {
			if ((i + j) < len) {
				buffer[j] = eventBuffer[i + j];
			}
		}

		encipher(32, (uint32_t*) buffer, key);
		memcpy(pt, buffer, 8);
		pt += 8;

	}

	return numOfBlocks;

}

void setup() {
	
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);

  pinMode(PIN_COFFEE, OUTPUT);
  
  Serial.begin(9600);
  
  Serial.println("Running");
  
}
	
int counting = 60;

void(* resetFunc) (void) = 0;

void loop() {
#define COMMAND(NAME, PACKET) strlen(NAME) <= packetSize && strncmp(NAME, PACKET, strlen(NAME)) == 0
	
	packetSize = Udp.parsePacket();
	
	//Serial.println("Waiting packet");
	
	if (packetSize > 0) {
		
		Serial.println("Parsing packet");
		
		Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
		
		if (COMMAND("turnOn", packetBuffer)) {
			
			digitalWrite(PIN_COFFEE, HIGH);
			
		} else if (COMMAND("turnOff", packetBuffer)) {
			
			digitalWrite(PIN_COFFEE, LOW);
			
		}
		
	} else {
		
		float temperature = dht.readTemperature();
		float humidity = dht.readHumidity();
		int blocks;

                /*if (counting < 60) {		
                  
		  Udp.beginPacket(broadcastIP, 10001);
		  sprintf(packetBuffer, "TEMP %d HUM %d", (int) temperature, (int) humidity);
		  Udp.write((uint8_t*) packetBuffer, strlen(packetBuffer));
		  Udp.endPacket();

                  counting++;

                } else {*/

  		  Udp.beginPacket(cloudIP, 9001);
		
		  sprintf(packetBuffer, "{ \"type\": \"event\", \"place\": \"home\", \"temperature\": \"%d\", \"humidity\": \"%d\" }",
				(int) temperature, (int) humidity);
		
		  //memset(bufferCrypted, 0, sizeof bufferCrypted);
		  //blocks = encipherEvent(packetBuffer, bufferCrypted);
		
		  Udp.write((uint8_t*) packetBuffer, strlen(packetBuffer) + 1);
		  Udp.endPacket();		

                  counting = 0;
                  
                  //Serial.println("Cloud");

                //}
		
		//Serial.print("Temperature ");
		//Serial.println(temperature);
		
		//Serial.print("Humidity ");
		//Serial.println(humidity);		
		
	}

        //Ethernet.begin(mac, ip);
        //Udp.begin(localPort);	
	delay(60000);

        resetFunc();
	
}
