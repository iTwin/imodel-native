#!Python
#--------------------------------------------------------------------------------------
#
#     $Source: mki/BisSchemasValidation/SchemaValidationRunner.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys, subprocess, csv

if (len(sys.argv) < 3): # Not enough arguments given
    print("This script will call schemavalidator.exe against given directories containing schemas.")
    print("Example call: python SchemaValidationRunner.py D:/SchemaValidator/  D:/MySchemas1/;D:/MySchemas2/ -r D:/AdditionalSchemaRef/  -o D:/LogOut/")
    print("")
    print("Arguments required:")
    print("VALIDATOR_EXE_DIR       - The 'schemavalidator.exe' directory")
    print("DIR0[;DIR1;...;DIRN]    - The directory containing desired schemas to validate seperated by semicolon. ")
    print("")
    print("Other options:")
    print("-r DIR0[;DIR1;...;DIRN] - Any additional schema reference directories. By default, all given schema directories will be used")
    print("-o OUTPUT_LOG_DIR       - The desired output log directory. By default, the output log will be the location of this script + \\ValidationLogs\\")
    sys.exit(9)

# Return a string separated by semicolons into a list 
def semiColonSeparatedToList(s):
    return s.split(';')
    
# Return a string of last directory in a given path with an added \ character
def getLastDirInPath(path):
    return os.path.basename(os.path.normpath(path)) + '\\'

# Formats the filename for use in SharePoint
def formattedFileName(fileName):
    formattedName = fileName.split('.')[0]
    return formattedName

validatorExeDir = sys.argv[1]
schemaDirs = semiColonSeparatedToList(sys.argv[2])
schemaRefList = schemaDirs[:] # By default, schema references should be all given schema dirs ([:] = shallow copy) 
logOutputDir = sys.path[0] + '\ValidationLogs\\' # Default log output dir. May be overrid below

# Rather than call our SharePoint Wrapper directly,
# we're going to write to a file that will later be parsed and uploaded to SharePoint

# Parse optional args supplied
if (len(sys.argv) > 4): 
    if (sys.argv[3] == '-r'): # -r given
        schemaRefList.extend(semiColonSeparatedToList(sys.argv[4]))
        if (len(sys.argv) == 7): # and -o given
            logOutputDir = sys.argv[6]
    elif (sys.argv[3] == '-o'): # just -o given
        logOutputDir = sys.argv[4]

if not os.path.exists(logOutputDir): # If validation log folder doesn't exist, create it
    os.makedirs(logOutputDir)

schemaRefString = ' -r ' + ' '.join(schemaRefList)
SCHEMA_EXTENSION = '.ecschema.xml';
totalNumSchemasFailed = 0
totalNumSchemasScanned = 0
failedSchemasList = []
logFilesList = []
validationFile = logOutputDir + 'Validation_results.csv'

# track validation for reporting
reportsStorage = {
    'SchemasScanned': 0,
    'SchemasFailed': {}
}

# ensure no old validationFiles exist
if os.path.isfile(validationFile):
    os.remove(validationFile)

def writeToValidationFile(schemaName, validationResult, fileToUpload='None'):
    if os.path.exists(logOutputDir + '\\' + 'Validation_results.csv'):
        with open(logOutputDir + '\\' + 'Validation_results.csv', 'a+') as resultFile:
            writer = csv.writer(resultFile)
            writer.writerow([schemaName, validationResult, fileToUpload])
    else:
        with open(logOutputDir + '\\' + 'Validation_results.csv', 'wb') as resultFile:
            writer = csv.writer(resultFile)
            writer.writerow(['Schema', 'Validation Result', 'File to Upload'])
            writer.writerow([schemaName, validationResult, fileToUpload])

reportFile = logOutputDir + 'schema_report.log'

# ensure no old reportFiles exist
if os.path.isfile(reportFile):
    os.remove(reportFile)

