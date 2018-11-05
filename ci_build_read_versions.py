import argparse, json, os, sys

#----------------------------------------------------------------------------------------------------------------------------------------------------
def set_vsts_env(key, value): print('##vso[task.setvariable variable={0}]{1}'.format(key, value))

#----------------------------------------------------------------------------------------------------------------------------------------------------
def main():
    defaultConfigFile = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'ci_build_config.json')

    argParser = argparse.ArgumentParser(description="Runs BB commands based on a JSON config file.")
    argParser.add_argument('-c', '--config', default=defaultConfigFile, help='Path to configuration file')
    argParser.add_argument('-i', '--input', help='Path to read version JSON data.')

    args = argParser.parse_args()

    with open(args.config, 'r') as configFile:
        config = json.load(configFile)

    with open(args.input, 'r') as verDataFile:
        verData = json.load(verDataFile)

    for stratConfig in config['strategies']:
        if not 'version_env' in stratConfig or not stratConfig['version_env'] in verData:
            continue

        print('Read version {0}={1}'.format(stratConfig['version_env'], verData[stratConfig['version_env']]))
        set_vsts_env(stratConfig['version_env'], verData[stratConfig['version_env']])

    return 0

#----------------------------------------------------------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
