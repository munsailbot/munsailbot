#include "../../BeagleUtil/include/BeagleUtil.h"
#include "TinyGPSPlus/TinyGPS++.h"
#include "SailbotBrain.h"
#include "ArduinoEncoder.h"
#include "Autonomy.h"
#include "Utility.h"
#include "HanningFilter.h"
#include "ConstKalmanFilter.h"
#include "WindFilter.h"
#include "WindStatFilter.h"
#include "Timer.h"
#include "filterAlphaBeta.h"
#include "AverageAngle.h"
//tmp
#include <curses.h>
#include <ctime>
#include <string>

#include <sys/stat.h>
#include <sys/sendfile.h>

state_t currentState;
Timer* timer = new Timer(); //timer has to be global and i hate myself for it.

unsigned int lerpCur = 1;
unsigned int lerpMax = 5;

uint8_t lerp(uint8_t v0, uint8_t v1, float t){
        return v0 + static_cast<uint8_t>(floorf(t*(static_cast<float>(v1) - static_cast<float>(v0))));
}

uint8_t desiredMain = 0;
uint8_t desiredJib = 0;
uint8_t desiredRud = 35;
uint8_t desiredSail = 0;

int lastWind = 9999;

double lx = 0.0;
double ly = 0.0;

uint8_t bin1Count = 0;
uint8_t bin2Count = 0;
uint8_t bin3Count = 0;
uint8_t bin4Count = 0;

