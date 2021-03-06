// This #include statement was automatically added by the Particle IDE.
#include "PietteTech_DHT/PietteTech_DHT.h"

//
// combination of 
// http://github.com/nicjansma/spark-water-alarm
// and DHT examples
//

//
// Constants
//

#define WATER_SENSOR D6
#define ALARMLED D1
#define BUZZER D5

// How long to wait before noting that the alarm has switched states -- this
// helps stop a super-sensitive sensor from ping-ponging back and forth a lot.
#define DEBOUNCE_SECONDS 2

//DHT systems defines
#define DHTTYPE  DHT22              // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   2         	    // Digital pin for communications
#define DHT_SAMPLE_INTERVAL   60000  // Sample every two seconds

//declaration
void dht_wrapper(); // must be declared before the lib initialization

// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

// Global Vars
String version("0.1.2-2015082604");
// the alarm state: 0 = off, 1 = on
int alarmState = 0;
// the last time we switched alarm states (ms since Unix epoch)
int lastStateSwitchTime = 0;
unsigned int DHTnextSampleTime;	    // Next time we want to start sample
bool bDHTstarted;		    // flag to indicate we started acquisition
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
    // initialize our pins I/O
	pins_init();
	
	// tell the world we're online
	Particle.publish("online");
	Particle.publish("version", version, 60, PRIVATE);
	
	// publish the alarm state variable
	Particle.variable("alarmState", &alarmState, INT);
	Particle.variable("temperature", &tfs, STRING);
	Particle.variable("humidity", &hs, STRING);
	
	DHTnextSampleTime = 0;  // Start the first sample immediately
}

// This wrapper is in charge of calling
// mus be defined like this for the lib work
void dht_wrapper() {
    DHT.isrCallback();
}

// Spark loop
void loop()
{
	if(isExposedToWater()) {
	    // Alarm ON
	    if (alarmState == 0) {
	        // only alarm if we're past the debounce interval
	        int now = Time.now();
	        if (now - lastStateSwitchTime > DEBOUNCE_SECONDS) {
    	        alarmState = 1;
    	        lastStateSwitchTime = now;
	    	    digitalWrite(ALARMLED, HIGH);
	    	    //digitalWrite(BUZZER, HIGH);
		        Particle.publish("alarm", "on", 60, PRIVATE);
	        }
	    }
	} 
	else {
	    // Alarm off
	    if (alarmState == 1) {
	        // only alarm if we're past the debounce interval
	        int now = Time.now();
	        if (now - lastStateSwitchTime > DEBOUNCE_SECONDS) {
    	        alarmState = 0;    
    	        lastStateSwitchTime = now;
	            digitalWrite(ALARMLED, LOW);
	            //digitalWrite(BUZZER, LOW);
	            Particle.publish("alarm", "off", 60, PRIVATE);
	        }
	    }
	}//end alarm if
	
	// Check if we need to start the next sample
  if (millis() > DHTnextSampleTime) {
	if (!bDHTstarted) {		// start the sample
	    DHT.acquire();
	    bDHTstarted = true;
	}

	if (!DHT.acquiring()) {		// has sample completed?

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
	    
	    String tfs = String(tf,2);
	    //String tcs(tc,2);
	    String hs = String(h,2);

	    Particle.publish("tempF", tfs, 60, PRIVATE);
	    //Particle.publish("tempC", tcs, 60, PRIVATE);
	    Particle.publish("humidity", hs, 60, PRIVATE);
     }   

	    n++;  // increment counter
	    bDHTstarted = false;  // reset the sample flag so we can take another
	    DHTnextSampleTime = millis() + DHT_SAMPLE_INTERVAL;  // set the time for next sample
	}//end acquiring if
  }//end DHT if
}

// initialize our pins
void pins_init()
{
    pinMode(ALARMLED, OUTPUT);
	pinMode(WATER_SENSOR, INPUT);
	//pinMode(BUZZER, OUTPUT);
}

// determine if we're exposed to water or not
boolean isExposedToWater()
{
	if (digitalRead(WATER_SENSOR) == HIGH) {
		return true;
	} else {
	    return false;
	}
}
