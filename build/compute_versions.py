#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import argparse, json, os, re, subprocess, sys

DEFAULT_VERSION = '99.99.99.99'

#----------------------------------------------------------------------------------------------------------------------------------------------------
def isDependentOf(config, stratMask, queryStrat):
    for stratConfig in config['strategies']:
        if (stratConfig['name'].lower() not in stratMask) and ('*' not in stratMask):
            continue

        if ('depends' not in stratConfig):
            continue

        depends = [s.lower() for s in stratConfig['depends']]
        if queryStrat.lower() in depends:
            print('    (dependency of ' + stratConfig['name'] + ')')
            return True

        for d in depends:
            if isDependentOf(config, d, queryStrat):
                return True

    return False

#----------------------------------------------------------------------------------------------------------------------------------------------------
def main():
    defaultConfigFile = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'config.json')

    argParser = argparse.ArgumentParser(description="Runs BB commands based on a JSON config file.")
    argParser.add_argument('-c', '--config', default=defaultConfigFile, help='Path to configuration file')
    argParser.add_argument('-o', '--output', required=True, help='Path to write version JSON data.')
    argParser.add_argument('-r', '--reserve', action='store_true', help='Whether to reserve the next version from the BDF server.')
    argParser.add_argument('-v', '--verbosity', default='4')
    argParser.add_argument('strategies', metavar='strategies', nargs='*', help='one or more strategies to officially version; can be "*"')

    args = argParser.parse_args()

    expandedStrategies = []
    for s in args.strategies:
        expandedStrategies.extend(s.split())
    args.strategies = expandedStrategies

    print('==================================================')
    print('compute_versions.py args:')
    print('    config = ' + (args.config if args.config else '<None>'))
    print('    output = ' + (args.output if args.output else '<None>'))
    print('    reserve = ' + (str(args.reserve) if args.reserve else '<None>'))
    print('    verbosity = ' + (args.verbosity if args.verbosity else '<None>'))
    print('    strategies = ' + ((','.join(args.strategies)) if args.strategies else '<None>'))
    print('==================================================')

    args.strategies = [s.lower() for s in args.strategies]

    with open(args.config, 'r') as configFile:
        config = json.load(configFile)

    verData = {}

    for stratConfig in config['strategies']:
        currStratName = stratConfig['name']
        ver = None

        print('Processing ' + stratConfig['name'] + '...')

        if not args.strategies:
            ver = DEFAULT_VERSION
            print('    No strategy mask => ' + ver)
        
        elif (currStratName.lower() not in args.strategies) and ('*' not in args.strategies) and (not isDependentOf(config, args.strategies, currStratName)):
            ver = DEFAULT_VERSION
            print('    Not in the strategy mask (or dependent of) => ' + ver)
        
        elif 'version_file' in stratConfig:
            with open(os.path.expandvars(stratConfig['version_file']), 'r') as verFile:
                ver = verFile.read().strip()

            while len(ver.split('.')) < 4:
                ver = ver + '.0'
        
            print('    Version from file ' + stratConfig['version_file'] + ' => ' + ver)

        elif 'version_seed' in stratConfig:
            reserveFlag = (' --reserve' if args.reserve else '')
            cmd = 'bb.bat -v {0} -s {1} prodversion -p {1} -n {2}{3}'.format(args.verbosity, currStratName, stratConfig['version_seed'], reserveFlag)
            bbNextVersion = subprocess.check_output(cmd.split(' '))
            
            verMatch = re.search(r'Next version:\s*(\d+\.\d+\.\d+\.\d+)', bbNextVersion)
            if verMatch:
                ver = verMatch.group(1)

            print('    Version from BDF server (from seed ' + stratConfig['version_seed'] + ') => ' + ver)
            print('    This version was ' + ('RESERVED' if args.reserve else 'NOT reserved') + ' from the BDF server.')

        if not ver:
            ver = DEFAULT_VERSION
            print('WARNING: No version seed or file was provided for ' + currStratName + ', so it will NOT be versioned => ' + ver)

        verData[currStratName] = ver
    
    outputDir = os.path.dirname(args.output)
    if not os.path.exists(outputDir):
        os.makedirs(outputDir)
    
    with open(args.output, 'w') as outputFile:
        json.dump(verData, outputFile)

    return 0

#----------------------------------------------------------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
