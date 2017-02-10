#include "Logger.h"
#include <stdio.h>
#include <dirent.h>
#include <cstdlib>

std::string Logger::Timestamp(){
	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	strftime(buffer, sizeof(buffer),"%Y%m%d_%H%M%S.",now);
	std::string datetime = std::string(buffer);
  return datetime;
}

void Logger::LogInit(std::string timestamp) {
  std::ofstream fout;
  name = "/" + timestamp + ".txt";
  fout.open(name, std::ios::out | std::ios::app);
  fout << "Initialized...starting main loop!" << std::endl;
  fout.close();
}

void Logger::TrackInit(std::string timestamp) {
  std::string name = "/" + timestamp + ".csv";
  std::ofstream tout;
  tout.open (name, std::ios::out | std::ios::app);
  tout << "CourseToPoint,DistanceToPoint,SailState,BoatSpeed,Lat,Lon,
  GPSHeading,WindDirection,MagHeading" << std::endl;
  tout.close();
}

void Logger::LogStep(std::string timestamp, std::vector<Waypoint> _waypoints,
  SAIL_STATE _sailState, double wpCourse, double wpDist)) {
  std::ofstream fout;
  name = "/" + timestamp + ".txt";
  fout.open(filename, std::ios::out | std::ios::app);

  fout << "Waypoint: " << _waypoints[_wpId].lat << ", " << _waypoints[_wpId].lon << std::endl;
  fout << "Course To Point: " << wpCourse << std::endl;
  fout << "Distance To Point: " << wpDist << std::endl;
  fout << "Sail State: " << _sailState << std::endl;
  fout << "Speed: " << state.speed << std::endl;
  fout << "Lat: " << std::setiosflags(std::ios::fixed) << std::setprecision(6)
    << state.latitude << std::endl;
  fout << "Lon: " << std::setiosflags(std::ios::fixed) << std::setprecision(6)
    << state.longitude << std::endl;
  fout << "Course: " << state.gpsHeading << std::endl;
  fout << "Wind: " << state.windDirection << std::endl;
  fout << "Mag Heading: " << state.magHeading << std::endl;

  fout.close();
}

void Logger::TrackStep(std::string timestamp, std::vector<Waypoint> _waypoints,
  SAIL_STATE _sailState, double wpCourse, double wpDist) {
  std::string name = "/" + timestamp + ".txt";
  std::ofstream tout;
  tout.open (name, std::ios::out | std::ios::app);
  tout << wpCourse << "," << wpDist << "," << _sailState << "," << state.speed
    << "," << std::setiosflags(std::ios::fixed) << std::setprecision(6)
    << state.latitude << "," << std::setiosflags(std::ios::fixed) << std::setprecision(6)
    << state.longitude << "," << state.gpsHeading << "," << state.windDirection
    << "," << state.magHeading << std::endl;

  tout.close();
}

void Logger::TrackStep(std::string timestamp, std::vector<Waypoint> _waypoints,
  SAIL_STATE _sailState, double wpCourse, double wpDist) {
  std::string name = "/" + timestamp + ".txt";
  std::ofstream tout;
  tout.open (name, std::ios::out | std::ios::app);
  tout << wpCourse << "," << wpDist << "," << _sailState << "," << state.speed
    << "," << std::setiosflags(std::ios::fixed) << std::setprecision(6)
    << state.latitude << "," << std::setiosflags(std::ios::fixed) << std::setprecision(6)
    << state.longitude << "," << state.gpsHeading << "," << state.windDirection
    << "," << state.magHeading << std::endl;

  tout.close();
}

void Logger::CheckFiles() {
  DIR *theFolder = opendir("/Users/bryan/log");
    struct dirent *next_file;
    if(!theFolder) { system("exec mkdir -p /Users/bryan/log/"); }
    char filepath[256];
    int i = 0;
    while ( (next_file = readdir(theFolder)) != NULL )
    {
        // build the path for each file in the folder
        sprintf(filepath, "%s/%s", "/Users/bryan/log", next_file->d_name);
        std::cout << filepath << std::endl;
        i++;
        if (i > 10){
        	remove(filepath);
        }
    }
    closedir(theFolder);
}
