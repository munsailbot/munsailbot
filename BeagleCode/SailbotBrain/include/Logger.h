#ifndef __LOGGER_H
#define __LOGGER_H

#include <string>
#include <ctime>

class Logger{
public:
  std::string Timestamp();
  char buffer [30];
  LogInit(std::string timestamp);
  TrackInit(std::string timestamp);
  LogStep(std::string timestamp, std::vector<Waypoint> _waypoints,
    SAIL_STATE _sailState, double wpCourse, double wpDist);
  TrackStep(std::string timestamp, std::vector<Waypoint> _waypoints,
    SAIL_STATE _sailState, double wpCourse, double wpDist);
}

#endif // __LOGGER_H
