import argparse, json, os, sys, time

#----------------------------------------------------------------------------------------------------------------------------------------------------
def doubleToTimeString(start, end):
    d = (end - start)

    if d < 60:
        return "{:0.2f} second(s)".format(d)
    
    if d < 60*60:
        return str(d/60.0) + " minutes(s)"

    return str(d/(60.0*60.0)) + " hour(s)"

#----------------------------------------------------------------------------------------------------------------------------------------------------
def main():
    defaultConfigFile = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'ci_build_config.json')

    argParser = argparse.ArgumentParser(description="Runs BB commands based on a JSON config file.")
    argParser.add_argument("action",                                    help='One of pull|build|bdf|checkunused|createnugets')
    argParser.add_argument("-c", "--config", default=defaultConfigFile, help='Path to configuration file')
    argParser.add_argument("-a", "--arch",                              help='Limits actions to strategies enabled for given arch')
    argParser.add_argument("-b", "--bdfdir",                            help='(bdf) Directory to write BDF files to -or- (pull) Directory where BDF files are stored')
    argParser.add_argument("-v", "--verbosity", default='3')

    args = argParser.parse_args()
    args.action = args.action.lower()

    with open(args.config, 'r') as configFile:
        config = json.load(configFile)

    callTimes = {}
    status = 0

    totalTimeStart = time.time()

    for stratConfig in config['strategies']:
        if args.arch and not args.arch.lower() in stratConfig['archs'].lower().split('+'):
            continue

        bbStrats = stratConfig['name']
        if stratConfig['augments']:
            bbStrats += ';' + stratConfig['augments']

        if 'version_env' in stratConfig and stratConfig['version_env'] in os.environ:
            version = os.environ[stratConfig['version_env']].split('.')
            print('Using version ' + '.'.join(version) + ' from ' + stratConfig['version_env'])
            os.environ['REL_V'] = version[0].rjust(2, '0')
            os.environ['MAJ_V'] = version[1].rjust(2, '0')
            os.environ['MIN_V'] = version[2].rjust(2, '0')
            os.environ['SUBMIN_V'] = version[3].rjust(2, '0')

        noArch = False

        if 'bdf' == args.action:
            # BDF names must be lower-case because BentleyBootstrap.py always lower-cases its input, which affects case-sensitive file systems.
            bdfPath = os.path.join(args.bdfdir, stratConfig['name'].lower() + '.xml')
            action = 'taglist -f ' + bdfPath
        elif 'pull' == args.action:
            action = 'pull'
            if args.bdfdir:
                # BDF names must be lower-case because BentleyBootstrap.py always lower-cases its input, which affects case-sensitive file systems.
                action += ' -r ' + os.path.join(args.bdfdir, stratConfig['name'].lower() + '.xml')
            if config['pull_augment']:
                bbStrats += ';' + config['pull_augment']
        elif 'build' == args.action:
            action = 'build'
        elif 'checkunused' == args.action:
            if not 'version_env' in stratConfig:
                print('WARNING: {0} has no version_env value, so will NOT be validated.'.format(stratConfig['name']))
                continue

            if not stratConfig['version_env'] in os.environ:
                print('WARNING: {0} is NOT in the environment for {1}, so will NOT be validated.'.format(stratConfig['version_env'], stratConfig['name']))
                continue

            action = 'prodversion -p ' + stratConfig['name'] + ' -u ' + os.environ[stratConfig['version_env']]
            noArch = True
        elif 'createnugets' == args.action:
            action = 'savenuget --noStream'
        
        if os.name == 'nt':
            bbCmd = 'bb'
        elif os.name == 'posix':
            bbCmd = 'bash $SRCROOT/bsicommon/BuildAgentScripts/unix_runBB.sh'
        else:
            sys.stderr.write('Unknown OS name "' + os.name + '".\n')
            sys.exit(1)

        archArg = ('-a ' + (args.arch if args.arch else stratConfig['archs'])) if not noArch else ''

        cmd = bbCmd + ' -v {0} -s "{1}" {2} {3}'.format(
            args.verbosity,
            bbStrats,
            archArg,
            action)
        
        print(cmd)
        cmdStartTime = time.time()
        status = os.system(cmd)
        callTimes[stratConfig['name']] = doubleToTimeString(cmdStartTime, time.time())
        
        # On Linux, `bash` seems to return 256 on an error, regardless of what the SH script actually returns.
        # Linux + VSTS agent only expects error codes 0..255.
        # I don't think the actual code is important here, so remap to 0/1.
        if status != 0:
            status = 1
            break
    
    print('--------------------------------------------------')
    for cmd in callTimes:
        print('{0} of {1} took {2}'.format(args.action, cmd, callTimes[cmd]))

    print('')
    print('Total time: ' + doubleToTimeString(totalTimeStart, time.time()))
    print('--------------------------------------------------')

    return status

#----------------------------------------------------------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
