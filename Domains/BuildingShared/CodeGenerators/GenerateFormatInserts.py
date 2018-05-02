#-----------------------------------------------------------
# Generate script by Martynas.Saulius              04/2018
#-----------------------------------------------------------
import sys
import xml.etree.ElementTree as ET
import os.path
import re
import string
import ntpath

levelcount = 0

func = lambda s: s[:1].lower() + s[1:] if s else ''

def CheckFiles(Files):
    for f in Files:
        if not os.path.isfile(f):
            return 1
    return 0

def EscapeChars(string):
    string = string.encode('ascii', 'backslashreplace')
    string = re.sub(r'\\', r'\\\\', string)
    string = re.sub(r'"', r'\\"', string)
    return string

def RemoveLF(string):
     return re.sub('\n', " ", string)


def ParseSection(section, level, t): 
    for subsection in section.findall('Section'):
        if level is levelcount:
            output ="    ClassificationPtr subsection" + str(level) + t + ";\n"
            f.write(output)
            global levelcount
            levelcount+=1
        if subsection.find('Name').text is None:
            return subsection.find('Code').text
        else:
            output = ""
            name = RemoveLF(EscapeChars(subsection.find('Name').text))
            code = RemoveLF(subsection.find('Code').text)
            for i in range(0, level):
				output+="    "
            output += "    subsection"+ str(level) + t + " = InsertClassification(*" + func(t) + "System, \"" + name + "\", \"" + code + "\", \"\", nullptr, "
	    if level is 0:
                output += "nullptr);\n"
            else: 
                output += "subsection" + str(level-1) + t + ".get());\n"
        f.write(output)
        if subsection.findall('Section'):
            level+=1
            status = ParseSection(subsection, level, t)
            if status is None:
                print("Parent " + subsection.find('Code').text + " child <null> was skipped")
            elif status is not 0:
                print("Parent " + subsection.find('Code').text + " child " + status + " was skipped")
            level-=1
    return 0
#TODO remove duplicate code at Recursion
types = ['UniFormat','MasterFormat','OmniClass']


if (len(sys.argv) < 2) or (not os.path.isfile(sys.argv[1])) or (sys.argv[1] == '--help') or (sys.argv[3] not in types):
    print("GenerateFormatInserts.py [--help] XmlFilesDirectoryPath OutputFilePath\
\n\nTakes classification xml file, located in InputFilePath and parses it to C++ \
insert methods located at OutputFilePath\
\n\nOptions:\n  --help     Shows this help message and exit terminates this script")
    sys.exit()
t = sys.argv[3]
#fileList = list(filter(None, fileList))
f = open(sys.argv[2], 'w')
f.write('''/*--------------------------------------------------------------------------------------+
|
|  $Source: CodeGenerators/GenerateFormatInserts.py $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//===========================================================================================
// WARNING: This is an automatically generated code for building classification inserts 
// WARNING: To generate, call "bb -r BuildingShared -f BuildingShared -p CodeGenerators b -c"
//===========================================================================================\n

#include <ClassificationSystems/ClassificationSystemsApi.h>
#include <DgnClientFx/DgnClientApp.h>
#include <BuildingShared\BuildingSharedApi.h>


namespace BS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE\n''')

tree = ET.parse(sys.argv[1])
root = tree.getroot()
tp = ntpath.basename(sys.argv[2])
tp = re.sub("Generated", "", tp)
tp = re.sub("Inserts.cpp", "", tp)
f.write('void ClassificationSystemsDomain::Insert' + tp + 'Definitions(Dgn::DgnDbR db) const\n')
f.write('    {\n')
f.write('    ClassificationSystemCPtr ' + func(t) + 'System = TryAndGetSystem(db, "' + t + '");\n')
ParseSection(root, 0, t)
f.write('    }\n\nEND_CLASSIFICATIONSYSTEMS_NAMESPACE\n')
f.close()


    
