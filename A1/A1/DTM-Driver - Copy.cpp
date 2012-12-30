// File:     DTM-Driver.cpp
//
// Author:   Rey Zeng
// SID:      37745114
// UserID:   s6g8
// Lab:      L1E
// Modified: 2012 Oct 5
//
// Main driver file that runs tha program.

#include <vector>
#include <cmath>
#include <string>
#include <algorithm> // for sorting only
#include "BMP.hpp"
#include "DTM.hpp"

using namespace std;

typedef Pixel** PixelMatrix;
typedef Pixel* PixelVector;
typedef PixelVector RenderingMap;

struct pointT {short i, j;};
struct coordT {float lat, lon;};

short lowest, highest; // for convenience. prefer these to be somewhere else, though.

short tbase[NUMROW][NUMCOL];
DTM DTMtest;
RenderingMap rm;

void writeImgFile(string filename, PixelMatrix image, int numRow, int numCol)
{
	BMP imageFile(filename, numRow, numCol);

	// Write the image out to the BMP file, bottom row first
	for (int i = numRow - 1; i >= 0; i--) {
		// since vector store its elements continuously
		PixelVector oneRow = image[i];
		imageFile.writeRow(oneRow);
	}

	imageFile.close();

	string winOpen = "start " + filename;
	string linOpen = "open " + filename;

	// Display the resulting image
#if 1
	system(winOpen.c_str());
#else
	system(linOpen.c_str());
#endif
}

bool MyDataSortPredicate(const RenderCode& d1, const RenderCode& d2)
{
	return (d1.range.lower < d2.range.lower);
}

/**
* Generates a linear color range from c1 to c2.
*/
PixelVector makeGradient(Bound b, Pixel c1, Pixel c2)
{
	short diff = b.upper - b.lower;
	PixelVector grad = new Pixel[diff + 1];
	float upper, lower;
	// red
	upper = ((float)c1.red < 0) ? (float)c1.red + 256: (float)c1.red;
	lower = ((float)c2.red < 0) ? (float)c2.red + 256: (float)c2.red;
	float redInc = (upper - lower) / (float)diff;
	// green
	upper = ((float)c1.green < 0) ? (float)c1.green + 256: (float)c1.green;
	lower = ((float)c2.green < 0) ? (float)c2.green + 256: (float)c2.green;
	float greenInc = (upper - lower) / (float)diff;
	// blue
	upper = ((float)c1.blue < 0) ? (float)c1.blue + 256: (float)c1.blue;
	lower = ((float)c2.blue < 0) ? (float)c2.blue + 256: (float)c2.blue;
	float blueInc = (upper - lower) / (float)diff;
	for (int i = 0; i < diff + 1; i++) {
		Pixel newOne(char((float)c1.red - redInc*i), char((float)c1.green - greenInc*i), char((float)c1.blue - blueInc*i));
		grad[i] = newOne;
	}
	return grad;
}

/**
* Reads the color for an elevation range from file and stores it in an array of "Pixel".
* Implicitly, each index of the array represents an elevation.
* Index 0 has the color info for the lowest elevation, index size() - 1 has the infor for the highest elevation.
* More context: if difference between the higherst elevation and the lowest elevation is (8000 - (-11000)) = 19000, the array will have a length of 19000.
* Trading memory for run time. Program runs significantly faster this way.
*/
RenderingMap readRenderingInstructions()
{
	cout << "Enter LUT filename: ";
	string filename;
	cin >> filename;
	RenderingMap rm;
	vector<RenderCode> allCodes;
	lowest = 0, highest = 0;
	ifstream infile(filename.c_str());
	if (!infile.is_open()) {return rm;}
	while (!infile.eof()) {
		Bound newBound;
		Pixel newPixel;
		short temp;
		infile >> newBound.lower;
		infile >> newBound.upper;
		infile >> temp;
		newPixel.red = temp;
		infile >> temp;
		newPixel.green = temp;
		infile >> temp;
		newPixel.blue = temp;
		short curElev = newBound.lower;
		if (newBound.lower < lowest) lowest = newBound.lower;
		if (newBound.upper > highest) highest = newBound.upper;
		RenderCode newCode;
		newCode.color = newPixel;
		newCode.range = newBound;
		allCodes.push_back(newCode);
	}
	// sorts the range info, makes life easier.
	sort(allCodes.begin(), allCodes.end(), MyDataSortPredicate);
	short mapSize = highest - lowest + 1;
	short curIndex = 0;
	rm = new Pixel[mapSize];
	Pixel prevColor, curColor;
	short curLow, curHigh;
	int colorCount = 0;
	for(vector<RenderCode>::iterator it = allCodes.begin(); it != allCodes.end(); ++it) {
		colorCount++;
		curLow = (it->range).lower, curHigh = (it->range).upper;
		short curElev = curLow;
		if (colorCount == 1) {
			curColor = it->color;
			while (curElev <= curHigh) {
				rm[curIndex] = curColor;
				curElev++;
				curIndex++;
			}
		} else {
			prevColor = curColor;
			curColor = it->color;
			Bound curBound(curHigh, curLow);
			PixelVector newGrad = makeGradient(curBound, prevColor, curColor);
			int counter = 0;
			while (curElev <= curHigh) {
				rm[curIndex] = newGrad[counter];
				curElev++;
				curIndex++;
				counter++;
			}
		}		
	}
	infile.close();
	return rm;
}

