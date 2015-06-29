#!perl
use strict;

use File::Path;
use File::Copy;

my $distribDir = getArgValue("distribDir");
my $vortexAPIDir = $distribDir."\\VortexAPI";
my $removeClash = getArgValue("removeClash");
my $removeClientServer = getArgValue("removeClientServer");
my $removeMetaData = getArgValue("removeMetaData");
my $removeClipping = getArgValue("removeClipping");
my $removePDBs = getArgValue("removePDBs");



if ($removePDBs eq 1)
{
	printf "*** Removing PDBs\n";
	
	File::Path::remove_tree($vortexAPIDir."\\pdb");
}

if ($removeClash eq 1)
{
	printf "*** Removing clash module\n";
	
	removeExampleFolder("clash");
	
	File::Path::remove_tree($vortexAPIDir."\\lib");
	
	removeFromSolutionFile($vortexAPIDir."\\examples\\examples.sln", "Clash");
	removeFromSolutionFile($vortexAPIDir."\\examples\\examples_2012.sln", "Clash");
	
	removeStringFromFile($distribDir."\\PointoolsVortexAPI-ReleaseNotes-2.0.0.215.txt", "clash");
	removeStringFromFile($distribDir."\\PointoolsVortexAPI-ReleaseNotes-2.0.0.215.txt", "IClashTree");
	
	unlink($vortexAPIDir."\\include\\IClashNode.h") or die "\nFailed to remove file IClashNode.h\n\n";
	unlink($vortexAPIDir."\\include\\IClashObject.h") or die "\nFailed to remove file IClashObject.h\n\n";
	unlink($vortexAPIDir."\\include\\IClashObjectManager.h") or die "\nFailed to remove file IClashObjectManager.h\n\n";
	unlink($vortexAPIDir."\\include\\IClashTree.h") or die "\nFailed to remove file IClashTree.h\n\n";
	unlink($vortexAPIDir."\\include\\VortexObjects_import.h") or die "\nFailed to remove file IClashTree.h\n\n";
	
	unlink($vortexAPIDir."\\examples\\include\\IClashNode.h") or die "\nFailed to remove file IClashNode.h\n\n";
	unlink($vortexAPIDir."\\examples\\include\\IClashObject.h") or die "\nFailed to remove file IClashObject.h\n\n";
	unlink($vortexAPIDir."\\examples\\include\\IClashObjectManager.h") or die "\nFailed to remove file IClashObjectManager.h\n\n";
	unlink($vortexAPIDir."\\examples\\include\\IClashTree.h") or die "\nFailed to remove file IClashTree.h\n\n";
	unlink($vortexAPIDir."\\examples\\include\\VortexObjects_import.h") or die "\nFailed to remove file VortexObjects_import.h\n\n";
	
#	unlink($vortexAPIDir."\\examples\\src\\ClashTool.cpp") or die "\nFailed to remove file ClashTool.cpp\n\n";
}
if ($removeClientServer eq 1)
{
	printf "*** Removing client server module\n";
	
	removeExampleFolder("clientserver");
	
	removeFromSolutionFile($vortexAPIDir."\\examples\\examples.sln", "ClientServer");
	removeFromSolutionFile($vortexAPIDir."\\examples\\examples_2012.sln", "ClientServer");
	
	removeStringFromFile($distribDir."\\PointoolsVortexAPI-ReleaseNotes-2.0.0.215.txt", "streaming");
		
	removeStringFromFile($vortexAPIDir."\\include\\PointoolsVortexAPI_import.h", "ptProcessServerRequestClientID2");		
	removeStringFromFile($vortexAPIDir."\\include\\PointoolsVortexAPI_import.h", "ptSetViewportPointsBudget");
	removeStringFromFile($vortexAPIDir."\\include\\PointoolsVortexAPI_import.h", "ptGetViewportPointsBudget");
	removeStringFromFile($vortexAPIDir."\\include\\PointoolsVortexAPI_import.h", "ptGetViewportPointsBudget");	
	
	removeStringFromFile($vortexAPIDir."\\src\\PointoolsVortexAPI_import.cpp", "ptProcessServerRequestClientID2");		
	removeStringFromFile($vortexAPIDir."\\src\\PointoolsVortexAPI_import.cpp", "ptSetViewportPointsBudget");
	removeStringFromFile($vortexAPIDir."\\src\\PointoolsVortexAPI_import.cpp", "ptGetViewportPointsBudget");
	removeStringFromFile($vortexAPIDir."\\src\\PointoolsVortexAPI_import.cpp", "ptGetViewportPointsBudget");	

	# leave this in, it is referenced by other projects	
#	unlink($vortexAPIDir."\\examples\\src\\ClientServerTool.cpp") or die "\nFailed to remove file ClientServerTool.cpp\n\n";
}
if ($removeClipping eq 1)
{
	printf "*** Removing clipping module\n";
	
	removeExampleFolder("clipping");
	
	removeFromSolutionFile($vortexAPIDir."\\examples\\examples.sln", "Clipping");
	removeFromSolutionFile($vortexAPIDir."\\examples\\examples_2012.sln", "Clipping");
	
	unlink($vortexAPIDir."\\examples\\include\\ClippingTool.h") or die "\nFailed to remove file ClippingTool.h\n\n";
	
	unlink($vortexAPIDir."\\examples\\src\\ClippingTool.cpp") or die "\nFailed to remove file ClippingTool.cpp\n\n";
}
if ($removeMetaData)
{
	printf "*** Removing meta data example\n";
	
	removeExampleFolder("metadata");
	
	removeFromSolutionFile($vortexAPIDir."\\examples\\examples.sln", "MetaData ");
	removeFromSolutionFile($vortexAPIDir."\\examples\\examples_2012.sln", "MetaData ");
}


