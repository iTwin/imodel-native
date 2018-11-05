import argparse, json, os, re, subprocess, sys

#----------------------------------------------------------------------------------------------------------------------------------------------------
def main():
    defaultConfigFile = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'ci_build_config.json')

    argParser = argparse.ArgumentParser(description="Runs BB commands based on a JSON config file.")
    argParser.add_argument('-c', '--config', default=defaultConfigFile, help='Path to configuration file')
    argParser.add_argument('-o', '--output', help='Path to write version JSON data.')
    argParser.add_argument('-v', '--verbosity', default='3')

    args = argParser.parse_args()

    isForRelease = ('IS_FOR_RELEASE' in os.environ and 'true' == os.environ['IS_FOR_RELEASE'])
    if isForRelease and not 'SUBNET_PRG' in os.environ:
        sys.stderr.write('Release builds must be done in the "PRG iModel AU" agent queue.\n')
        sys.exit(1)
    elif not isForRelease and 'SUBNET_PRG' in os.environ:
        sys.stderr.write('CI builds must be done in the "iModelTechCI" agent queue.\n')
        sys.exit(1)

    with open(args.config, 'r') as configFile:
        config = json.load(configFile)

    verDict = dict()

    for stratConfig in config['strategies']:
        if not 'version_env' in stratConfig:
            continue

        ver = None

        if not isForRelease:
            ver = '99.99.99.99'

        elif 'version_file' in stratConfig:
            with open(os.path.expandvars(stratConfig['version_file']), 'r') as verFile:
                ver = verFile.read().strip()

            while len(ver.split('.')) < 4:
                ver = ver + '.0'
        
        elif 'version_seed' in stratConfig:
            cmd = 'bb.bat -v {0} -s {1} prodversion -p {2} -n {3}'.format(
                args.verbosity,
                stratConfig['name'],
                stratConfig['name'],
                stratConfig['version_seed'])
            
            bbNextVersion = subprocess.check_output(cmd.split(' '))
            verMatch = re.search(r'Next version:\s*(\d+\.\d+\.\d+\.\d+)', bbNextVersion)
            if verMatch:
                ver = verMatch.group(1)

        if not ver:
            sys.stderr.write('Could not determine version from file or BB.\n')
            sys.exit(1)

        print('Writing version {0}={1}'.format(stratConfig['version_env'], ver))
        verDict[stratConfig['version_env']] = ver
    
    outputDir = os.path.dirname(args.output)
    if not os.path.exists(outputDir):
        os.makedirs(outputDir)
    
    with open(args.output, 'w') as outputFile:
        json.dump(verDict, outputFile)

    return 0

#----------------------------------------------------------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
