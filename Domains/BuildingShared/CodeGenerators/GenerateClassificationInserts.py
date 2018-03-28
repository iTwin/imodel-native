import sys
import xml.etree.ElementTree as ET
import os.path

if (len(sys.argv) < 2) or (not os.path.isfile(sys.argv[1])) or (sys.argv[1] == '--help'):
    print("GenerateClassificationInserts.py [--help] InputFilePath OutputFilePath\
\n\nTakes classification xml file, located in InputFilePath and parses it to C++ \
insert methods located at OutputFilePath\
\n\nOptions:\n  --help     Shows this help message and exit terminates this script")
    sys.exit()
tree = ET.parse(sys.argv[1])
root = tree.getroot()
f = open(sys.argv[2], 'w')
f.write('void InsertDefinitionSystems(Dgn::DgnDbR db)\n')
f.write('    {\n')


for standardRoom in root.find('List').findall('StandardRoom'):
    output = '    '
    name = standardRoom.find('Name').text
    category = standardRoom.find('Category').text
    calcStandard = standardRoom.find('CalculationStandard').text
    if calcStandard == 'eCIBSE':
        output+= "InsertCIBSE"
    elif calcStandard == 'eASHRAE':
        catalogue = standardRoom.find('Catalogue').text
        output+= "InsertASHRAE"+catalogue
    output+="(db, \""+name+"\", \""+category+"\");\n"
    f.write(output)
f.write('    }\n')
f.close()
