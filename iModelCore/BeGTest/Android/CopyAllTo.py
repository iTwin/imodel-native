import sys,shutil

if __name__ == '__main__':
    dest = len(sys.argv)-1

    for i in range(0,dest):
        shutil.copy (sys.argv[i], sys.argv[dest])
