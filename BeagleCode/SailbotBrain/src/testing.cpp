#include "ArduinoEncoder.h"
#include "Utility.h"
#include "Logger.h"
#include "Testing.h"
#include "TinyGPSPlus/TinyGPS++.h"

const int WINDTEST = 0;
const int SAILTEST = 0;
const int RUDTEST = 0;

// Log test values
ArduinoEncoder* encoder = new ArduinoEncoder(ard);
TinyGPSPlus* tinyGps = new TinyGPSPlus();
TinyGPSCustom windDirection(*tinyGps, "WIMWV", 1);
TinyGPSCustom windSpeed(*tinyGps, "WIMWV", 5);
TinyGPSCustom magHeading(*tinyGps, "HCHDT", 1);
TinyGPSCustom tmg(*tinyGps, "GPVTG", 1);
TinyGPSCustom sog(*tinyGps, "GPVTG", 5);
ConstKalmanFilter<int> windFilter;
ConstKalmanFilter<double> compassFilter;
WindFilter* bayesWindFilter = new WindFilter();
WindStatFilter windStatFilter;

void Testing::GPS(){
    
}

void Testing::Wind(){
    
}

void Testing::Compass(){
    
}
void Testing::Remote(){
    
}

void Testing::Encoder(){
    ArduinoEncoder* encoder = new ArduinoEncoder(ard);
}

void Testing::Sail(){
    
}

void Testing::Rudder(){

} 

delete encoder;
delete tinyGps;