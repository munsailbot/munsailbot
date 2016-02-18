#include "TinyGPSPlus/TinyGPS++.h"
#include "Autonomy.h"
#include "Utility.h"

#include <curses.h>

#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>


// TODO: ROUND BUOY ON THE LEFT FOR NAV TEST EVENT (BOAT'S RIGHT)


Autonomy::Autonomy(Timer* timer){
    _sailState = MOVING_CHECK;

    _wpId = 0;
    //_waypoints = new Waypoint[13];                                            // Waypoint pointers?

    std::ofstream fout;
    fout.open("/log.txt", std::ios::out | std::ios::app);
    fout << "reading autonomy settings" << std::endl;

    std::ifstream fin("/root/waypoints.txt", std::ios::in);

    std::string mode;
    std::getline(fin, mode);
    fout << "Mode: " << mode << std::endl;

    if(mode == "ld"){                                                           // LONG DISTANCE
        this->setMode(LONG_DISTANCE);
    }
    else if(mode == "sk"){                                                      // STATION KEEPING
        this->setMode(STATION_KEEPING_STRAT1);
    }
    else{
        fout << "Invalid autonomy mode" << std::endl;
    }


    for(std::string line; std::getline(fin, line); ){
        std::istringstream in(line);                                            // make a stream for the line itself

        double lat,lon;

        in >> lat >> lon;

        Waypoint point;
        point.lat = lat;
        point.lon = lon;
        fout << std::setiosflags(std::ios::fixed) << std::setprecision(11) << point.lat << " " << point.lon << std::endl;

        _waypoints.push_back(point);
    }

    fin.close();

    _numWaypoints = _waypoints.size();
    fout << "Num waypoints: " << _numWaypoints << std::endl;
    fout.close();

    _lastMain = 0;
    //_lastJib = 0;    								// No Jib currently
    _lastRud = 35;
    _recoveryTime = 0;
    _initialWindRelative = 9999;
    _desiredWindRelative = 9999;
    _startedTack = false;
    _startedRecovery = false;
    _initialCoordsCaptured = false;
    _recentTack = false;                                                         // Has the boat tacked recently?

    _downwindCount = 0;
    _upwindCount = 0;
    _startedUpwind = false;

    _offset = 150;                                                              // How much is 150 in feet? 30
    _tackEvent = 0;

    _timer = timer;
    _skTimerSet = false;
}

Autonomy::~Autonomy(){
    //delete [] _waypoints;
}

void Autonomy::setMode(MODE m){
    _mode = m;
    if(_mode == STATION_KEEPING_STRAT1) _sailState = MOVE_TO_POINT;
    }