PixelMatrix getPixelMatrix(int numRows, int numCols)
{
	PixelMatrix arr = new Pixel*[numRows];
	for(int i = 0; i < numRows; i++) {
		arr[i] = new Pixel[numCols];
	}
	return arr;
}

short convertHeightToIndex(short height) {
	return (height - lowest);
}

void convertFromLLtoij(float lat, float lon, pointT &pt)
{
	pt.i = (short)((90 - lat) * 12);
	pt.j = (short)(abs(-180 - lon) * 12);
}

void convertFromijToLL (short i, short j, coordT &pt)
{
	pt.lat = 90.0 - (float)i / 12.0;
	pt.lon = -180.0 + (float)j / 12.0;
}

/**
* Generates 3d projection by mapping latitide and longitude to their corresponding coordinates on the sphere.
* Only the x and y coordinates are displayed.
* Span 180 deg latitude, 180 deg longitude
*/
void draw3DProj(string filename, float lon) {
	short type;
	cout << "Projection type (0 for help): " << endl;
	while (true) {
		cin >> type;
		if (type >= 1 && type <= 5) break;
		if (type == 0) {
			cout << "1. Orthographic" << endl;
			cout << "2. Sanson-Flamsteed" << endl;
			cout << "3. Stereographic" << endl;
			cout << "4. Vertical Perspective" << endl;
			cout << "5. Lambert Azimuthal Equal-Area" << endl;
		}
	}
	float dist;
	if (type == 4) {
		cout << "Enter Vertical distance (> 0): ";
		dist = -1;
		while (dist <= 0) {
			cin >> dist;
		}
	}
	cout << "Generating..." << endl;

	float lat = 0;
	float height = 180, width = 180;
	float top = (lat + (height / 2.0));
	float bot = (lat - (height / 2.0));
	float left = (lon - (width / 2.0));
	float right = (lon + (width / 2.0));
	float curi = top, curj = left, step = 1.0/12.0;

	short elev = 0;
	// calculate the num of cells and initialize an image array.
	short numRow = (short)(height * 12.0) + 1, numCol = (short)(width * 12.0) + 1;
	PixelMatrix regionalImg = getPixelMatrix(numRow, numCol);
	float maxx = 0, maxy = 0, minx = 0, miny = 0;
	//cout << (short)(width * 12.0) + 1 << " " << (short)(height * 12.0) + 1 << endl;
	// iterate through the complete region, the bad points will be mapped to their corresponding positions.
	while (curi >= bot) {
		while (curj <= right) {
			// in radian
			float lat_new = atan((sin(curi * PI / 180.0)) / abs(cos(curi * PI / 180.0))) * 180.0 / PI;
			float lon_new = atan2(sin(curj * PI / 180.0), cos(curj * PI / 180.0)) * 180.0 / PI;
			float lat_new_rad = lat_new * PI / 180.0;
			float lat_rad = lat * PI / 180.0;
			float lon_new_rad = lon_new * PI / 180.0;
			float lon_rad = lon * PI / 180.0;
			float lon_diff_rad = (lon_new - lon) * PI / 180.0;
			// transformation constants, only some are required for each transform
			float c = (sin(lat_rad)*sin(lat_new_rad)+cos(lat_rad)*cos(lat_new_rad)*cos(lon_diff_rad));
			float k = 1.0 / (1 + sin(lat_rad) * sin(lat_new_rad) + cos(lat_rad) * cos(lat_new_rad) * cos(lon_diff_rad));
			float P = dist; // vertical perspective
			float kp = (P - 1) / (P - c); // vertical perspective
			float kpp = sqrt(2.0*k); // Lambert Azimuthal Equal-Area
			// conic equidistant
			float x, y;

			switch (type) {
			case 1: //Orthographic: any longitude				
				x = NUMROW / 3.5 * cos(lat_new_rad) * sin(lon_diff_rad);
				y = NUMROW / 3.5 * (-1.0)*(cos(lat_rad)*sin(lat_new_rad) - sin(lat_rad)*cos(lat_new_rad)*cos(lon_diff_rad));
				break;
			case 2: //Sanson-Flamsteed: longitude = [-90, 90]				
				x = NUMROW / 3.15 * lon_diff_rad * cos(lat_new_rad);
				y = (-1.0)*NUMROW / 3.15 * lat_new_rad;
				break;
			case 3: //Stereographic: any longitude
				x = NUMROW / 2.0 * k * cos(lat_new_rad)*sin(lon_diff_rad);
				y = (-1) * NUMROW / 2.0 * k * (cos(lat_rad) * sin(lat_new_rad) - sin(lat_rad) * cos(lat_new_rad) * cos(lon_diff_rad));
				break;
			case 4: //Vertical Perspective: any longitude (dependent on variable P)
				x = NUMROW / 2.0 * kp * cos(lat_new_rad) * sin(lon_diff_rad);
				y = (-1.0) * NUMROW / 2.0 * kp * (cos(lat_rad)*sin(lat_new_rad)-sin(lat_rad)*cos(lat_new_rad)*cos(lon_diff_rad));
				break;
			case 5: //Lambert Azimuthal Equal-Area
				x = NUMROW / 3.0 * kpp * cos(lat_new_rad)*sin(lon_diff_rad);
				y =  (-1) * NUMROW / 3.0 * kpp * (cos(lat_rad) * sin(lat_new_rad) - sin(lat_rad) * cos(lat_new_rad) * cos(lon_diff_rad));
				break;
			}

			//maxx = (x > maxx) ? x : maxx;
			//maxy = (y > maxy) ? y : maxy;
			//minx = (x < minx) ? x : minx;
			//miny = (y < miny) ? y : miny;

			pointT pt;
			convertFromLLtoij(lat_new, lon_new, pt);

			short elev = tbase[pt.i][pt.j];
			regionalImg[(short)(y + NUMROW / 2.0)][(short)(x + NUMROW / 2.0)] = rm[convertHeightToIndex(elev)];

			curj += step;
		}
		curj = left;
		curi -= step;
	}
	//cout << minx << " " << miny << " " << maxx << " " << maxy << endl;
	writeImgFile(filename, regionalImg, numRow, numCol);
	for (int i = 0; i < numRow; i++) {
		delete regionalImg[i];
	}
}

