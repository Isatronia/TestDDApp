#include "UvcImage.h"

template<typename T>
void Swap(T& a, T& b) { T c; c = a; a = b; b = c; };

UvcBitMap::UvcBitMap()
{
	mBitMapFileHeader = { 0 };
	mBitMapInfoHeader = { 0 };
	memset(&mPalette, 0, sizeof(mPalette));
	mBuffer = nullptr;
	mBitCnt = 0;
}

UvcBitMap::UvcBitMap(const char* fileName)
{
	UvcBitMap::loadImage(fileName);
}

int UvcBitMap::loadImage(const char* fileName)
{
	// TODO: �ڴ˴����ʵ�ִ���.

	int fileHandle,	// the file handle
		index;			// looping index

	UCHAR* tempBuffer = nullptr;	// used to convert 24bit image to 16bit
	OFSTRUCT fileData;
	memset(&fileData, 0, sizeof(fileData));

	if (-1 == (fileHandle = OpenFile(fileName, &fileData, OF_READ)))
	{
		return 0;
	}

	// load the bitmap file header
	_lread(fileHandle, &mBitMapFileHeader, sizeof(BITMAPFILEHEADER));

	// test if is a Bitmap
	if (mBitMapFileHeader.bfType != 0x4D42)
	{
		_lclose(fileHandle);
		return 0;
	}// end if

	// now load the bitmap file header
	_lread(fileHandle, &mBitMapInfoHeader, sizeof(BITMAPINFOHEADER));

	// what we need done for 8-bit bitmap
	if (mBitMapInfoHeader.biBitCount == 8)
	{
		_lread(fileHandle, &mPalette,
			256 * sizeof(PALETTEENTRY));

		for (index = 0; index < 256; index++)
		{
			// reverse the red and greed fields, maybe.
			Swap(mPalette[index].peBlue, mPalette->peRed);

			// aleays set the flags word to this
			mPalette->peFlags = PC_NOCOLLAPSE;
		}
	}// end if

	// in case of 16 bits
	_llseek(fileHandle, -(int)(mBitMapFileHeader.bfSize), SEEK_END);

	// now read in the Image
	if (mBitMapInfoHeader.biBitCount == 8 ||
		mBitMapInfoHeader.biBitCount == 16 ||
		mBitMapInfoHeader.biBitCount == 24)
	{
		// if have another bitmap before, del it.
		if (mBuffer != nullptr)
		{
			free(mBuffer);
		}

		if (!(mBuffer = (UCHAR*)malloc(mBitMapInfoHeader.biSizeImage)))
		{
			_lclose(fileHandle);
			return 0;
		}

		// now read.
		_lread(fileHandle, mBuffer,
			mBitMapInfoHeader.biSizeImage);
	}// end if
	else
	{
		return 0;
	}

	_lclose(fileHandle);

	Flip();

	return 1;
}

int UvcBitMap::unloadImage()
{
	if (mBuffer != nullptr)
	{
		free(mBuffer);
		mBuffer = nullptr;
	}
	return 1;
}

bool UvcBitMap::Flip()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	UCHAR* buffer = (UCHAR*)malloc(sizeof(mBuffer));

	if (mBitMapInfoHeader.biHeight < 0) return true; // the BM do not need flip.
	if (mBitMapInfoHeader.biBitCount < 8) return false;	// not deal with 1&4bit bitmap.

	mBitCnt = mBitMapInfoHeader.biBitCount / 8;
	// Flip the bitmap
	for (int y = 0; y < mBitMapInfoHeader.biHeight; y++)
	{
		memcpy(buffer + y * mBitMapInfoHeader.biHeight,
			mBuffer + (mBitMapInfoHeader.biHeight - y) * mBitMapInfoHeader.biHeight,
			mBitMapInfoHeader.biWidth);
	}

	free(mBuffer);
	mBuffer = buffer;
	buffer = nullptr;
	return true;
}

LPDIRECTDRAWSURFACE7 UvcImage::CreatDDSurface(int Height, int Width, int mem_flags)
{
	DDSURFACEDESC2 ddsd;
	LPDIRECTDRAWSURFACE7 lpdds7;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

	ddsd.dwHeight = Height;
	ddsd.dwWidth = Width;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | mem_flags;

	if (FAILED(mlpDD7->CreateSurface(&ddsd, &lpdds7, NULL)))
	{
		__POSTERR(L"Gif Surface Creat Failed");
		return false;
	}

	return lpdds7;
}