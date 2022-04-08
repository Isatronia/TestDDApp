//***************************************************************************************
// UvcDDApp.h by Aaron Jsc (C) 2020 All Rights Reserved.
// licenced under CC BY-NC-SA.
//***************************************************************************************
#pragma once

#include <Windows.h>
#include <iostream>
#include <conio.h>

#include <ddraw.h>

class UvcImage
{
public:
	UvcImage();
	UvcImage(const char* fileName);
	~UvcImage();

	int loadImage(const char* fileName);
	int unloadImage();

	bool Flip();

	RECT GetRect();
	RECT GetDestRect(long x, long y);

public:

	BITMAPFILEHEADER mBitMapFileHeader;

	BITMAPINFOHEADER mBitMapInfoHeader;

	PALETTEENTRY mPalette[256];

	UCHAR* mBuffer;
	int mBitCnt;
};
