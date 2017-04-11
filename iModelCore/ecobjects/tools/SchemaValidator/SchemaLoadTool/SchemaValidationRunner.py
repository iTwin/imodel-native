import glob, os, sys, subprocess

if (len(sys.argv) < 4):
    print("The executable dir, schemas dir, and schema reference dir are needed as arguments. A fourth, optional argument is accepted to specify the output log directory")
    sys.exit()
    
validatorExeDir = sys.argv[1] # schemavalidator.exe dir
schemaDir = sys.argv[2] 
schemaRef = sys.argv[3]

# Optional 3rd argument as log output location
if (len(sys.argv) > 4):
    logOutputDir = sys.argv[4] 
else:
    logOutputDir = sys.path[0] + '\ValidationLogs\\' # Output logs to a validation folder in the current dir if no path is specified
 
schemaExtension = '.ecschema.xml';

for fileName in os.listdir(schemaDir):
    if fileName.endswith(schemaExtension):
        if not os.path.exists(logOutputDir): # If validation log folder doesn't exist, create it
            os.makedirs(logOutputDir)
        logFileName = logOutputDir + fileName + '.log'
        logFile = open(logFileName, 'w') # Create log
        os.chdir(validatorExeDir) # Change to appropriate dir to call validator exe. 
        exeCall = 'schemavalidator.exe -i ' + schemaDir + fileName + ' -r ' + schemaRef
        outCode = subprocess.call(exeCall, stdout = logFile) # Get success code and print output to logfile instead of console 
        print("Created " + logFileName)
        if (outCode != 0): # If an error occured, write to log file that an error occured 
            print("There was an error validating schema" + fileName + ".  See log file at " + logFileName + " for more") # Print error to console
            with open(logFileName, "a") as appendedFile:
                appendedFile.write("-------------------------------------------------------------------------------------------------------------------\n")
                appendedFile.write("-------------------------------------------------------------------------------------------------------------------\n")
                appendedFile.write("The schema '" + fileName + "' FAILED validation. See above for details\n")
                appendedFile.write("-------------------------------------------------------------------------------------------------------------------\n")
                appendedFile.write("-------------------------------------------------------------------------------------------------------------------\n")

        with open(logFileName, 'r') as original: data = original.read()
        with open(logFileName, 'w') as prependedFile:
            prependedFile.write("-------------------------------------------------------------------------------------------------------------------\n")
            prependedFile.write("-------------------------------------------------------------------------------------------------------------------\n")
            prependedFile.write("Attempting to validate '" + fileName + "'...\n")
            prependedFile.write("-------------------------------------------------------------------------------------------------------------------\n")
            prependedFile.write("-------------------------------------------------------------------------------------------------------------------\n" + data)
        
        logFile.close()