// Executes a step. The frequency of these steps is determined externally.
void Autonomy::step(state_t state, TinyGPSPlus* tinyGps, BeagleUtil::UARTInterface* serial){
    uint8_t main;
    //uint8_t jib;
    uint8_t rud;
    main = _lastMain;
    //jib = _lastJib;
    rud = _lastRud;

    std::ofstream fout;
    fout.open("/log.txt", std::ios::out | std::ios::app);

    double wpCourse = tinyGps->courseTo(state.latitude, state.longitude, _waypoints[_wpId].lat, _waypoints[_wpId].lon);
    double wpDist = tinyGps->distanceBetween(state.latitude, state.longitude, _waypoints[_wpId].lat, _waypoints[_wpId].lon);

    //Log some 'tings
    mvprintw(8, 2, "Current Waypoint: %f, %f\n", _waypoints[_wpId].lat, _waypoints[_wpId].lon);
    fout << "Waypoint: " << _waypoints[_wpId].lat << ", " << _waypoints[_wpId].lon << std::endl;

    mvprintw(11, 4, "Course to Point: %f (deg)\n", wpCourse);
    mvprintw(12, 4, "Distance to Point: %f (m)\n", wpDist);
    fout << "Course To Point: " << wpCourse << std::endl;
    fout << "Distance To Point: " << wpDist << std::endl;

    fout << "Sail State: " << _sailState << std::endl;

    fout << "Speed: " << state.speed << std::endl;
    fout << "Lat: " << state.latitude << std::endl;
    fout << "Lon: " << state.longitude << std::endl;
    fout << "Course: " << state.gpsHeading << std::endl;
    fout << "Wind: " << state.windDirection << std::endl;
    fout << "Mag Heading: " << state.magHeading << std::endl;

    if(_mode == LONG_DISTANCE)  fout << "Mode: Long Distance" << std::endl; //mvprintw(1, 40, "MODE: Long Distance\n");
    if(_mode == STATION_KEEPING_STRAT1) fout << "Mode: Station Keeping (Strategy 1)" << std::endl; //mvprintw(1, 4, "MODE: Station Keeping (Strat 1)");

    if(_initialCoordsCaptured == false){
        if(state.latitude != 99.99 && state.longitude != 99.99){
            _initialLatLon.x = state.latitude;
            _initialLatLon.y = state.longitude;
            _initialCoordsCaptured = true;

            fout << "Initial Lat: " << _initialLatLon.x << std::endl;
            fout << "Initial Lon: "  << _initialLatLon.y << std::endl;
        }
    }
                                                                                // TODO: ADD ANGLEDTACKLINES TO UPWIND CASE
    if(_mode == LONG_DISTANCE){
        motorstate_t r;
        switch(_sailState){
            case MOVING_CHECK:
                mvprintw(10, 2, "STATE: MOVING CHECK\n");
                if(wpDist <= 5){                                                // Within range of waypoint
                    _sailState = REACHED_POINT;
                }
                else{
                    if(state.speed >= 0.5){                                     // Does it have momentum
                        _sailState = MOVE_TO_POINT;
                    }
                    else{                                                       // Sailing against the wind?
                        if(state.windDirection > -15 && state.windDirection < 15){
                            break;
                        }
                        else{
                            if(state.windDirection < 0)                         // Adjust sails for wind direction
                                r = courseByWind(state.windDirection, -90);
                            else
                                r = courseByWind(state.windDirection, 90);

                            rud = (r.rudder + 35);                              // Rudder + 35??
                            main = r.main;
                            // jib = r.jib;
                        }
                    }
                }
                break;

            case MOVE_TO_POINT:
                mvprintw(10, 2, "STATE: MOVE TO POINT\n");
                if((state.latitude != 0.0) && (state.longitude != 0.0) && (state.latitude != 99.99) && (state.longitude != 99.99)){

                    if(wpDist <= 5){                                            // Is it within distance of waypoint?
                        _sailState = REACHED_POINT;
                    }
                    else{                                                       // TODO: If not, find absolute wind and set sail mode
                        uint32_t windAbs = (state.windDirection + static_cast<uint32_t>(floor(state.gpsHeading))) % 360;
                        fout << "Absolute Wind: " << windAbs << std::endl;
                        if(angleBetween(cardinalToStandard(windAbs), cardinalToStandard(wpCourse)) <= 75){
                            _sailState = UPWIND;                                // Head on is 0 degrees, P = -75, S = +75
                        }
                        else{
                            _sailState = DOWNWIND;                              // If greater than 75 degrees, wind is from rear
                        }
                    }
                }

                break;

            case DOWNWIND:
                mvprintw(10, 2, "STATE: DOWNWIND\n");
                _downwindCount++;

                mvprintw(15, 1, "Upwind Count: %d", _upwindCount);
                mvprintw(16, 1, "Downwind Count: %d", _downwindCount);

                //TODO: we need to check if we need to tack through the wind to face our point

                //TODO: course and heading are raw values, should compute track and true heading

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
                        jib = r.jib;

                        _sailState = MOVING_CHECK;
                    }
                    else{
                        _sailState = MOVING_CHECK;
                    }
                }

                break;

            case UPWIND:
                mvprintw(10, 2, "STATE: UPWIND\n");
                _upwindCount++;

                mvprintw(15, 1, "Upwind Count: %d", _upwindCount);
                mvprintw(16, 1, "Downwind Count: %d", _downwindCount);
                                                                                // TODO: IS THIS SECTION CORRECT?
                if(wpDist <= 150){                                               // Create angled tack lines if within 30 feet
                  if(wpDist <= 5){
                      _sailState = REACHED_POINT;
                  }
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
                          mvprintw(17, 1, "Initial XY: %d, %d", _initialCart.x, _initialCart.y);

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
                          mvprintw(17, 1, "Boat XY: %f, %f", _boatCart.x, _boatCart.y);
                          mvprintw(20, 1, "Boat r, a: %f, %f", _boatPolar.x, _boatPolar.y);
                          fout << "Boat XY: " << _boatCart.x << " , " << _boatCart.y << std::endl;

                          std::pair<Line, Line> lines = generateControlLines(_initialCart, _waypointCart, _offset);
                          std::pair<Line, Line> angledLines = generateAngledControlLines(_initialCart, _waypointCart, _offset);

                          mvprintw(18, 1, "Line 1: %f, %f, %f, %f", lines.first.a.x, lines.first.a.y, lines.first.b.x, lines.first.b.y);
                          mvprintw(19, 1, "Line 2: %f, %f, %f, %f", lines.second.a.x, lines.second.a.y, lines.second.b.x, lines.second.b.y);
                          fout << "Line1: " << lines.first.a.x << ","  << lines.first.a.y << "," << lines.first.b.x << "," << lines.first.b.y << std::endl;
                          fout << "Line2: " << lines.second.a.x << ","  << lines.second.a.y << "," << lines.second.b.x << "," << lines.second.b.y << std::endl;
                          fout << "Line3: " << angledLines.first.a.x << ","  <<  angledLines.first.a.y << "," <<  angledLines.first.b.x << "," << angledLines.first.b.y << std::endl;
                          fout << "Line4: " << angledLines.second.a.x << ","  << angledLines.second.a.y << "," << angledLines.second.b.x << "," << angledLines.second.b.y << std::endl;
                          uint32_t windAbs = (state.windDirection + static_cast<uint32_t>(floor(state.gpsHeading))) % 360;

                          if((pointBelowLine(_boatCart, angledLines.first) && pointBelowLine(_boatCart, angledLines.second)) ||
                          (pointAboveLine(_boatCart, angledLines.first) && pointAboveLine(_boatCart, angledLines.second)) ||
                          ((pointBelowLine(_boatCart, angledLines.first) && pointBelowLine(_boatCart, angledLines.second) &&
                          pointBelowLine(_boatCart, lines.first) && pointBelowLine(_boatCart, lines.second))) ||
                          ((pointBelowLine(_boatCart, angledLines.first) && pointBelowLine(_boatCart, angledLines.second) &&
                          pointAboveLine(_boatCart, lines.first) && pointAboveLine(_boatCart, lines.second)) ||
                          (pointAboveLine(_boatCart, angledLines.first) && pointAboveLine(_boatCart, angledLines.second) &&
                          pointBelowLine(_boatCart, lines.first) && pointBelowLine(_boatCart, lines.second)) )){
                              _sailState = TACK;

                              if(_tackEvent == 0){
                                  //_offset -= 3;
                                  _tackEvent = 1;
                              }
                          }

                          if(pointBelowLine(_boatCart, lines.second) && pointAboveLine(_boatCart, lines.first)){
                              if(pointBelowLine(_boatCart, angledLines.second) && pointAboveLine(_boatCart, angledLines.first)){
                                _tackEvent = 0;
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
                                  jib = r.jib;
                                  rud = static_cast<uint8_t>(r.rudder + 35);
                                  _sailState = MOVING_CHECK;
                              }
                          }

                      }
                     }
                  else{
                      _sailState = MOVING_CHECK;
                  }

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
                            mvprintw(17, 1, "Initial XY: %d, %d", _initialCart.x, _initialCart.y);

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
                            mvprintw(17, 1, "Boat XY: %f, %f", _boatCart.x, _boatCart.y);
                            mvprintw(20, 1, "Boat r, a: %f, %f", _boatPolar.x, _boatPolar.y);
                            fout << "Boat XY: " << _boatCart.x << " , " << _boatCart.y << std::endl;

                            std::pair<Line, Line> lines = generateControlLines(_initialCart, _waypointCart, _offset);
                            std::pair<Line, Line> angledLines = generateAngledControlLines(_initialCart, _waypointCart, _offset);

                            mvprintw(18, 1, "Line 1: %f, %f, %f, %f", lines.first.a.x, lines.first.a.y, lines.first.b.x, lines.first.b.y);
                            mvprintw(19, 1, "Line 2: %f, %f, %f, %f", lines.second.a.x, lines.second.a.y, lines.second.b.x, lines.second.b.y);
                            fout << "Line1: " << lines.first.a.x << ","  << lines.first.a.y << "," << lines.first.b.x << "," << lines.first.b.y << std::endl;
                            fout << "Line2: " << lines.second.a.x << ","  << lines.second.a.y << "," << lines.second.b.x << "," << lines.second.b.y << std::endl;
                            fout << "Line3: " << angledLines.first.a.x << ","  <<  angledLines.first.a.y << "," <<  angledLines.first.b.x << "," << angledLines.first.b.y << std::endl;
                            fout << "Line4: " << angledLines.second.a.x << ","  << angledLines.second.a.y << "," << angledLines.second.b.x << "," << angledLines.second.b.y << std::endl;
                            uint32_t windAbs = (state.windDirection + static_cast<uint32_t>(floor(state.gpsHeading))) % 360;

                            if((pointBelowLine(_boatCart, angledLines.first) && pointBelowLine(_boatCart, angledLines.second)) ||
                            (pointAboveLine(_boatCart, angledLines.first) && pointAboveLine(_boatCart, angledLines.second)) ||
                            ((pointBelowLine(_boatCart, angledLines.first) && pointBelowLine(_boatCart, angledLines.second) &&
                            pointBelowLine(_boatCart, lines.first) && pointBelowLine(_boatCart, lines.second))) ||
                            ((pointBelowLine(_boatCart, angledLines.first) && pointBelowLine(_boatCart, angledLines.second) &&
                            pointAboveLine(_boatCart, lines.first) && pointAboveLine(_boatCart, lines.second)) ||
                            (pointAboveLine(_boatCart, angledLines.first) && pointAboveLine(_boatCart, angledLines.second) &&
                            pointBelowLine(_boatCart, lines.first) && pointBelowLine(_boatCart, lines.second)) )){
                                _sailState = TACK;

                                if(_tackEvent == 0){
                                    //_offset -= 3;
                                    _tackEvent = 1;
                                }
                            }
                                                                                //CHECKME: THIS IS POINTLESS
                            if((pointBelowLine(_boatCart, lines.second) && pointAboveLine(_boatCart, lines.first))
                            || (pointAboveLine(_boatCart, lines.second) && pointBelowLine(_boatCart, lines.first))){
                                _tackEvent = 0;
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
                                    // jib = r.jib;
                                    rud = static_cast<uint8_t>(r.rudder + 35);
                                    _sailState = MOVING_CHECK;
                                }
                            }
                        }
                    }
                    else{
                        _sailState = MOVING_CHECK;
                    }
                }
                break;

            case TACK:
                mvprintw(10, 2, "STATE: TACK\n");

                r = tack(_tackTime, state.windDirection, _initialWindRelative, _desiredWindRelative);
                rud = r.rudder + 35; //Calculations done (-35,35), must be sent (0,70)
                main = 0;
                // jib = 0;

                if(wpDist <= 5){
                    _sailState = REACHED_POINT;
                }

                else if (_recentTack == false){                                 // TODO: RETURN RECE?NTTACK TO FALSE SOMEWHERE
                    if(_startedTack == false){
                        //_tackTime = 0;
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
                      int initialWindTack = state.windDirection;
                      if (initialWindTack > 0){
                          if(initialWindTack >= (initialWindTack * (-1))){
                              if(_startedRecovery == false){
                                  _recoveryTime = 0;
                                  if(state.windDirection > 0)
                                      r = courseByWind(state.windDirection, 65);
                                  else
                                      r = courseByWind(state.windDirection, -65);
                                  rud = r.rudder + 35;
                                  main = r.main;
                                  // jib = r.jib;

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
                                      // jib = r.jib;

                                      _recoveryTime++;
                                  }
                              }
                          }
                          else{
                              _recentTack == true;
                          }
                      else if (initialWindTack < 0){
                        if(initialWindTack <= (initialWindTack * (-1))){
                            if(_startedRecovery == false){
                                _recoveryTime = 0;
                                if(state.windDirection > 0)
                                    r = courseByWind(state.windDirection, 65);
                                else
                                    r = courseByWind(state.windDirection, -65);
                                rud = r.rudder + 35;
                                main = r.main;
                                // jib = r.jib;

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
                                    // jib = r.jib;

                                    _recoveryTime++;
                                }
                            }
                        }
                        else {
                          _recentTack == true;
                        }
                      }
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
                    mvprintw(10, 2, "STATE: MOVE TO POINT\n");
                    if((state.latitude != 0.0) && (state.longitude != 0.0) && (state.latitude != 99.99) && (state.longitude != 99.99)){
                        if(wpDist <= 1){
                            _sailState = REACHED_POINT;
                        }
                        else{
                            uint32_t windAbs = (state.windDirection + static_cast<uint32_t>(floor(state.gpsHeading))) % 360;		// TODO: PROPER VECTOR = - BOAT WIND (DIR) + APP WIND
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
                    mvprintw(10, 2, "STATE: DOWNWIND\n");
                    _downwindCount++;

                    mvprintw(15, 1, "Upwind Count: %d", _upwindCount);
                    mvprintw(16, 1, "Downwind Count: %d", _downwindCount);

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
                            // jib = r.jib;

                            _sailState = MOVE_TO_POINT;
                        }
                        else{
                            _sailState = MOVE_TO_POINT;
                        }
                    }

                    break;

                case UPWIND:
                  mvprintw(10, 2, "STATE: UPWIND\n");
                  _upwindCount++;

                  mvprintw(15, 1, "Upwind Count: %d", _upwindCount);
                  mvprintw(16, 1, "Downwind Count: %d", _downwindCount);

                  if(wpDist <= 1){
                      _sailState = REACHED_POINT;
                  }
                  else{                                                         // TODO: ADD STEEPER LINES HERE AS WELL
                      if(_upwindCount >= 3){ // How long has wind been blowing
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
                              mvprintw(17, 1, "Initial XY: %d, %d", _initialCart.x, _initialCart.y);

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
                              mvprintw(17, 1, "Boat XY: %f, %f", _boatCart.x, _boatCart.y);
                              mvprintw(20, 1, "Boat r, a: %f, %f", _boatPolar.x, _boatPolar.y);
                              fout << "Boat XY: " << _boatCart.x << " , " << _boatCart.y << std::endl;

                              std::pair<Line, Line> lines = generateControlLines(_initialCart, _waypointCart, _offset);
                              std::pair<Line, Line> angledLines = generateAngledControlLines(_initialCart, _waypointCart, _offset);

                              mvprintw(18, 1, "Line 1: %f, %f, %f, %f", lines.first.a.x, lines.first.a.y, lines.first.b.x, lines.first.b.y);
                              mvprintw(19, 1, "Line 2: %f, %f, %f, %f", lines.second.a.x, lines.second.a.y, lines.second.b.x, lines.second.b.y);
                              fout << "Line1: " << lines.first.a.x << ","  << lines.first.a.y << "," << lines.first.b.x << "," << lines.first.b.y << std::endl;
                              fout << "Line2: " << lines.second.a.x << ","  << lines.second.a.y << "," << lines.second.b.x << "," << lines.second.b.y << std::endl;
                              fout << "Line3: " << angledLines.first.a.x << ","  <<  angledLines.first.a.y << "," <<  angledLines.first.b.x << "," << angledLines.first.b.y << std::endl;
                              fout << "Line4: " << angledLines.second.a.x << ","  << angledLines.second.a.y << "," << angledLines.second.b.x << "," << angledLines.second.b.y << std::endl;
                              uint32_t windAbs = (state.windDirection + static_cast<uint32_t>(floor(state.gpsHeading))) % 360;

                              if((pointBelowLine(_boatCart, angledLines.first) && pointBelowLine(_boatCart, angledLines.second)) ||
                              (pointAboveLine(_boatCart, angledLines.first) && pointAboveLine(_boatCart, angledLines.second)) ||
                              ((pointBelowLine(_boatCart, angledLines.first) && pointBelowLine(_boatCart, angledLines.second) &&
                              pointBelowLine(_boatCart, lines.first) && pointBelowLine(_boatCart, lines.second))) ||
                              ((pointBelowLine(_boatCart, angledLines.first) && pointBelowLine(_boatCart, angledLines.second) &&
                              pointAboveLine(_boatCart, lines.first) && pointAboveLine(_boatCart, lines.second)) ||
                              (pointAboveLine(_boatCart, angledLines.first) && pointAboveLine(_boatCart, angledLines.second) &&
                              pointBelowLine(_boatCart, lines.first) && pointBelowLine(_boatCart, lines.second)) )){
                                  _sailState = TACK;

                                  if(_tackEvent == 0){
                                      //_offset -= 3;
                                      _tackEvent = 1;
                                  }
                              }

                              if((pointBelowLine(_boatCart, lines.second) && pointAboveLine(_boatCart, lines.first)) ||
                              (pointAboveLine(_boatCart, lines.second) && pointBelowLine(_boatCart, lines.first))){
                                  _tackEvent = 0;
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
                                      // jib = r.jib;
                                      rud = static_cast<uint8_t>(r.rudder + 35);
                                      _sailState = MOVE_TO_POINT;
                                  }
                              }

                          }
                      }
                      else{
                          _sailState = MOVE_TO_POINT;
                      }
                  }
                  break;

                case TACK:
                    mvprintw(10, 2, "STATE: TACK\n");

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
                            if(_tackTime >= 6){                                 // TODO: WINDCHEK CONDITION HERE INSTEAD
                                if(_startedRecovery == false){
                                    _recoveryTime = 0;
                                    if(state.windDirection > 0)
                                        r = courseByWind(state.windDirection, 75);
                                    else
                                        r = courseByWind(state.windDirection, -75);
                                    rud = r.rudder + 35;
                                    main = r.main;
                                    // jib = r.jib;

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
                                        // jib = r.jib;

                                        _recoveryTime++;
                                    }
                                }
                            }
                            else{
                                _tackTime++;                                    // TODO: WINDCHECK HERE
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

            //motorstate_t r;

            if(state.windDirection >= 0)
                r = courseByWind(state.windDirection, 90);
            else
                r = courseByWind(state.windDirection, -90);

            rud = r.rudder + 35;
            main = r.main;
            // jib = r.jib;
        }
    }

    _lastMain = main;
    // _lastJib = jib;
    _lastRud = rud;

    fout << "Sail: " << std::dec << static_cast<int>(_lastMain) << std::endl;
    fout << "Rudder: " << std::dec << static_cast<int>(_lastRud) - 35 << std::endl;
    fout << "---------------" << std::endl;
    fout.close();

    _lastState = state;
}

