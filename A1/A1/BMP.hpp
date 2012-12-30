// File:     BMP.hpp
//
// Author:   Rey Zeng
// SID:      37745114
// UserID:   s6g8
// Lab:      L1E
// Modified: 2012 Nov 6
//
// Declares the BMP class for the BMP class.

#ifndef BMP_H
#define BMP_H

#include <iostream>
#include <fstream>

using namespace std;

struct Pixel {
	char blue;
	char green;
	char red;
	
	Pixel() : red(0), green(0), blue(0) {}
	Pixel(const char A,const char B, const char C) : red(A), green(B), blue(C) {}
};

struct Bound {
	short upper, lower;
	Bound() : upper(0), lower(0) {}
	Bound(const short A,const short B) : upper(A), lower(B) {}
	bool isInRange (const short height) const {return ((upper >= height) && (lower <= height));}
};

struct RenderCode {
	Pixel color;
	Bound range;
};

class BMP
{
public:
	BMP( string filename, int height, int width );
	void writeHeader();
	void writeRow( Pixel* );
	void close();

private:
	void writeShort( short value );
	void writeWord( int value );
	static const int headersize = 14;
	static const int infoheadersize = 40;
	static const int offset = 54;
	static const int planes = 1;
	static const int bits = 24;
	static const int verticalresolution = 2835;
	static const int horizontalresolution = 2835;
	int filesize;
	int columns;
	int rows;
	int rowpadding;
	int datasize;
	ofstream outfile;
};

#endif
