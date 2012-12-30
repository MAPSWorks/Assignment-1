// File:     BMP.cpp
//
// Author:   Rey Zeng
// SID:      37745114
// UserID:   s6g8
// Lab:      L1E
// Modified: 2012 Nov 6
//
// Contains definitions of functions on how to export a 2D Pixel array to a .BMP file.

#include "BMP.hpp"

using namespace std;

// Initialize a BMP file writer by remembering the size of the image and
// opening the file and writing a .bmp format header to the file.
BMP::BMP( string filename, int height, int width )
{
	rows = height;
	columns = width;
	int rowsize = width*bits/8;
	rowpadding = ( rowsize % 4 ) ? 4 - ( rowsize % 4 ) : 0;
	datasize = headersize + infoheadersize + height*(rowsize + rowpadding);
	filesize = datasize;
	outfile.open( filename.c_str(), ios::binary | ios::out );
	writeHeader();
}

// Write a single 32-bit word to the BMP file
void BMP::writeWord( int value )
{
	outfile.write( (char*) &value, 4 );
}

// Write a single 16-bit halfword to the BMP file
void BMP::writeShort( short value )
{
	outfile.write( (char*) &value, 2 );
}

// Write one row of Pixels to the BMP file
void BMP::writeRow( Pixel* values )
{
	outfile.write( (char*) values, columns*bits/8 );
	// Write enough zero bytes to fill out a full 32-bit word for
	// the row because the format requires this.
	int zeros = 0;
	if ( rowpadding ) outfile.write( (char*) &zeros, rowpadding );
}

// Close the BMP file
void BMP::close()
{
	outfile.close();
}

// Write a 54-byte header to the BMP file that has the appropriate
// settings for an image of the correct size and 24-bit pixels.
//
// The format is explained in Wikipedia
//	http://en.wikipedia.org/wiki/BMP_file_format
void BMP::writeHeader()
{
	// Bytes 0-1: Magic word. It contains the letters 'B' and 'M'.
	outfile.write( "BM", 2 );

	// Bytes 2-5: File size.
	writeWord( filesize );

	// Bytes 6-7 and 8-9: Application dependent. Leave these zeroed.
	writeShort( 0 );
	writeShort( 0 );

	// Bytes 10-13: Start of the bitmap in the file.
	writeWord( offset );

	// Bytes 14-17: Size of the Inforheader.  Normally set to 40.
	// The Inforheader is assumed to start at offset 14.
	writeWord( infoheadersize );

	// Bytes 18-21: Width.
	writeWord( columns );

	// Bytes 22-25: Height.
	writeWord( rows );

	// Bytes 26-27: Word. Planes. Should be 1.
	writeShort( planes );

	// Bytes 29-30: Word. Bits per pixel. 24 for full RGB.
	writeShort( bits );

	// Bytes 31-34: Compression. Leave it zero.
	writeWord( 0 );

	// Bytes 35-38: Compressed size. No compression, leave it zero.
	writeWord( 0 );

	// Bytes 39-42: Horizontal resolution. Pixels per meter.
	writeWord( horizontalresolution );

	// Bytes 43-46: Vertical resolution. Pixels per meter.
	writeWord( verticalresolution );

	// Bytes 47-50: Number of colors in the palette. Leave as zero.
	writeWord( 0 );

	// Bytes 51-54: Number of important colors. Leave as zero.
	writeWord( 0 );
}