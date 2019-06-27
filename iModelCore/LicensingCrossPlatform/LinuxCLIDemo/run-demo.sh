#!/bin/bash

# In case we want to check the clang version
# clangVersionGrep=$(clang++ --version | head -n 1 | grep -o -E "[[:digit:]].[[:digit:]].[[:digit:]]" | uniq | sort)
# echo $clangVersionGrep

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
		fi

		echo "In the ~/.hgrc file, please replace ui.username your real Bentley login name. Press enter in this shell when you are done."
		if type "vim" > /dev/null; then
			gnome-terminal -- vim ~/.hgrc
		fi
		read x

		echo "In the ~/scripts/ntlmaps.cfg file, please replace USER with your real Bentley login name. Press enter in this shell when you are done."
		if type "vim" > /dev/null; then
			gnome-terminal -- vim ~/scripts/ntlmaps.cfg
		fi
		read x

		rm -rf ~/GetAndBuildBim2DcsOnLinux
	fi

	#TODO: check if Ubuntu/Linux etc for the correct terminal to open
	gnome-terminal -- ~/scripts/hgproxy.sh

	echo "Type in your Bentley password in the new shell that opened. Press enter in this shell when you are done."
	read x

	echo "Cloning Licensing repo."
	hg clone http://bim0200.hgbranches.bentley.com/selserver/LicensingCrossPlatform ./${srcdir}/LicensingCrossPlatform

	if [ ! -f "${srcdir}/LinuxDemo.h" ]; then
		cp ./${srcdir}/LicensingCrossPlatform/LinuxCLIDemo/LinuxDemo.h ./${srcdir}/LinuxDemo.h
	fi
	if [ ! -f "${srcdir}/LinuxDemo.cpp" ]; then
		cp ./${srcdir}/LicensingCrossPlatform/LinuxCLIDemo/LinuxDemo.cpp ./${srcdir}/LinuxDemo.cpp
	fi
	if [ ! -d "${srcdir}/assets" ]; then
		cp -R ./${srcdir}/LicensingCrossPlatform/LinuxCLIDemo/assets ./${srcdir}/assets
	fi
	echo "Copied the demo files from the licensing code"
	
	echo "Deleting cloned source"
	rm -rf ./${srcdir}/LicensingCrossPlatform

}

# look in srcdir and if the imodelcore folder is not there, install the package
if [ -d "$srcdir" ]; then
	echo "directory found"
	if [ ! -d "${srcdir}/iModelCoreNuget_LinuxX64.${nugetVersion}" ]; then
		if ! type "nuget" > /dev/null; then
			echo "nuget is not installed, please install nuget. Recommended command: 'sudo apt install nuget'"
			exit 1
		fi
		echo "Installing iModelCore nuget package"
		nuget install iModelCoreNuget_LinuxX64 -Version ${nugetVersion} -Source http://nuget.bentley.com/nuget/Default -OutputDirectory ./${srcdir}
	fi
	if [ ! -d "${srcdir}/LinuxDemo.h" ] || [ ! -d "${srcdir}/LinuxDemo.cpp" ]; then
		PullLinuxCLISource
	fi

else
	echo "Demo directory not found, need to create directory and install everything"

	mkdir -p ${srcdir}
	mkdir -p ${outdir}

	PullLinuxCLISource

	if ! type "nuget" > /dev/null; then
		echo "nuget is not installed, please install nuget. Recommended command: 'sudo apt install nuget'"
	fi
	
	echo "Installing iModelCore nuget package"
	nuget install iModelCoreNuget_LinuxX64 -Version ${nugetVersion} -Source http://nuget.bentley.com/nuget/Default -OutputDirectory ./${srcdir}
fi

if [ ! -d "$outdir" ]; then
	mkdir -p ${outdir}
fi

if ! type "clang" > /dev/null; then
	echo "clang is not installed, please install clang. Recommended command: 'sudo apt install clang'"
	exit 1
elif type "clang" > /dev/null; then
	echo "clang is installed!"
fi

#TODO: check for other requirements

clang++ -std=c++14 -stdlib=libstdc++ --include-directory=${srcdir}/iModelCoreNuget_LinuxX64.${nugetVersion}/native/include --library-directory=${srcdir}/iModelCoreNuget_LinuxX64.${nugetVersion}/native/lib -o ./${outdir}/LinuxDemo ./${srcdir}/LinuxDemo.cpp -Wl,--start-group -lBaseGeoCoord -lBeCsmapStatic -lBeCurl -lBeFolly -lBeHttp -lBeIcu4c -lBeJpeg -lBeJsonCpp -lBeLibJpegTurbo -lBeLibxml2 -lBentley -lBentleyGeom -lBentleyGeomSerialization -lBeOpenSSL -lBePng -lBeSecurity -lBeSQLite -lBeSQLiteEC -lBeXml -lBeZlib -lDgnPlatform -lECObjects -lECPresentation -lfreetype2 -lLicensing -llzma -lnapi -lpskernel -lsnappy -lUnits -lWebServicesClient -lpthread -ldl -Wl,--end-group

./${outdir}/LinuxDemo
