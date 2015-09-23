#include "Analog.h"

namespace BeagleUtil
{

    Analog::Analog(std::string pin){
        _path = pin;
    }

    int Analog::getValue(){
        std::ifstream fin;
        fin.open(_path.c_str(), std::ios::in);

        fin >> _value;

        return _value;
    }

}
