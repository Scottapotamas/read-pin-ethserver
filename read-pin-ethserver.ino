/*
 * Web server which allows the state of pins to be queried through GET
 * Useful for connecting hardware to something needing a _really_ simple interface
 * like a CLI based pass/fail status
 * 
 * This currently setup to report ON or OFF when the various hardware-connected test VM's
 * need to check the hardware correctly sets a pin.
 * 
 * Pin -> GET strings are managed in pin_get_strings[]
 * 
 * To test this, hit http://ipaddress/linux0, or "wget -qO- 192.168.1.125/osx" for example.
 * 
 * I used an Adafruit Feather 32u4 with Ethernet Featherwing.
 */

#include <SPI.h>
#include <Ethernet.h>

// Enable output formatting for web-page use
//#define MODE_HTML

// The IP address will be dependent on the local network's DHCP server if not forced
byte mac[] = 
{
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

//IPAddress ip(192, 168, 1, 177);

EthernetServer server( 80 );

typedef struct 
{
  const char* platform;
  uint8_t io_pin;
} request_io;

request_io pin_get_strings[] = 
{ 
  { "linux0",   0 },
  { "linux1",   1 },
  { "linux2",   2 },
  { "osx",      3 },
  { "windows",  4 },
};

char raw_get_text[1024];
uint16_t get_text_i = 0;

void setup() 
{
  Serial.begin( 115200 );

  // Set the pin states
  for( uint8_t i = 0; i < sizeof(pin_get_strings)/sizeof(pin_get_strings[0]); i++ )
  {
    pinMode( pin_get_strings[i].io_pin, INPUT );
  }
  
  Ethernet.begin(mac);  //, ip); 

  // Check hardware is connected
  if( Ethernet.hardwareStatus() == EthernetNoHardware ) 
  {
    Serial.println("Ethernet hardware not found...");
    
    // do nothing, no point running without Ethernet hardware
    while( true ) 
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100); 
      digitalWrite(LED_BUILTIN, LOW);
      delay(100); 
    }
  }
  
  if (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() 
{
  // listen for incoming client connections
  EthernetClient client = server.available();
  
  if( client )
  {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean firstLine = true;
    
    while( client.connected() ) 
    {
      if( client.available() ) 
      {
        char c = client.read();

        // Copy the first line of the HTTP request into a buffer
        if( firstLine )
        {
          raw_get_text[ get_text_i ] = c;
          get_text_i += 1;
        }

        // The http request has ended, so send a reply
        if (c == '\n' && currentLineIsBlank)
        {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();

#ifdef MODE_HTML
          // Minimum markup for valid web page
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
#endif

          // Work out which pin was requested, if any, by matching strings we have stored against pin allocations
          request_io *pin_requested = 0;
          for(uint8_t i = 0; i < sizeof(pin_get_strings)/sizeof(pin_get_strings[0]); i++ )
          {
            if( strstr( raw_get_text, pin_get_strings[i].platform) )
            {
              pin_requested = &pin_get_strings[i];
            }
          }
          
          //Handle the inbound pin request and output the state
          if( pin_requested )
          {
            if( digitalRead(pin_requested->io_pin) )
            {
              client.println("ON");
            }
            else
            {
              client.println("OFF");
            }
          }
          else
          {
            // Invalid request string
            client.println("ERROR");
          }
          
#ifdef MODE_HTML
          client.println("</html>");
#endif
          break;
        }

        if (c == '\n')
        {
          // Start a new line
          currentLineIsBlank = true;
          firstLine = false;
          raw_get_text[get_text_i] = 0;
        }
        else if (c != '\r')
        {
          // Character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    
    // Tiny delay before closing the connection
    delay(1);
    client.stop();

    // Cleanup for next request
    memset(raw_get_text, 0, sizeof(raw_get_text));
    get_text_i = 0;
  }
}
