import argparse, json, os, subprocess, sys, time

#----------------------------------------------------------------------------------------------------------------------------------------------------
def doubleToTimeString(start, end):
    d = (end - start)

    if d < 60:
        return "{:0.2f} second(s)".format(d)
    
    if d < 60*60:
        return str(d/60.0) + " minutes(s)"

    return str(d/(60.0*60.0)) + " hour(s)"

#----------------------------------------------------------------------------------------------------------------------------------------------------
def getBBCmd():
    # VSTS pushes variables into the environment CAPITALIZED. Unix environment variables are case-sensitive; remap what we know we care about.
    if os.name != 'nt':
        if not 'SrcRoot' in os.environ:
            os.environ['SrcRoot'] = os.environ['SRCROOT']
        if not 'OutRoot' in os.environ:
            os.environ['OutRoot'] = os.environ['OUTROOT']

    return 'python ' + os.path.join(os.environ['SrcRoot'], 'BentleyBuild', 'BentleyBuild.py')

#----------------------------------------------------------------------------------------------------------------------------------------------------
def callForPull(args, config):
    callTimes = {}
    status = 0

    allArchs = []
    allStrats = []

    # If a single architecture is requested, use only it and filter (still support multiple strategies).
    # Otherwise glom the architectures together.

    if args.arch:
        allArchs.append(args.arch)

    for stratConfig in config['strategies']:
        if args.arch and (not args.arch.lower() in stratConfig['archs'].lower().split('+')):
            continue
        
        if not args.arch:
            for arch in stratConfig['archs'].split('+'):
                if not arch in allArchs:
                    allArchs.append(arch)

        strat = stratConfig['name']
        
        if ('augments' in stratConfig) and stratConfig['augments']:
            strat += ';' + stratConfig['augments']
        
        strat += ';' + config['pull_augment']

        allStrats.append(strat)

    cmd = getBBCmd() + ' -v {0} -a "{1}" -s "{2}" pull'.format(args.verbosity, '+'.join(allArchs), '+'.join(allStrats))

    if args.bdfdir:
        # BDF names must be lower-case because BentleyBootstrap.py always lower-cases its input, which affects case-sensitive file systems.
        cmd += ' -r ' + os.path.join(args.bdfdir, stratConfig['name'].lower() + '.xml')

    print(cmd)
    cmdStartTime = time.time()
    status = subprocess.call(cmd, shell=True)
    callTimes['Seed Pull'] = doubleToTimeString(cmdStartTime, time.time())
    
    # On Linux, `bash` seems to return 256 on an error, regardless of what the SH script actually returns.
    # Linux + VSTS agent only expects error codes 0..255.
    # I don't think the actual code is important here, so remap to 0/1.
    if status != 0:
        status = 1

    return (callTimes, status)

#----------------------------------------------------------------------------------------------------------------------------------------------------
def callEachStrategy(args, config, verData):
    callTimes = {}
    status = 0

    isFirstStrategy = True

    for stratConfig in config['strategies']:
        print('Processing ' + stratConfig['name'] + '...')
        
        if args.arch and not args.arch.lower() in stratConfig['archs'].lower().split('+'):
            continue

        bbStrats = stratConfig['name']
        if ('augments' in stratConfig) and stratConfig['augments']:
            bbStrats += ';' + stratConfig['augments']

        version = None
        if stratConfig['name'] in verData:
            version = str(verData[stratConfig['name']])
            print('Using version ' + version)
            
            versionSplit = version.split('.')
            os.environ['REL_V'] = versionSplit[0].rjust(2, '0')
            os.environ['MAJ_V'] = versionSplit[1].rjust(2, '0')
            os.environ['MIN_V'] = versionSplit[2].rjust(2, '0')
            os.environ['SUBMIN_V'] = versionSplit[3].rjust(2, '0')
        else:
            print('Could not find a version for ' + stratConfig['name'] + ' in version data, using 99.99.99.99.')
            os.environ['REL_V'] = '99'
            os.environ['MAJ_V'] = '99'
            os.environ['MIN_V'] = '99'
            os.environ['SUBMIN_V'] = '99'

        noArch = False
        bbEnv = None

        if 'bdf' == args.action:
            # If present, associate the BDF with a PRG ID for release tracking.
            if ('prg_id' in stratConfig) and stratConfig['prg_id']:
                os.environ['PrgProductId'] = str(stratConfig['prg_id'])

            # BDF names must be lower-case because BentleyBootstrap.py always lower-cases its input, which affects case-sensitive file systems.
            bdfPath = os.path.join(args.bdfdir, stratConfig['name'].lower() + '.xml')
            action = 'taglist -f ' + bdfPath
        elif 'build' == args.action:
            action = 'build'
            if isFirstStrategy:
                action += ' --tmrbuild --noprompt'
        elif 'checkunused' == args.action:
            if not version or '99.99.99.99' == version:
                print('WARNING: No valid version was computed for {0}, so it will NOT be validated.'.format(stratConfig['name']))
                continue

            action = 'prodversion -p ' + stratConfig['name'] + ' -u ' + version
            noArch = True
        elif 'createnugets' == args.action:
            action = 'savenuget --noStream'
        elif 'createinstallers' == args.action:
            if os.name != 'nt':
                print('INFO: Not creating installers on this non-Windows platform.')
                continue

            # Installers are expensive... try to short-circuit.
            if ('has_installers' not in stratConfig) or (stratConfig['has_installers'] != 'true'):
                print('INFO: Not creating installers for ' + stratConfig['name'] + ' because configuration does not indicate it creates any.')
                continue
            
            if ('STRATS_TO_RELEASE' not in os.environ) or ((stratConfig['name'] not in os.environ['STRATS_TO_RELEASE']) and (os.environ['STRATS_TO_RELEASE'] != '*')):
                print('INFO: Not creating installers for ' + stratConfig['name'] + ' because it is not in STRATS_TO_RELEASE (' + os.environ['STRATS_TO_RELEASE'] + ').')
                continue

            # Build agents are non-admin services... not sure how to support WIX ICE validators yet...
            print("WARNING: SKIPPING WIX INSTALLER ICE VALIDATION")
            bbEnv = os.environ.copy()
            bbEnv['INSTALLER_SKIP_VALIDATION'] = '1'

            action = 'buildinstallset'
        else:
            action = args.action

        archArg = ('-a ' + (args.arch if args.arch else stratConfig['archs'])) if not noArch else ''

        cmd = getBBCmd() + ' -v {0} -s "{1}" {2} {3}'.format(args.verbosity, bbStrats, archArg, action)
        
        print(cmd)
        cmdStartTime = time.time()
        status = subprocess.call(cmd, shell=True, env=(bbEnv if bbEnv else os.environ))
        callTimes[stratConfig['name']] = doubleToTimeString(cmdStartTime, time.time())
        
        # On Linux, `bash` seems to return 256 on an error, regardless of what the SH script actually returns.
        # Linux + VSTS agent only expects error codes 0..255.
        # I don't think the actual code is important here, so remap to 0/1.
        if status != 0:
            status = 1
            break
        
        isFirstStrategy = False

    return (callTimes, status)

