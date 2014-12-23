// EncryptShaders.cpp : Defines the entry point for the console application.
//

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>

#include <iostream>
#include <fstream>
#include <shlwapi.h>

// very simple encryption to avoid file reading - a good hacker could overcome easily

int _hash_out(char*txt)
{
	static int p=0;

	static const char* mask = "Vertex program initialization";
	int i = 0;
	while (txt[i]!='\0')
	{
		if (txt[i] == 9) txt[i] = ' '; //tab 	
		txt[i++]  ^= mask[i%29];
	}
	txt[i] = 0xaa + (++p % 60);
	return ++i;
}

void encryptFile( const char *filepath, const char*name, const char *outFilepath )
{
	char buff[260];
	char cbuff[260];
	char shaderName[64];
	int l = strlen(outFilepath);
	sprintf( buff, "%senc", outFilepath );
	sprintf( cbuff, "%s.cpp", outFilepath );
	strcpy(shaderName, name);
	buff[l-1] = '-';


	PathRemoveExtensionA(shaderName);

	std::ofstream enc, code;
	enc.open(buff, std::ios::out | std::ios::binary);
	if (!enc.is_open())
	{
		std::cout << "attempt to open " << buff << " failed " << std::endl;
		return;
	}
	code.open(cbuff, std::ios::out);
	if (!code.is_open())
	{
		std::cout << "attempt to open " << cbuff << " failed " << std::endl;
		return;
	}

	std::ifstream input;
	input.open(filepath);

	if (!input.is_open())
	{
		std::cout << "attempt to open " << filepath << " failed " << std::endl;
		return;
	}
	if (enc.is_open() && code.is_open())
	{
//		code << "namespace pointsengine_private {" << std::endl << std::endl;

		code << "const ubyte " << shaderName << "_" << &(::PathFindExtensionA(shaderName))[1] << 
			" [] = { " << std::endl << "\t  ";

		int index = 0;

		while (!input.eof())
		{
			char buffer[512];
			input.getline( buffer, 512 );

			int llen = _hash_out(buffer);
			for (int j=0; j<llen; j++)
			{
				enc.put(buffer[j]);

				{
					if (index) code << ", ";
					int val = buffer[j];
					val = ((unsigned int)val) & ~0xffffff00;

					code << "0x" << std::hex << val;
				}
				if (index % 10 == 9) code << std::endl << "\t";
				++index;

			}
			
		}
		code << std::endl << "};" << std::endl ;//<< "}; //namespace" << std::endl;

		code.close();
		enc.close();
		input.close();

		std::cout << "encrypted " << filepath << " to file" << std::endl;
		std::cout << "encrypted " << filepath << " to source" << std::endl;
	}
}
void encryptFiles( const char * appFolder, const char * searchExt, const char *outFolder)
{
/* process files in the folder its in */ 
	WIN32_FIND_DATA ff;
	char searchpath[260];    
	char filepath[260];    
	char filepathOut[260];    
	char folder[260];
	
	/* get the folder that this is in */ 
	strcpy( searchpath, appFolder );
	::PathRemoveFileSpec( searchpath );

	strcpy( folder, searchpath );
	sprintf_s( searchpath, 260, "%s\\%s", folder, searchExt );

	/* search folder for files */ 
	HANDLE hFind = ::FindFirstFile(searchpath, &ff);
	
	int i=0;

	if (hFind != INVALID_HANDLE_VALUE)
	{			
		sprintf( filepath, "%s\\%s", folder, ff.cFileName );
		sprintf( filepathOut, "%s\\%s", outFolder, ff.cFileName );
		encryptFile( filepath, ff.cFileName, filepathOut);

		while (::FindNextFile(hFind, &ff))
		{
			sprintf( filepath, "%s\\%s", folder, ff.cFileName );
			sprintf( filepathOut, "%s\\%s", outFolder, ff.cFileName );
			encryptFile( filepath, ff.cFileName, filepathOut);
		}
	}
	FindClose(hFind);	
}
int _tmain( int argc, _TCHAR* argv[] )
{
	// Get the file path of this application
	char buff[MAX_PATH] = {0};
	char outPath[MAX_PATH] = {0};

	if(argc == 1)
	{
		::GetModuleFileName(NULL, buff, MAX_PATH);
		::GetModuleFileName(NULL, outPath, MAX_PATH);
	}
	else
	if(argc == 2)
	{
		strcpy_s(buff, MAX_PATH, argv[1]);
		strcpy_s(outPath, MAX_PATH, argv[1]);
	}
	else
	if(argc == 3)
	{
		strcpy_s(buff, MAX_PATH, argv[1]);
		strcpy_s(outPath, MAX_PATH, argv[2]);
	}

	std::cout << "Encrypting shaders at location " << buff << " To " << outPath << std::endl;

	encryptFiles( buff, "*.vsh", outPath);
	encryptFiles( buff, "*.vert", outPath);
	encryptFiles( buff, "*.frag", outPath);
	return 0;
}

