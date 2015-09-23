#include "WindFilter.h"
#include <curses.h>

WindFilter::WindFilter(){
    for(int i=0; i<10; i++){
        stateVector[i] = 0;
        belief[i] = 0.1f;
    }
}

void WindFilter::addMeasurement(int8_t wind){
    for(int i=0; i<9; i++){
        stateVector[i+1] = stateVector[i];
    }
    stateVector[0] = wind;
}

int8_t WindFilter::getWindDirection(){
    float beliefTemp[10];
    float sum = 0.0f;
    for(int i=0; i<10; i++){
        beliefTemp[i] = 0.0f;

        for(int j=0; j<10; j++){
            beliefTemp[i] += probability(i, j)*belief[j];
            sum += beliefTemp[i];
        }
    }

    for(int i=0; i<10; i++){
        belief[i] = beliefTemp[i];
        belief[i] /= sum;
    }

    //mvprintw(11, 1, "%d %d %d %d\n", stateVector[0],stateVector[1],stateVector[2],stateVector[3]);
    return stateVector[bestStateIdx()];

}

float WindFilter::probability(int i, int j){
    if((abs(stateVector[i]) - abs(stateVector[j])) > 30){
        return 0.2f;
    }

    if((stateVector[i] == 0 && stateVector[j] != 0) || (stateVector[i] != 0 && stateVector[j] == 0)) {
        return 0.0f;
    }

    return 0.8;
}

int WindFilter::bestStateIdx(){
    float max = 0.0;
    int idx = 0;
    for(int i=0; i<10; i++){
        if(belief[i] > max){
            max = belief[i];
            idx = i;
        }
    }

    return idx;
}