#----------------------------------------------------------------------------------------------------------------------------------------------------
def main():
    defaultConfigFile = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'config.json')
    defaultVersionsFile = os.path.join(os.environ['SYSTEM_ARTIFACTSDIRECTORY'], 'bdf', 'versions.json') if 'SYSTEM_ARTIFACTSDIRECTORY' in os.environ else ''

    argParser = argparse.ArgumentParser(description="Runs BB commands based on a JSON config file.")
    argParser.add_argument("action",                                        help='One of pull|build|bdf|checkunused|createnugets|createinstallers')
    argParser.add_argument("-c", "--config", default=defaultConfigFile,     help='Path to configuration file')
    argParser.add_argument("-r", "--versions", default=defaultVersionsFile, help='(!pull) Path to versions file')
    argParser.add_argument("-s", "--strat",                                 help='Overrides configuration to use a specific build strategy')
    argParser.add_argument("-a", "--arch",                                  help='Limits actions to strategies enabled for given arch')
    argParser.add_argument("-b", "--bdfdir",                                help='(bdf) Directory to write BDF files to -or- (pull) Directory where BDF files are stored')
    argParser.add_argument("-v", "--verbosity", default='3')

    args = argParser.parse_args()

    print('==================================================')
    print('call_bb.py args:')
    print('    action = ' + (args.action if args.action else '<None>'))
    print('    config = ' + (args.config if args.config else '<None>'))
    print('    versions = ' + (args.versions if args.versions else '<None>'))
    print('    strat = ' + (args.strat if args.strat else '<None>'))
    print('    arch = ' + (args.arch if args.arch else '<None>'))
    print('    bdfdir = ' + (args.bdfdir if args.bdfdir else '<None>'))
    print('    verbosity = ' + (args.verbosity if args.verbosity else '<None>'))
    print('==================================================')

    args.action = args.action.lower()

    with open(args.config, 'r') as configFile:
        config = json.load(configFile)

    print('Parsed configurations for:')
    for stratConfig in config['strategies']:
        print('    ' + stratConfig['name'])
    print('==================================================')

    if os.path.exists(args.versions):
        with open(args.versions, 'r') as verDataFile:
            verData = json.load(verDataFile)
    else:
        verData = {}

    print('Parsed version data for:')
    for k, v in verData.iteritems():
        print('    ' + k + ' = ' + v)
    print('==================================================')

    if args.strat:
        if not args.arch:
            print('ERROR: You must provide an architecture if you provide a strategy override.')
            return 1

        config['strategies'] = [{
            "name": args.strat,
            "archs": args.arch,
            "version_env": "VER_INVALID",
            "version_seed": "0.0.0"
        }]

        print('Overriding configuration with startegy ' + args.strat + ' and architecture ' + args.arch + '.')
        print('==================================================')

    totalTimeStart = time.time()

    # Utilize BB's multi-strategy/architecture optimizations during a pull.
    if 'pull' == args.action:
        (callTimes, status) = callForPull(args, config)
    else:
        (callTimes, status) = callEachStrategy(args, config, verData)
    
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
