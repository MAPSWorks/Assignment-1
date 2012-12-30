// File:     DTM.cpp
//
// Author:   Rey Zeng
// SID:      37745114
// UserID:   s6g8
// Lab:      L1E
// Modified: 2012 Nov 6
//
// Contains functions that calculates various statistics required for this assignment and export the stats to a file.

#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <string>
#include "DTM.hpp"

/**
 * Constructor
 */
DTM::DTM()
{
	highest.x = highest.y = lowest.x = lowest.y = centroid.x = centroid.y = 0;
	percentWater = volWater = avgElev = avgASElev = 0;
}

/**
 * Mutator
 */
//Basic
//Coordinates of the highest and the lowest points (lat/long and m)
void DTM::calcPartOne(short earthInfo[][NUMCOL])
{
	for (int i = 0; i < NUMROW; i++) {
		for (int j = 0; j < NUMCOL; j++) {
			if (earthInfo[i][j] > earthInfo[highest.x][highest.y]) {
				highest.x = i;
				highest.y = j;
			}
			if (earthInfo[i][j] < earthInfo[lowest.x][lowest.y]) {
				lowest.x = i;
				lowest.y = j;
			}
		}
	}
}

//Intermediate
//Percentage of water surface (%) and Total volume of water (m3)
void DTM::calcPartTwo(short earthInfo[][NUMCOL])
{
	double longitd = 0, latitd = 0;
	double waterSurfaceArea = 0, landSurfaceArea = 0;
	double area = 0, totalSurfaceArea = 0, toplength = 0, botlength = 0, height = 0;
	for (int i = 0; i < NUMROW; i++) {
		convertFromXYToLL(i, 0, latitd, longitd);
		toplength = calcDistance(latitd, latitd, longitd, longitd + 1/12.0);
		botlength = calcDistance(latitd + 1/12.0, latitd + 1/12.0, longitd, longitd + 1/12.0);
		height = calcDistance(latitd, latitd + 1/12.0, longitd, longitd);
		area = (toplength + botlength) / 2.0 * height;
		for (int j = 0; j < NUMCOL; j++) {
			totalSurfaceArea += area;
			if (earthInfo[i][j] < 0) {
				waterSurfaceArea += area;
				volWater += area * earthInfo[i][j] * -1;
			}
			avgElev += earthInfo[i][j] * area;
			if (earthInfo[i][j] > 0) {
				landSurfaceArea += area;
				avgASElev += earthInfo[i][j] * area;
			}
		}
	}
	percentWater = waterSurfaceArea / totalSurfaceArea * 100;
	avgElev /= totalSurfaceArea;
	avgASElev /= landSurfaceArea;
}

//Advanced
//Coordinates of the centroid of the map (lat/long)
void DTM::calcPartThree(short earthInfo[][NUMCOL])
{
	double longitd = 0, latitd = 0;
	double area = 0, toplength = 0, botlength = 0, height = 0;
	double watermass = 1.0, landmass = watermass * 2.5;
	double lowestElev = (double)earthInfo[lowest.x][lowest.y];
	double mx = 0, my = 0, mass = 0;

	for (int i = 0; i < NUMROW - 1; i++) {
		convertFromXYToLL(i, 0, latitd, longitd);
		toplength = calcDistance(latitd, latitd, longitd, longitd + 1/12.0);
		botlength = calcDistance(latitd + 1/12.0, latitd + 1/12.0, longitd, longitd + 1/12.0);
		height = calcDistance(latitd, latitd + 1/12.0, longitd, longitd);
		area = (toplength + botlength) / 2.0 * height;
		for (int j = 0; j < NUMCOL - 1; j++) {
			// mass of solid
			my += ((float)earthInfo[i][j] - lowestElev) * area * landmass * i;
			mx += ((float)earthInfo[i][j] - lowestElev) * area * landmass * j;
			mass += ((float)earthInfo[i][j] - lowestElev) * area * landmass;
			// mass of water
			if (earthInfo[i][j] <= 0) {				
				my += (float)earthInfo[i][j] * area * watermass * -1 * i;
				mx += (float)earthInfo[i][j] * area * watermass * -1 * j;
				mass += (float)earthInfo[i][j] * area * watermass * -1;
			}
		}
	}
	centroid.x = my/mass;
	centroid.y = mx/mass;
}

//Find all stats
void DTM::calcAll(short earthInfo[][NUMCOL]) {
	calcPartOne(earthInfo);
	calcPartTwo(earthInfo);
	calcPartThree(earthInfo);
}

//Helper
//Convert from i, j in the array to longitude, latitude (degree)
void DTM::convertFromXYToLL(int i, int j, double &latitude, double &longitude)
{
	latitude = 90.0 - ((double)i) / 12.0;
	longitude = -180.0 + ((double)j) / 12.0;
}

//haversine function
double DTM::haversine (double theta)
{
	return sin(theta/2.0) * sin(theta/2.0);
}

//Convert theta from degree to radian
double DTM::toRadian(double theta)
{
	return (theta * PI / 180.0);
}

double DTM::toDegree(double theta)
{
	return (theta * 180.0 / PI);
}

//Find the distance between two points on the surface of the earth specified by longitude and latitude
double DTM::calcDistance(double latitd1, double latitd2, double longitd1, double longitd2)
{
	double temp = haversine(toRadian(latitd2 - latitd1)) + cos(toRadian(latitd1)) * cos(toRadian(latitd2)) * haversine(toRadian(longitd2 - longitd1));
	double d = 2 * RADIUS * asin(sqrt(temp));
	return d;
}

/**
 * File access
 */
void DTM::readFile(string filename, short* buffer, int rows, int columns)
{
	ifstream infile;
	infile.open(filename.c_str(), ios::binary | ios::in | ios::ate);
	if ((int)(infile.tellg()) != 2 * NUMROW * NUMCOL)
	{
		cerr << infile.tellg()  << " is not " << 2 * rows * columns << endl;
		exit(-1);
	}
	infile.seekg(0);
	infile.read((char*) buffer, 2 * rows * columns);
	infile.close();
}

/**
 * Write all stats to "Earthstats.txt".
 */
void DTM::writeToFile(short earthInfo[][NUMCOL])
{
	ofstream fout("Earthstats.txt");
	double lat, lon;
	//fout << "P1:" << endl;
	convertFromXYToLL(highest.x, highest.y, lat, lon);
	//fout << highest.x << " (" << lat << " degree), " << " " << highest.y << " (" << lon << " degree)" << ":" << earthInfo[highest.x][highest.y] << endl;
	fout << lat << " " << lon << " " << earthInfo[highest.x][highest.y] << endl;
	convertFromXYToLL(lowest.x, lowest.y, lat, lon);
	//fout << lowest.x << " (" << lat << " degree), " << " " << lowest.y << " (" << lon << " degree)" << ":" << earthInfo[lowest.x][lowest.y] << endl;
	fout << lat << " " << lon << " " << earthInfo[lowest.x][lowest.y] << endl;

	//fout << "P2:" << endl;
	fout << percentWater << endl;
	fout << volWater << endl;
	fout << avgElev << endl;
	fout << avgASElev << endl;

	//fout << "P3:" << endl;	
	convertFromXYToLL(centroid.x, centroid.y, lat, lon);
	//fout << centroid.x << " (" << lat << " degree), " << centroid.y << " (" << lon << " degree)" << endl;
	fout << lat << " " << lon << endl;

	fout.close();
}

/**
 * Calculate and write all stats to "Earthstats.txt".
 */
void DTM::writeAndCalc(short earthInfo[][NUMCOL])
{
	calcAll(earthInfo);
	writeToFile(earthInfo);
}