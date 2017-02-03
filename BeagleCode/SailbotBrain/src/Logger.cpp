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
  filename = "/Log-" + timestamp + ".csv";
  std::ofstream fout;
  fout.open(filename, std::ios::out | std::ios::app);
  // TODO: pass state/waypoints into function
  // TODO: should state contain wpCourse and wpDist?

  /*fout << "Waypoint: " << _waypoints[_wpId].lat << ", " << _waypoints[_wpId].lon << std::endl;
  fout << "Course To Point: " << wpCourse << std::endl;
  fout << "Distance To Point: " << wpDist << std::endl;
  fout << "Sail State: " << _sailState << std::endl;
  fout << "Speed: " << state.speed << std::endl;
  fout << "Lat: " << state.latitude << std::endl;
  fout << "Lon: " << state.longitude << std::endl;
  fout << "Course: " << state.gpsHeading << std::endl;
  fout << "Wind: " << state.windDirection << std::endl;
  fout << "Mag Heading: " << state.magHeading << std::endl;
  */
  
  fout.close();
}

void Logger::TrackStep(std::string timestamp) {
  std::string name = "/Track-" + timestamp + ".csv";
  std::ofstream tout;
  tout.open (name, std::ios::out | std::ios::app);
  tout << "CourseToPoint,DistanceToPoint,SailState,BoatSpeed,Lat,Lon,GPSHeading,WindDirection,MagHeading" << std::endl;
  tout.close();
}
