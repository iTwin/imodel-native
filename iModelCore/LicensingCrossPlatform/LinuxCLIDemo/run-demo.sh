#!/bin/bash

# clangVersionGrep=$(clang++ --version | head -n 1 | grep -o -E "[[:digit:]].[[:digit:]].[[:digit:]]" | uniq | sort)
# echo $clangVersionGrep

# clangVersion=$(clang++ -dumpversion)
# echo $clangVersion

demodir="LicensingDemoTemp"

srcdir="${demodir}/src"
outdir="${demodir}/out"

nugetVersion="2.1.0.192"

echo ${srcdir}

#TODO: pull source for demo
PullLinuxCLISource() {
	
	echo "Have you set up the scripts to authenticate with mercurial? (y/n)"
	read yn
	if [[ ${yn} == "n" ]]; then
		# get the proxy server
		echo "Check out ntlmaps"
		svn checkout http://svn.code.sf.net/p/ntlmaps/code/trunk ~/lib/ntlmaps

		echo "Get the scripts to authenticate mercurial"
		rsync -r rsync-vcs1.bentley.com::tools/GetAndBuildBim2DcsOnLinux ~

		echo "Make those scripts executable"
		chmod u+x ~/GetAndBuildBim2DcsOnLinux/scripts/*.sh

		# check for scripts folder
		if [ ! -d "~/scripts" ]; then
			cp -R ~/GetAndBuildBim2DcsOnLinux/scripts ~
		else
			cp ~/scripts/*.sh ~/scripts
		fi
		echo "Copied the scripts to home"

		if [ ! -f "~/.hgrc" ]; then
			cp ~/GetAndBuildBim2DcsOnLinux/hgrc ~/.hgrc
			echo "Copied the .hgrc file to home"
		else
			echo "Please merge the ~/GetAndBuildBim2DcsOnLinux/hgrc file into the ~/.hgrc file. Press enter in this shell when you are done."
			read x
			# TODO: do this for them
		fi

		echo "In the ~/.hgrc file, please replace ui.username your real Bentley login name. Press enter in this shell when you are done."
		gnome-terminal -- vim ~/GetAndBuildBim2DcsOnLinux/hgrc
		read x
		# TODO: do this for them, prompt for username

		echo "In the ~/scripts/ntlmaps.cfg file, please replace USER with your real Bentley login name. Press enter in this shell when you are done."
		gnome-terminal -- vim ~/scripts/ntlmaps.cfg
		read x
		# TODO: do this for them, prompt for username

		rm -rf ~/GetAndBuildBim2DcsOnLinux
	fi

	gnome-terminal -- ~/scripts/hgproxy.sh

	echo "Type in your Bentley password in the new shell that opened. Press enter in this shell when you are done."
	read x

	echo "Cloning Licensing repo."
	hg clone http://bim0200.hgbranches.bentley.com/selserver/LicensingCrossPlatform ${srcroot}

	if [ ! -d "${srcdir}/LinuxDemo.h" ]; then
		cp ./${srcdir}/LicensingCrossPlatform/LinuxCLIDemo/LinuxDemo.h ./${srcdir}/LinuxDemo.h
	fi
	if [ ! -d "${srcdir}/LinuxDemo.cpp" ]; then
		cp ./${srcdir}/LicensingCrossPlatform/LinuxCLIDemo/LinuxDemo.cpp ./${srcdir}/LinuxDemo.cpp
	fi
	echo "Copied the demo files from the licensing code"

}

#TODO: check if nuget package is installed
# look in srcdir and if the imodelcore folder is not there, install the package

if [ -d "$srcdir" ]; then
	echo "directory found"
	if [ ! -d "${srcdir}/iModelCoreNuget_LinuxX64.2.1.0.192" ]; then
		if ! type "nuget" > /dev/null; then
			#TODO: prompt for install
			echo "nuget not installed, installing nuget now"
			sudo apt install nuget
		fi
		nuget install iModelCoreNuget_LinuxX64 -Version ${nugetVersion} -Source http://nuget.bentley.com/nuget/Default -OutputDirectory ./${srcdir}
	fi
	if [ ! -d "${srcdir}/LinuxDemo.h" ] || [ ! -d "${srcdir}/LinuxDemo.cpp" ]; then
		#TODO replace this with pulling source

		PullLinuxCLISource

		# cp ./LicensingDemo/src/LinuxDemo.h ./${srcdir}/LinuxDemo.h
		# cp ./LicensingDemo/src/LinuxDemo.cpp ./${srcdir}/LinuxDemo.cpp
	fi
	# if [ ! -d "${srcdir}/LinuxDemo.cpp" ]; then
	# 	#TODO replace this with pulling source
	# 	cp ./LicensingDemo/src/LinuxDemo.cpp ./${srcdir}/LinuxDemo.cpp
	# fi
else
	echo "Demo directory not found, need to create directory and install everything"

	mkdir -p ${srcdir}
	mkdir -p ${outdir}

	#temporary to provide the .h and .cpp files
	#TODO replace this with pulling source
	PullLinuxCLISource

	if ! type "nuget" > /dev/null; then
		#TODO: prompt for install
		echo "nuget not installed, installing nuget now"
		sudo apt install nuget
	fi
	nuget install iModelCoreNuget_LinuxX64 -Version 2.1.0.192 -Source http://nuget.bentley.com/nuget/Default -OutputDirectory ./${srcdir}
fi

if [ ! -d "$outdir" ]; then
	mkdir -p ${outdir}
fi

#TODO: check if installed

if ! type "clang" > /dev/null; then
	#TODO: prompt for install
	echo "clang not installed, installing clang now"
	sudo apt install clang
elif type "clang" > /dev/null; then
	echo "clang installed!"
fi

#TODO: check for other requirements

clang++ -std=c++14 -stdlib=libstdc++ --include-directory=./${srcdir}/iModelCoreNuget_LinuxX64.2.1.0.192/native/include --library-directory=${srcdir}/iModelCoreNuget_LinuxX64.2.1.0.192/native/lib -o ${outdir}/LinuxDemo ${srcdir}/LinuxDemo.cpp -Wl,--start-group -lBaseGeoCoord -lBeCsmapStatic -lBeCurl -lBeFolly -lBeHttp -lBeIcu4c -lBeJpeg -lBeJsonCpp -lBeLibJpegTurbo -lBeLibxml2 -lBentley -lBentleyGeom -lBentleyGeomSerialization -lBeOpenSSL -lBePng -lBeSecurity -lBeSQLite -lBeSQLiteEC -lBeXml -lBeZlib -lDgnPlatform -lECObjects -lECPresentation -lfreetype2 -lLicensing -llzma -lnapi -lpskernel -lsnappy -lUnits -lWebServicesClient -lpthread -ldl -Wl,--end-group

${outdir}demo