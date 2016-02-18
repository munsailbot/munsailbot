#include <BeagleUtil.h>
#include "TinyGPSPlus/TinyGPS++.h"
#include "SailbotBrain.h"
#include "ArduinoEncoder.h"
#include "Autonomy.h"
#include "Utility.h"
#include "HanningFilter.h"
#include "WindFilter.h"
#include "WindStatFilter.h"
#include "Timer.h"
#include "filterAlphaBeta.h"
#include "VectorFilter.h"
#include "AverageAngle.h"

//tmp
#include <curses.h>

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
    remove("/log.last.txt");

    int input, output;
    if ((input = open("/log.txt", O_RDONLY)) != -1){
        if ((output = open("/log.last.txt", O_RDWR | O_CREAT, S_IRWXU)) == -1){
            close(input);
        }
    }

    if(input != -1 && output != -1){
        off_t bytesCopied = 0;
        struct stat fileinfo = {0};
        fstat(input, &fileinfo);
        sendfile(output, input, &bytesCopied, fileinfo.st_size);

        close(output);
        close(input);
    }

    //initscr();
    remove("/log.txt");

    //Enable tx/rx pins
    /*BeagleUtil::UART* tx1 = new BeagleUtil::UART(BeagleUtil::UART1_TXD, BeagleUtil::UART_USE_PIN, B9600);
    BeagleUtil::UART* rx1 = new BeagleUtil::UART(BeagleUtil::UART1_RXD, BeagleUtil::UART_USE_PIN|BeagleUtil::UART_INPUT, B9600);

    BeagleUtil::UART* tx2 = new BeagleUtil::UART(BeagleUtil::UART2_TXD, BeagleUtil::UART_USE_PIN, B57600);
    BeagleUtil::UART* rx2 = new BeagleUtil::UART(BeagleUtil::UART2_RXD, BeagleUtil::UART_USE_PIN|BeagleUtil::UART_INPUT, B57600);*/

    BeagleUtil::UART* uart4 = new BeagleUtil::UART(BeagleUtil::UART4, BeagleUtil::UART_USE_PIN, B4800);
    BeagleUtil::UART* uart5 = new BeagleUtil::UART(BeagleUtil::UART5, BeagleUtil::UART_USE_PIN, B4800);
    //BeagleUtil::UART* rx5 = new BeagleUtil::UART("", BeagleUtil::UART_USE_PIN|BeagleUtil::UART_INPUT, B4800);

    //Open up a serial port using these pins
    BeagleUtil::UARTInterface* ard = new BeagleUtil::UARTInterface(uart4);
    BeagleUtil::UARTInterface* gps = new BeagleUtil::UARTInterface(uart5);

    //Create an encoder instance and an autonomy instance
    ArduinoEncoder* encoder = new ArduinoEncoder(ard);
    Autonomy* autonomy = new Autonomy(timer);

    //Initialize TinyGPS
    TinyGPSPlus* tinyGps = new TinyGPSPlus();
    TinyGPSCustom windDirection(*tinyGps, "WIMWV", 1);				// WIND ANGLE		(degrees)
    TinyGPSCustom magHeading(*tinyGps, "HCHDT", 1);				
    TinyGPSCustom tmg(*tinyGps, "GPVTG", 1);					// TRACK MADE GOOD	(degrees)
    TinyGPSCustom sog(*tinyGps, "GPVTG", 5);					// SPEED OVER GROUND	(unit?)

    //We will use a hanning filter to filter the incoming wind direction
    HanningFilter<int> windFilter;
    HanningFilter<double> compassFilter;
    WindFilter* bayesWindFilter = new WindFilter();
    WindStatFilter windStatFilter;

    filterAlphaBeta filterWindX(5, 4);
    filterAlphaBeta filterWindY(5, 4);
    filterWindX.init();
    filterWindY.init();

    VectorFilter vecFilter;
    AverageAngle windAverage;

    //Set the decision mode; changes per competition
    //autonomy->setMode(LONG_DISTANCE);

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
    //std::cout << "before" << std::endl;

    std::ofstream fout;
    fout.open("/log.txt", std::ios::out | std::ios::app);
    fout << "Initialized...starting main loop!" << std::endl;
    fout.close();

    while(1){
        //std::cout << "after" << std::endl;
        //Read from the GPS
        char c = gps->readByte();
        //std::cout << c;
        if(tinyGps->encode(c)){
            if(tinyGps->location.isValid()){
                currentState.latitude = tinyGps->location.lat();
                currentState.longitude = tinyGps->location.lng();
                //if(tinyGps->location.lng() < -50.0){
                //    currentState.longitude = tinyGps->location.lng();
                //}
            }
            else{
                currentState.latitude = 99.99;
                currentState.longitude = 99.99;
            }
            /*currentState.gpsHeading = tinyGps->course.deg();
            currentState.speed = tinyGps->speed.knots();
            std::cout << tinyGps->speed.value();*/

            if(windDirection.isUpdated()){
                currentState.windDirection = windFilter.getFilteredValue(atof(windDirection.value()));
                if(currentState.windDirection > 180)currentState.windDirection = currentState.windDirection - 360;
            }

            if(magHeading.isUpdated()){
                currentState.magHeading = compassFilter.getFilteredValue(atof(magHeading.value()));
            }
            if(sog.isUpdated()){
                currentState.speed = atof(sog.value());
                //std::cout << currentState.latitude << std::endl;
                //std::cout << currentState.speed << std::endl;
            }
            if(tmg.isUpdated()){
                currentState.gpsHeading = atof(tmg.value());
            }
        }
        //std::cout << magHeading.value() << std::endl;
        //std::cout << "after gps" << std::endl;

        //Read in a byte from the serial port (the Arduino), and feed it to the encoder
        //When encode returns 'true', we have a new sentence to parse
        //std::cout << "run " << std::endl;
        char c2 = ard->readByte();
        //std::cout << c2;
        if(encoder->encode(c2))
            encoder->parse();

        //::cout << "after ard" << std::endl;

        if((timer->millis() - lastTime) >= 500){ //running at 2hz for now
            lastTime = timer->millis();

            //Update the boat's current state
            StringMap params = encoder->getParams();
            //currentState.latitude = Utility::strToDouble(params["LAT"])/1000000.0;
            //currentState.longitude = Utility::strToDouble(params["LON"])/1000000.0;

            currentState.main = Utility::strToInt(params["MAIN"]);
            currentState.jib = Utility::strToInt(params["JIB"]);
            currentState.rudder = Utility::strToInt(params["RUD"]);

            if(Utility::strToInt(params["EN"]) != 9999){
                enableAutonomy = Utility::strToInt(params["EN"]);
                if(enableAutonomy == 0 && lastAutonomy == 1){
                    autonomy->resetTimer();
                }
                lastAutonomy = enableAutonomy;
            }

            clear();

            //bayesWindFilter->addMeasurement(static_cast<int8_t>(currentState.windDirection));
            //currentState.windDirection = bayesWindFilter->getWindDirection();

            //dummy values
            /*currentState.latitude = 47.567842 + lx;
            currentState.longitude = -52.707067 + ly;
            currentState.speed = 1.5;
            currentState.gpsHeading = 45;

            lx+=0.000003; ly+=0.000003;*/


            //std::cout << "WIND: " << params["WIND"] << std::endl;
            //std::cout << "WIND: " << std::dec << currentState.windDirection << std::endl;
            //std::cout << "LAT: " << std::dec << currentState.latitude << std::endl;
            //std::cout << "LON: " << std::dec << currentState.longitude << std::endl;
            //std::cout << "SPEED: " << std::dec << currentState.speed << std::endl;
            //std::cout << "C.O.G: " << std::dec << currentState.gpsHeading << std::endl;


            mvprintw(1, 1, "LAT: %f\n", currentState.latitude);
            mvprintw(2, 1, "LON: %f\n", currentState.longitude);
            mvprintw(3, 1, "C.O.G: %f\n", currentState.gpsHeading);
            mvprintw(4, 1, "SPEED: %f\n", currentState.speed);
            mvprintw(5, 1, "WIND: %4d\n", currentState.windDirection);
            mvprintw(6, 1, "MAG: %4d\n", currentState.magHeading);
            mvprintw(7, 1, "GPS: %4d\n", tinyGps->goodSentences());
            mvprintw(7, 25, "GPS CHARS: %4d\n", tinyGps->charsProcessed());
            mvprintw(6, 25, "AUT: %4d\n", enableAutonomy);
            mvprintw(8, 1, "RAW WIND: %s\n", windDirection.value());

            //uint8_t lastMain = autonomy->getMain();
            //uint8_t lastJib = autonomy->getJib();
            uint8_t lastRud = autonomy->getRud();

            if(enableAutonomy){
                autonomy->step(currentState, tinyGps, ard); //execute a single step of autonomous decision
            }

            if(/*(autonomy->getMain() != lastMain) || (autonomy->getJib() != lastJib) || */(autonomy->getRud() != lastRud)){
                lerpCur = 1;

                //desiredMain = autonomy->getMain();
                //desiredJib = autonomy->getJib();
                desiredRud = autonomy->getRud();
            }
            //uint8_t lerpMain = lerp(lastMain, desiredMain, static_cast<float>(lerpCur)/static_cast<float>(lerpMax));
            //uint8_t lerpJib = lerp(lastJib, desiredJib, static_cast<float>(lerpCur)/static_cast<float>(lerpMax));
            uint8_t lerpRud = lerp(lastRud, desiredRud, static_cast<float>(lerpCur)/static_cast<float>(lerpMax));

            if(abs(currentState.windDirection) < 55){
                bin1Count++;

                if(bin1Count >= 6){
                    bin2Count = 0;
                    bin3Count = 0;
                    bin4Count = 0;

                    desiredSail = 0;
                }
            }
            if(abs(currentState.windDirection) >= 60 && abs(currentState.windDirection) < 85){
                bin2Count++;

                if(bin2Count >= 6){
                    bin1Count = 0;
                    bin3Count = 0;
                    bin4Count = 0;

                    desiredSail = 20;
                }
            }
            if(abs(currentState.windDirection) >= 90 && abs(currentState.windDirection) < 120){
                bin3Count++;

                if(bin3Count >= 6){
                    bin1Count = 0;
                    bin2Count = 0;
                    bin4Count = 0;

                    desiredSail = 45;
                }
            }
            if(abs(currentState.windDirection) >= 125 && abs(currentState.windDirection) <= 180){
                bin4Count++;

                if(bin4Count >= 6){
                    bin1Count = 0;
                    bin2Count = 0;
                    bin3Count = 0;

                    desiredSail = 90;
                }
            }

            //TODO: if the rudder is scaled, the -35 conversion on the arduino side must be scaled as well
            float scaledRud = static_cast<float>(autonomy->getRud()) * 0.6f;
            uint8_t scaledRudInt = static_cast<uint8_t>(floorf(scaledRud));

            if(enableAutonomy == true){
                Utility::sendMotorValues(ard, desiredSail, desiredSail, scaledRudInt); //Send motor values back to the Arduino
            }

            mvprintw(1, 25, "RUDDER: %3d, %3d, %3d, %3d\n", scaledRudInt-21, autonomy->getRud()-35, lastRud-35, desiredRud-35);
            mvprintw(2, 25, "MAIN: %3d, %3d\n", desiredSail, autonomy->getMain());
            // mvprintw(3, 25, "JIB: %3d, %3d\n", desiredSail, autonomy->getJib());

            lerpCur++;
            if(lerpCur > lerpMax) lerpCur = lerpMax;

            refresh();
        }
        //Utility::sendMotorValues(ard, autonomy->getMain(), autonomy->getJib(), autonomy->getRud()); //Send motor values back to the Arduino
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

    endwin();

    return 0;
}