/**
* Draw region draws an image with centre at (lat, lon) with height and width in degrees.
* 1. Converts the centre point from height and width to the corresponding indices in the "tbase" array.
* 2. Calculates the bounding rectangle in the "tbase" array.
* 3. Draws the image using the indices.
* Prefer to have only draw3DProj, but this runs faster.
*/
void drawRegion(string filename, float lat, float lon, float height, float width)
{
	pointT centrePt;
	convertFromLLtoij(lat, lon, centrePt);
	height *= 12;
	width *= 12;
	short topi = (centrePt.i - (short)(height / 2)); // top latitude
	short boti = (centrePt.i + (short)(height / 2)); // bot latitude
	short leftj = (centrePt.j - (short)(width / 2)); // left longitude
	short rightj = (centrePt.j + (short)(width / 2)); // right longitude
	short numRow = (short)(height) + 1, numCol = (short)(width) + 1; // size of image in terms of pixels
	PixelMatrix regionalImg = getPixelMatrix(numRow, numCol); // creates an empty image with the desired size
	short x = 0, y = 0;
	//short minelev = 0, maxelev = 0;
	for (short i = topi; i < boti; i++) {
		for (short j = leftj; j < rightj; j++) {
			short adjustedi = i, adjustedj = j;
			while (adjustedi <= -NUMROW) {
				adjustedi += NUMROW;
			}
			while (adjustedi >= NUMROW * 2) {
				adjustedi -= NUMROW;
			}
			// -NUMROW < adjustedi < NUMROW
			if (adjustedi < 0) { //  -NUMROW < adjustedi < 0
				adjustedi = (i * (-1));
				adjustedj = (((j + NUMCOL/2)%NUMCOL)+NUMCOL)%NUMCOL;
			} else if (adjustedi >= NUMROW) { // NUMROW <= adjustedi < NUMROW * 2
				adjustedi = (NUMROW - (i % NUMROW));
				adjustedj = (((j + NUMCOL/2)%NUMCOL)+NUMCOL)%NUMCOL;
			} else { // 0 <= adjustedi < NUMROW
				adjustedj = (((j)%NUMCOL)+NUMCOL)%NUMCOL;
			}

			short elev = tbase[adjustedi][adjustedj];
			regionalImg[x][y] = rm[convertHeightToIndex(elev)];
			//minelev = (elev < minelev) ? elev : minelev;
			//maxelev = (elev > maxelev) ? elev : maxelev;
			y++;
		}
		x++;
		y = 0;
	}

	// Transform any 180 by 180 2D array into 3D projection. Not necessarily correct.
	//PixelMatrix newRegionalImg = getPixelMatrix(numRow, numCol);
	//for (short i = 0; i < numRow; i++) {
	//	for (short j = 0; j < numCol; j++) {
	//		float lat = 90 - i / 12.0;
	//		float lon = -90 + j / 12.0;
	//		float x = NUMROW / 2.0 * cos(lat * PI / 180.0) * sin(lon * PI / 180.0);
	//		float y = (-1) * NUMROW / 2.0 * sin(lat * PI / 180.0);
	//		//cout << x << " " << y << endl;
	//		newRegionalImg[(int)(y + NUMROW / 2.0)][(int)(x + NUMROW / 2.0)] = regionalImg[i][j];
	//	}
	//}
	writeImgFile(filename, regionalImg, numRow, numCol);
	//writeImgFile(filename, newRegionalImg, numRow, numCol);
	//cout << minelev << " " << maxelev << endl;
	for (int i = 0; i < numRow; i++) {
		delete regionalImg[i];
		//delete newRegionalImg[i];
	}
}


