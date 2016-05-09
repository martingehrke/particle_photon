// This #include statement was automatically added by the Particle IDE.
#include "HttpClient/HttpClient.h"

// This #include statement was automatically added by the Particle IDE.
#include "PietteTech_DHT/PietteTech_DHT.h"

//DHT systems defines
#define DHTTYPE  DHT22              // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   2              // Digital pin for communications
#define DHT_SAMPLE_INTERVAL   120000  // Sample every two seconds

//declaration
void dht_wrapper(); // must be declared before the lib initialization

// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

// Global Vars
String version("0.1.5-20160509");
unsigned int DHTnextSampleTime;     // Next time we want to start sample
bool bDHTstarted;           // flag to indicate we started acquisition
int n;                              // counter
float tf;
float tc;
float h;
String tfs = String();
String hs = String();

//
// Functions
//

// Spark setup
void setup()
{
    // tell the world we're online
    Particle.publish("online");
    Particle.publish("version", version, 60, PRIVATE);
    
    DHTnextSampleTime = 0;  // Start the first sample immediately
}

// This wrapper is in charge of calling
// mus be defined like this for the lib work
void dht_wrapper() {
    DHT.isrCallback();
}

void loop() {
 

 if (millis() > DHTnextSampleTime) {
    
    if (!bDHTstarted) {     // start the sample
        DHT.acquire();
        bDHTstarted = true;

    }
    
    

    if (!DHT.acquiring()) {     // has sample completed?

        // get DHT status
        int result = DHT.getStatus();
        char status[64];

        switch (result) {
        case DHTLIB_OK:
            sprintf(status,"OK");
            break;
        case DHTLIB_ERROR_CHECKSUM:
            sprintf(status,"Error\n\r\tChecksum error");
            break;
        case DHTLIB_ERROR_ISR_TIMEOUT:
            sprintf(status,"Error\n\r\tISR time out error");
            break;
        case DHTLIB_ERROR_RESPONSE_TIMEOUT:
            sprintf(status,"Error\n\r\tResponse time out error");
            break;
        case DHTLIB_ERROR_DATA_TIMEOUT:
            sprintf(status,"Error\n\r\tData time out error");
            break;
        case DHTLIB_ERROR_ACQUIRING:
            sprintf(status,"Error\n\r\tAcquiring");
            break;
        case DHTLIB_ERROR_DELTA:
            sprintf(status,"Error\n\r\tDelta time to small");
            break;
        case DHTLIB_ERROR_NOTSTARTED:
            sprintf(status,"Error\n\r\tNot started");
            break;
        default:
            sprintf(status,"Unknown error");
            break;
        }//end switch
        
    Particle.publish("STATUS", status, 60, PRIVATE);

     if(result == DHTLIB_OK) {
        
        h = DHT.getHumidity();
        //tc = DHT.getCelsius();
        tf = DHT.getFahrenheit();
        
        tfs = String(tf,2);
        //String tcs(tc,2);
        hs = String(h,2);

        //Particle.publish("tempF", tfs, 60, PRIVATE);
        //Particle.publish("tempC", tcs, 60, PRIVATE);
        //Particle.publish("humidity", hs, 60, PRIVATE);
        //String datetime = Time.timeStr();
        //Particle.publish("UT", datetime, 60, PRIVATE);
        
        Particle.variable("temperature", tfs);
        Particle.variable("humidity", hs);
        //Particle.variable("updatetime", Time.year());
     }   
     
     else {
        Particle.variable("temperature", NULL);
        Particle.variable("humidity", NULL);
     }

        n++;  // increment counter
        bDHTstarted = false;  // reset the sample flag so we can take another
        DHTnextSampleTime = millis() + DHT_SAMPLE_INTERVAL;  // set the time for next sample
        
    }//end acquiring if
  }//end DHT if
}


