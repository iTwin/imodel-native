#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

from ctypes import windll, Structure, c_short, c_ushort, byref, wintypes
from contextlib import contextmanager

SHORT = c_short
WORD = c_ushort

class COORD(Structure):
    """struct in wincon.h."""
    _fields_ = [
        ("X", SHORT),
        ("Y", SHORT)]

class SMALL_RECT(Structure):
    """struct in wincon.h."""
    _fields_ = [
        ("Left", SHORT),
        ("Top", SHORT),
        ("Right", SHORT),
        ("Bottom", SHORT)]

class CONSOLE_SCREEN_BUFFER_INFO(Structure):
    """struct in wincon.h."""
    _fields_ = [
        ("dwSize", COORD),
        ("dwCursorPosition", COORD),
        ("wAttributes", WORD),
        ("srWindow", SMALL_RECT),
        ("dwMaximumWindowSize", COORD)]

# winbase.h
STD_INPUT_HANDLE = -10
STD_OUTPUT_HANDLE = -11
STD_ERROR_HANDLE = -12

# These are only here to help with further modifications/understanding where all of this came from.
# Don't use these.

# On windows these are actually indices into the shell; if custom colors were set then these names
# will not reflect the actual colors.

# wincon.h
#FOREGROUND_BLACK     = 0x0000
#FOREGROUND_BLUE      = 0x0001
#FOREGROUND_GREEN     = 0x0002
#FOREGROUND_CYAN      = 0x0003
#FOREGROUND_RED       = 0x0004
#FOREGROUND_MAGENTA   = 0x0005
#FOREGROUND_YELLOW    = 0x0006
#FOREGROUND_GREY      = 0x0007
#FOREGROUND_INTENSITY = 0x0008 # foreground color is intensified.

#BACKGROUND_BLACK     = 0x0000
#BACKGROUND_BLUE      = 0x0010
#BACKGROUND_GREEN     = 0x0020
#BACKGROUND_CYAN      = 0x0030
#BACKGROUND_RED       = 0x0040
#BACKGROUND_MAGENTA   = 0x0050
#BACKGROUND_YELLOW    = 0x0060
#BACKGROUND_GREY      = 0x0070
#BACKGROUND_INTENSITY = 0x0080 # background color is intensified.

BLACK           = 0x0000
WHITE           = 0x000F
UNCHANGED       = None

# This is supposedly to support x64 better; force it to be a handle.
windll.kernel32.GetStdHandle.restype = wintypes.HANDLE

__stdoutHandle = windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
# Store the original console color so we don't have to look it up constantly.
__defaultConsoleColor = None

def __getDefaultConsoleColors():

    # This method sets it the first time through and after that it just returns the base colors.
    global __defaultConsoleColor
    if __defaultConsoleColor != None:
        return __defaultConsoleColor

    csbi = CONSOLE_SCREEN_BUFFER_INFO()

    result = windll.kernel32.GetConsoleScreenBufferInfo(__stdoutHandle, byref(csbi))
    if result == 0 and __reopenStdOut(result):
        result = windll.kernel32.GetConsoleScreenBufferInfo(__stdoutHandle, byref(csbi))

        if result == 0:
            # return default, grey on black
            __defaultConsoleColor = 0x07

    __defaultConsoleColor = csbi.wAttributes
    return __defaultConsoleColor

def __reopenStdOut (result):
    global __stdoutHandle

    if result == 0:
        error = windll.kernel32.GetLastError()
        # When the error is ERROR_INVALID_HANDLE, reopen stdout.
        # Same handle is returned, but it has all needed access rights
        if error == 6:
            __stdoutHandle = windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
            return True
    return False

def __setConsoleTextColor (colorAttr):
    result = windll.kernel32.SetConsoleTextAttribute (__stdoutHandle, colorAttr)
    if result == 0 and __reopenStdOut(result):
        windll.kernel32.SetConsoleTextAttribute(__stdoutHandle, colorAttr)

# This is used in a "with" statement so that it sets and unsets
#  the color. It is now always unset back to the original console
#  color since that is more stable. Every line sets its own color
#  so it really is not a push/pop stack.
@contextmanager
def setTextColor (foreground, background):
    originalColor = __getDefaultConsoleColors() 

    if foreground == UNCHANGED:
        foreground = originalColor & 0x000F

    if background == UNCHANGED:
        background = originalColor & 0x00F0
    else:
        background = background << 4

    __setConsoleTextColor (background | foreground)
    
    yield

    __setConsoleTextColor (originalColor)


