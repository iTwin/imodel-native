// CacheMultiAccess.cpp : Defines the entry point for the console application.
//

#include "CacheMultiAccessPch.h"
#include "HRFCacheFile.h"
#include "HRFCacheFileEditor.h"
#include <iostream>

#include <chrono>
#include <string.h>

#include <Imagepp/all/h/HFCAccessMode.h>
#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFResolutionEditor.h>
#include <Imagepp/all/h/HRFcTiffFile.h>
#include <Imagepp/all/h/HRFCacheSequentialBlockEditor.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>


#include <stdlib.h> 
#include <vector> 
#include <algorithm>
#include <Imagepp/all/h/HRFFileFormats.h>

using namespace std;

IMPLEMENT_DEFAULT_IMAGEPP_LIBHOST(MyImageppLibHost)


int wmain()
	{

	//Initialize imagePP to create tiff file
	ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());


	// create and open a itiff
	const WString path = L"C:\\Users\\nicolas.marquis\\Desktop\\images\\ATLANTA.SID.iTIFF64";
	HFCPtr<HFCURL> pURL = new HFCURLFile(HFCURLFile::s_SchemeName() + L"://" + path);

	HFCPtr<HRFRasterFile>  pSrcFile  = HRFRasterFileFactory::GetInstance()->OpenFile(pURL, HFC_READ_ONLY, 0);
	
	HRFResolutionEditor* pSrcResolutionEditor = pSrcFile->CreateResolutionEditor(0, 0, pSrcFile->GetAccessMode());
	
	

	// Create a new cache file
	// false = on header on disk, true = in memory
	HFCPtr<HRFCacheFile> cFile = new HRFCacheFile(true);
	HRFCacheFileEditor cacheEditor(cFile, 0, 0, pSrcResolutionEditor);

	// Create a old cache file
	const WString path2 = L"C:\\Users\\nicolas.marquis\\Desktop\\images\\test.ctiff";
	HFCPtr<HFCURL> pURL2 = new HFCURLFile(HFCURLFile::s_SchemeName() + L"://" + path2);
	HFCPtr<HRFRasterFile> oldCache = HRFcTiffCreator::GetInstance()->Create(pURL2, HFC_READ_WRITE_CREATE);

	oldCache->AddPage(pSrcFile->GetPageDescriptor(0));

	HRFResolutionEditor* pCacheResolutionEditor = oldCache->CreateResolutionEditor(0, 0, oldCache->GetAccessMode());

	HRFResolutionEditor* pEditor = new HRFCacheSequentialBlockEditor(oldCache, 0, 0, HFC_READ_WRITE_CREATE, pCacheResolutionEditor, pCacheResolutionEditor);


	// init operation for 10000 blocks, ATLANTA.SID.iTIFF64 has more then that
	int blocksPerWidth = 100;
	int blocksPerHeight = 100;

	int blockWidth = pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockWidth();
	int blockHeight = pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockHeight();

	Byte* buffer = new Byte[blockWidth * blockHeight * 3];
	memset(buffer, 100, blockWidth * blockHeight * 3);

	//Build random offsets vector to access the bloc randomly
	std::vector<int> listIndex;
	for (int i = 0; i < 100; ++i) listIndex.push_back(i);
	std::random_shuffle(listIndex.begin(), listIndex.end());

	///////////////////////////////////////WRITE/////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////

	//auto t_start = std::chrono::high_resolution_clock::now();

	//for (int w = 0; w < blocksPerWidth; ++w)
	//	{
	//	for (int h = 0; h < blocksPerHeight; ++h)
	//		pEditor->WriteBlock(listIndex[w] * blockWidth, listIndex[h] * blockHeight, buffer, 0);
	//	}

	//auto t_end = std::chrono::high_resolution_clock::now();

	//std::cout << "WRITE old cache !!! : "
	//	<< std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count()
	//	<< " ms\n\n";


	/*auto t_start = std::chrono::high_resolution_clock::now();

	for (int w = 0; w < blocksPerWidth; ++w)
		{
		for (int h = 0; h < blocksPerHeight; ++h)
			cacheEditor.Write(listIndex[w] * blockWidth, listIndex[h] * blockHeight, buffer);
		}

	auto t_end = std::chrono::high_resolution_clock::now();

	std::cout << "WRITE new cache !!! : "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count()
		<< " ms\n\n";*/



	/////////////////////////////////initialization for read///////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////

	for (int w = 0; w < blocksPerWidth; ++w)
		{
		for (int h = 0; h < blocksPerHeight; ++h)
			{
			if (H_SUCCESS == pSrcResolutionEditor->ReadBlock(w*blockWidth, h*blockHeight, buffer))
				{
				cacheEditor.Write(w*blockWidth, h*blockHeight, buffer);
				pEditor->WriteBlock(w*blockWidth, h*blockHeight, buffer, 0);
				}
			}
		}

				
	

	////////////////////////////////////////READ/////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////

	auto t_start = std::chrono::high_resolution_clock::now();

	for (int w = 0; w < blocksPerWidth; ++w)
		{
		for (int h = 0; h < blocksPerHeight; ++h)
			pEditor->ReadBlock(listIndex[w] * blockWidth, listIndex[h] * blockHeight, buffer);
		}
	
	auto t_end = std::chrono::high_resolution_clock::now();

	std::cout << "READ old cache !!! : "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count()
		<< " ms\n\n";

	t_start = std::chrono::high_resolution_clock::now();

	for (int w = 0; w < blocksPerWidth; ++w)
		{
		for (int h = 0; h < blocksPerHeight; ++h)
			{
			cacheEditor.Read(listIndex[w] * blockWidth, listIndex[h] * blockHeight, buffer);
			}
		}

	t_end = std::chrono::high_resolution_clock::now();

	std::cout << "READ new cache !!! : "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count()
		<< " ms\n\n";


	std::cout << "Press any key to quit";
	std::getchar();

	return 0;

	}