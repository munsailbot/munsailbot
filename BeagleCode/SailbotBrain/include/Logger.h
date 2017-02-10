#ifndef __LOGGER_H
#define __LOGGER_H

#include <string>
#include <ctime>
#include <vector>

class Logger{
public:
  std::string Timestamp();
  char buffer [30];
  LogInit(std::string timestamp);
  TrackInit(std::string timestamp);
  LogStep(std::string timestamp, vector<Waypoint> _waypoints, SAIL_STATE _sailState, double wpCourse, double wpDist);
  TrackStep(std::string timestamp, vector<Waypoint> _waypoints, SAIL_STATE _sailState, double wpCourse, double wpDist);
  CheckFiles();
};

#endif // __LOGGER_H
