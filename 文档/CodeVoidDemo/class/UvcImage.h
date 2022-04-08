//***************************************************************************************
// UvcImage.h by Aaron Jsc (C) 2020 All Rights Reserved.
// licenced under CC BY-NC-SA License.
//***************************************************************************************
#pragma once

#include <Windows.h>
#include <iostream>
#include <conio.h>

#include <ddraw.h>

struct RGBInfo
{
	UCHAR r;
	UCHAR g;
	UCHAR b;

	RGBInfo()
	{
		r = 0;
		g = 0;
		b = 0;
		return;
	}
	explicit RGBInfo(int R, int G, int B)
	{
		r = R; g = G; b = B;
		return;
	}
	inline bool operator==(const RGBInfo& col)
	{
		if (col.r == r && col.g == g && col.b == b) return true;
		else return false;
	}
	inline bool operator!=(const RGBInfo& col)
	{
		if (col.r != r || col.g != g || col.b != b) return true;
		else return false;
	}
};

class UvcImage
{
public:

	UvcImage();
	UvcImage(const char* fileName);
	UvcImage(const UvcImage& UImg);
	~UvcImage();
public:
	void operator=(UvcImage src);

public:
	// load Bitmap by file name.
	int loadImage(const char* fileName);
	// unload the Bitmap
	int unloadImage();

	// Flip the bitmap which is loaded.
	bool Flip();

	// Get Rect of the bitmap, from (0,0) to (Width, Height)
	RECT GetRect();
	// Get Rect of the bitmap, from (x,y) to (x + Width, y + Height)
	RECT GetDestRect(long x, long y);
	// 画地图时候用的Rect资源：
	RECT GetCameraRECT(LONG Camerax, LONG Cameray, LONG Width, LONG Height);

	// Get pixel's RGB info which at (x, y), save in parament.
	void GetPixelRGB(int x, int y, UCHAR& r, UCHAR& g, UCHAR& b);
	RGBInfo GetPixelRGB(int x, int y);

public:

	// bitmap Info
	BITMAPFILEHEADER mBitMapFileHeader;

	BITMAPINFOHEADER mBitMapInfoHeader;

	PALETTEENTRY mPalette[256];

	UCHAR* mBuffer;
	int mBitCnt;
};
