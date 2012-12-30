// File:     DTM.hpp
//
// Author:   Rey Zeng
// SID:      37745114
// UserID:   s6g8
// Lab:      L1E
// Modified: 2012 Nov 6
//
// This file declares the DTM class for the DTM class.

#ifndef _DTM_HPP_
#define _DTM_HPP_

#include <string>

using namespace std;

const double PI = 3.14159265358;
const double RADIUS = 6371000;
const int NUMROW = 2160;
const int NUMCOL = 4320;

class DTM {
	public:
	//Constructor
		DTM();
		
    //Basic
		//Coordinates of the highest and the lowest points (lat/long and m)
		void calcPartOne(short earthInfo[][NUMCOL]);

    //Intermediate
		//Percentage of water surface (%), Total volume of water (m3), Average elevation and Average above-sea elevation (m)(m)
		void calcPartTwo(short earthInfo[][NUMCOL]);

    //Advanced
		//Coordinates of the centroid of the map (lat/long)
		void calcPartThree(short earthInfo[][NUMCOL]);

	//Others
		//Find all stats
		void calcAll(short earthInfo[][NUMCOL]);
		//Read from tbase file
		void readFile(string filename, short* buffer, int rows, int columns);
		//Write statistics to file
		void writeToFile(short earthInfo[][NUMCOL]);
		//Calculate stats and write to file
		void writeAndCalc(short earthInfo[][NUMCOL]);
		//Convert from i, j in the array to longitude, landitude (degree)
		void convertFromXYToLL(int i, int j, double &longitude, double &landitude);
		//haversine function
		double haversine(double theta);
		//Convert theta from degree to radian
		double toRadian(double theta);
		//Convert theta from radian to degree
		double toDegree(double theta);
		//Find the distance between two points on the surface of the earth specified by longitude and landitude
		double calcDistance(double landitd1, double landitd2, double longitd1, double longitd2);

	private:

		struct pointT {
			int x, y;
		};
		pointT highest, lowest, centroid;
		double percentWater, volWater, avgElev, avgASElev;
};

#endif