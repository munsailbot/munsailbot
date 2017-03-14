#include "TinyGPSPlus/TinyGPS++.h"
#include "Autonomy.h"
#include "Utility.h"
#include <curses.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>

Autonomy::Autonomy(Timer* timer, std::string timestamp, Logger* log){
    _sailState = MOVING_CHECK;
    //For now, declare waypoints here
    _wpId = 0;

    // Take the waypoints input via txt file in the root folder and create
    // a vector of waypoints and strategies by parsing the text
    //TODO: Check if there is a text file
    std::ifstream fin("/root/waypoints.txt", std::ios::in);
    std::string mode;
    std::getline(fin, mode);

    char filename[256];
	  std::sprintf(filename, "%s/%s.txt", log->logdir, log->buffer);
  	std::ofstream fout;
  	fout.open (filename, std::ios::out | std::ios::app);
  	fout << "Mode: " << mode << std::endl;

    if(mode == "ld"){
        this->setMode(LONG_DISTANCE);
    }
    else if(mode == "sk"){
        this->setMode(STATION_KEEPING_STRAT1);
    }
    else if(mode == "ntp"){
      this->setMode(NAVIGATION_TRIAL);
      _roundDir = -1;
    }
    else if(mode == "ntsb"){
      this->setMode(NAVIGATION_TRIAL);
      _roundDir = 1;
    }
    else{
      fout << "Invalid autonomy mode" << std::endl;
    }

    for(std::string line; std::getline(fin, line); ){
        //make a stream for the line itself
        std::istringstream in(line);
        double lat,lon;

        in >> lat >> lon;

        Waypoint point;
        point.lat = lat;
        point.lon = lon;
        fout << std::setiosflags(std::ios::fixed) << std::setprecision(6)
        << point.lat << " " << point.lon << std::endl;

        _waypoints.push_back(point);
    }

    fin.close();

    _numWaypoints = _waypoints.size();
    fout << "Num waypoints: " << _numWaypoints << std::endl;
    fout.close();

    _lastMain = 0;
    _lastRud = 35;

    _tackTime = 0;
    _recoveryTime = 0;
    _initialWindRelative = 9999;
    _desiredWindRelative = 9999;
    _startedTack = false;
    _startedRecovery = false;
    _initialCoordsCaptured = false;
    _recentTack = false;
    _setRound = false;
    _secondRound = false;

    _downwindCount = 0;
    _upwindCount = 0;
    _startedUpwind = false;

    _offset = 30;
    _tackEvent = 0;
    _tackTimer = 60;
    _timer = timer;
    _skTimerSet = false;

}

Autonomy::~Autonomy(){}

void Autonomy::setMode(MODE m){
    _mode = m;
    if(_mode == STATION_KEEPING_STRAT1 || _mode == NAVIGATION_TRIAL)
      _sailState = MOVE_TO_POINT;
}

