#-----------------------------------------------------------
# Generate script by Martynas.Saulius              04/2018
#-----------------------------------------------------------
import sys
import xml.etree.ElementTree as ET
import os.path

from collections import defaultdict

def Insert(standardDict, standard, name, category):
    if standard not in standardDict:
        standardDict[standard] = defaultdict(list)
    standardDict[standard][category].append(name)

if (len(sys.argv) < 2) or (not os.path.isfile(sys.argv[1])) or (sys.argv[1] == '--help'):
    print("GenerateClassificationInserts.py [--help] InputFilePath OutputFilePath\
\n\nTakes classification xml file, located in InputFilePath and parses it to C++ \
insert methods located at OutputFilePath\
\n\nOptions:\n  --help     Shows this help message and exit terminates this script")
    sys.exit()
tree = ET.parse(sys.argv[1])
root = tree.getroot()
f = open(sys.argv[2], 'w')
f.write('''/*--------------------------------------------------------------------------------------+
|
|  $Source: CodeGenerators/GenerateStandardRoomsInserts.py $
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


f.write('void ClassificationSystemsDomain::InsertStandardDefinitionSystems(Dgn::DgnDbR db) const\n')
f.write('    {\n')
f.write('    ClassificationSystemPtr system;\n')
f.write('    ClassificationGroupPtr group;\n')

roomDict = {}
for standardRoom in root.find('List').findall('StandardRoom'):
    name = standardRoom.find('Name').text
    category = standardRoom.find('Category').text
    calcStandard = standardRoom.find('CalculationStandard').text
    if calcStandard == 'eCIBSE':
        Insert(roomDict, 'CIBSE', name, category)
    elif calcStandard == 'eASHRAE':
        catalogue = standardRoom.find('Catalogue').text
        Insert(roomDict, ('ASHRAE'+catalogue), name, category)
for standard in roomDict:
    output = "    system = InsertSystem(db, \""+standard+"\");\n"
    f.write(output);
    for category in roomDict[standard]:
        output = "    group = InsertGroup(*system, \""+category+"\");\n"
        f.write(output);
        for name in roomDict[standard][category]:
            output = "    InsertClassification(*system, \""+name+"\", \""+ name +"\", \"\", group.get(), nullptr);\n"
            f.write(output);
f.write('    }\n\nEND_CLASSIFICATIONSYSTEMS_NAMESPACE\n')
f.close()
for standard in roomDict:
    print(standard + "\n")
    print(roomDict[standard])
    print("\n--------------------------------\n")
