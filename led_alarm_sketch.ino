/*

  DNS and DHCP-based Web client

 This sketch connects to a website (http://www.google.com)

 using an Arduino Wiznet Ethernet shield.

 Circuit:

 * Ethernet shield attached to pins 10, 11, 12, 13

 created 18 Dec 2009

 by David A. Mellis

 modified 9 Apr 2012

 by Tom Igoe, based on work by Adrian McEwen

 */
#include <TimeLib.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield

const byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
const byte ip[] = {192, 168, 1, 50};
//char serverName[] = "www.google.com";
EthernetServer server(80); // Можно выбрать и другой порт
IPAddress timeServer(128, 138, 140, 44 );

unsigned int localPort = 8888;
const int NTP_PACKET_SIZE= 48;     
byte packetBuffer[NTP_PACKET_SIZE]; 
EthernetUDP Udp;  

void setup() {

 // Open serial communications and wait for port to open:
  
  Serial.begin(9600);

   while (!Serial) {

    ; // wait for serial port to connect. Needed for Leonardo only

  }
  Serial.println("Serial.begin succeeded");


  // start the Ethernet connection:

  Ethernet.begin(mac, ip);
  // give the Ethernet shield a second to initialize:
  delay(1000);
  // Запустили сервер
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  // source: https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
  // initialize digital pin LED_BUILTIN as an output
  pinMode(8, OUTPUT);

  setSyncInterval(30); // Set seconds between re-sync (5s for test only)

  Udp.begin(localPort);
  Serial.print("Time: ");
  Serial.println(getNtpTime());
  Serial.print("Time Status before setSyncProvider:");
  Serial.println(timeStatus());
  setSyncProvider(getNtpTime);
  Serial.print("Time Status after setSyncProvider:");
  Serial.println(timeStatus());
  digitalClockDisplay();


}

void loop()
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");

    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    
    // give the web browser time to receive the data
    delay(1000);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
    // default: 30 seconds delay
    digitalWrite(8, HIGH);
    delay(6000);
    Serial.println("Wake up, sunshine!");
    delay(6000);
    digitalWrite(8, LOW);

  }
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding
  // colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.print(" ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println(" ");
}

/*-------- NTP code ----------*/
// source: https://lms.onnocenter.or.id/wiki/index.php/Arduino:_Ethernet_NTP_Time_Sync
unsigned long getNtpTime()
{
   sendNTPpacket(timeServer); // send an NTP packet to a time server
     delay(500);
    
     if ( Udp.parsePacket() ) {
       Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read packet into buffer  
 
     //the timestamp starts at byte 40, convert four bytes into a long integer
     unsigned long hi = word(packetBuffer[40], packetBuffer[41]);
     unsigned long low = word(packetBuffer[42], packetBuffer[43]);
     // this is NTP time (seconds since Jan 1 1900
     unsigned long secsSince1900 = hi << 16 | low; 
     // Unix time starts on Jan 1 1970
     const unsigned long seventyYears = 2208988800UL;     
     // useful page: https://www.epochconverter.com/timezones
     const unsigned long timeZoneOffset = 21600UL;
     unsigned long epoch = secsSince1900 - seventyYears + timeZoneOffset;  // subtract 70 years, add Daylight Saving and Timezone adjust
     return epoch;
  }
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
//sendNTPpacket(IPAddress address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);  // set all bytes in the buffer to 0

  // Initialize values needed to form NTP request
  packetBuffer[0] = B11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum
  packetBuffer[2] = 6;     // Max Interval between messages in seconds
  packetBuffer[3] = 0xEC;  // Clock Precision
  // bytes 4 - 11 are for Root Delay and Dispersion and were set to 0 by memset
  packetBuffer[12]  = 49;  // four-byte reference ID identifying
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // send the packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket();
}