//Executes a single state->action step. The frequency of these steps is determined externally.
void Autonomy::step(state_t state, Logger* log, TinyGPSPlus* tinyGps, BeagleUtil::UARTInterface* serial, std::string timestamp){
    uint8_t main, rud;
    main = _lastMain;
    rud = _lastRud;

    double wpCourse = tinyGps->courseTo(state.latitude, state.longitude,
      _waypoints[_wpId].lat, _waypoints[_wpId].lon);
    double wpDist = tinyGps->distanceBetween(state.latitude, state.longitude,
      _waypoints[_wpId].lat, _waypoints[_wpId].lon);

    log->LogStep(_waypoints, _sailState, state, wpCourse, wpDist, _wpId);
    log->TrackStep(_waypoints, _sailState, state, wpCourse, wpDist, _wpId);

    if(_mode == LONG_DISTANCE)  fout << "Mode: Long Distance" << std::endl;
    if(_mode == STATION_KEEPING_STRAT1) fout << "Mode: Station Keeping (Strategy 1)" << std::endl;
    if(_mode  == NAVIGATION_TRIAL) fout << "Mode: Navigation Trial" << std::endl;

    if(_initialCoordsCaptured == false){
        if(state.latitude != 99.99 && state.longitude != 99.99){
            _initialLatLon.x = state.latitude;
            _initialLatLon.y = state.longitude;
            _initialCoordsCaptured = true;

            fout << "Initial Lat: " << _initialLatLon.x << std::endl;
            fout << "Initial Lon: "  <<_initialLatLon.y << std::endl;

            log->TrackStep(_waypoints, _sailState, wpCourse, wpDist);
        }
    }

    if(_mode == LONG_DISTANCE){
        motorstate_t r;
        switch(_sailState){
            case MOVING_CHECK:
                if(wpDist <= 5){
                    _sailState = REACHED_POINT;
                }
                else{
                    if(state.speed >= 0.5){
                        _sailState = MOVE_TO_POINT;
                    }
                    else{
                        if(state.windDirection > -15 && state.windDirection < 15){
                            break;
                        }
                        else{
                            if(state.windDirection < 0)
                                r = courseByWind(state.windDirection, -90);
                            else
                                r = courseByWind(state.windDirection, 90);

                            rud = (r.rudder + 35);
                            main = r.main;
                        }
                    }
                }
                break;

            case MOVE_TO_POINT:
                if((state.latitude != 0.0) && (state.longitude != 0.0) && (state.latitude != 99.99) && (state.longitude != 99.99)){

                    if(wpDist <= 5){
                        _sailState = REACHED_POINT;
                    }
                    else{
                        uint32_t windAbs = (state.windDirection + static_cast<uint32_t>(floor(state.gpsHeading))) % 360;
                        fout << "Absolute Wind: " << windAbs << std::endl;
                        if(angleBetween(cardinalToStandard(windAbs), cardinalToStandard(wpCourse)) <= 75){
                            _sailState = UPWIND;
                        }
                        else{
                            _sailState = DOWNWIND;
                        }
                    }
                }

                break;

            case DOWNWIND:
                _downwindCount++;

                if(wpDist <= 5){
                    _sailState = REACHED_POINT;
                }
                else{
                    if(_downwindCount >= 3){
                        _upwindCount = 0;
                        _startedUpwind = false;

                        //motorstate_t r;
                        r = courseByHeading(state.windDirection, state.gpsHeading, static_cast<uint32_t>(floor(wpCourse)));
                        rud = static_cast<uint8_t>(r.rudder + 35);
                        main = r.main;

                        _sailState = MOVING_CHECK;
                    }
                    else{
                        _sailState = MOVING_CHECK;
                    }
                }

                break;

            case UPWIND:
                _upwindCount++;

                if(wpDist <= 5){
                    _sailState = REACHED_POINT;
                }
                else{
                    if(_upwindCount >= 3){
                        _downwindCount = 0;

                        std::cout << "Started upwind: " << _startedUpwind << std::endl;
                        if(_startedUpwind == false){
                            Point<double> llp;
                            llp.x = state.latitude;
                            llp.y = state.longitude;

                            //_boatPolar.x = distanceBetweenPoints(_initialLatLon, llp);
                            _boatPolar.x = tinyGps->distanceBetween(_initialLatLon.x, _initialLatLon.y, llp.x, llp.y);
                            _boatPolar.y = cardinalToStandard(tinyGps->courseTo(_initialLatLon.x, _initialLatLon.y, state.latitude, state.longitude));
                            _boatCart = polarToCartesian(_boatPolar.x, _boatPolar.y);
                            _initialCart.x = _boatCart.x;
                            _initialCart.y = _boatCart.y;
                            fout << "Initial Boat XY: " << _initialCart.x << ", " << _initialCart.y << std::endl;
                            fout << "Boat XY: " << _boatCart.x << " , " << _boatCart.y << std::endl;

                            Point<double> llw;
                            llw.x = _waypoints[_wpId].lat;
                            llw.y = _waypoints[_wpId].lon;
                            //_waypointPolar.x = distanceBetweenPoints(_initialLatLon, llw);
                            _waypointPolar.x = tinyGps->distanceBetween(_initialLatLon.x, _initialLatLon.y, llw.x, llw.y);
                            _waypointPolar.y = cardinalToStandard(tinyGps->courseTo(_initialLatLon.x, _initialLatLon.y, llw.x, llw.y));
                            _waypointCart = polarToCartesian(_waypointPolar.x, _waypointPolar.y);

                            _startedUpwind = true;
                        }
                        else{
                            Point<double> llp;
                            llp.x = state.latitude;
                            llp.y = state.longitude;

                            //_boatPolar.x = distanceBetweenPoints(_initialLatLon, llp);
                            _boatPolar.x = tinyGps->distanceBetween(_initialLatLon.x, _initialLatLon.y, llp.x, llp.y);
                            _boatPolar.y = cardinalToStandard(tinyGps->courseTo(_initialLatLon.x, _initialLatLon.y, state.latitude, state.longitude));
                            _boatCart = polarToCartesian(_boatPolar.x, _boatPolar.y);
                            fout << "Boat XY: " << _boatCart.x << " , " << _boatCart.y << std::endl;

                            std::pair<Line, Line> lines = generateControlLines(_initialCart, _waypointCart, _offset);
                            fout << "Line1: " << lines.first.a.x << ","  << lines.first.a.y << "," << lines.first.b.x << "," << lines.first.b.y << std::endl;
                            fout << "Line2: " << lines.second.a.x << ","  << lines.second.a.y << "," << lines.second.b.x << "," << lines.second.b.y << std::endl;

                            uint32_t windAbs = (state.windDirection + static_cast<uint32_t>(floor(state.gpsHeading))) % 360;

                            if((pointBelowLine(_boatCart, lines.second) && pointBelowLine(_boatCart, lines.first)) || (pointAboveLine(_boatCart, lines.second) && pointAboveLine(_boatCart, lines.first))){
                                _sailState = TACK;

                                if(_tackEvent == 0){
                                    //_offset -= 3;
                                    _tackEvent = 1;
                                }
                            }
                            else{
                                if(state.windDirection > -15 && state.windDirection < 15 && state.speed < 0.5){
                                    break;
                                }
                                else{
                                    if(state.windDirection < 0)
                                        r = courseByWind(state.windDirection, -65);
                                    else
                                        r = courseByWind(state.windDirection, 65);
                                    main = r.main;
                                    rud = static_cast<uint8_t>(r.rudder + 35);
                                    _sailState = MOVING_CHECK;
                                }
                            }

                            if((pointBelowLine(_boatCart, lines.second) && pointAboveLine(_boatCart, lines.first)) || (pointAboveLine(_boatCart, lines.second) && pointBelowLine(_boatCart, lines.first))){
                                _tackEvent = 0;
                            }
                        }
                    }
                    else{
                        _sailState = MOVING_CHECK;
                    }
                }
                break;

            case TACK:

                if(wpDist <= 5){
                    _sailState = REACHED_POINT;
                }
                else{
                    if(_startedTack == false){
                        _tackTime = 0;
                        _initialWindRelative = state.windDirection;

                        if(_initialWindRelative > 0){
                            rud = 70;
                        }
                        if(_initialWindRelative < 0){
                            rud = 0;
                        }

                        _startedTack = true;
                    }
                    else{
                        if(_tackTime >= 10){
                            if(_startedRecovery == false){
                                _recoveryTime = 0;
                                if(state.windDirection > 0)
                                    r = courseByWind(state.windDirection, 65);
                                else
                                    r = courseByWind(state.windDirection, -65);
                                rud = r.rudder + 35;
                                main = r.main;

                                _startedRecovery = true;
                            }
                            else{
                                if(_recoveryTime >= 12){
                                    _sailState = MOVE_TO_POINT;

                                    _startedTack = false;
                                    _startedRecovery = false;
                                    _initialWindRelative = 9999;
                                }
                                else{
                                    if(state.windDirection > 0)
                                        r = courseByWind(state.windDirection, 65);
                                    else
                                        r = courseByWind(state.windDirection, -65);
                                    rud = r.rudder + 35;
                                    main = r.main;

                                    _recoveryTime++;
                                }
                            }
                        }
                        else{
                            _tackTime++;
                        }
                    }
                }
                break;

            case REACHED_POINT:
                _startedUpwind = false;
                _startedTack = false;
                _downwindCount = 0;
                _upwindCount = 0;

                _wpId = (_wpId + 1) % _numWaypoints;

                _sailState = MOVING_CHECK;
                break;

            default:
                //Skipper's drunk, don't do anything.
                break;
        }

    }
    else if(STATION_KEEPING_STRAT1){
        motorstate_t r;

        if(_skTimerSet == false){
            _skTimer = _timer->millis();
            _skTimerSet = true;

            fout << "SK Timer: " << _skTimer << std::endl;
        }

        if((_timer->millis() - _skTimer) < ((1000*60)*5)){
            switch(_sailState){

                case MOVE_TO_POINT:
                    if((state.latitude != 0.0) && (state.longitude != 0.0) && (state.latitude != 99.99) && (state.longitude != 99.99)){
                        if(wpDist <= 1){
                            _sailState = REACHED_POINT;
                        }
                        else{
                            uint32_t windAbs = (state.windDirection + static_cast<uint32_t>(floor(state.gpsHeading))) % 360;
                            fout << "Absolute Wind: " << windAbs << std::endl;
                            if(angleBetween(cardinalToStandard(windAbs), cardinalToStandard(wpCourse)) <= 75){
                                _sailState = UPWIND;
                            }
                            else{
                                _sailState = DOWNWIND;
                            }
                        }
                    }

                    break;

                case DOWNWIND:
                    _downwindCount++;

                    if(wpDist <= 1){
                        _sailState = REACHED_POINT;
                    }
                    else{
                        if(_downwindCount >= 3){
                            _upwindCount = 0;
                            _startedUpwind = false;

                            //motorstate_t r;
                            r = courseByHeading(state.windDirection, state.gpsHeading, static_cast<uint32_t>(floor(wpCourse)));
                            rud = static_cast<uint8_t>(r.rudder + 35);
                            main = r.main;

                            _sailState = MOVE_TO_POINT;
                        }
                        else{
                            _sailState = MOVE_TO_POINT;
                        }
                    }

                    break;

                case UPWIND:
                    _upwindCount++;

                    if(wpDist <= 1){
                        _sailState = REACHED_POINT;
                    }
                    else{
                        if(_upwindCount >= 3){
                            _downwindCount = 0;

                            std::cout << "Started upwind: " << _startedUpwind << std::endl;
                            if(_startedUpwind == false){
                                Point<double> llp;
                                llp.x = state.latitude;
                                llp.y = state.longitude;

                                //_boatPolar.x = distanceBetweenPoints(_initialLatLon, llp);
                                _boatPolar.x = tinyGps->distanceBetween(_initialLatLon.x, _initialLatLon.y, llp.x, llp.y);
                                _boatPolar.y = cardinalToStandard(tinyGps->courseTo(_initialLatLon.x, _initialLatLon.y, state.latitude, state.longitude));
                                _boatCart = polarToCartesian(_boatPolar.x, _boatPolar.y);
                                _initialCart.x = _boatCart.x;
                                _initialCart.y = _boatCart.y;
                                fout << "Initial Boat XY: " << _initialCart.x << ", " << _initialCart.y << std::endl;
                                fout << "Boat XY: " << _boatCart.x << " , " << _boatCart.y << std::endl;

                                Point<double> llw;
                                llw.x = _waypoints[_wpId].lat;
                                llw.y = _waypoints[_wpId].lon;
                                _waypointPolar.x = tinyGps->distanceBetween(_initialLatLon.x, _initialLatLon.y, llw.x, llw.y);
                                _waypointPolar.y = cardinalToStandard(tinyGps->courseTo(_initialLatLon.x, _initialLatLon.y, llw.x, llw.y));
                                _waypointCart = polarToCartesian(_waypointPolar.x, _waypointPolar.y);

                                _startedUpwind = true;
                            }
                            else{
                                Point<double> llp;
                                llp.x = state.latitude;
                                llp.y = state.longitude;

                                _boatPolar.x = tinyGps->distanceBetween(_initialLatLon.x, _initialLatLon.y, llp.x, llp.y);
                                _boatPolar.y = cardinalToStandard(tinyGps->courseTo(_initialLatLon.x, _initialLatLon.y, state.latitude, state.longitude));
                                _boatCart = polarToCartesian(_boatPolar.x, _boatPolar.y);
                                fout << "Boat XY: " << _boatCart.x << " , " << _boatCart.y << std::endl;

                                std::pair<Line, Line> lines = generateControlLines(_initialCart, _waypointCart, _offset);
                                fout << "Line1: " << lines.first.a.x << ","  << lines.first.a.y << "," << lines.first.b.x << "," << lines.first.b.y << std::endl;
                                fout << "Line2: " << lines.second.a.x << ","  << lines.second.a.y << "," << lines.second.b.x << "," << lines.second.b.y << std::endl;

                                uint32_t windAbs = (state.windDirection + static_cast<uint32_t>(floor(state.gpsHeading))) % 360;

                                if((pointBelowLine(_boatCart, lines.second) && pointBelowLine(_boatCart, lines.first)) || (pointAboveLine(_boatCart, lines.second) && pointAboveLine(_boatCart, lines.first))){
                                    _sailState = TACK;

                                    if(_tackEvent == 0){
                                        _tackEvent = 1;
                                    }
                                }
                                else{
                                    if(state.windDirection > -15 && state.windDirection < 15 && state.speed < 0.5){
                                        break;
                                    }
                                    else{
                                        if(state.windDirection < 0)
                                            r = courseByWind(state.windDirection, -75);
                                        else
                                            r = courseByWind(state.windDirection, 75);
                                        main = r.main;
                                        rud = static_cast<uint8_t>(r.rudder + 35);
                                        _sailState = MOVE_TO_POINT;
                                    }
                                }

                                if((pointBelowLine(_boatCart, lines.second) && pointAboveLine(_boatCart, lines.first)) || (pointAboveLine(_boatCart, lines.second) && pointBelowLine(_boatCart, lines.first))){
                                    _tackEvent = 0;
                                }
                            }
                        }
                        else{
                            _sailState = MOVE_TO_POINT;
                        }
                    }
                    break;

                case TACK:

                    if(wpDist <= 1){
                        _sailState = REACHED_POINT;
                    }
                    else{
                        if(_startedTack == false){
                            _tackTime = 0;
                            _initialWindRelative = state.windDirection;

                            if(_initialWindRelative > 0){
                                rud = 70;
                            }
                            if(_initialWindRelative < 0){
                                rud = 0;
                            }

                            _startedTack = true;
                        }
                        else{
                            if(_tackTime >= 6){
                                if(_startedRecovery == false){
                                    _recoveryTime = 0;
                                    if(state.windDirection > 0)
                                        r = courseByWind(state.windDirection, 75);
                                    else
                                        r = courseByWind(state.windDirection, -75);
                                    rud = r.rudder + 35;
                                    main = r.main;

                                    _startedRecovery = true;
                                }
                                else{
                                    if(_recoveryTime >= 5){
                                        _sailState = MOVE_TO_POINT;

                                        _startedTack = false;
                                        _startedRecovery = false;
                                        _initialWindRelative = 9999;
                                    }
                                    else{
                                        if(state.windDirection > 0)
                                            r = courseByWind(state.windDirection, 75);
                                        else
                                            r = courseByWind(state.windDirection, -75);
                                        rud = r.rudder + 35;
                                        main = r.main;

                                        _recoveryTime++;
                                    }
                                }
                            }
                            else{
                                _tackTime++;
                            }
                        }
                    }
                    break;

                case REACHED_POINT:
                    _startedUpwind = false;
                    _startedTack = false;
                    _downwindCount = 0;
                    _upwindCount = 0;

                    _wpId = (_wpId + 1) % _numWaypoints;

                    _sailState = MOVE_TO_POINT;
                    break;
                default:
                    break;
            }
        }
        else{
            fout << "SK Exit: " << _timer->millis() << std::endl;
            _mode = LONG_DISTANCE;
            outpoint.lat = 44.22369;
            outpoint.lon = -76.483736;
            _waypoints.push_back(outpoint);
            _wpId++;

        }
    }
    else if(NAVIGATION_TRIAL){

      // CHANGED: CREATED BUOY ROUNDING WAYPOINT

      double lat,lon;
      Waypoint point, point1, point2, point3, point4;

      point.lat = _waypoints[_wpId].lat;
      point.lon = _waypoints[_wpId].lat;
      point3.lat = _initialLatLon.x;
      point3.lon = _initialLatLon.y;

      if(_initialLatLon.x > _waypoints[_wpId].lat){
          if(_initialLatLon.y > _waypoints[_wpId].lon){
            _buoyPoint = BOT_LEFT;
          }
          else {_buoyPoint = BOT_RIGHT;};
        }
      else if (_initialLatLon.x < _waypoints[_wpId].lat){
          if(_initialLatLon.y > _waypoints[_wpId].lon){
            _buoyPoint = TOP_LEFT;
          }
          else {_buoyPoint = TOP_RIGHT;}
        }

      if (_setRound == false){
        int roundDist = 0.0001;

        switch (_buoyPoint) {
          case TOP_LEFT:

              point.lat = _waypoints[_wpId].lat;
              point.lon = _waypoints[_wpId].lon + (roundDist * _roundDir);
              point1.lat = _waypoints[_wpId].lat - (roundDist * _roundDir);
              point1.lon = _waypoints[_wpId].lon;
              point2.lat = _waypoints[_wpId].lat;
              point2.lon = _waypoints[_wpId].lon - (roundDist * _roundDir);
              _waypoints.push_back(point);
              _waypoints.push_back(point1);
              _waypoints.push_back(point2);
              _setRound = true;

          case TOP_RIGHT:

              point.lat = _waypoints[_wpId].lat;
              point.lon = _waypoints[_wpId].lon + (roundDist * _roundDir);
              point1.lat = _waypoints[_wpId].lat - (roundDist * _roundDir);
              point1.lon = _waypoints[_wpId].lon;
              point2.lat = _waypoints[_wpId].lat;
              point2.lon = _waypoints[_wpId].lon - (roundDist * _roundDir);
              _waypoints.push_back(point);
              _waypoints.push_back(point1);
              _waypoints.push_back(point2);
              _setRound = true;

          case BOT_RIGHT:

              point.lat = _waypoints[_wpId].lat;
              point.lon = _waypoints[_wpId].lon - (roundDist * _roundDir);
              point1.lat = _waypoints[_wpId].lat + (roundDist * _roundDir);
              point1.lon = _waypoints[_wpId].lon;
              point2.lat = _waypoints[_wpId].lat;
              point2.lon = _waypoints[_wpId].lon + (roundDist * _roundDir);
              _waypoints.push_back(point);
              _waypoints.push_back(point1);
              _waypoints.push_back(point2);
              _setRound = true;

          case BOT_LEFT:

              point.lat = _waypoints[_wpId].lat;
              point.lon = _waypoints[_wpId].lon - (roundDist * _roundDir);
              point1.lat = _waypoints[_wpId].lat + (roundDist * _roundDir);
              point1.lon = _waypoints[_wpId].lon;
              point2.lat = _waypoints[_wpId].lat;
              point2.lon = _waypoints[_wpId].lon + (roundDist * _roundDir);
              _waypoints.push_back(point);
              _waypoints.push_back(point1);
              _waypoints.push_back(point2);
              _setRound = true;

          }
      }

      motorstate_t r;

      switch(_sailState){
        case MOVING_CHECK:
            _tackTimer++;
            if(wpDist <= 5){
              // Within range of waypoint
                _sailState = REACHED_POINT;
            }
            else{
                if(state.speed >= 0.5){
                  // Does it have momentum
                    _sailState = MOVE_TO_POINT;
                }
                else{
                  // Sailing against the wind?
                    if(state.windDirection > -15 && state.windDirection < 15){
                        break;
                    }
                    else{
                        if(state.windDirection < 0)
                        // Adjust sails for wind direction
                            r = courseByWind(state.windDirection, -90);
                        else
                            r = courseByWind(state.windDirection, 90);

                        rud = (r.rudder + 35);
                        // Rudder + 35??
                        main = r.main;
                    }
                }
            }
            break;

        case MOVE_TO_POINT:

            _tackTimer++;
            if((state.latitude != 0.0) && (state.longitude != 0.0) && (state.latitude != 99.99) && (state.longitude != 99.99)){

                if(wpDist <= 5){                                              // Is it within distance of waypoint?
                    _sailState = REACHED_POINT;
                }

                else{
                    uint32_t windAbs = (state.windDirection + static_cast<uint32_t>(floor(state.gpsHeading))) % 360;
                    fout << "Absolute Wind: " << windAbs << std::endl;
                    if(angleBetween(cardinalToStandard(windAbs), cardinalToStandard(wpCourse)) <= 75){
                        _sailState = UPWIND;
                    }
                    else{
                        _sailState = DOWNWIND;
                    }
                }
            }

            break;

        case DOWNWIND:
            _downwindCount++;

            if(wpDist <= 5){
                _sailState = REACHED_POINT;
            }
            else{
                if(_downwindCount >= 3){
                    _upwindCount = 0;
                    _startedUpwind = false;

                    r = courseByHeading(state.windDirection, state.gpsHeading, static_cast<uint32_t>(floor(wpCourse)));
                    rud = static_cast<uint8_t>(r.rudder + 35);
                    main = r.main;

                    _sailState = MOVING_CHECK;
                }
                else{
                    _sailState = MOVING_CHECK;
                }
            }

            break;

        case UPWIND:
            _upwindCount++;

            if(wpDist <= 5){
                _sailState = REACHED_POINT;
            }
            else{
                if(_upwindCount >= 3){
                    _downwindCount = 0;

                    std::cout << "Started upwind: " << _startedUpwind << std::endl;
                    if(_startedUpwind == false){
                        Point<double> llp;
                        llp.x = state.latitude;
                        llp.y = state.longitude;

                        _boatPolar.x = tinyGps->distanceBetween(_initialLatLon.x, _initialLatLon.y, llp.x, llp.y);
                        _boatPolar.y = cardinalToStandard(tinyGps->courseTo(_initialLatLon.x, _initialLatLon.y, state.latitude, state.longitude));
                        _boatCart = polarToCartesian(_boatPolar.x, _boatPolar.y);
                        _initialCart.x = _boatCart.x;
                        _initialCart.y = _boatCart.y;
                        fout << "Initial Boat XY: " << _initialCart.x << ", " << _initialCart.y << std::endl;
                        fout << "Boat XY: " << _boatCart.x << " , " << _boatCart.y << std::endl;

                        Point<double> llw;
                        llw.x = _waypoints[_wpId].lat;
                        llw.y = _waypoints[_wpId].lon;
                        _waypointPolar.x = tinyGps->distanceBetween(_initialLatLon.x, _initialLatLon.y, llw.x, llw.y);
                        _waypointPolar.y = cardinalToStandard(tinyGps->courseTo(_initialLatLon.x, _initialLatLon.y, llw.x, llw.y));
                        _waypointCart = polarToCartesian(_waypointPolar.x, _waypointPolar.y);

                        _startedUpwind = true;
                    }
                    else{
                        Point<double> llp;
                        llp.x = state.latitude;
                        llp.y = state.longitude;

                        _boatPolar.x = tinyGps->distanceBetween(_initialLatLon.x, _initialLatLon.y, llp.x, llp.y);
                        _boatPolar.y = cardinalToStandard(tinyGps->courseTo(_initialLatLon.x, _initialLatLon.y, state.latitude, state.longitude));
                        _boatCart = polarToCartesian(_boatPolar.x, _boatPolar.y);
                        fout << "Boat XY: " << _boatCart.x << " , " << _boatCart.y << std::endl;

                        std::pair<Line, Line> lines = generateControlLines(_initialCart, _waypointCart, _offset);
                        fout << "Line1: " << lines.first.a.x << ","  << lines.first.a.y << "," << lines.first.b.x << "," << lines.first.b.y << std::endl;
                        fout << "Line2: " << lines.second.a.x << ","  << lines.second.a.y << "," << lines.second.b.x << "," << lines.second.b.y << std::endl;

                        uint32_t windAbs = (state.windDirection + static_cast<uint32_t>(floor(state.gpsHeading))) % 360;

                        if((pointBelowLine(_boatCart, lines.second) && pointBelowLine(_boatCart, lines.first)) || (pointAboveLine(_boatCart, lines.second) && pointAboveLine(_boatCart, lines.first))){
                            _sailState = TACK;

                            if(_tackEvent == 0){
                                //_offset -= 3;
                                _tackEvent = 1;
                            }
                        }
                        else{
                            if(state.windDirection > -15 && state.windDirection < 15 && state.speed < 0.5){
                                break;
                            }
                            else{
                                if(state.windDirection < 0)
                                    r = courseByWind(state.windDirection, -65);
                                else
                                    r = courseByWind(state.windDirection, 65);
                                main = r.main;
                                rud = static_cast<uint8_t>(r.rudder + 35);
                                _sailState = MOVING_CHECK;
                            }
                        }

                        if((pointBelowLine(_boatCart, lines.second) && pointAboveLine(_boatCart, lines.first)) || (pointAboveLine(_boatCart, lines.second) && pointBelowLine(_boatCart, lines.first))){
                            _tackEvent = 0;
                        }
                    }
                }
                else{
                    _sailState = MOVING_CHECK;
                }
            }
            break;

        case TACK:

            if(wpDist <= 1){
                _sailState = REACHED_POINT;
            }
            else{
                if(_startedTack == false){
                    _tackTime = 0;
                    _initialWindRelative = state.windDirection;

                    if(_initialWindRelative > 0){
                        rud = 70;
                    }
                    if(_initialWindRelative < 0){
                        rud = 0;
                    }

                    _startedTack = true;
                }
                else{
                      if(_tackTime >= 10){
                        if(_startedRecovery == false){
                            _recoveryTime = 0;
                            if(state.windDirection > 0)
                                r = courseByWind(state.windDirection, 65);
                            else
                                r = courseByWind(state.windDirection, -65);
                            rud = r.rudder + 35;
                            main = r.main;

                            _startedRecovery = true;
                        }
                        else{
                            if(_recoveryTime >= 12){
                                _sailState = MOVE_TO_POINT;

                                _startedTack = false;
                                _startedRecovery = false;
                                _initialWindRelative = 9999;
                            }
                            else{
                                if(state.windDirection > 0)
                                    r = courseByWind(state.windDirection, 65);
                                else
                                    r = courseByWind(state.windDirection, -65);
                                rud = r.rudder + 35;
                                main = r.main;

                                _recoveryTime++;
                            }
                        }
                    }
                    else{
                        _tackTime++;
                    }
                }
            }
            break;

        case REACHED_POINT:
            _startedUpwind = false;
            _startedTack = false;
            _downwindCount = 0;
            _upwindCount = 0;

            _wpId = (_wpId + 1) % _numWaypoints;

            _sailState = MOVING_CHECK;
            break;

        default:
              //Skipper's drunk, don't do anything.
              break;
      }

      _lastMain = main;
      _lastRud = rud;

      fout << "Sail: " << std::dec << static_cast<int>(_lastMain) << std::endl;
      fout << "Rudder: " << std::dec << static_cast<int>(_lastRud) - 35 << std::endl;
      fout << "---------------" << std::endl;
      fout.close();
      tout.close();

      _lastState = state;
      _tackTimer++;
    };

    _lastMain = main;
    _lastRud = rud;

    fout << "Sail: " << std::dec << static_cast<int>(_lastMain) << std::endl;
    fout << "Rudder: " << std::dec << static_cast<int>(_lastRud) - 35 << std::endl;
    fout << "---------------" << std::endl;
    fout.close();
    tout.close();

    _lastState = state;
    _tackTimer++;
}

