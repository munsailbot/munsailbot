#ifndef __AUTONOMY_H
#define __AUTONOMY_H

#include <BeagleUtil.h>
#include <vector>
#include "SailbotBrain.h"
#include <string>

typedef enum{
    LONG_DISTANCE,
    STATION_KEEPING_STRAT1,
    NAVIGATION_TRIAL,
    MACHINE_VISION
} MODE;

typedef enum{
    MOVING_CHECK,
    MOVE_TO_POINT,
    DOWNWIND,
    UPWIND,
    TACK,
    REACHED_POINT,
    REACHED_END
} SAIL_STATE;

typedef enum{
    TOP_LEFT,
    TOP_RIGHT,
    BOT_RIGHT,
    BOT_LEFT
} BUOY_QUAD;

#include "Logger.h"

template<typename T> class Point{
public:
    T x, y;
};

struct Line{
    Point<double> a, b;
};

struct Waypoint{
    double lat, lon;
};

class TinyGPSPlus;


class Autonomy{

private:
    MODE _mode;
    std::string filename;

    SAIL_STATE _sailState;
    BUOY_QUAD _buoyPoint;
    std::vector<Waypoint> _waypoints;
    size_t _numWaypoints;
    uint8_t _wpId;
    uint8_t _roundDir;

    Point<double> _initialLatLon;
    bool _initialCoordsCaptured;

    state_t _lastState;
    uint8_t _lastMain;
    uint8_t _lastJib;
    uint8_t _lastRud;

    //Tacking parameters
    int _tackTime;
    int _recoveryTime;
    int _initialWindRelative;
    int _desiredWindRelative;
    bool _startedTack;
    bool _startedRecovery;
    bool _recentTack;
    bool _setRound;
    bool _secondRound;
    int _rudderOrientation;

    //State tracking
    uint8_t _downwindCount;
    uint8_t _upwindCount;
    bool _startedUpwind;

    // -- Upwind things
    Point<double> _boatPolar;
    Point<double> _boatCart;
    Point<double> _initialCart;
    Point<double> _waypointPolar;
    Point<double> _waypointCart;
    int _offset;
    int _tackEvent;
    uint8_t _tackTimer;

    // Station keeping
    Timer* _timer;
    uint64_t _skTimer;
    bool _skTimerSet;

public:
    Autonomy(Timer* timer, size_t timestamp, class Logger* log);
    ~Autonomy();

    void setMode(MODE m);
    void step(state_t state, class Logger* log, TinyGPSPlus* tinyGps, BeagleUtil::UARTInterface* serial, size_t timestamp);

    uint8_t getMain();
    uint8_t getRud();
    uint8_t rudderOrientation();

    void resetTimer();

private:
    motorstate_t courseByWind(int windRelative, int angleToSail);
    motorstate_t courseByHeading(int windRelative, int heading, int courseToPoint);
    motorstate_t tack(int x, int windRelative, int initialWindRelative, int desiredWindRelative);

    uint32_t angleBetween(uint32_t angle1, uint32_t angle2);
    inline uint32_t cardinalToStandard(uint32_t angle);
    Point<double> polarToCartesian(double r, double a);
    uint8_t angleQuadrant(uint32_t angle);

    bool pointAboveLine(Point<double> p, Line l);
    bool pointBelowLine(Point<double> p, Line l);

    uint32_t addAngle(uint32_t a, uint32_t b);
    uint32_t subtractAngle(uint32_t a, uint32_t b);

    double distanceBetweenPoints(Point<double> p1, Point<double> p2);
    std::pair<Line, Line> generateControlLines(Point<double> initPos, Point<double> destPos, int offset);
    std::pair<Line, Line> generateAngledControlLines(Point<double> initPos, Point<double> destPos, int offset);
};

#endif
