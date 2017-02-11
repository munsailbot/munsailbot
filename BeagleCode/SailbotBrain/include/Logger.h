#ifndef __LOGGER_H
#define __LOGGER_H

#include <string>
#include <vector>

class Logger{
public:
  char name[256];
  char buffer [30];
  char logdir [30];
  void SetDir(char dir[30]);
  void Timestamp();
  void LogInit();
  void TrackInit();
  void LogStep(std::vector<Waypoint> _waypoints,
    SAIL_STATE _sailState, double wpCourse, double wpDist);
  void TrackStep(std::vector<Waypoint> _waypoints,
    SAIL_STATE _sailState, double wpCourse, double wpDist);
  void CheckFiles(uint8_t n,uint8_t m);
};

#endif // __LOGGER_H