uint8_t Autonomy::getMain(){
    return _lastMain;
}

uint8_t Autonomy::getRud(){
    return _lastRud;
}

void Autonomy::resetTimer(){
    _skTimerSet = false;
}

/* Attempts to sail the boat by a specific angle to the wind
 * windRelative is the current relative angle of the wind to the boat
 * angleToSail is the desired angle of the wind to the boat
 * Assumes windRelative and angleToSail both have the same sign!
*/
motorstate_t Autonomy::courseByWind(int windRelative, int angleToSail){
    motorstate_t out;

    int32_t theta = static_cast<uint32_t>(angleToSail - windRelative);
    out.rudder = static_cast<int>(floorf((35.0f/180.0f)*static_cast<float>(theta)));
    out.rudder *= -1;

    if(abs(windRelative) < 45){
        out.main = 0;
    }
    else{
        float sailPos = static_cast<float>(abs(windRelative) - 45)*0.66f;
        out.main = static_cast<int>(floor(sailPos));
    }

    return out;
}

/* attempts to sail by gps heading; needs to be verified */
motorstate_t Autonomy::courseByHeading(int windRelative, int heading, int courseToPoint){
    motorstate_t out;

    int theta = courseToPoint - heading;
    if(theta > 180){
        theta -= 360;
    }
    if(theta < -180){
        theta += 360;
    }

    out.rudder = static_cast<int>(floor((35.0f/180.0f)*static_cast<float>(theta)));

    if(abs(windRelative) < 45){
        out.main = 0;
    }
    else{
        float sailPos = static_cast<float>(abs(windRelative) - 45)*0.66f;
        out.main = static_cast<int>(floor(sailPos));
    }

    return out;
}