def writeReport(storage):
    """Iterate through our reportsStorage dictionary
    And write necessary information to a log file for later printing to stdout"""

    totalFails = 0
    failureString = ""
    with open(logOutputDir + '\\' + 'schema_report.log', 'w') as resultFile:
        totalSchemaOut = "total schemas checked: {0}\n".format(str(reportsStorage["SchemasScanned"]))
        currentDir = ""
        
        for schemaDir, schemaFails in reportsStorage['SchemasFailed'].items():
            numberOfFails = len(schemaFails)
            totalFails += int(numberOfFails)
            failureString += (str(numberOfFails) + " validation failure(s) in " + schemaDir + "\n")

            for sFail in schemaFails:
                failureString += (sFail + "\n")

        resultFile.write(totalSchemaOut)
        resultFile.write("total validation failure(s): " + str(totalFails) + '\n')
        resultFile.write(failureString)

print("==========================================================================================")

# Loop through all requested directories 
for dir in schemaDirs:
    print("Checking " + dir)
    print("==========================================================================================")
    numSchemasFailed = 0

    if not os.path.exists(dir):
        print("The schema directory '" + dir + "' does not exist")
        sys.exit(-1)

    if not os.path.exists(logOutputDir + getLastDirInPath(dir)): # Organize by schema dir
        os.makedirs(logOutputDir + getLastDirInPath(dir))

    for fileName in os.listdir(dir): # Loop through each file in each directory 
        if not fileName.endswith(SCHEMA_EXTENSION):
            continue

        logFileName = logOutputDir + getLastDirInPath(dir) + fileName.replace(SCHEMA_EXTENSION,'.validation.log')

        with open(logFileName, 'w') as logFile: # Create log
            os.chdir(validatorExeDir) # Change to appropriate dir to call validator exe. 
            exeCall = 'schemavalidator.exe -i ' + dir + fileName + schemaRefString
            totalNumSchemasScanned += 1
            outCode = subprocess.call(exeCall, stdout = logFile) # Get success code and print output to logfile instead of console 

            if (outCode != 0): # If an error occured, write to log file that an error occured 
                numSchemasFailed += 1
                logFile.write("--------------------------------------------------------------------------------------------------------------------------\n")
                logFile.write("The schema '" + fileName + "' FAILED validation. See ERRORs above for details.\n")
                logFile.write("--------------------------------------------------------------------------------------------------------------------------\n")
                print("[ FAILURE ] Failed to validate schema '" + fileName + "'.  See log at " + logFileName)
                
                # Store validation reports in a dictionary for later reporting.
                if dir in reportsStorage['SchemasFailed']:
                    reportsStorage['SchemasFailed'][dir].append("Failed to validate schema '" + fileName + "'.  See log at " + logFileName)
                else:
                    reportsStorage['SchemasFailed'][dir] = ["Failed to validate schema '" + fileName + "'.  See log at " + logFileName]
                
                # append the failed log file and logfilename for later writing/parsing to/from csv
                logFilesList.append([formattedFileName(fileName), logFile])
    
            else:
                logFile.write("--------------------------------------------------------------------------------------------------------------------------\n")
                logFile.write("The schema '" + fileName + "' SUCCEEDED validation.\n")
                logFile.write("--------------------------------------------------------------------------------------------------------------------------\n")
                print("[    OK   ] Validation of '" + fileName + "' was successful");
                
                # add this result to our validation file for updating SharePoint at a later time.
                writeToValidationFile(formattedFileName(fileName), 'Passed')

    if (numSchemasFailed > 0): # Show failure if exists
        print(str(numSchemasFailed) + " validation failure(s) in " + dir)
        totalNumSchemasFailed += numSchemasFailed
        
        # add this result to our validation file for updating SharePoint at a later time.                
        for eachFile in logFilesList:
            writeToValidationFile(eachFile[0], 'Failed', eachFile[1].name)

    reportsStorage['SchemasScanned'] = totalNumSchemasScanned
    writeReport(reportsStorage)
    
    print("==========================================================================================")