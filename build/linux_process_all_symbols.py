import os, subprocess, sys

if not 'STRATS_TO_RELEASE' in os.environ:
    print('Skipping ALL directories because no strategies are being released.')
    sys.exit(0)

symbolScript = os.path.join(os.environ['SRCROOT'], 'imodel02', 'build', 'linux_process_symbols.py')
configs = [
    {
        'dir': os.path.join(os.environ['BUILD_BINARIESDIRECTORY'], 'LinuxX64', 'Product', 'iModelCoreNuget'),
        'strategy': 'iModelCore'
    },
    {
        'dir': os.path.join(os.environ['BUILD_BINARIESDIRECTORY'], 'LinuxX64', 'Product', 'iModelCoreTestingNuget'),
        'strategy': 'iModelCore'
    },
    {
        'dir': os.path.join(os.environ['BUILD_BINARIESDIRECTORY'], 'LinuxX64', 'imodeljsnodeaddon_pkgs', 'imodeljs-linux-x64'),
        'strategy': 'iModelJsNodeAddon',
        'upload': True,
        'glob': '*.node'
    },
    {
        'dir': os.path.join(os.environ['BUILD_BINARIESDIRECTORY'], 'LinuxX64', 'imodelbankaddon_pkgs', 'imodel-bank-linux-x64'),
        'strategy': 'iModelBankAddon',
        'glob': '*.node'
    }
]

def callOrRaise(cmd):
  if 0 != subprocess.call(cmd.split(' ')):
    raise RuntimeError('Failed to call "' + cmd + '"')

for config in configs:
    print('\nProcessing directory ' + config['dir'] + '...\n')

    if config['strategy'] not in os.environ['STRATS_TO_RELEASE']:
        print('Skipping ' + config['dir'] + ' because the ' + config['strategy'] + ' strategy is not being released.')
        continue
    
    glob = ' ' + config['glob'] if 'glob' in config else ''

    if 'upload' in config and config['upload']:
        callOrRaise('/usr/bin/env python {0} -d {1} compress{2}'.format(symbolScript, config['dir'], glob))
        callOrRaise('/usr/bin/env python {0} -d {1} -t {2} -o {3} -p {4} upload{5}'.format(symbolScript, config['dir'], os.environ['SENTRY_AUTH_TOKEN'], os.environ['SENTRY_ORG_SLUG'], os.environ['SENTRY_PROJECT_SLUG'], glob))

    callOrRaise('/usr/bin/env python {0} -d {1} strip{2}'.format(symbolScript, config['dir'], glob))
