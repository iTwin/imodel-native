#-----------------------------------------------------------
# Generate script by Martynas.Saulius              04/2018
#-----------------------------------------------------------
import sys
import xml.etree.ElementTree as ET
import os.path
from datetime import datetime
from collections import defaultdict

helpOutput = '''
GenerateClassificationInserts.py [--help] InputFilePath OutputFilePath

Takes classification xml file, located in InputFilePath and parses it to C++ insert methods located at OutputFilePath

Options:
    --help     Shows this help message and exit terminates this script
'''

fileTemplate = '''/*--------------------------------------------------------------------------------------+
|
|  {dollarSign}Source: Domain/GeneratedInserts/{generatedFileName} {dollarSign}
|
+--------------------------------------------------------------------------------------*/
//===========================================================================================
// WARNING: This is an automatically generated code for building classification inserts 
// WARNING: To generate, call "bb -r BuildingShared -f BuildingShared -p CodeGenerators b -c"
//===========================================================================================


#include "PublicApi/GeneratedInsertsApi.h"
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h> 


namespace BS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

void GeneratedInserts::InsertStandardDefinitionSystems(Dgn::DgnDbR db, Dgn::DgnModelCR model) const
    {{
    ClassificationSystemPtr system;
    ClassificationTablePtr table;
    ClassificationGroupPtr group;

{code}
    }}

END_CLASSIFICATIONSYSTEMS_NAMESPACE
'''

def Insert(standardDict, standard, name, category):
    if standard not in standardDict:
        standardDict[standard] = defaultdict(list)
    standardDict[standard][category].append(name)

if (len(sys.argv) < 2) or (not os.path.isfile(sys.argv[1])) or (sys.argv[1] == '--help'):
    print(helpOutput)
    sys.exit()

tree = ET.parse(sys.argv[1])
root = tree.getroot()
roomDict = {}
code = ""
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
    edition = standard[6:] if standard.startswith("ASHRAE") else ""
    code += '    system = InsertSystem(db, model, "{standard}", "{edition}");\n'.format(standard=standard, edition=edition)
    code += '    table = InsertTable(*system, "{standard} Table");\n'.format(standard=standard)

    for category in roomDict[standard]:
        code += '\n    group = InsertGroup(*table, "{category}");\n'.format(category = category)

        for name in roomDict[standard][category]:
            code += '    InsertClassification(*table, "{name}", "{name}", "", group.get(), nullptr);\n'.format(name = name)

outputPath = sys.argv[2]

filledTemplate = fileTemplate.format(
    generatedFileName = os.path.basename(outputPath),
    currentYear = datetime.now().strftime('%Y'),
    code = code,
    dollarSign = "$" # Needed, because template gets messed up when pushed to remote
)

f = open(outputPath, 'w')
f.write(filledTemplate)
f.close()
for standard in roomDict:
    print(standard + "\n")
    print(roomDict[standard])
    print("\n--------------------------------\n")