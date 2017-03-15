#ifndef __LOGGER_H
#define __LOGGER_H

#include "Autonomy.h"
#include "SailbotBrain.h"
#include <string>
#include <BeagleUtil.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <dirent.h>
#include <cstdlib>
#include <sys/stat.h>

class Logger{
public:
  char name[256];
  char buffer [30];
  char logdir [30];
  void SetDir(std::string dir);
  size_t timestamp;
  void Timestamp();
  void LogInit();
  void TrackInit();
  void LogStep(std::vector <struct Waypoint> _waypoints, SAIL_STATE _sailState, state_t state, double wpCourse,	double wpDist, uint8_t _wpId);
  void TrackStep(std::vector <struct Waypoint> _waypoints, SAIL_STATE _sailState, state_t state, double wpCourse,	double wpDist, uint8_t _wpId);
  void CheckFiles(uint8_t n,uint8_t mb);
};

#endif // __LOGGER_H