/* a single step of the tack state
* x represents time as an integer for the rudder movement
* windRelative is the current relative wind angle as read from the sensor
* initialWindRelative is the relative wind angle captured at the beginning of the tack state
* desiredWindRaltive is the relative wind angle we will stop tacking at, usually the negative of initialWindRelative
*/
motorstate_t Autonomy::tack(int x, int windRelative, int initialWindRelative, int desiredWindRelative){
    motorstate_t out;

    uint8_t aggression = 5; //this may be learned or controlled from outside later

    if(x > 10){
        _sailState = MOVING_CHECK;
        _initialWindRelative = 9999;
        _desiredWindRelative = 9999;
        _tackTime = -10;
    }

    if(initialWindRelative > 0){
        float sig = sqrt(0.05f);
        float mu = 0.0f;

        //Model our rudder movement on a gaussian bell curve
        float gauss = floor((1.0f/sig)*exp(-pow(x - mu, 2) / 2.0f * pow(sig, 2.0f)) * 2.0f);

        out.rudder = static_cast<int>(gauss)*aggression;
        if(out.rudder > 30)
            out.rudder = 30;

        if(desiredWindRelative < initialWindRelative){
            if(windRelative <= desiredWindRelative){
                _sailState = MOVING_CHECK;
                _initialWindRelative = 9999;
                _desiredWindRelative = 9999;
                _tackTime = -10;
            }
        }
        else if(desiredWindRelative > initialWindRelative){
            if(windRelative >= desiredWindRelative){
                _sailState = MOVING_CHECK;
                _initialWindRelative = 9999;
                _desiredWindRelative = 9999;
                _tackTime = -10;
            }
        }
    }
    else if(initialWindRelative < 0){
        float sig = sqrt(0.05f);
        float mu = 0.0f;

        float gauss = -floor((1.0f/sig)*exp(-pow(x - mu, 2) / 2.0f * pow(sig, 2.0f)) * 2.0f);

        out.rudder = static_cast<int>(gauss)*aggression;
        if(out.rudder < -30)
            out.rudder = -30;

        if(desiredWindRelative < initialWindRelative){
            if(windRelative <= desiredWindRelative){
                _sailState = MOVING_CHECK;
                _initialWindRelative = 9999;
                _desiredWindRelative = 9999;
                _tackTime = -10;
            }
        }
        else if(desiredWindRelative > initialWindRelative){
            if(windRelative >= desiredWindRelative){
                _sailState = MOVING_CHECK;
                _initialWindRelative = 9999;
                _desiredWindRelative = 9999;
                _tackTime = -10;
            }
        }
    }

    return out;
}

