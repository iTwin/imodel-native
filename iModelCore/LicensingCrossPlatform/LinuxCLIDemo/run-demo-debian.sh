#!/bin/bash

# In case we want to check the clang version
# clangVersionGrep=$(clang++ --version | head -n 1 | grep -o -E "[[:digit:]].[[:digit:]].[[:digit:]]" | uniq | sort)
# echo $clangVersionGrep

#if ! [ -x "$(command -v git)" ]; then
#  echo 'Error: git is not installed.' >&2
#  exit 1
#fi

#$ command -v foo >/dev/null 2>&1 || { echo >&2 "I require foo but it's not installed.  Aborting."; exit 1; }

demodir="LicensingSampleApp"

srcdir="${demodir}/src"
outdir="${demodir}/out"

nugetVersion="2.1.0.230"

echo ${srcdir}

PullLinuxCLISource() {

	# echo "Get the scripts to authenticate mercurial"
	# rsync -r rsync-vcs1.bentley.com::tools/GetAndBuildBim2DcsOnLinux ~

	# echo "Make those scripts executable"
	# chmod u+x ~/GetAndBuildBim2DcsOnLinux/scripts/*.sh

	# # check for scripts folder
	# if [ ! -d "~/scripts" ]; then
	# 	cp -R ~/GetAndBuildBim2DcsOnLinux/scripts ~
	# else
	# 	cp ~/scripts/*.sh ~/scripts
	# fi
	# echo "Copied the scripts to home"

	# check if we rebooted via this script after setting up proxy
	if [ -f ./resume_after_reboot.txt ]; then
		rm -rf ./resume_after_reboot.txt
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

	# check if we rebooted manually after setting up proxy
	elif [ -f ./resume_before_reboot.txt ]; then
		echo "Did you reboot manually? (y/n)"
		read ynrebooted
		if [ ${ynrebooted} == "y" ] || [ ${ynrebooted} == "Y" ]; then
			rm -rf ./resume_before_reboot.txt
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
		elif [ ${ynrebooted} == "n" ] || [ ${ynrebooted} == "N" ]; then
			echo "Reboot is required to continue. Continue and reboot now? (y/n)"
			read ynreboot
			if [ ${ynreboot} == "y" ] || [ ${ynreboot} == "Y" ]; then
				echo "resume licensing CLI setup shell after reboot" > ./resume_after_reboot.txt
				sudo shutdown -r now
				exit 1
			elif [ ${ynreboot} == "n" ] || [ ${ynreboot} == "N" ]; then
				echo "resume before reboot" > ./resume_before_reboot.txt
				echo "A reboot is required to continue. Exiting."
				exit 1
			else
				echo "invalid input"
				exit 1
			fi
		else
			echo "invalid input"
			exit 1
		fi
	else
		# set up ntl server and mercurial files
		echo "Enter your username (e.g. First.Last):"
		read username

		echo "Enter your domain password (see readme for information about how your password is stored and used within this script):"
		read -s password

		#need sudo here for /etc
		echo \
		$"[GENERAL]\
		LISTEN_PORT:5865\
		PARENT_PROXY:\
		PARENT_PROXY_PORT:\
		PARENT_PROXY_TIMEOUT:\
		ALLOW_EXTERNAL_CLIENTS:0\
		FRIENDLY_IPS:\
		URL_LOG:0\
		MAX_CONNECTION_BACKLOG:5\
		[NTLM_AUTH]\
		NT_HOSTNAME:\
		NT_DOMAIN:BENTLEY\
		USER:${username}\
		PASSWORD:${password}\
		LM_PART:1\
		NT_PART:1\
		NTLM_FLAGS:07820000\
		NTLM_TO_BASIC:0\
		[DEBUG]\
		DEBUG:0\
		BIN_DEBUG:0\
		SCR_DEBUG:0\
		AUTH_DEBUG:0" | sudo tee /etc/ntlmaps/server.cfg 1> /dev/null

		# check for the bsi-createremote.py file and create it if it doesn't exist
		if [ ! -d "/usr/bin/bentley" ]; then
			mkdir /usr/bin/bentley
			rsync rsync-vcs1.bentley.com::tools/GetAndBuildBim2DcsOnLinux/scripts/bsi-createremote.py /usr/bin/bentley
		elif [ ! -f "/usr/bin/bentley/bsi-createremote.py" ]; then
			rsync rsync-vcs1.bentley.com::tools/GetAndBuildBim2DcsOnLinux/scripts/bsi-createremote.py /usr/bin/bentley
		fi

		echo \
		$"[ui]\
		username = ${username}\
		[http_proxy]\
		host = 127.0.0.1:5865\
		[extensions]\
		bsi-createremote = /usr/bin/bentley/bsi-createremote.py\
		largefiles ="> ~/.hgrc

		echo "Reboot is required to continue. Continue and reboot now? (y/n)"
		read ynreboot
		if [ ${ynreboot} == "y" ] || [ ${ynreboot} == "Y" ]; then
			echo "resume licensing CLI setup shell after reboot" > ./resume_after_reboot.txt
			sudo shutdown -r now
			exit 1
		elif [ ${ynreboot} == "n" ] || [ ${ynreboot} == "N" ]; then
			echo "resume before reboot" > ./resume_before_reboot.txt
			echo "A reboot is required to continue. Exiting."
			exit 1
		else
			echo "invalid input"
			exit 1
		fi
	fi

}

