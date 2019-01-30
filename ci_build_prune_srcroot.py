import datetime, os, shutil, time

pruneList = [
#       Dir to prune (in SrcRoot)       Valid date
    [   os.path.join('bin', 'mrsid'),   None    ]
]

for pruneData in pruneList:
    prunePath = os.path.join(os.environ['SRCROOT'], pruneData[0])
    pruneTime = time.mktime(pruneData[1].timetuple()) if pruneData[1] else None

    if os.path.exists(prunePath) and (not pruneTime or (os.path.getmtime(prunePath) < pruneTime)):
        print('Pruning directory ' + prunePath + '...')
        shutil.rmtree(prunePath)
