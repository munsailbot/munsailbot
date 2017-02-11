#ifndef __LOGGER_H
#define __LOGGER_H

#include <string>
#include <vector>

class Logger{
public:
  void Timestamp();
  void SetDir(char dir[30]);
  char name[256];
  char buffer [30];
  char logdir [30];
  void LogInit();
  void TrackInit();
  void LogStep(std::vector<Waypoint> _waypoints,
    SAIL_STATE _sailState, double wpCourse, double wpDist);
  void TrackStep(std::vector<Waypoint> _waypoints,
    SAIL_STATE _sailState, double wpCourse, double wpDist);
  void CheckFiles(uint8_t n);
  // GetFolderSize(char logdir[],unsigned long long & f_size);
};

#endif // __LOGGER_H
