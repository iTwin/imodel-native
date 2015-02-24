//----------------------------------------------------------------------------
//
// WriteBmp.cpp
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>

//***************************************************************
// Write BMP File from HBITMAP
//***************************************************************
void WriteBMPFile(HBITMAP bitmap, const wchar_t *filename, HDC hDC)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD cClrBits;
	HANDLE hf; // file handle
	BITMAPFILEHEADER hdr; // bitmap file-header
	PBITMAPINFOHEADER pbih; // bitmap info-header
	LPBYTE lpBits; // memory pointer
	DWORD dwTotal; // total count of bytes
	DWORD cb; // incremental count of bytes
	BYTE *hp; // byte pointer
	DWORD dwTmp;

	// create the bitmapinfo header information
	if (!GetObject(bitmap, sizeof(BITMAP), (LPSTR)&bmp ))
	{
		std::cout << "Could not retrieve bitmap info" << std::endl;
		return;
	}

	// Convert the color format to a count of bits.
	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
	cClrBits = 24;

	// Allocate memory for the BITMAPINFO structure.
	pbmi = (PBITMAPINFO) LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));

	// Initialize the fields in the BITMAPINFO structure.

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	pbmi->bmiHeader.biCompression = BI_RGB;
	pbmi->bmiHeader.biSizeImage = (pbmi->bmiHeader.biWidth + 7) /8 * pbmi->bmiHeader.biHeight * cClrBits;
	pbmi->bmiHeader.biClrImportant = 0;

	// now open file and save the data
	pbih = (PBITMAPINFOHEADER) pbmi;
	lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	if (!lpBits) 
	{
		std::cout << "writeBMP::Could not allocate memory" << std::endl;
		return;
	}

	// Retrieve the color table (RGBQUAD array) and the bits
	if (!GetDIBits(hDC, HBITMAP(bitmap), 0, (WORD) pbih->biHeight, lpBits, pbmi,DIB_RGB_COLORS)) 
	{
		std::cout << "writeBMP::GetDIB error" << std::endl;
		return;
	}

	// Create the .BMP file.
	hf = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, (DWORD) 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,(HANDLE) NULL);
	
	if (hf == INVALID_HANDLE_VALUE)
	{
		std::cout << "Could not create file for writing" << std::endl;
		return;
	}
	hdr.bfType = 0x4d42;
	// Compute the size of the entire file.
	hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) +
	pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	// Compute the offset to the array of color indices.
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof (RGBQUAD);

	// Copy the BITMAPFILEHEADER into the .BMP file.
	if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), (LPDWORD) &dwTmp, NULL)) 
	{
		std::cout << "Could not write in to file" << std::endl;
		return;
	}

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
	if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof (RGBQUAD),
		(LPDWORD) &dwTmp, ( NULL)))
	{
		std::cout << "Could not write in to file" << std::endl;
		return;
	}


	// Copy the array of color indices into the .BMP file.
	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;
	if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL))
	{
		std::cout << "Could not write in to file" << std::endl;
		return;
	}

	// Close the .BMP file.
	if (!CloseHandle(hf))
	{
		std::cout << "Could not close file" << std::endl;
		return;
	}

	// Free memory.
	GlobalFree((HGLOBAL)lpBits);
}