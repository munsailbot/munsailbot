#include "WindStatFilter.h"

void WindStatFilter::addRawValue(int val){
    _values.push_back(val);

    if(_values.size() > 7){
        _values.erase(_values.begin());
    }
}

int WindStatFilter::getFilteredMean(){
    if(_values.size() >= 7){
        maxMinFilter();

        if(containsBothSigns()){
            std::vector<int> r1, r2;
            for(size_t i=0; i<_values.size(); i++){
                if(_values[i] >= -130 && _values[i] <= 130){
                    r1.push_back(_values[i]);
                }
                if(_values[i] >= -50 && _values[i] <= 50){
                    r2.push_back(_values[i]);
                }
            }

            if(r1.size() > r2.size()){
                return translateAndFilter(r1, 130);
            }
            if(r1.size() < r2.size()){
                return translateAndFilter(r2, 50);
            }
            if(r1.size() == r2.size()){
                int last = _values[_values.size() - 1];
                if(180 - last > 90) return translateAndFilter(r2, 50);
                else return translateAndFilter(r1, 130);
            }
        }
        else{
            //medianFilter(_values);

            int sum = 0;
            for(size_t i=0; i<_values.size(); i++){
                sum += _values[i];
            }

            int mean = sum / static_cast<int>(_values.size());

            //_values.clear();

            return mean;
        }

        /*int sum = 0;
        for(size_t i=0; i<_values.size(); i++){
            sum += _values[i];
        }

        int mean = sum / static_cast<int>(_values.size());

        //_values.clear();

        return mean;*/
    }
    else{
        return 9999;
    }
}

bool WindStatFilter::isReady(){
    if(_values.size() >= 7) return true;
    else return false;
}

void WindStatFilter::maxMinFilter(){
    int max, min, maxIdx, minIdx;
    max = _values[0];
    min = _values[0];
    maxIdx = 0;
    minIdx = 0;


    for(size_t i=0; i<_values.size(); i++){
        if(_values[i] >= max) maxIdx = i;
    }

    _values.erase(_values.begin()+maxIdx);

    for(size_t i=0; i<_values.size(); i++){
        if(_values[i] <= min) minIdx = i;
    }

    _values.erase(_values.begin()+minIdx);
}

void WindStatFilter::medianFilter(std::vector<int>& r){
    int s = r.size();
    //std::cout << "s: " << s << std::endl;

    int tmp[3];
    for (int i = 0; i < 3; i++){
        tmp[0] = r[i % s];
        tmp[1] = r[(i+1) % s];
        tmp[2] = r[(i+2) % s];

        r[(i+1) % s] = getMedianVal(tmp);
    }

}

int WindStatFilter::getMedianVal(int* s){
    if(s[0]>s[1]){
        std::swap(s[0],s[1]);
    }

    if(s[1]>s[2]){
        std::swap(s[1],s[2]);
    }

    if(s[0]>s[2]){
        std::swap(s[0],s[2]);
    }

    if(s[0]>s[1]){
         std::swap(s[0],s[1]);
    }

    if(s[1]>s[2]){
     std::swap(s[1],s[2]);
    }

    return s[1];
}

int WindStatFilter::translateAndFilter(std::vector<int> r, int val){
    for(size_t i=0; i<r.size(); i++){
        if(r[i] < 0){
            r[i] = val - abs(r[i]);
        }
        if(r[i] > 0){
            r[i] += val;
        }
    }

   // medianFilter(r);

    int sum = 0;
    for(size_t i=0; i<r.size(); i++){
        sum += r[i];
    }

    int mean = sum / static_cast<int>(r.size());
    return mean - val;
}

bool WindStatFilter::containsBothSigns(){
    bool neg = false;
    bool pos = false;

    for(size_t i=0; i<_values.size(); i++){
        if(_values[i] < 0) neg = true;
        if(_values[i] > 0) pos = true;
    }

    if(neg && pos) return true;
    else return false;
}
