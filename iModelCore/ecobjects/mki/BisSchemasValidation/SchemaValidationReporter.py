import sys

if len(sys.argv) < 2:
    print("This script parses a schema_report.log file to give feedback on which schemas failed validation")
    print("The functionality was extracted from SchemaValidationRunner.py to ensure that ValidateSchemas.mki completes")
    print("Before the extraction, SchemaValidationRunner.py would exit(-1) and prevent the rest of ValidateSchemas.mki from running")
    print("Example call: python SchemaValidationReporter.py LOCATION_OF_LOG_FILE")
    print("")
    print("Arguments required:")
    print("LOCATION_OF_LOG_FILE    - The 'schema_report.log' file this script parses. ex: C:/source/bim0200dev/BISOut/Winx64/Product/AllBisSchemas/ValidationLogs/schema_report.log")
    sys.exit(9)

logFile = sys.argv[1]

def noValidationFailures(logLine):
    "total validation failure(s)" in logLine and int(logLine.split('total validation failure(s): ')[1]) == 0

with open(logFile, 'r') as log:
    for line in log:
        if noValidationFailures(line):
            sys.exit(0) # No failures? We out.
        else:
            print(line)
    sys.exit(-1) # if we haven't exited with code 0 by now, then we definitely have an error