# look in srcdir and if the imodelcore folder is not there, install the package
if [ -d "$srcdir" ]; then
	echo "directory found"
	if [ ! -d "${srcdir}/iModelCoreNuget_LinuxX64.${nugetVersion}" ]; then
		if ! [ -x "$(command -v nuget)" ]; then
			echo "nuget is not installed, please install nuget. Recommended command: 'sudo apt-get install nuget'"
			exit 1
		fi
		echo "Installing iModelCore nuget package"
		nuget install iModelCoreNuget_LinuxX64 -Version ${nugetVersion} -Source http://nuget.bentley.com/nuget/Default -OutputDirectory ./${srcdir}
	fi
	if [ ! -f "${srcdir}/LinuxDemo.h" ] || [ ! -f "${srcdir}/LinuxDemo.cpp" ] || [ ! -d "${srcdir}/assets" ]; then
		PullLinuxCLISource
	fi

else
	echo "Demo directory not found, need to create directory and install everything"

	mkdir -p ${srcdir}
	mkdir -p ${outdir}

	PullLinuxCLISource

	if ! [ -x "$(command -v nuget)" ]; then
		echo "nuget is not installed, please install nuget. Recommended command: 'sudo apt-get install nuget'"
	fi
	
	echo "Installing iModelCore nuget package"
	nuget install iModelCoreNuget_LinuxX64 -Version ${nugetVersion} -Source http://nuget.bentley.com/nuget/Default -OutputDirectory ./${srcdir}
fi

if [ ! -d "$outdir" ]; then
	mkdir -p ${outdir}
fi

if ! [ -x "$(command -v clang)" ]; then
	if ! [ -x "$(command -v clang-3.8)" ]; then
		echo "clang is not installed, please install clang. Recommended command: 'sudo apt-get install clang'"
		exit 1
	else
		clang++-3.8 -std=c++14 -stdlib=libstdc++ --include-directory=${srcdir}/iModelCoreNuget_LinuxX64.${nugetVersion}/native/include --library-directory=${srcdir}/iModelCoreNuget_LinuxX64.${nugetVersion}/native/lib -o ./${outdir}/LinuxDemo ./${srcdir}/LinuxDemo.cpp -Wl,--start-group -lBaseGeoCoord -lBeCsmapStatic -lBeCurl -lBeFolly -lBeHttp -lBeIcu4c -lBeJpeg -lBeJsonCpp -lBeLibJpegTurbo -lBeLibxml2 -lBentley -lBentleyGeom -lBentleyGeomSerialization -lBeOpenSSL -lBePng -lBeSecurity -lBeSQLite -lBeSQLiteEC -lBeXml -lBeZlib -lDgnPlatform -lECObjects -lECPresentation -lfreetype2 -lLicensing -llzma -lnapi -lpskernel -lsnappy -lUnits -lWebServicesClient -lpthread -ldl -Wl,--end-group
	fi
else
	clang++ -std=c++14 -stdlib=libstdc++ --include-directory=${srcdir}/iModelCoreNuget_LinuxX64.${nugetVersion}/native/include --library-directory=${srcdir}/iModelCoreNuget_LinuxX64.${nugetVersion}/native/lib -o ./${outdir}/LinuxDemo ./${srcdir}/LinuxDemo.cpp -Wl,--start-group -lBaseGeoCoord -lBeCsmapStatic -lBeCurl -lBeFolly -lBeHttp -lBeIcu4c -lBeJpeg -lBeJsonCpp -lBeLibJpegTurbo -lBeLibxml2 -lBentley -lBentleyGeom -lBentleyGeomSerialization -lBeOpenSSL -lBePng -lBeSecurity -lBeSQLite -lBeSQLiteEC -lBeXml -lBeZlib -lDgnPlatform -lECObjects -lECPresentation -lfreetype2 -lLicensing -llzma -lnapi -lpskernel -lsnappy -lUnits -lWebServicesClient -lpthread -ldl -Wl,--end-group
fi

#TODO: check for other requirements

# pass source dir into the program to determine assets directory
fullSourcePath=$(pwd)

./${outdir}/LinuxDemo "${fullSourcePath}/${srcdir}"