uint8_t Autonomy::getMain(){
    return _lastMain;
}

/* uint8_t Autonomy::getJib(){
    return _lastJib;
} 				*/

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

    //int side = -1;
    //if(windRelative < 0)
    //    side = -side;

    int32_t theta = static_cast<uint32_t>(angleToSail - windRelative);
    out.rudder = static_cast<int>(floorf((35.0f/180.0f)*static_cast<float>(theta)));
    out.rudder *= -1;

    if(abs(windRelative) < 45){
        out.main = 0;
        // out.jib = 0;
    }
    else{
        float sailPos = static_cast<float>(abs(windRelative) - 45)*0.66f;
        out.main = static_cast<int>(floor(sailPos));
        // out.jib = static_cast<int>(floor(sailPos));
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
    //out.rudder *= side;

    if(abs(windRelative) < 45){
        out.main = 0;
        // out.jib = 0;
    }
    else{
        float sailPos = static_cast<float>(abs(windRelative) - 45)*0.66f;
        out.main = static_cast<int>(floor(sailPos));
        // out.jib = static_cast<int>(floor(sailPos));
    }

    return out;
}

                                                                                // TODO: SET RECENT TACK CHECK

										// TODO: Set timer point to use for 60 second determination?
/* a single step of the tack state
* x represents time as an integer for the rudder movement
* windRelative is the current relative wind angle as read from the sensor
* initialWindRelative is the relative wind angle captured at the beginning of the tack state
* desiredWindRaltive is the relative wind angle we will stop tacking at, usually the negative of initialWindRelative
*/
motorstate_t Autonomy::tack(int x, int windRelative, int initialWindRelative, int desiredWindRelative, ){
    motorstate_t out;

    mvprintw(10, 20, "TACKING\n");

    uint8_t aggression = 5;                                                     // this may be learned or controlled from outside later

    if(initialWindRelative > 0){                                                // TACK DIRECTION ONE
        float sig = sqrt(0.05f);
        float mu = 0.0f;

        //Model our rudder movement on a gaussian bell curve
        float gauss = floor((1.0f/sig)*exp(-pow(x - mu, 2) / 2.0f * pow(sig, 2.0f)) * 2.0f);

                                                                                //TODO: Replace timer with wind direction for Calculations
                                                                                //TODO: HOLD AT 30 UNTIL WIND IS OPPOSING

        out.rudder = static_cast<int>(gauss)*aggression;
        if(out.rudder > 30)
            out.rudder = 30;

        //uint64_t now = timer->millis();
        //while((timer->millis() - now) < 200);

        if(desiredWindRelative < initialWindRelative){
            if(windRelative <= desiredWindRelative){
                _sailState = MOVING_CHECK;
                _initialWindRelative = 9999;
                _desiredWindRelative = 9999;
                //_tackTime = -10;
            }
        }

        else if(desiredWindRelative > initialWindRelative){
            if(windRelative >= desiredWindRelative){
                _sailState = MOVING_CHECK;
                _initialWindRelative = 9999;
                _desiredWindRelative = 9999;
                // _tackTime = -10;
            }
        }
    }
    else if(initialWindRelative < 0){                                           // TACK DIRECTION TWO
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
                //_tackTime = -10;
            }
        }
        else if(desiredWindRelative > initialWindRelative){
            if(windRelative >= desiredWindRelative){
                _sailState = MOVING_CHECK;
                _initialWindRelative = 9999;
                _desiredWindRelative = 9999;
                // _tackTime = -10;
            }
        }
    }
    _recentTack= true;                                                          // Has tacked recently
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

