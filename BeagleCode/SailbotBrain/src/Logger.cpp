#include "Logger.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <dirent.h>
#include <cstdlib>
#include <sys/stat.h>
#include <filesystem>
using namespace std;
using namespace std::tr2::sys;

void Logger::Timestamp(){
	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	strftime(buffer, sizeof(buffer),"%Y%m%d_%H%M%S",now);
}

void Logger::SetDir(char dir[30]){
	sprintf(logdir, "%s", dir);
}

void Logger::LogInit() {
  std::ofstream fout;
  sprintf(name, "%s/%s.txt", logdir, buffer);
  fout.open(name, std::ios::out | std::ios::app);
  fout << "Initialized...starting main loop!" << std::endl;
  fout.close();
}

void Logger::TrackInit() {
  sprintf(name, "%s/%s.csv", logdir, buffer);
  std::ofstream tout;
  tout.open (name, std::ios::out | std::ios::app);
  tout << "CourseToPoint,DistanceToPoint,SailState,BoatSpeed,Lat,Lon,
  GPSHeading,WindDirection,MagHeading" << std::endl;
  tout.close();
}

void Logger::LogStep(std::vector<Waypoint> _waypoints,
  SAIL_STATE _sailState, double wpCourse, double wpDist)) {
		std::ofstream fout;
	  sprintf(name, "%s/%s.txt", logdir, buffer);
	  fout.open(name, std::ios::out | std::ios::app);

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


void Logger::TrackStep(std::vector<Waypoint> _waypoints,
  SAIL_STATE _sailState, double wpCourse, double wpDist) {
	sprintf(name, "%s/%s.csv", logdir, buffer);
  std::ofstream tout;
  tout.open (name, std::ios::out | std::ios::app);
  tout << wpCourse << "," << wpDist << "," << _sailState << "," << state.speed
    << "," << std::setiosflags(std::ios::fixed) << std::setprecision(6)
    << state.latitude << "," << std::setiosflags(std::ios::fixed) << std::setprecision(6)
    << state.longitude << "," << state.gpsHeading << "," << state.windDirection
    << "," << state.magHeading << std::endl;

  tout.close();
}

// Delete all files older than n days in log folder
void Logger::CheckFiles(uint8_t n) {
	char* buffer;
	DIR *dir = opendir(logdir);
	if(!dir) {
		asprintf(&buffer,"exec mkdir -p %s", logdir);
		system(buffer);
	}
	struct stat t_stat;
	struct dirent *next_file;
	char filepath[256];
	time_t now = time(&now);
	while ( (next_file = readdir(dir)) != NULL )
	{
		stat(filepath, &t_stat);
		sprintf(filepath, "%s/%s", logdir, next_file->d_name);
		if ((now-t_stat.st_mtime) > (n * 86400)) remove(filepath);
	}
	closedir(dir);
	delete buffer;
}
