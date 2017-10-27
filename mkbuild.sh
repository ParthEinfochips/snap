#!/bin/bash
# eInfochips.	@2017/05/24
#				Nilesh Suryawanshi.
#Directory Structure :
#
# Packages         - Package Dir
# Packages/App     - App Dir
# Packages/App/lib - Contains Alljoyn lib.
# Packages/App/bin - Contains Alljoyn bin.
# Packages/sAgent  - Software Agent Dir Contains SoftwareAgent
#		     		 bin and config file.

####################################################
# Defines
####################################################

##Define Path
HOME="$PWD"
PACKAGES_DIR_PATH="packages"
RELEASE_DIR_PATH="packages/Release$PV_DEFAULT_VERSION"
ZWAVE_DIR_PATH="Library/PTL/Zwave" #NOT IN USE
MODBUS_DIR_PATH="Library/PTL/MODBUS" #NOT IN USE

##Platform Veriable
PV_X86="x86_64"       #<PackName>_x86_64.zip
PV_ARMV7="armv7l"      #<PackName>_armv7l.zip
PV_AARCH64="aarch64"  #<PackName>_aarch64.zip
PV_ANSIBLE="ansible"
PV_DEFAULT=$PV_X86
PV_DEFAULT_PREFIX="_"$PV_X86
PV_DEFAULT_VERSION="v1.0.0"
PV_VERSION="_"$PV_DEFAULT_VERSION

##Progress bar
count=0
total=100
pstr="[----------------------------------------------------------------------]"

##Color Scheme
RED='\033[0;31m'
GREEN='\033[1;32m'
BLUE='\033[1;34m'
WHITE='\033[1;37m'
NC='\033[0m'

progress_indicator()
{
	count=$1
	pd=$(( $count * 73 / $total ))
	printf "${GREEN}\r%3d.%1d%% %.${pd}s${NC}" $(( $count * 100 / $total )) $(( ($count * 1000 / $total) % 10 )) $pstr
}

process_clean()
{
	echo "INFO : Cleaning Project .. .. "
	if [ "DevApp" == $1 ]||[ "ajDev" == $1 ]||[ "aj" == $1 ]||[ "MqttApp" == $1 ];then
		rm -vrf packages
		echo "INFO : Cleaning - $1 "
		cd SnapbricksRouter
		scons BINDINGS=c,cpp SERVICES_CUSTOM=ajDev,DevApp,Mqtt VARIANT=release -j4 -c
		cd $HOME
	elif [ "snapAgent" == $1 ];then
		echo "INFO : Cleaning - $1 "
		cd App/softwareAgent
		bash ./mkbuild.sh clean
		cd $HOME
	elif [ "zwave" == $1 ];then
		echo "INFO : Cleaning - $1 "
		cd Library/PTL/Zwave
		rm -rf build
		cd $HOME
		rm -rf packages/PTL/libZwave.so
	elif [ "modbus" == $1 ];then
		echo "INFO : Cleaning - $1 "
		cd Library/PTL/MODBUS
		sh setup.sh clean
		cd $HOME
		rm -rf packages/PTL/libModbus.so
	elif [ "Dbug" == $1 ];then
		echo "INFO : Cleaning - $1 "
		cd Library/PTL/common/src
		rm -r *.o
		sudo rm -r libDebugLog.a /opt/HawkEdge/lib/libDebugLog.a
		cd $HOME
	fi
	echo "INFO : Done .. .."
}