// Generates control lines for upwind sailing

std::pair<Line, Line> Autonomy::generateControlLines(Point<double> initPos, Point<double> destPos, int offset){
    Line l1, l2;
                                                                                // GPS point to create 60 deg line
    l1.a.x = initPos.x - offset;                                                // OFFSET IS 150 (30 FT) BY DEFAULT
    l1.a.y = initPos.y;
    l1.b.x = destPos.x - offset;
    l1.b.y = destPos.y;

    l2.a.x = initPos.x + offset;
    l2.a.y = initPos.y;
    l2.b.x = destPos.x + offset;
    l2.b.y = destPos.y;

    double m1 = (l1.b.y - l1.a.y) / (l1.b.x - l1.a.x);
    double m2 = (l2.b.y - l2.a.y) / (l2.b.x - l2.a.x);

    if((m1 < 0.5) || (m2 < 0.5)){
        l2.a.x = initPos.x;
        l2.a.y = initPos.y - offset;
        l2.b.x = destPos.x;
        l2.b.y = destPos.y - offset;

        l1.a.x = initPos.x;
        l1.a.y = initPos.y + offset;
        l1.b.x = destPos.x;
        l1.b.y = destPos.y + offset;
    }

    return std::make_pair(l1, l2);
}

/* Generates additional angled lines for upwind sailing if close to target      // TODO: IS THIS ACCURATE?
*/
std::pair<Line, Line> Autonomy::generateAngledControlLines(Point<double> initPos, Point<double> destPos, int offset){
    Line l1, l2;
                                                                                // GPS point to create 60 deg line
    l1.a.x = initPos.x - offset;
    l1.a.y = initPos.y;
    l1.b.x = destPos.x;
    l1.b.y = destPos.y;

    l2.a.x = initPos.x + offset;
    l2.a.y = initPos.y;
    l2.b.x = destPos.x;
    l2.b.y = destPos.y;

    double m1 = (l1.b.y - l1.a.y) / (l1.b.x - l1.a.x);
    double m2 = (l2.b.y - l2.a.y) / (l2.b.x - l2.a.x);

    if((m1 < 0.9) || (m2 < 0.9)){                                               // Are tack lines generated at 60 degrees?
      if (m2 < 0.9) {
        l2.a.x = initPos.x;
        l2.a.y = initPos.y - offset;                                            // Even out the lines creation
        l2.b.x = destPos.x;
        l2.b.y = destPos.y;

        l1.a.x = initPos.x;
        l1.a.y = initPos.y + offset;                                            // Bring up the opposing lines offset height
        l1.b.x = destPos.x;                                                     // Is 30 feet offset too much for this?
        l1.b.y = destPos.y;
      }
      else if (m1 < 0.9){
        l2.a.x = initPos.x;
        l2.a.y = initPos.y + offset;                                            // Even out the lines creation
        l2.b.x = destPos.x;
        l2.b.y = destPos.y;

        l1.a.x = initPos.x;
        l1.a.y = initPos.y - offset;                                            ///
        l1.b.x = destPos.x;
        l1.b.y = destPos.y;
      }
    }

    return std::make_pair(l1, l2);
}
