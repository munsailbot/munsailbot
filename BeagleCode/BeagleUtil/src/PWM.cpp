/*
Daniel Cook 2013
daniel@daniel-cook.net
*/

#include "PWM.h"

namespace BeagleUtil
{

    PWM::PWM(std::string path){
        std::ofstream fout1, fout2, fout3, fout4, fout5, fout6;
        fout1.open("/sys/kernel/debug/omap_mux/gpmc_ad8", std::ios::out);
        fout2.open("/sys/kernel/debug/omap_mux/gpmc_ad9", std::ios::out);
        fout3.open("/sys/kernel/debug/omap_mux/gpmc_a2", std::ios::out);
        fout4.open("/sys/kernel/debug/omap_mux/gpmc_a3", std::ios::out);
        fout5.open("/sys/kernel/debug/omap_mux/mcasp0_aclkx", std::ios::out);
        fout6.open("/sys/kernel/debug/omap_mux/mcasp0_fsx", std::ios::out);

        fout1 << 4;
        fout2 << 4;
        fout3 << 6;
        fout4 << 6;
        fout5 << 1;
        fout6 << 1;

        fout1.close();
        fout2.close();
        fout3.close();
        fout4.close();
        fout5.close();
        fout6.close();

        _path = path;
        _dutyPercent = 0;
    }

    void PWM::run(){
        std::ofstream fout;
        fout.open(std::string(_path+"/run").c_str(), std::ios::out);

        fout << 1;

        fout.close();

        if(!_dutyPercent)
            std::cout << "Warning: duty_percent = 0" << std::endl;
    }

    void PWM::stop(){
        std::ofstream fout;
        fout.open(std::string(_path+"/run").c_str(), std::ios::out);

        fout << 0;

        fout.close();
    }

    bool PWM::request(){
        if(isFree()){
            std::ofstream fout;
            fout.open(std::string(_path+"/request").c_str(), std::ios::out);

            fout << 1;

            fout.close();

            return true;
        }
        else
            return false;
    }

    bool PWM::release(){
        if(!isFree()){
            std::ofstream fout;
            fout.open(std::string(_path+"/request").c_str(), std::ios::out);

            fout << 0;

            fout.close();

            return true;
        }
        else
            return false;
    }

    void PWM::setDutyPercent(float percent){
        std::ofstream fout;
        fout.open(std::string(_path+"/duty_percent").c_str(), std::ios::out);

        fout << percent;
        _dutyPercent = percent;

        fout.close();
    }

    void PWM::setPeriodFreq(float freq){
        setDutyPercent(0);

        std::ofstream fout;
        fout.open(std::string(_path+"/period_freq").c_str(), std::ios::out);

        fout << freq;

        fout.close();
    }

    void PWM::setPeriodFreqWithDutyPercent(float freq, float percent){
        setPeriodFreq(freq);
        setDutyPercent(percent);
    }

    float PWM::getDutyPercent(){
        std::ifstream fin(std::string(_path+"/duty_percent").c_str(), std::ios::in);

        float ret;
        fin >> ret;

        fin.close();
        return ret;
    }

    float PWM::getPeriodFreq(){
        std::ifstream fin(std::string(_path+"/period_freq").c_str(), std::ios::in);

        float ret;
        fin >> ret;

        fin.close();
        return ret;
    }

    bool PWM::isFree(){
        std::ifstream fin;
        fin.open(std::string(_path+"/request").c_str(), std::ios::in);

        //std::string ret1, ret2, ret3;
        //fin >> ret1 >> ret2 >> ret3;
        char line[64];
        fin.getline(line, 64);
        std::string status = std::string(line);

        fin.close();

        //std::string status = ret1+ret2+ret3;

        if(status.find("free") != std::string::npos)
            return true;
        else
            return false;
    }

}
