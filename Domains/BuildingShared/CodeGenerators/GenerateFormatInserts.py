#-----------------------------------------------------------
# Generate script by Martynas.Saulius              04/2018
#-----------------------------------------------------------
import sys
import xml.etree.ElementTree as ET
import os.path
import re
import string
import ntpath
from datetime import datetime

helpOutput = """
GenerateFormatInserts.py [--help] XmlFilesDirectoryPath OutputFilePath SystemType

Takes classification xml file, located in InputFilePath and parses it to C++ insert methods located at OutputFilePath

Options:
  --help     Shows this help message and exit terminates this script
"""

fileTemplate = """/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//===========================================================================================
// WARNING: This is an automatically generated code for building classification inserts 
// WARNING: To generate, call "bb -r BuildingShared -f BuildingShared -p CodeGenerators b -c"
//===========================================================================================\n

#include <ClassificationSystems/ClassificationSystemsApi.h>
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>


namespace BS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

void ClassificationSystemsDomain::{functionName}(Dgn::DgnDbR db) const
    {{
    ClassificationSystemCPtr {classSystemVariableName} = TryAndGetSystem(db, "{systemType}");
    ClassificationTableCPtr {classTableVariableName} = TryAndGetTable(*{classSystemVariableName} , "{tableName}");

{code}
    }}

END_CLASSIFICATIONSYSTEMS_NAMESPACE
"""

levelcount = 0
codeOutput = ""

toCamelCase = lambda s: s[:1].lower() + s[1:] if s else ''

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


def ParseSection(section, level, systemType, classTableVariableName): 
    global levelcount
    global codeOutput

    for subsection in section.findall('Section'):
        if level is levelcount:
            codeOutput += "    ClassificationPtr subsection" + str(level) + systemType + ";\n"
            levelcount+=1

        if subsection.find('Name').text is None:
            return subsection.find('Code').text
        else:
            name = RemoveLF(EscapeChars(subsection.find('Name').text))
            code = RemoveLF(subsection.find('Code').text)
            for i in range(0, level):
				codeOutput +="    "
            codeOutput += "    subsection"+ str(level) + systemType + " = InsertClassification(*" + classTableVariableName + ", \"" + name + "\", \"" + code + "\", \"\", nullptr, "
	    
        if level is 0:
            codeOutput += "nullptr);\n"
        else: 
            codeOutput += "subsection" + str(level-1) + systemType + ".get());\n"
        
        if subsection.findall('Section'):
            level+=1
            status = ParseSection(subsection, level, systemType, classTableVariableName)
            if status is None:
                print("Parent " + subsection.find('Code').text + " child <null> was skipped")
            elif status is not 0:
                print("Parent " + subsection.find('Code').text + " child " + status + " was skipped")
            level-=1
    return 0

#TODO remove duplicate code at Recursion
types = ['UniFormat','MasterFormat','OmniClass']


if (len(sys.argv) < 2) or (not os.path.isfile(sys.argv[1])) or (sys.argv[1] == '--help') or (sys.argv[3] not in types):
    print(helpOutput)
    sys.exit()

systemType = sys.argv[3]
tree = ET.parse(sys.argv[1])
root = tree.getroot()
tp = ntpath.basename(sys.argv[2])
tp = re.sub("Generated", "", tp)
tp = re.sub("Inserts.cpp", "", tp)

classSystemVariableName = toCamelCase(systemType) + "System"
classTableVariableName = toCamelCase(systemType) + "Table"

ParseSection(root, 0, systemType, classTableVariableName)

filledTemplate = fileTemplate.format(
    generatedFileName = ntpath.basename(sys.argv[2]),
    currentYear = datetime.now().strftime('%Y'),
    functionName = "Insert{}Definitions".format(tp),
    systemType = systemType,
    classSystemVariableName = classSystemVariableName,
    classTableVariableName = classTableVariableName,
    tableName = root.find("Name").text,
    code = codeOutput
)

f = open(sys.argv[2], 'w')
f.write(filledTemplate)
f.close()


    
