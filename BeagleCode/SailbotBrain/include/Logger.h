#ifndef __LOGGER_H
#define __LOGGER_H

#include <string>
#include <ctime>

class Logger{
public:
  std::string Timestamp();
private:
  LogInit(std::string timestamp);
  TrackInit(std::string timestamp);
  LogStep(std::string timestamp);
  TrackStep(std::string timestamp);
}

#endif // __LOGGER_H