/* Finds the angle between two unit-circle angles
*/
uint32_t Autonomy::angleBetween(uint32_t angle1, uint32_t angle2){
    uint32_t theta = angle1 - angle2;

    if(abs(theta) > 180){
        if(theta < 0)
            theta += 360;
        else
            theta = 360 - theta;
    }
    else{
        theta = abs(theta);
    }

    return theta;
}

/* Given an angle assumed to be in cardinal degrees,
* returns the equivalent angle on the unit circle
*/
uint32_t Autonomy::cardinalToStandard(uint32_t angle){
    return abs(angle - 450) % 360;
}

/* Given polar coordinates (r, a), returns cartesian coordinates (x, y) */
Point<double> Autonomy::polarToCartesian(double r, double a){
    Point<double> p;
    p.x = r*cos(radians(a));
    p.y = r*sin(radians(a));

    return p;
}

uint8_t Autonomy::angleQuadrant(uint32_t angle){
    uint32_t a = angle % 360;

    if((a >= 0) && (a < 90))
        return 1;
    if((a >= 90) && (a < 180))
        return 2;
    if((a >= 180) && (a < 270))
        return 3;
    if((a >= 270) && (a < 360))
        return 4;
    else
        return -1;
}

/* returns true if a point p is above a line l in cartesian space */
bool Autonomy::pointAboveLine(Point<double> p, Line l){
    double m = (l.b.y - l.a.y) / (l.b.x - l.a.x);
    double b = l.b.y - m * l.b.x;

    if(p.y > (m * p.x + b))
        return true;
    else
        return false;
}

