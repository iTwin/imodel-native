#--------------------------------------------------------------------------------------
#
#     $Source: CommonTasks/printing.py $
#
#  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    07/2017
#-------------------------------------------------------------------------------------------
def printColored(string, color, bold):
    colorCodes = {'red' : '31', 'green' : '32', 'cyan' : '36'}
    attr = []
    attr.append(colorCodes[color.lower()])
    if bold:
        attr.append('1')
    return '\x1b[%sm%s\x1b[0m' % (';'.join(attr), string)

