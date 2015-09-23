#include "ArduinoEncoder.h"

ArduinoEncoder::ArduinoEncoder(BeagleUtil::UARTInterface* serial){
    _serial = serial;

    _validStrings = 0;
    _checksum = 0;
    _eos = false;
}

ArduinoEncoder::~ArduinoEncoder(){
}

bool ArduinoEncoder::encode(char byte){
    if(byte == '$'){ //Start a new string
        _string = "";
        _checksum = 0;
        _eos = false;

        return false;
    }
    else if(byte == '#'){ //End of data, checksum value should follow
        _string += byte;
        _checksum += static_cast<size_t>(byte);
        _eos = true;

        return false;
    }
    else if(byte == '|'){ //End of checksum
        if(_string.find_last_of("#") != std::string::npos){
            std::string sub = _string.substr(_string.find_last_of("#")+1);

            std::stringstream ss;
            ss.clear();
            ss << sub;

            size_t c;
            ss >> c;

            //std::cout << _string << std::endl;
            //std::cout << std::dec << sub << std::endl;
            //std::cout << std::dec << _checksum << std::endl;

            //if(atoi(sub.c_str()) == _checksum){ //valid
                _validStrings++;
                return true;
            /*}
            else{ //checksum failed
                std::cout << std::dec << sub << std::endl;
                std::cout << std::dec << _checksum << std::endl;
                std::cout << "failed" << std::endl;
                return false;
            }*/
        }
        else
            return false;
    }
    else{ //Data byte, just add it to the string and add it to the checksum
        _string += byte;
        if(!_eos)
            _checksum += static_cast<size_t>(byte);

        return false;
    }
}

StringMap ArduinoEncoder::getParams(){
    return _params;
}

size_t ArduinoEncoder::getValidStrings(){
    return _validStrings;
}

//Parses arguments from the currently stored string; should be called when a complete sentence has been encoded
void ArduinoEncoder::parse(){
    std::string sub;
    STRING_TYPE type;
    size_t arg = 0;
    size_t last = 0;

    while(last != std::string::npos){
        last = _string.find_first_of(",");
        sub = _string.substr(0, last);

        if(arg == 0){ //First arg is the sentence type
            if(sub == "GPS")
                type = GPS_STR;
            if(sub == "COM")
                type = COM_STR;
            if(sub == "WND")
                type = WND_STR;
            if(sub == "AUT")
                type = AUT_STR;
            if(sub == "LCK")
                type = LCK_STR;
            if(sub == "ULCK")
                type = UCK_STR;
        }
        else{
            switch(type){ //Pull arguments out depending on sentence type
            case GPS_STR:
                if(arg == 1)
                    _params["LAT"] = sub;
                if(arg == 2)
                    _params["LON"] = sub;
                if(arg == 3)
                    _params["COURSE"] = sub;
                if(arg == 4)
                    _params["SPEED"] = sub;
                break;

            case COM_STR:
                if(arg == 1)
                    _params["HEAD"] = sub;
                break;

            case WND_STR:
                if(arg == 1)
                    _params["WIND"] = sub;
                break;

            case MTR_STR:
                if(arg == 1)
                    _params["MAIN"] = sub;
                if(arg == 2)
                    _params["JIB"] = sub;
                if(arg == 3)
                    _params["RUD"] = sub;
                break;

            case AUT_STR:
                if(arg == 1)
                    _params["EN"] = sub;
                break;

            default:
                break;
            }
        }

        _string = _string.erase(0, last+1);
        arg++;
    }
}