/* returns true if a point p is below a line l in cartesian space */
bool Autonomy::pointBelowLine(Point<double> p, Line l){
    double m = (l.b.y - l.a.y) / (l.b.x - l.a.x);
    double b = l.b.y - m * l.b.x;

    if(p.y < (m* p.x + b))
        return true;
    else
        return false;
}

/* Adds two angles a, b mod 360 */
uint32_t Autonomy::addAngle(uint32_t a, uint32_t b){
    return (a + b) % 360;
}

/* Subtracts two angles a, b mod 360 */
uint32_t Autonomy::subtractAngle(uint32_t a, uint32_t b){
    if((a - b) < 0)
        return (a - b) + 360;
    else
        return (a - b);
}

/* Finds the distance between two points */
double Autonomy::distanceBetweenPoints(Point<double> p1, Point<double> p2){
    return sqrt(pow(p2.x - p1.x, 2) + (pow(p2.y - p1.y, 2)));
}

/* Generates control lines for upwind sailing */
std::pair<Line, Line> Autonomy::generateControlLines(Point<double> initPos, Point<double> destPos, int offset){
    Line l1, l2;

    l1.a.x = initPos.x - offset; l1.a.y = initPos.y;
    l1.b.x = destPos.x - offset; l1.b.y = destPos.y;

    l2.a.x = initPos.x + offset; l2.a.y = initPos.y;
    l2.b.x = destPos.x + offset; l2.b.y = destPos.y;

    double m1 = (l1.b.y - l1.a.y) / (l1.b.x - l1.a.x);
    double m2 = (l2.b.y - l2.a.y) / (l2.b.x - l2.a.x);

    if((m1 < 0.5) || (m2 < 0.5)){
        l2.a.x = initPos.x; l2.a.y = initPos.y - offset;
        l2.b.x = destPos.x; l2.b.y = destPos.y - offset;

        l1.a.x = initPos.x; l1.a.y = initPos.y + offset;
        l1.b.x = destPos.x; l1.b.y = destPos.y + offset;
    }

    return std::make_pair(l1, l2);
}