int main(int argc, char* argv[])
{
        //CHANGED: New Logger class
        Logger* log = new Logger();
        log->Timestamp();
        log->SetDir("/log");
        //TODO: This causes segmentation fault
        //log->CheckFiles(1,10);
        log->TrackInit();
        log->LogInit();
        
        BeagleUtil::UART* uart4 = new BeagleUtil::UART(BeagleUtil::UART4, BeagleUtil::UART_USE_PIN, B4800);
        BeagleUtil::UART* uart5 = new BeagleUtil::UART(BeagleUtil::UART5, BeagleUtil::UART_USE_PIN, B4800);

        //Open up a serial port using these pins
        BeagleUtil::UARTInterface* ard = new BeagleUtil::UARTInterface(uart4);
        BeagleUtil::UARTInterface* gps = new BeagleUtil::UARTInterface(uart5);

        //Create an encoder instance and an autonomy instance
        ArduinoEncoder* encoder = new ArduinoEncoder(ard);
        Autonomy* autonomy = new Autonomy(timer, log->timestamp, log);

        // Initialize TinyGPS
        TinyGPSPlus* tinyGps = new TinyGPSPlus();
        TinyGPSCustom windDirection(*tinyGps, "WIMWV", 1);
        TinyGPSCustom windSpeed(*tinyGps, "WIMWV", 5);
        TinyGPSCustom magHeading(*tinyGps, "HCHDT", 1);
        TinyGPSCustom tmg(*tinyGps, "GPVTG", 1);
        TinyGPSCustom sog(*tinyGps, "GPVTG", 5);
        //TODO: Kalman filter instead
        HanningFilter<int> windFilter;
        HanningFilter<double> compassFilter;
        WindFilter* bayesWindFilter = new WindFilter();
        WindStatFilter windStatFilter;

        filterAlphaBeta filterWindX(5, 4);
        filterAlphaBeta filterWindY(5, 4);
        filterWindX.init();
        filterWindY.init();

        //Initialize motors
        Utility::sendMotorValues(ard, 90, 90, 35);

        //Initialize state
        currentState.latitude = 99.99;
        currentState.longitude = 99.99;
        currentState.gpsHeading = 99.99;
        currentState.speed = 99.99;

        uint64_t lastTime = timer->millis();
        uint64_t lastWindTime = timer->millis();

        int8_t enableAutonomy = 0;
        int8_t lastAutonomy = 0;

        while(1) {
                //Read from the GPS
                char c = gps->readByte();
                if(tinyGps->encode(c)) {
                        if(tinyGps->location.isValid()) {
                                currentState.latitude = tinyGps->location.lat();
                                currentState.longitude = tinyGps->location.lng();
                        }
                        else{
                                currentState.latitude = 99.99;
                                currentState.longitude = 99.99;
                        }

                        if(windDirection.isUpdated()) {
                                currentState.windDirection = windFilter.getFilteredValue(atof(windDirection.value()));
                                if(currentState.windDirection > 180) currentState.windDirection = currentState.windDirection - 360;
                        }

                        if(windSpeed.isUpdated()) {
                                currentState.windSpeed = windFilter.getFilteredValue(atof(windSpeed.value()));
                        }

                        if(magHeading.isUpdated()) {
                                currentState.magHeading = compassFilter.getFilteredValue(atof(magHeading.value()));
                        }
                        if(sog.isUpdated()) {
                                currentState.speed = atof(sog.value());
                        }
                        if(tmg.isUpdated()) {
                                currentState.gpsHeading = atof(tmg.value());
                        }
                }

                char c2 = ard->readByte();

                if(encoder->encode(c2))
                      encoder->parse();

                if((timer->millis() - lastTime) >= 500) {
                      //running at 2hz for now
                      lastTime = timer->millis();

                      //Update the boat's current state
                      StringMap params = encoder->getParams();

                      currentState.main = Utility::strToInt(params["MAIN"]);
                      currentState.jib = Utility::strToInt(params["JIB"]);
                      currentState.rudder = Utility::strToInt(params["RUD"]);

                        if(Utility::strToInt(params["EN"]) != 9999) {
                                enableAutonomy = Utility::strToInt(params["EN"]);
                                if(enableAutonomy == 0 && lastAutonomy == 1) {
                                        autonomy->resetTimer();
                                }
                                lastAutonomy = enableAutonomy;
                        }

                        clear();

                        uint8_t lastRud = autonomy->getRud();

                        if(enableAutonomy) {
                                autonomy->step(currentState, log, tinyGps, ard, log->timestamp);
                                //execute a single step of autonomous decision
                        }

                        if((autonomy->getRud() != lastRud)) {
                                lerpCur = 1;
                                desiredRud = autonomy->getRud();
                        }

                        uint8_t lerpRud = lerp(lastRud, desiredRud, static_cast<float>(lerpCur)/static_cast<float>(lerpMax));

                        if(abs(currentState.windDirection) < 55) {
                                bin1Count++;

                                if(bin1Count >= 6) {
                                        bin2Count = 0;
                                        bin3Count = 0;
                                        bin4Count = 0;

                                        desiredSail = 0;
                                }
                        }
                        if(abs(currentState.windDirection) >= 60 && abs(currentState.windDirection) < 85) {
                                bin2Count++;

                                if(bin2Count >= 6) {
                                        bin1Count = 0;
                                        bin3Count = 0;
                                        bin4Count = 0;

                                        desiredSail = 20;
                                }
                        }
                        if(abs(currentState.windDirection) >= 90 && abs(currentState.windDirection) < 120) {
                                bin3Count++;

                                if(bin3Count >= 6) {
                                        bin1Count = 0;
                                        bin2Count = 0;
                                        bin4Count = 0;

                                        desiredSail = 45;
                                }
                        }
                        if(abs(currentState.windDirection) >= 125 && abs(currentState.windDirection) <= 180) {
                                bin4Count++;

                                if(bin4Count >= 6) {
                                        bin1Count = 0;
                                        bin2Count = 0;
                                        bin3Count = 0;

                                        desiredSail = 90;
                                }
                        }

                        // TODO: if the rudder is scaled, the -35 conversion
                        // on the arduino side must be scaled as well
                        // TODO: Is scaled rudder giving 35 for our design?
                        // CHANGED: Flipped rudder control direction for reversed motor
                        float scaledRud = static_cast<float>(autonomy->getRud()) * 0.6f * autonomy->rudderOrientation();
                        uint8_t scaledRudInt = static_cast<uint8_t>(floorf(scaledRud));

                        if(enableAutonomy == true) {
                                Utility::sendMotorValues(ard, desiredSail, desiredSail, scaledRudInt);
                                //Send motor values back to the Arduino
                        }

                        lerpCur++;
                        if(lerpCur > lerpMax) lerpCur = lerpMax;
                        // CHANGED: Commented out refresh() - LCD screen?
                        // refresh();
                }
        }


        //Clean up
        delete bayesWindFilter;
        delete tinyGps;
        delete autonomy;
        delete encoder;
        delete ard;
        delete gps;
        delete uart5;
        delete uart4;
        delete timer;
        delete log;
        return 0;
}