# Removes a folder from the "examples" area of the distribition, first arg passed should be the name of the folder to remove
sub removeExampleFolder
{	
	my $fullDir = $vortexAPIDir."\\examples\\".@_[0];
	printf "Removing folder ".$fullDir." \n";	
	File::Path::remove_tree($fullDir);
}

# Removes a project from a Visual Studio solution file, finds the project name and removes
# everything from the preceding "Project" tags and following "EndProject" tag
sub removeFromSolutionFile
{
	my $solutionFile = @_[0];
	my $projectName = @_[1];
	my $writeLine = 1;
	my $searchingForEnd = 0;
	
	
	printf "Removing \"".$projectName."\" from solution: ".$solutionFile."\n";
	
	if (open(INPUT,"< $solutionFile") != 1)
	{
		die "\nFAILED to open ".$solutionFile." -----------------------------------\n\n";
	}
	my $tempfile = $solutionFile.".tmp";
	if (open (TMP, "> $tempfile") != 1)
	{	
		die "\nFAILED to open ".$tempfile." -----------------------------------\n\n";
	}	
	while(<INPUT>) 
	{
		$writeLine = 1;
		
		# while searching for the EndProject tag do not write any lines, go back to writing lines after the
		# lines it is found on
		if ($searchingForEnd eq 1)
		{
			$writeLine = 0;
			if (m/\bEndProject\b/) # match exactly EndProject
			{
				$searchingForEnd = 0;
			}
		}
		
		# find the project name in the solution file
		if(m/"$projectName/) 
		{		
			# if the first string on this line is "Project" then remove the whole line and the next lines
			# including the first one found that contains "EndProject"
			if (index($_, "Project") eq 0)
			{
				$writeLine = 0;
				$searchingForEnd = 1;
			}		
		}				
		if ($writeLine eq 1)
		{
			print TMP $_;
		}
	}
	close(INPUT);
	close(TMP);
	rename $tempfile, $solutionFile; 
}

# remove the passed function definition from PointoolsVortexAPI_import.h 
sub removeFunctionFromImportHeader
{
	my $headerFile = $vortexAPIDir."\\include\\PointoolsVortexAPI_import.h";
	my $functionName = @_[0];
	my $writeLine = 1;
	
	printf "Removing ".$functionName." from solution: ".$headerFile."\n";
	
	if (open(INPUT,"< $headerFile") != 1)
	{
		die "\nFAILED to open ".$headerFile." -----------------------------------\n\n";
	}
	my $tempfile = $headerFile.".tmp";
	if (open (TMP, "> $tempfile") != 1)
	{	
		die "\nFAILED to open ".$tempfile." -----------------------------------\n\n";
	}	
	while(<INPUT>) 
	{
		$writeLine = 1;
		
		# find exactly the function name in the header file, don't write any lines containing the function name
		if(m/\b$functionName\b/i) 
		{					
			$writeLine = 0;				
		}		
		if ($writeLine eq 1)
		{
			print TMP $_;
		}
	}
	close(INPUT);
	close(TMP);
	rename $tempfile, $headerFile; 
}
# remove all lines containing the passed string (case insensitive) from the chosen file
# (this is pretty much the same as removeFunctionFromImportHeader() except you can pass a filename,
# I've kept them separate in case removeFunctionFromImportHeader() seems extra functionality added specific to 
# the import header)
sub removeStringFromFile
{
	my $filename = @_[0];
	my $removeString = @_[1];
	my $writeLine = 1;
	
	printf "Removing ".$removeString." from file: ".$filename."\n";
	
	if (open(INPUT,"< $filename") != 1)
	{
		die "\nFAILED to open ".$filename." -----------------------------------\n\n";
	}
	my $tempfile = $filename.".tmp";
	if (open (TMP, "> $tempfile") != 1)
	{	
		die "\nFAILED to open ".$tempfile." -----------------------------------\n\n";
	}	
	while(<INPUT>) 
	{
		$writeLine = 1;
		
		# find exactly the string to remove in the file, (case insensitive) don't write any lines containing the string
		if(m/\b$removeString\b/i) 
		{					
			$writeLine = 0;				
		}		
		if ($writeLine eq 1)
		{
			print TMP $_;
		}
	}
	close(INPUT);
	close(TMP);
	rename $tempfile, $filename; 
}

# Checks the command line arguments to see if one contains the string passed as a parameter to this subroutine followed by =<value>
# e.g. if command line was "include_clash=true", can call getArgValue("include_clash") and get true returned
# e.g. if command line was "license_system=Pointools", can call getArgValue("license_system") and get "Pointools" returned
# returns 0 if the passed argument was not found
sub getArgValue
{	
	foreach(@ARGV)
	{
		my $index = index($_, @_[0]);
		if ($index == 0)
		{
			my $arg = substr($_, $index);
			$index = index($arg, "=");
			if ($index > 0)
			{
				my $val = substr($arg, $index+1);					
				return $val;
			}
		}
	}
	
	return 0;
}