process_app()
{
	echo "INFO : Building App - $1"
	echo "INFO : Making directory structure.. .."
	if [ -z "$2" ];then
		PV_DEFAULT="$PV_X86"
	else
		PV_DEFAULT="$2"
	fi
	PV_DEFAULT_PREFIX="_"$PV_DEFAULT
	if [ "all" == $1 ];then
		ver_SC="ajDev,DevApp,MqttApp"
	else
		ver_SC="ajDev,$1"
	fi
	for liblist in "scons" "libstdc++-4.8-dev" "libcap-dev"
	do
	PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $liblist|grep "install ok installed")
	echo INFO : Checking for $liblist - $PKG_OK
	if [ "" == "$PKG_OK" ];
	then
	  echo "INFO : $liblist not installed. Setting up $liblist."
	  sudo apt-get --force-yes --yes install $liblist
	fi
	done
	mkdir -v $PACKAGES_DIR_PATH
	rm -rf $PACKAGES_DIR_PATH/$PV_DEFAULT_PREFIX/App/lib/* $PACKAGES_DIR_PATH/$PV_DEFAULT_PREFIX/App/bin/*
	#rm -vrf packages/App/lib/* packages/App/bin/* packages/sAgent/*
	echo "INFO : Building Project - $ver_SC"
	cd SnapbricksRouter
	if [ "x86_64" == $PV_DEFAULT ];then
    	scons BINDINGS=c,cpp SERVICES_CUSTOM="$ver_SC" VARIANT=release -j4 CRYPTO=builtin
    elif [ "armv7l" == $PV_DEFAULT ];then
    	scons OS=linux CPU=armv7 BINDINGS=c,cpp SERVICES_CUSTOM="$ver_SC" VARIANT=release -j4 CRYPTO=builtin
    elif [ "aarch64" == $PV_DEFAULT ];then
    	scons OS=linux CPU=aarch64 BINDINGS=c,cpp SERVICES_CUSTOM="$ver_SC" VARIANT=release -j4 CRYPTO=builtin
    fi
	echo "INFO : Move Packages Dir.. .."
	cp -rf packages/* ../$PACKAGES_DIR_PATH/$PV_DEFAULT/
	#cp -uvrf packages/* ../packages
	cd ..
	echo "INFO : Done .. .."
}

process_snapAgent()
{
	echo "INFO : Building Software Agent.. .."
	mkdir -vp packages/snapAgent
	#cp -vn App/softwareAgent/configSA.xml  packages/snapAgent
	rsync -ua App/softwareAgent/configSA.xml packages/snapAgent/configSA.xml
	rsync -ua App/softwareAgent/MQTT.conf    packages/snapAgent/MQTT.conf
	rsync -ua App/softwareAgent/runSA.sh     packages/snapAgent/runSA.sh
	cd App/softwareAgent
	if [ -d build ]; then
		sudo rm -rf build
	fi
	bash ./mkbuild.sh
	cd ../..
	cp -v App/softwareAgent/build/snapAgent packages/snapAgent/
	echo "INFO : Done .. .."
}

process_zwave()
{
	echo "INFO : Building Zwave .. .."
	if [ -z "$1" ];then
		PV_DEFAULT="$PV_X86"
	else
		PV_DEFAULT="$1"
	fi
	mkdir -vp $PACKAGES_DIR_PATH
	mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT
	mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/PTL
	for liblist in libudev-dev libusb-dev
	do
	PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $liblist|grep "install ok installed")
	echo INFO : Checking for $liblist - $PKG_OK
	if [ "" == "$PKG_OK" ];
	then
	  echo "INFO : $liblist not installed. Setting up $liblist."
	  sudo apt-get --force-yes --yes install $liblist
	fi
	done
	echo "**************************************"
	echo "*		$PV_DEFAULT               *"
	echo "**************************************"
	cd Library/PTL/Zwave
	file="build/Makefile"
	if [ -f "$file" ];then
		echo "INFO : $file found."
		cd build
		rm -rf *
	else
		echo "INFO : $file not found."
		mkdir build
		cd build
		rm -rf *
	fi
	if [ "x86_64" == $PV_DEFAULT ];then
		cmake ../src/
    elif [ "armv7l" == $PV_DEFAULT ];then
		cmake ../src/ -DCMAKE_TOOLCHAIN_FILE=../src/cross-arm-linux-gnueabihf.cmake
    elif [ "aarch64" == $PV_DEFAULT ];then
    	cmake ../src/ -DCMAKE_TOOLCHAIN_FILE=../src/cross-aarch64.cmake
    fi
	make
	cd $HOME
	cp Library/PTL/Zwave/build/lib/* $PACKAGES_DIR_PATH/$PV_DEFAULT/PTL/
	echo "INFO : Done .. .."
}

process_ble()
{
	echo "INFO : Building BLE .. ..Not Supported Yet!!"

	echo "INFO : Done .. .."
}

process_zigbee()
{
	echo "INFO : Building BLE .. ..Not Supported Yet!!"

	echo "INFO : Done .. .."
}

process_opc()
{
	echo "INFO : Building OPC .. ..Not Supported Yet!!"

	echo "INFO : Done .. .."
}

process_modbus()
{
	echo "INFO : Building MODBUS !!"
	if [ -z "$1" ];then
		PV_DEFAULT="$PV_X86"
	else
		PV_DEFAULT="$1"
	fi
	mkdir -vp $PACKAGES_DIR_PATH
	mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT
	mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/PTL

    if [ -f "/opt/ptl/lib/libModbus.so" ]; then
    	echo "INFO : Cleaning Existing MODBUS library!!"
        process_clean modbus
    fi
	echo "INFO : Checking Pre-requisits !!"
	for liblist in libudev-dev libusb-dev automake autoconf libtool build-essential g++
	do
	PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $liblist|grep "install ok installed")
	echo INFO : Checking for $liblist - $PKG_OK
	if [ "" == "$PKG_OK" ];	then
	  echo "INFO : $liblist not installed. Setting up $liblist."
	  sudo apt-get --force-yes --yes install $liblist
	fi
	done
	cd Library/PTL/MODBUS
	echo "Create Shared library named \"libModbus.so\" in Release Mode."
    ./newSetup.sh $PV_DEFAULT
    cp modbus/lib/libModbus.so $HOME/$PACKAGES_DIR_PATH/$PV_DEFAULT/PTL/
	cd $HOME
	echo "INFO : Done .. .."
}

process_mqttlib()
{
	echo "INFO : Building MQTT lib .. .. "
	if [ -z "$1" ];then
		PV_DEFAULT="$PV_X86"
	else
		PV_DEFAULT="$1"
	fi
	cd Library/CTL/
	mkdir -p build && cd build && rm -rf *
	if [ "x86_64" == $PV_DEFAULT ];then
		cmake ../src/ && make 
    elif [ "armv7l" == $PV_DEFAULT ];then
		cmake ../src/ -DCMAKE_TOOLCHAIN_FILE=../src/cross-arm-linux-gnueabihf.cmake && make 
    elif [ "aarch64" == $PV_DEFAULT ];then
    	cmake ../src/ -DCMAKE_TOOLCHAIN_FILE=../src/cross-aarch64.cmake && make
    fi
	cd $HOME
	echo "INFO : Done .. .."
}

process_RuleEngineApp()
{
	echo "INFO : Building RuleEngineApp .. .. "
	if [ -z "$1" ];then
		PV_DEFAULT="$PV_X86"
	else
		PV_DEFAULT="$1"
	fi
	cd App/RuleEngine/
	mkdir -p build && cd build && rm -rf *
	if [ "x86_64" == $PV_DEFAULT ];then
		cmake ../ && make
    elif [ "armv7l" == $PV_DEFAULT ];then
		cmake ../ -DCMAKE_TOOLCHAIN_FILE=../cross-arm-linux-gnueabihf.cmake && make
    #elif [ "aarch64" == $PV_DEFAULT ];then
    #	cmake ../ -DCMAKE_TOOLCHAIN_FILE=../cross-aarch64.cmake && make
    fi
	cd $HOME
	echo "INFO : Done .. .."
}

process_SchedulerApp() 
{
	echo "INFO : Building SchedulerApp .. .. "
	if [ -z "$1" ];then
		PV_DEFAULT="$PV_X86"
	else
		PV_DEFAULT="$1"
	fi
	cd App/Scheduler/
	mkdir -p build && cd build && rm -rf *
	if [ "x86_64" == $PV_DEFAULT ];then
		cmake ../ && make
    elif [ "armv7l" == $PV_DEFAULT ];then
		cmake ../ -DCMAKE_TOOLCHAIN_FILE=../cross-arm-linux-gnueabihf.cmake && make 
    #elif [ "aarch64" == $PV_DEFAULT ];then
    #	cmake ../ -DCMAKE_TOOLCHAIN_FILE=../cross-aarch64.cmake && make
    fi
	cd $HOME
	echo "INFO : Done .. .."
}

process_Simulator() 
{
	echo "INFO : Building Simulator .. .. "
	mkdir -vp $PACKAGES_DIR_PATH
	mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT
	mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/PTL

	if [ -z "$1" ];then
		PV_DEFAULT="$PV_X86"
	else
		PV_DEFAULT="$1"
	fi
	cd Library/PTL/Simulator
	mkdir -p build && cd build && rm -rf *
	if [ "x86_64" == $PV_DEFAULT ];then
		cmake ../ && make
    elif [ "armv7l" == $PV_DEFAULT ];then
		cmake ../ -DCMAKE_TOOLCHAIN_FILE=../cross-arm-linux-gnueabihf.cmake && make 
    elif [ "aarch64" == $PV_DEFAULT ];then
    	cmake ../ -DCMAKE_TOOLCHAIN_FILE=../cross-aarch64.cmake && make
    fi
	cd $HOME
	cp Library/PTL/Simulator/build/lib/* $PACKAGES_DIR_PATH/$PV_DEFAULT/PTL/
	echo "INFO : Done .. .."
}

process_Dbug()
{
	echo "INFO : Building Dbug Lib .. .."
	cd Library/PTL/common/src
	g++ -c DebugLog.cc -o DebugLog.o -I ../inc
	ar rvs libDebugLog.a DebugLog.o
	sudo cp libDebugLog.a /opt/HawkEdge/lib/
	cd ../../../../
	echo "INFO : Done .. .."
}

process_pack()
{
	if [ -z "$3" ];then
		PV_DEFAULT="$PV_X86"
	else
		PV_DEFAULT="$3"
	fi
	
	if [ "v1.1" = $4 ];then
		PV_DEFAULT_VERSION="v1.1"
		PV_VERSION="_"$PV_DEFAULT_VERSION
	else
		PV_DEFAULT_VERSION="v1.0"
		PV_VERSION="_"$PV_DEFAULT_VERSION
	fi

	PV_PREFIX=$PV_VERSION"_"$PV_DEFAULT
	mkdir -vp $PACKAGES_DIR_PATH
	mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT
	mkdir -vp $PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION
	mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION
	if [ "zwave" == $1 ];then
##ZWAVE
		echo "INFO : GOT COMMAND - $1 "
		rm -rf $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/Z-Wave$PV_PREFIX*
		mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/Z-Wave$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Z-Wave$PV_VERSION
		process_zwave $PV_DEFAULT
		dirbuild=$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/Z-Wave$PV_PREFIX
		if test -d "$dirbuild" && test -x "$dirbuild";
		then
			cd Library/PTL/Zwave/
			sh package.sh
			cd $HOME
			cp -r Library/PTL/Zwave/zwave/* $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/Z-Wave$PV_PREFIX/
			cd $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			zip -r Z-Wave$PV_PREFIX.zip Z-Wave$PV_PREFIX
			cp -r Z-Wave$PV_PREFIX/ $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Z-Wave$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/
			zip -r Z-Wave$PV_VERSION.zip Z-Wave$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			if [ "push" == $2 ];then
				if  [ "CloudFtp" == $5 ];then
					echo "INFO : Pushing Cloud FTP Server .. .."
					progress_indicator 10
					sshpass -p "Cisco123" scp Z-Wave$PV_PREFIX.zip admin@10.107.2.219:/srv/ftp/
					sshpass -p "Cisco123" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Z-Wave$PV_VERSION.zip admin@10.107.2.219:/srv/ftp/
					progress_indicator 100
					echo ''
				elif  [ "LocalFtp" == $5 ];then
					echo "INFO : Pushing Local FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp Z-Wave$PV_PREFIX.zip localFtp@10.103.3.124:/home/localFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Z-Wave$PV_VERSION.zip localFtp@10.103.3.124:/home/localFtp/ftp
					progress_indicator 100
					echo ''
				elif  [ "QaFtp" == $5 ];then
					echo "INFO : Pushing QA FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp Z-Wave$PV_PREFIX.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Z-Wave$PV_VERSION.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					progress_indicator 100
					echo ''
				fi
			fi
			cd $HOME
			echo "INFO : Zwave Package DONE .. .."
		fi
	elif [ "modbus" == $1 ];then
##Modbus
		echo "INFO : GOT COMMAND - $1 "
		rm -rf $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/Modbus$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/Modbus$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Modbus$PV_VERSION
		dirbuild=$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/Modbus$PV_PREFIX
		if test -d "$dirbuild" && test -x "$dirbuild";
		then
			process_modbus $PV_DEFAULT
			chmod 777 Library/PTL/MODBUS/modbus
			cd $HOME
			cp -r Library/PTL/MODBUS/modbus/* $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/Modbus$PV_PREFIX/
			cd $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			zip -r Modbus$PV_PREFIX.zip Modbus$PV_PREFIX
			cp -r Modbus$PV_PREFIX/ $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Modbus$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/
			zip -r Modbus$PV_VERSION.zip Modbus$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			if [ "push" == $2 ];then
				if  [ "CloudFtp" == $5 ];then
					echo "INFO : Pushing Cloud FTP Server .. .."
					progress_indicator 10
					sshpass -p "Cisco123" scp Modbus$PV_PREFIX.zip admin@10.107.2.219:/srv/ftp/
					sshpass -p "Cisco123" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Modbus$PV_VERSION.zip admin@10.107.2.219:/srv/ftp/
					progress_indicator 100
					echo ''
				elif  [ "LocalFtp" == $5 ];then
					echo "INFO : Pushing Local FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp Modbus$PV_PREFIX.zip localFtp@10.103.3.124:/home/localFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Modbus$PV_VERSION.zip localFtp@10.103.3.124:/home/localFtp/ftp
					progress_indicator 100
					echo ''
				elif  [ "QaFtp" == $5 ];then
					echo "INFO : Pushing QA FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp Modbus$PV_PREFIX.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Modbus$PV_VERSION.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					progress_indicator 100
					echo ''
				fi
			fi
			cd $HOME
			echo "INFO : Modbus Package DONE .. .."
		fi
	elif [ "DevApp" == $1 ];then
##DeviceOnboardingApp
		echo "INFO : GOT COMMAND - $1 "
		rm -rf $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/DeviceOnboardingApp$PV_PREFIX*
		mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/DeviceOnboardingApp$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/DeviceOnboardingApp$PV_VERSION
		process_app "DevApp" $PV_DEFAULT
		dirbuild=$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/DeviceOnboardingApp$PV_PREFIX
		if test -d "$dirbuild" && test -x "$dirbuild";
		then
			cp $PACKAGES_DIR_PATH/$PV_DEFAULT/App/bin/DeviceOnboardingApp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/DeviceOnboardingApp$PV_PREFIX/
			cp -r App/DevApp/DeviceMgmt/packageName.json $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/DeviceOnboardingApp$PV_PREFIX/
			cd $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			zip -r DeviceOnboardingApp$PV_PREFIX.zip DeviceOnboardingApp$PV_PREFIX
			cp -r DeviceOnboardingApp$PV_PREFIX/ $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/DeviceOnboardingApp$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/
			zip -r DeviceOnboardingApp$PV_VERSION.zip DeviceOnboardingApp$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			if [ "push" == $2 ];then
				if  [ "CloudFtp" == $5 ];then
					echo "INFO : Pushing Cloud FTP Server .. .."
					progress_indicator 10
					sshpass -p "Cisco123" scp DeviceOnboardingApp$PV_PREFIX.zip admin@10.107.2.219:/srv/ftp/
					sshpass -p "Cisco123" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/DeviceOnboardingApp$PV_VERSION.zip admin@10.107.2.219:/srv/ftp/
					progress_indicator 100
					echo ''
				elif  [ "LocalFtp" == $5 ];then
					echo "INFO : Pushing Local FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp DeviceOnboardingApp$PV_PREFIX.zip localFtp@10.103.3.124:/home/localFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/DeviceOnboardingApp$PV_VERSION.zip localFtp@10.103.3.124:/home/localFtp/ftp
					progress_indicator 100
					echo ''
				elif  [ "QaFtp" == $5 ];then
					echo "INFO : Pushing QA FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp DeviceOnboardingApp$PV_PREFIX.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/DeviceOnboardingApp$PV_VERSION.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					progress_indicator 100
					echo ''
				fi
			fi
			cd $HOME
		fi
	elif [ "OTBus" == $1 ];then
##OTBus
		echo "INFO : GOT COMMAND - $1 "
		rm -rf $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OTBus$PV_PREFIX*
		mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OTBus$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/OTBus$PV_VERSION
		cd $HOME
		echo $HOME
		process_app all $PV_DEFAULT
		dirbuild=$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OTBus$PV_PREFIX
		if test -d "$dirbuild" && test -x "$dirbuild";
		then
			cp -r $PACKAGES_DIR_PATH/$PV_DEFAULT/App/bin/alljoyn-daemon $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OTBus$PV_PREFIX/OTBus
			cp -r SnapbricksRouter/alljoyn_core/packageName.json $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OTBus$PV_PREFIX/
			cd $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			zip -r OTBus$PV_PREFIX.zip OTBus$PV_PREFIX
			cp -r OTBus$PV_PREFIX/ $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/OTBus$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/
			zip -r OTBus$PV_VERSION.zip OTBus$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			if [ "push" == $2 ];then
				if  [ "CloudFtp" == $5 ];then
					echo "INFO : Pushing Cloud FTP Server .. .."
					progress_indicator 10
					sshpass -p "Cisco123" scp OTBus$PV_PREFIX.zip admin@10.107.2.219:/srv/ftp/
					sshpass -p "Cisco123" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/OTBus$PV_VERSION.zip admin@10.107.2.219:/srv/ftp/
					progress_indicator 100
					echo ''
				elif  [ "LocalFtp" == $5 ];then
					echo "INFO : Pushing Local FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp OTBus$PV_PREFIX.zip localFtp@10.103.3.124:/home/localFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/OTBus$PV_VERSION.zip localFtp@10.103.3.124:/home/localFtp/ftp
					progress_indicator 100
					echo ''
				elif  [ "QaFtp" == $5 ];then
					echo "INFO : Pushing QA FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp OTBus$PV_PREFIX.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/OTBus$PV_VERSION.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					progress_indicator 100
					echo ''
				fi
			fi
			cd $HOME
			echo "INFO : OTBus Package DONE .. .."
		fi
	elif [ "ajlib" == $1 ];then
##OT-Libs
		echo "INFO : GOT COMMAND - $1 "
		rm -rf $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OT-Libs$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OT-Libs$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/OT-Libs$PV_VERSION
		process_app all $PV_DEFAULT
		dirbuild=$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OT-Libs$PV_PREFIX
		if test -d "$dirbuild" && test -x "$dirbuild";
		then
			mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OT-Libs$PV_PREFIX/OTLibs
			cp $PACKAGES_DIR_PATH/$PV_DEFAULT/App/lib/* $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OT-Libs$PV_PREFIX/OTLibs
			cp -r SnapbricksRouter/install.sh $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OT-Libs$PV_PREFIX/
			cp -r SnapbricksRouter/uninstall.sh $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OT-Libs$PV_PREFIX/
			cp -r SnapbricksRouter/packageName.json $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OT-Libs$PV_PREFIX/
			cp -rf App/DevApp/DeviceMgmt/pythonLibs_ramlfication/dist-packages $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/OT-Libs$PV_PREFIX/
			cd $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			zip -r OT-Libs$PV_PREFIX.zip OT-Libs$PV_PREFIX
			cp -r OT-Libs$PV_PREFIX/ $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/OT-Libs$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/
			zip -r OT-Libs$PV_VERSION.zip OT-Libs$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			if [ "push" == $2 ];then
				if  [ "CloudFtp" == $5 ];then
					echo "INFO : Pushing Cloud FTP Server .. .."
					progress_indicator 10
					sshpass -p "Cisco123" scp OT-Libs$PV_PREFIX.zip admin@10.107.2.219:/srv/ftp/
					sshpass -p "Cisco123" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/OT-Libs$PV_VERSION.zip admin@10.107.2.219:/srv/ftp/
					progress_indicator 100
					echo ''
				elif  [ "LocalFtp" == $5 ];then
					echo "INFO : Pushing Local FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp OT-Libs$PV_PREFIX.zip localFtp@10.103.3.124:/home/localFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/OT-Libs$PV_VERSION.zip localFtp@10.103.3.124:/home/localFtp/ftp
					progress_indicator 100
					echo ''
				elif  [ "QaFtp" == $5 ];then
					echo "INFO : Pushing QA FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp OT-Libs$PV_PREFIX.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/OT-Libs$PV_VERSION.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					progress_indicator 100
					echo ''
				fi
			fi
			cd $HOME
			echo "INFO : OT-Libs Package DONE .. .."
		fi

	elif [ "RuleEngineApp" == $1 ];then
##RuleEngineApp
		echo "INFO : GOT COMMAND - $1 "
		rm -rf $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/RuleEngineApp$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/RuleEngineApp$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/RuleEngineApp$PV_VERSION
    	process_RuleEngineApp $PV_DEFAULT
		dirbuild=$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/RuleEngineApp$PV_PREFIX
		if test -d "$dirbuild" && test -x "$dirbuild";
		then
      		echo $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/RuleEngineApp$PV_PREFIX/
      		cp App/RuleEngine/build/bin/* $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/RuleEngineApp$PV_PREFIX/
			cp -r App/RuleEngine/package/* $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/RuleEngineApp$PV_PREFIX/
			cd $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			zip -r RuleEngineApp$PV_PREFIX.zip RuleEngineApp$PV_PREFIX
			cp -r RuleEngineApp$PV_PREFIX/ $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/RuleEngineApp$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/
			zip -r RuleEngineApp$PV_VERSION.zip RuleEngineApp$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			if [ "push" == $2 ];then
				if  [ "CloudFtp" == $5 ];then
					echo "INFO : Pushing Cloud FTP Server .. .."
					progress_indicator 10
					sshpass -p "Cisco123" scp RuleEngineApp$PV_PREFIX.zip admin@10.107.2.219:/srv/ftp/
					sshpass -p "Cisco123" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/RuleEngineApp$PV_VERSION.zip admin@10.107.2.219:/srv/ftp/
					progress_indicator 100
					echo ''
				elif  [ "LocalFtp" == $5 ];then
					echo "INFO : Pushing Local FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp RuleEngineApp$PV_PREFIX.zip localFtp@10.103.3.124:/home/localFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/RuleEngineApp$PV_VERSION.zip localFtp@10.103.3.124:/home/localFtp/ftp
					progress_indicator 100
					echo ''
				elif  [ "QaFtp" == $5 ];then
					echo "INFO : Pushing QA FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp RuleEngineApp$PV_PREFIX.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/RuleEngineApp$PV_VERSION.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					progress_indicator 100
					echo ''
				fi
			fi
			cd $HOME
			echo "INFO : RuleEngineApp Package DONE .. .."
		fi

	elif [ "SchedulerApp" == $1 ];then
##SchedulerApp
		echo "INFO : GOT COMMAND - $1 "
		rm -rf $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/SchedulerApp$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/SchedulerApp$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/SchedulerApp$PV_VERSION
    	process_SchedulerApp $PV_DEFAULT
		dirbuild=$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/SchedulerApp$PV_PREFIX
		if test -d "$dirbuild" && test -x "$dirbuild";
		then
      		cp App/Scheduler/build/bin/* $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/SchedulerApp$PV_PREFIX
      		cd App/Scheduler/
			./package.sh $PV_DEFAULT
			cd $HOME
			cp -r App/Scheduler/Scheduler/* $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/SchedulerApp$PV_PREFIX/
			cd $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			zip -r SchedulerApp$PV_PREFIX.zip SchedulerApp$PV_PREFIX
			cp -r SchedulerApp$PV_PREFIX/ $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/SchedulerApp$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/
			zip -r SchedulerApp$PV_VERSION.zip SchedulerApp$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			if [ "push" == $2 ];then
				if  [ "CloudFtp" == $5 ];then
					echo "INFO : Pushing Cloud FTP Server .. .."
					progress_indicator 10
					sshpass -p "Cisco123" scp SchedulerApp$PV_PREFIX.zip admin@10.107.2.219:/srv/ftp/
					sshpass -p "Cisco123" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/SchedulerApp$PV_VERSION.zip admin@10.107.2.219:/srv/ftp/
					progress_indicator 100
					echo ''
				elif  [ "LocalFtp" == $5 ];then
					echo "INFO : Pushing Local FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp SchedulerApp$PV_PREFIX.zip localFtp@10.103.3.124:/home/localFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/SchedulerApp$PV_VERSION.zip localFtp@10.103.3.124:/home/localFtp/ftp
					progress_indicator 100
					echo ''
				elif  [ "QaFtp" == $5 ];then
					echo "INFO : Pushing QA FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp SchedulerApp$PV_PREFIX.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/SchedulerApp$PV_VERSION.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					progress_indicator 100
					echo ''
				fi
			fi
			cd $HOME
			echo "INFO : SchedulerApp Package DONE .. .."
		fi

	elif [ "Simulator" == $1 ];then
##Simulator
		echo "INFO : GOT COMMAND - $1 "
		rm -rf $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/Simulator$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/Simulator$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Simulator$PV_VERSION
		dirbuild=$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/Simulator$PV_PREFIX
		if test -d "$dirbuild" && test -x "$dirbuild";
		then
			process_Simulator $PV_DEFAULT
      		cd Library/PTL/Simulator
			./package.sh
			cd $HOME
			cp -r Library/PTL/Simulator/simulator/* $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/Simulator$PV_PREFIX/
			cd $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			zip -r Simulator$PV_PREFIX.zip Simulator$PV_PREFIX
			cp -r Simulator$PV_PREFIX/ $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Simulator$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/
			zip -r Simulator$PV_VERSION.zip Simulator$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			if [ "push" == $2 ];then
				if  [ "CloudFtp" == $5 ];then
					echo "INFO : Pushing Cloud FTP Server .. .."
					progress_indicator 10
					sshpass -p "Cisco123" scp Simulator$PV_PREFIX.zip admin@10.107.2.219:/srv/ftp/
					sshpass -p "Cisco123" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Simulator$PV_VERSION.zip admin@10.107.2.219:/srv/ftp/
					progress_indicator 100
					echo ''
				elif  [ "LocalFtp" == $5 ];then
					echo "INFO : Pushing Local FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp Simulator$PV_PREFIX.zip localFtp@10.103.3.124:/home/localFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Simulator$PV_VERSION.zip localFtp@10.103.3.124:/home/localFtp/ftp
					progress_indicator 100
					echo ''
				elif  [ "QaFtp" == $5 ];then
					echo "INFO : Pushing QA FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp Simulator$PV_PREFIX.zip admin@10.107.2.219:/srv/ftp/
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/Simulator$PV_VERSION.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					progress_indicator 100
					echo ''
				fi
			fi
			cd $HOME
			echo "INFO : Simulator Package DONE .. .."
		fi

	elif [ "MqttApp" == $1 ];then
##MQTTApp
		echo "INFO : GOT COMMAND - $1 "
		rm -rf $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/MQTTApp$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/MQTTApp$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/MQTTApp$PV_VERSION
		process_app "MqttApp" $PV_DEFAULT
		dirbuild=$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/MQTTApp$PV_PREFIX
		if test -d "$dirbuild" && test -x "$dirbuild";
		then
			cp -r $PACKAGES_DIR_PATH/$PV_DEFAULT/App/bin/MQTTApp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/MQTTApp$PV_PREFIX/
			cp -r App/MqttApp/Mqtt/packageName.json $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/MQTTApp$PV_PREFIX/
			cd $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			zip -r MQTTApp$PV_PREFIX.zip MQTTApp$PV_PREFIX
			cp -r MQTTApp$PV_PREFIX/ $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/MQTTApp$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/
			zip -r MQTTApp$PV_VERSION.zip MQTTApp$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			if [ "push" == $2 ];then
				if  [ "CloudFtp" == $5 ];then
					echo "INFO : Pushing Cloud FTP Server .. .."
					progress_indicator 10
					sshpass -p "Cisco123" scp MQTTApp$PV_PREFIX.zip admin@10.107.2.219:/srv/ftp/
					sshpass -p "Cisco123" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/MQTTApp$PV_VERSION.zip admin@10.107.2.219:/srv/ftp/
					progress_indicator 100
					echo ''
				elif  [ "LocalFtp" == $5 ];then
					echo "INFO : Pushing Local FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp MQTTApp$PV_PREFIX.zip localFtp@10.103.3.124:/home/localFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/MQTTApp$PV_VERSION.zip localFtp@10.103.3.124:/home/localFtp/ftp
					progress_indicator 100
					echo ''
				elif  [ "QaFtp" == $5 ];then
					echo "INFO : Pushing QA FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp MQTTApp$PV_PREFIX.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/MQTTApp$PV_VERSION.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					progress_indicator 100
					echo ''
				fi
			fi
			cd $HOME
			echo "INFO : MqttApp Package DONE .. .."
		fi
	elif [ "mqttlib" == $1 ];then
##MQTT	MQTT_86_64
		echo "INFO : GOT COMMAND - $1 "
		mkdir -vp $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/MQTT$PV_PREFIX
		mkdir -vp $PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/MQTT$PV_VERSION
		dirbuild=$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/MQTT$PV_PREFIX
		if test -d "$dirbuild" && test -x "$dirbuild";
		then
			process_mqttlib $PV_DEFAULT
			cd Library/CTL/
			./package.sh $PV_DEFAULT
			cd $HOME
			cp -r Library/CTL/MQTT/* $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/MQTT$PV_PREFIX/
			cd $PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			zip -r MQTT$PV_PREFIX.zip MQTT$PV_PREFIX
			cp -r MQTT$PV_PREFIX/ $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/MQTT$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/
			zip -r MQTT$PV_VERSION.zip MQTT$PV_VERSION
			cd $HOME/$PACKAGES_DIR_PATH/$PV_DEFAULT/Release$PV_DEFAULT_VERSION/
			if [ "push" == $2 ];then
				if  [ "CloudFtp" == $5 ];then
					echo "INFO : Pushing Cloud FTP Server .. .."
					progress_indicator 10
					sshpass -p "Cisco123" scp MQTT$PV_PREFIX.zip admin@10.107.2.219:/srv/ftp/
					sshpass -p "Cisco123" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/MQTT$PV_VERSION.zip admin@10.107.2.219:/srv/ftp/
					progress_indicator 100
					echo ''
				elif  [ "LocalFtp" == $5 ];then
					echo "INFO : Pushing Local FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp MQTT$PV_PREFIX.zip localFtp@10.103.3.124:/home/localFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/MQTT$PV_VERSION.zip localFtp@10.103.3.124:/home/localFtp/ftp
					progress_indicator 100
					echo ''
				elif  [ "QaFtp" == $5 ];then
					echo "INFO : Pushing QA FTP Server .. .."
					progress_indicator 10
					sshpass -p "einfochips" scp MQTT$PV_PREFIX.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					sshpass -p "einfochips" scp $HOME/$PACKAGES_DIR_PATH/$PV_ANSIBLE/Release$PV_DEFAULT_VERSION/MQTT$PV_VERSION.zip qaFtp@10.103.3.124:/home/qaFtp/ftp
					progress_indicator 100
					echo ''
				fi
				cd $HOME
			fi
			echo "INFO : Mqttlib Package DONE .. .."
		fi
	fi
	cd $HOME
}

process_mkpackage()
{
	if [ -z "$3" ];then
		PUSH="NA"
	else
		PUSH="$3"
	fi

	for liblist in zip sshpass whiptail dialog
	do
	PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $liblist|grep "install ok installed")
	echo INFO : Checking for $liblist - $PKG_OK
	if [ "" == "$PKG_OK" ];	then
	  echo "INFO : $liblist not installed. Setting up $liblist."
	  sudo apt-get --force-yes --yes install $liblist
	fi
	done
	if [ "zwave" == $1 ];then
		echo "INFO : Making Zwave package .. .."
		process_pack "zwave" $PUSH $2 $4 $5
		echo "INFO : Zwave Package DONE .. .."
	elif [ "modbus" == $1 ];then
		echo "INFO : Making modbus package .. .."
		process_pack "modbus" $PUSH $2 $4 $5
		echo "INFO : Modbus Package DONE .. .."
	elif [ "DevApp" == $1 ];then
		echo "INFO : Making DevApp package .. .."
		process_pack "DevApp" $PUSH $2 $4 $5
		echo "INFO : DeviceOnboardingApp Package DONE .. .."
	elif [ "OTBus" == $1 ];then
		echo "INFO : Making OTBus package .. .."
		process_pack "OTBus" $PUSH $2 $4 $5
		echo "INFO : OTbus Package DONE .. .."
	elif [ "ajlib" == $1 ];then
		echo "INFO : Making ajlib package .. .."
		process_pack "ajlib" $PUSH $2 $4 $5
		echo "INFO : ajlib Package DONE .. .."
	elif [ "RuleEngineApp" == $1 ];then
		echo "INFO : Making RuleEngineApp package .. .."
		process_pack "RuleEngineApp" $PUSH $2 $4 $5
		echo "INFO : RuleEngineApp Package DONE .. .."
	elif [ "SchedulerApp" == $1 ];then
		echo "INFO : Making SchedulerApp package .. .."
		process_pack "SchedulerApp" $PUSH $2 $4 $5
		echo "INFO : SchedulerApp Package DONE .. .."
	elif [ "Simulator" == $1 ];then
		echo "INFO : Making Simulator package .. .."
		process_pack "Simulator" $PUSH $2 $4 $5
		echo "INFO : Simulator Package DONE .. .."
    elif [ "MqttApp" == $1 ];then
		echo "INFO : Making MQTTApp package .. .."
		process_pack "MqttApp" $PUSH $2 $4 $5
		echo "INFO : MQTTApp Package DONE .. .."
	elif [ "mqttlib" == $1 ];then
		echo "INFO : Making mqttlib package .. .."
		process_pack "mqttlib" $PUSH $2 $4 $5
		echo "INFO : mqttlib Package DONE .. .."
	elif [ "all" == $1 ];then
		echo "INFO : Making mqttlib package .. .."
		process_pack "mqttlib"          $PUSH $2 $4 $5
		process_pack "MqttApp"          $PUSH $2 $4 $5
		process_pack "ajlib"            $PUSH $2 $4 $5
		process_pack "OTBus"            $PUSH $2 $4 $5
		process_pack "modbus"           $PUSH $2 $4 $5
		process_pack "zwave"            $PUSH $2 $4 $5
		process_pack "DevApp"           $PUSH $2 $4 $5
		process_pack "RuleEngineApp"    $PUSH $2 $4 $5
		process_pack "SchedulerApp"     $PUSH $2 $4 $5
		process_pack "Simulator"     $PUSH $2 $4 $5
		echo "INFO : All Package DONE .. .."
	fi
}

process_help()
{
	echo -e "${BLUE}Building targets -"
	echo -e " 1)  ${GREEN}all${BLUE}              (Build Whole Project)"
	echo -e " 2)  ${GREEN}snapAgent${BLUE}        (Build Software Agent App)"
	echo -e " 3)  ${GREEN}aj${BLUE}               (Build Alljoyn App)"
	echo -e " 4)  ${GREEN}ajDev${BLUE}            (Build Alljoyn App)"
	echo -e " 5)  ${GREEN}DevApp${BLUE}           (Build Device App)"
	echo -e " 6)  ${GREEN}MqttApp${BLUE}          (Build Mqtt App)"
	echo -e " 7)  ${GREEN}zwave${BLUE}            (Build Zwave Lib)"
	echo -e " 8)  ${GREEN}ble${BLUE}              (Build BLE Lib)"
	echo -e " 9)  ${GREEN}zigbee${BLUE}           (Build Zigbee Lib)"
	echo -e " 10) ${GREEN}opc${BLUE}              (Build OPC Lib)"
	echo -e " 11) ${GREEN}modbus${BLUE}           (Build Modbus Lib)"
	echo -e " 12) ${GREEN}mqtt${BLUE}             (Build OPC Lib)"
	echo -e " 13) ${GREEN}Dbug${BLUE}             (Build Dbug Lib)"
	echo -e " 14) ${GREEN}clean <appName>${BLUE}  (Clean App)"
	echo -e " 15) ${GREEN}cleanall${BLUE}         (Clean Whole Project)"
	echo -e " 16) ${GREEN}mkpack <appName>${BLUE} (Will Make All Package : argu2 - AppName, argu3 - Architecture(armv7l / x86_64), argu4 - push [Will Push .zip on FTP Server], argu5 - Selects Version(v1.0.0 / v1.1.0))"
	echo -e " 17) ${GREEN}help${BLUE}             (Help)"
	echo -e ""
	echo -e " Ex. 1) Build Software Agent"
	echo -e "        ${GREEN}./mkbuild.sh snapAgent"${BLUE}
	echo -e "     2) Build Alljoyn App"
	echo -e "        ${GREEN}./mkbuild.sh aj"${BLUE}
	echo -e "     3) Clean App"
	echo -e "        ${GREEN}./mkbuild.sh clean <AppName>"${BLUE}
	echo -e "     4) Build Package"
	echo -e "        ${GREEN}./mkbuild.sh mkpack <AppName>"${BLUE}
	echo -e "     5) Build Package And Push FTP"
	echo -e "        ${GREEN}./mkbuild.sh mkpack <AppName> <Architecture> push <Version> <FtpServer>"${BLUE}
	echo -e "        AppName = mqttlib, MqttApp, ajlib, OTBus, modbus, zwave, DevApp, RuleEngineApp, SchedulerApp, Simulator"
	echo -e "        Architecture = x86_64, armv7l, aarch64."
	echo -e "        FtpServer = LocalFtp, CloudFtp, QaFTP."
	echo -e "     6) Help"
	echo -e "        ${GREEN}./mkbuild.sh help"${BLUE}
	echo -e "${NC}"
}

process_switch()
{
	B_SELECT="$1"
	case "$B_SELECT" in
	"all")
	process_app "$B_SELECT" $2
	process_app "MqttApp" $2
	process_snapAgent
	process_zwave  $2
	process_modbus $2
	process_RuleEngineApp $2
	process_SchedulerApp $2
	process_Simulator $2
	;;
	"snapAgent")
	process_snapAgent
	;;
	"aj")
	process_app "" $2
	;;
	"ajDev")
	process_app	"$B_SELECT" $2
	;;
	"DevApp")
	process_app	"$B_SELECT" $2
	;;
	"MqttApp")
	process_app	"$B_SELECT" $2
	;;
	"zwave")
	process_zwave $2
	;;
	"ble")
	process_ble
	;;
	"zigbee")
	process_zigbee
	;;
	"opc")
	process_opc
	;;
	"modbus")
	process_modbus $2
	;;
	"mqtt")
	process_mqttlib $2
	;;
    "RuleEngineApp")
	process_RuleEngineApp $2
	;;
    "SchedulerApp")
	process_SchedulerApp $2
	;;
	"Simulator")
	process_Simulator $2
	;;
	"Dbug")
	process_Dbug
	;;
	"mkpack")
	process_mkpackage $2 $3 $4 $5 $6
	;;
	"clean")
	process_clean $2 $4
	;;
	"cleanall")
	process_clean aj
	process_clean snapAgent
	process_clean zwave
	process_clean modbus
	;;
	"help")
	process_help
	;;
	esac
}

if [ -z "$1" ];then
    echo "INFO : No argument supplied !!"
    echo -n "You want to compile whole project ??(y/n)"
    read _select
    if [ "y" == $_select ]||[ "yes" == $_select ];then
    	echo "INFO : Building whole project .. .."
    	process_switch all
    elif [ "n" == $_select ]||[ "no" == $_select ];then
		process_help
		echo -n "Please Select Servies (Ex. aj) - "
		read _getServices
		process_switch $_getServices
    fi
else
    for var in "$@"
    do
    	if [ "clean" == $var ]||[ "mkpack" == $var ];then
    		if [ -z "$2" ];then
    			echo "Please Select the target !!"
    			exit
    		else
    			process_switch "$var" $2 $3 $4 $5 $6
    			break
    		fi
    	fi
    	process_switch "$var" $2
    	break
    done
fi