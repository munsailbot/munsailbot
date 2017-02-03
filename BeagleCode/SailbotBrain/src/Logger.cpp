#include "Logger.h"

void std::string Logger::Timestamp(){
  std::string timestamp;
  time_t t = time(0);   // get time now
  struct tm * now = localtime( & t );
  strftime (buffer,80,"%Y%m%d_%H%M%S.",now);
  std::string timestamp = std::string(datetime);
  return timestamp;
}

void Logger::LogInit(std::string timestamp) {
  std::ofstream fout;
  name = "/Log-" + timestamp + ".csv";
  fout.open(name, std::ios::out | std::ios::app);
  fout << "Initialized...starting main loop!" << std::endl;
  fout.close();
}

void Logger::TrackInit(std::string timestamp) {
  std::string name = "/Track-" + timestamp + ".csv";
  std::ofstream tout;
  tout.open (name, std::ios::out | std::ios::app);
  tout << "CourseToPoint,DistanceToPoint,SailState,BoatSpeed,Lat,Lon,GPSHeading,WindDirection,MagHeading" << std::endl;
  tout.close();
}

void Logger::LogStep(std::string timestamp) {
  std::ofstream fout;
  name = "/Log-" + timestamp + ".csv";
  fout.open(name, std::ios::out | std::ios::app);
  fout << "Initialized...starting main loop!" << std::endl;
  fout.close();
}

void Logger::TrackStep(std::string timestamp) {
  std::string name = "/Track-" + timestamp + ".csv";
  std::ofstream tout;
  tout.open (name, std::ios::out | std::ios::app);
  tout << "CourseToPoint,DistanceToPoint,SailState,BoatSpeed,Lat,Lon,GPSHeading,WindDirection,MagHeading" << std::endl;
  tout.close();
}
