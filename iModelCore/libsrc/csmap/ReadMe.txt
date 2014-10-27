Building CS-MAP on Windows and Linux

The CS-MAP distribution will produce a series of five directories:

Include: Contains all header files referenced the source code in the Source directory.

Source: Contains all the source code for the CS-MAP library itself.

Dictionaries: Contains the coordinate system dictionaries in source form, and the source code for a compiler which will convert the dictionary source to the operational binary form.

Test: Contains the source code for a console type test program and the test data which it uses.

TestCpp: Contains the source code for the C++ version of the console test program and the test data which it uses.

Data: Contains a series of data files used to construct name mapping files.

VC90: Contains solution and project files suitable for use with Microsoft Visual Studio 2008 (Version 9.0).

VC100: Contains solution and project files suitable for use with Microsoft Visual Studio 2010 (Version 10.0).

Building the entire product is a series of five steps:
	1> Build the CS-MAP library.
	2> Build the dictionary compiler.
	3> Run the dictionary compiler.
	4> Build the console test program.
	5> Execute the console test program.

After installation, and before building, it will be best to obtain a copy of the Canadian National Transformation file (NTV2_0.gsb) and copy it to the Dictionaries/Canada directory.  This data file may not be distributed!!!  Geomatics Canada reserves the right to distribute this file and maintain a list of those using it.  This is part of an ISO 9000 consideration.  Therefore, since we do not distribute the file as part of this open source distribution, we recommend strongly that you simply obtain a copy, even if only for testing purposes.

Chances are very good you already have a copy of this file on your system already.  If not, you can obtain one (no fee) at:

	http://www.geod.nrcan.gc.ca

You will need to inform the CS-MAP library of the existence of this file and where it is located.  In release 12 or earlier, this information is passed to the CS-MAP library via the "Nad27ToNad83.gdc" data file.  This is a simple text file which can be edited with any text editor.  Comments in the distribution version of this file are verbose and describe exactly what needs to be accomplished to activate access to the new data file.

In release 13 and thereafter, the existence and location of the NTV2_0.gsb file is conveyed to the CS-MAP library through one or more definitions in the Geodetic Transformation Dictionary.  Thus, you must modify the source file for this dictionary (GeodeticTransform.asc) and then recompile it using the dictionary compiler.  As of this writing, there is no UI or alternative means of modifying this information.

The TEST.DAT data file in the Test directory contains several hundred test points which are directly related to the above mentioned grid shift data file.  To prevent confusion and unnecessary technical support, these test points are commented out in the distribution.  After obtaining a copy of the above mentioned data file, these test should be uncommented back in, so that the test program will test this feature.

The situation described above concerning the Canadian National Transformation also applies to other sources of geodetic transformation data.  In these cases, it is not so much that we know we are not permitted to distribute the file, it is more that we are unable to determine that we are permitted to distribute the data file.  Thus, to be safe the files are not distributed.  At this writing, this situation applies to other CSRS related Canadian files and the Japanese Geodetic Datum of 2000 data file.  Check the appropriate locations in the distribution folder hierarchy for readme files which describe the situation for these locations and provides suggestions on how to obtain a copy of the file.