/**
 * Outputs Mercator projection of .BMP image file with the name "filename" centered at "lat", "lon" 
 * with size "height" and "width" at user's request.
 */
void processMercatorCommand() {
	while (true) {
		string filename;
		float lat, lon, height, width;
		cout << "Enter image name, center latitude, center longitude, height and width: ";
		cin >> filename >> lat >> lon >> height >> width;
		if (filename == "0") break;
		short posBMP = filename.find(".BMP");
		short posbmp = filename.find(".bmp");
		if (posBMP < 0 && posbmp < 0) {
			filename += ".BMP";
		}
		cout << "Generating..." << endl;
		drawRegion(filename, lat, lon, height, width);
	}
}

/**
 * Outputs 3D projection .BMP image file with the name "filename" centered the spcific longitude at user's request.
 */
void process3DCommand() {
	while (true) {
		string filename;
		float lon;
		cout << "Enter image name, center longitude: ";
		cin >> filename >> lon;
		if (filename == "0") break;
		short posBMP = filename.find(".BMP");
		short posbmp = filename.find(".bmp");
		if (posBMP < 0 && posbmp < 0) {
			filename += ".BMP";
		}
		draw3DProj(filename, lon);
	}
}

/**
* Get input from user and deliver service as discribed by self documentations.
*/
void processOption() {
	bool done = false;
	while (!done) {
		int option;
		cout << "Enter your option (0 to 3): ";
		cin >> option;
		switch(option) {
		case -1: // Exit
			done = true;
			break;
		case 0: // Diaplay options
			case0:
			cout << "-1: Quit" << endl;
			cout << "0: Display info." << endl;
			cout << "1: Write stats to file" << endl;
			cout << "2: Print Mercator projection" << endl;
			cout << "3: Print 3D projection" << endl;
			break;
		case 1: // Write stats to file
			DTMtest.writeAndCalc(tbase);
			cout << "Stats written." << endl;
			break;
		case 2: // Print Mercator projection
			cout << "Getting input for Mercator projection. Enter 0 for filename to quit." << endl;
			processMercatorCommand();
			break;
		case 3: // Print 3D projection
			cout << "Getting input for 3D projection. Enter 0 for filename to quit." << endl;
			process3DCommand();
			break;
		default: // case 0
			goto case0;
		}
	}
}

int main()
{
	DTMtest.readFile("tbase.bin", tbase[0], NUMROW, NUMCOL);
	DTMtest.writeAndCalc(tbase); // write stats to "EarthStats.txt"
	cout << "Earth stats written to file." << endl;

	rm = readRenderingInstructions(); // reads from file the color ranges and store it in a 'RenderingMap'

	processMercatorCommand();
	processOption();

	//drawRegion("Vancouver.bmp", 49.2505, -123.1119, 16, 16); // draw mercator projection
	//draw3DProj("3Dproj.bmp", 0); // draw 3D projection

	system("PAUSE");
	return 0;
}