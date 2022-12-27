# Building the iModel Native Library

# Requirements on All Platforms
- git
- Python 3 with lxml and xxhash packages installed via pip
- Additional requirements per-platform below

# Geting the Source
1. Create a directory to hold the build, such as "imodel"  
```cmd
mkdir imodel
```
2. Create a sudirectory "src"
```cmd
cd imodel
mkdir src
cd src
```
3. Clone the repositories
```cmd
git clone https://github.com/iTwin/imodel-native
git clone https://github.com/iTwin/bis-schemas BisSchemas
```

# Platform-Specific Requirements and Build Instructions

## Build on Windows

Only supports Windows 10 or newer. 

Requirements: 
- Visual Studio 2019
- Symlink permissions or run as administrator
  - To modify symlink permissions, as an administrator run gpedit.msc and modify "Computer Configuration" -> "Windows Settings" -> "Security Settings" -> "Local Policies" -> "User Rights Assignment" -> "Create symbolic links"

To build:

```cmd
cd imodel-native
build\buildwin.bat
```

---

## Build on Linux

The release builds are done on a Debian 9 (stretch) distribution. 

Requirements:
- Clang 7

To build:

```cmd
cd imodel-native
build\buildlinux.sh
```

---

## Build on Mac

Release builds are made on MacOS 12.2.1 or higher.

Requirements:
- Xcode 13.4 or higher
- Xcode command line tools

To build:

```cmd
cd imodel-native
build\buildmac.sh
```

---

## Build on Android

Adroid release builds are done on a Windows box.

Download AndroiddNDK r21e and expand it to imodel\src\AndroidNDK\r21e  
Download AndroidSDK 2020a and expand it to imodel\src\AndroidSDK\2020a  
Download Gradle 4.10.3 and expand it to imodel\src\Gradle\4.10.3  
Download JavaSDK 1.8.0.191 and expand it to imodel\src\JDK\1.8.0.191  

```cmd
set ANDROID_NDK_ROOT=%SrcRoot%AndroidNDK\r21e\
set ANDROID_SDK_ROOT=%SrcRoot%AndroidSDK\2020a\
set GRADLE_HOME=%SrcRoot%Gradle\4.10.3\
set JAVA_HOME=%SrcRoot%JDK\1.8.0.191\
set JAVA_JDK_ROOT=$(JAVA_HOME)

cd imodel-native
build\buildandroid.bat
```

---

## Build on iOS

iOS builds are done on MacOS 12.2.1 or higher

Requirements:
- Xcode 13.4 or higher
- Xcode command line tools

To build:

```cmd
cd imodel-native
build\buildios.bat
```