A similar situation exists with regard to the Danish System 34 conversion modules.  We are unable to distribute the code for this transformation, even though the code is free available on the Internet.  Should you need to, or desire to, include the Danish System 34 conversion in your application, please carefully read the introductory comments of the provided "CSsys34KMS.c" (distributed in the 'Source" folder).

OK. Now for building on your system:

For Windows:

1> Build the CS-MAP Library:
	Make the 'Source' directory your current working directory.
	Use the MSVC set variables script to set the environmental variables correctly.
	Use the 'nmake' command and supply it with the 'Library.nmk' make file.  E.g.
		'nmake /fLibrary.nmk'
2> Build the Dictionary Compiler (CS_comp)
	Make the 'Dictionaries' directory your current working directory.
	Use the MSVC set variables script to set the environmental variables correctly.
	Use the 'nmake' command and supply it with the 'Compiler.nmk' make file.  E.g.
		'nmake /fCompiler.nmk'
3> Run the Dictionary Compiler
	Make the 'Dictionaries' directory your current working directory.
	Execute the 'CS_comp' program.  E.g.
		'CS_Comp . .'
	Note that the first argument is the directory containing the dictionary source, the
	second argument is the directory to which the binary dictionary files are written. 
4> Build the Console Test program (CS_Test)
	Make the 'Test' directory your current working directory.
	Use the MSVC set variables script to set the environmental variables correctly.
	Use the 'nmake' command and supply it with the 'Test.nmk' make file.  E.g.
		'nmake /fTest.nmk'
5> Execute the console test program
	Make the 'Test' directory your current working directory.
	Execute the 'CS_Test' program.  E.g.
		'CS_Test /d..\Dictionaries'
	Note that the /d argument is the directory which the test program is to look to
	for the dictionaries and related data files.

For Linux:

1> Build the CS-MAP Library:
	Make the 'Source' directory your current working directory.
	Use the 'make' command and supply it with the 'Library.mak' make file.  E.g.
		'make -fLibrary.mak'
2> Build the Dictionary Compiler (CS_Comp)
	Make the 'Dictionaries' directory your current working directory.
	Use the 'make' command and supply it with the 'Compiler.mak' make file.  E.g.
		'make -fCompiler.mak'
3> Run the Dictionary Compiler
	Make the 'Dictionaries' directory your current working directory.
	Execute the 'CS_Comp' program.  E.g.
		'./CS_Comp . .'
	Note that the first argument is the directory containing the dictionary source,
	the second argument is the directory to which the binary dictionary files are
	written. 
4> Build the Console Test program (CS_Test)
	Make the 'Test' directory your current working directory.
	Use the 'make' command and supply it with the 'Test.mak' make file.  E.g.
		'make -fTest.mak'
5> Execute the console test program
	Make the 'Test' directory your current working directory.
	Execute the 'CS_Test' program.  E.g.
		'./CS_Test -d../Dictionaries'
	Note that the /d argument is the directory which the test program is to look
	to for the dictionaries and related data files.

MS VC++ 2008 (Version 9):

The CS-MAP Open Source distribution will deposit in a folder named 'VC90' a Microsoft Visual C++ Version 9.0 (VS 2008) solution file.  This file references project files in the same 'VC90' folder.  This solution file and its related project files can be used to manufacture the library, dictionary compiler, the test module, and compile the dictionary source code.  Win32 and X64 platforms are both supported.  Use the 'Batch Build' feature of Visual Studio 2008 to first clean, and then build, all 20 projects.

Object library results of the build will be placed in one of the platform/configuration dependent sub-folders of the 'lib90' folder.  Resulting executable modules (including, perhaps, DLL's in the future) will be placed in the one of the platform/configuration dependent sub-folders of the 'bin90' folder.  All intermediate files produced during the build process will be stored in one of the sub-folders of the 'obj90' folder.


MS VC++ 2010 (Version 10):

The CS-MAP Open Source distribution will deposit in a folder named 'VC100' a Microsoft Visual C++ Version 10.0 (VS 2010) solution file.  This file references project files in the same 'VC100' folder.  This solution file and its related project files can be used to manufacture the library, dictionary compiler, the test module, and compile the dictionary source code.  Win32 and X64 platforms are both supported.  Use the 'Batch Build' feature of Visual Studio 2010 to first clean, and then build, all 20 projects.

Object library results of the build will be placed in one of the platform/configuration dependent sub-folders of the 'lib100' folder.  Resulting executable modules (including, perhaps, DLL's in the future) will be placed in the one of the platform/configuration dependent sub-folders of the 'bin100' folder.  All intermediate files produced during the build process will be stored in one of the sub-folders of the 'obj100' folder.

Defects and Enhancements

Please report any defects in the code, build process, and/or documentation using the Trac facility located at "trac.osgeo.org/csmap"  While CS-MAP shares a subversion repository with other coordinate system related products, it has its own defect tracking database at the indicated location.  You will need to create a login for yourself to record a bug, but anyone can create a login and there is no fee.  You can also use this means to request an enhancement to the product.

With regard to defects and/or enhancements, there are no guarantees that any action will be initiated in any given time frame.  As the product gains wider use, there will be more and more developers interested in a (relatively) bug free product and the chances of a timely correction or implementation will increase.

If you are a developer with experience in the area of coordinate system conversion and/or transformation, please consider becoming a contributor to the project.  Visit

	http://trac.osgeo.org/metacrs/

for more information.  Of course, you can send a patch to any of the listed commiters requesting that the change be officially implemented.  This us the fastest way to get a bug fixed.