std::pair<Line, Line> Autonomy::generateAngledControlLines(Point<double> initPos, Point<double> destPos, int offset){
    Line l1, l2;

    // GPS point to create 60 deg line to destination
    l1.a.x = initPos.x - offset; l1.a.y = initPos.y;
    l1.b.x = destPos.x; l1.b.y = destPos.y;

    l2.a.x = initPos.x + offset; l2.a.y = initPos.y;
    l2.b.x = destPos.x; l2.b.y = destPos.y;

    // can be reduced to single m value?
    double m1 = (l1.b.y - l1.a.y) / (l1.b.x - l1.a.x);
    double m2 = (l2.b.y - l2.a.y) / (l2.b.x - l2.a.x);

    if((m1 < 0.9) || (m2 < 0.9)){
      if (m2 < 0.9) {
        l2.a.x = initPos.x; l2.a.y = initPos.y - offset;
        // Even out the lines creation
        l2.b.x = destPos.x; l2.b.y = destPos.y;

        l1.a.x = initPos.x; l1.a.y = initPos.y + offset;
        // Bring up the opposing lines offset height
        l1.b.x = destPos.x; l1.b.y = destPos.y;
        // Is 30 feet offset too much for this?
      }

      else if (m1 < 0.9){
        l2.a.x = initPos.x;
        l2.a.y = initPos.y + offset;
        // Even out the lines creation
        l2.b.x = destPos.x;
        l2.b.y = destPos.y;

        l1.a.x = initPos.x;
        l1.a.y = initPos.y - offset;
        l1.b.x = destPos.x;
        l1.b.y = destPos.y;
      }
    }

    return std::make_pair(l1, l2);
}
