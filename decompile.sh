#!/usr/bin/bash

#create directory glXXX, where XXX is release number without periods
#place apk file glXXX.apk and this script into directory glXXX
#run the script, while passing the name of apk file. i.e.: ./_decompile.sh gl180.apk



#https://gl.akamai.tap4fun.com/client/20161221_extra/Android/extraFleetInfo/v1/ExtraFleetInfo.tfl
#https://gl.akamai.tap4fun.com/client/20161221_extra/Android/extraFleetInfo/v2/ExtraFleetInfo.tfl
#https://gl.akamai.tap4fun.com/client/20161221_extra/Android/text/EXTRA_TEXT.tfl
#https://gl.akamai.tap4fun.com/client/20161221_extra/Android/netProtocal/AlertDataListExtra.tfl
#https://gl.akamai.tap4fun.com/client/20161221_extra/Android/images/pcl.tfl

set -e

export PATH=$PATH:.

XX_APK_FILENAME="$1"
XX_TOOLS_ARCHIVE="gl_tools.zip"
XX_ASSETS_ARCHIVE="tap4fun.zip"

if [ -z "${XX_APK_FILENAME}" ]; then
  echo "APK file path parameter required."
  exit 1
fi

if [ ! -f "${XX_TOOLS_ARCHIVE}" ]; then
  echo "Tools file '${XX_TOOLS_ARCHIVE}' not found."
  exit 1
fi

XX_PWD=`pwd`
XX_APK_NAME=`basename "${XX_APK_FILENAME}" | awk -F . '{print $1}'`
XX_APK_PATH="${XX_PWD}/${XX_APK_FILENAME}"

#set directory permissions
chmod 777 ./

cd ${XX_PWD}

#extract tools
if [ ! -f "xortool.exe" ]; then
  echo "Unzipping ${XX_TOOLS_ARCHIVE}"
  unzip -n "${XX_TOOLS_ARCHIVE}" >/dev/null
fi

#extract assets file
if [ ! -f "${XX_ASSETS_ARCHIVE}" ]; then
  echo "Unzipping ${XX_ASSETS_ARCHIVE} from ${XX_APK_FILENAME}"
  unzip -n "${XX_APK_FILENAME}" "assets/${XX_ASSETS_ARCHIVE}" >/dev/null
  mv "assets/${XX_ASSETS_ARCHIVE}" "${XX_PWD}"
  rm -rf "assets"
fi

#extract asset files
if [ ! -f "data1.pak" ]; then
  echo "Unzipping specific files from ${XX_ASSETS_ARCHIVE}"
  unzip -n "${XX_ASSETS_ARCHIVE}" "tap4fun/galaxylegend/AppOriginalData/*.pak" >/dev/null
  unzip -n "${XX_ASSETS_ARCHIVE}" "tap4fun/galaxylegend/AppOriginalData/*.tfl" >/dev/null
  unzip -n "${XX_ASSETS_ARCHIVE}" "tap4fun/galaxylegend/AppOriginalData/data2/text/*" >/dev/null
  unzip -n "${XX_ASSETS_ARCHIVE}" "tap4fun/galaxylegend/AppOriginalData/data2/*.tfs" >/dev/null
  cp -rf "tap4fun/galaxylegend/AppOriginalData/"* "${XX_PWD}"
  rm -rf "tap4fun"
fi

#unpack data archives
XX_PATHS=`find -maxdepth 1 -name "*.pak"` || exit 1
for XX_PATH in ${XX_PATHS}
do
  echo "Checking if need unpacking ${XX_PATH}"
  XX_DIR=`dirname "${XX_PATH}"`
  XX_FILE=`basename "${XX_PATH}"`
  XX_NAME=`echo "${XX_FILE}" | awk -F . '{print $1}'`
  if [ ! -d "${XX_DIR}/${XX_NAME}" ]; then
    echo ".. Unpacking ${XX_PATH}"
    xortool "${XX_PATH}" "${XX_DIR}/${XX_NAME}" 4 >/dev/null
  fi
done

#decrypt program files
XX_PATHS=`find -maxdepth 1 -name "*.tfl"` || exit 1
for XX_PATH in ${XX_PATHS}
do
  echo "Checking if need decrypting ${XX_PATH}"
  XX_DIR=`dirname "${XX_PATH}"`
  XX_FILE=`basename "${XX_PATH}"`
  XX_NAME=`echo "${XX_FILE}" | awk -F . '{print $1}'`
  if [ ! -f "${XX_DIR}/${XX_NAME}.luaq" ]; then
    echo ".. Decrypting ${XX_PATH}"
    xortool "${XX_PATH}" "${XX_DIR}/${XX_NAME}.luaq" 2 >/dev/null
  fi
done

#uncompress program files
XX_DIR_PATHS=`find -maxdepth 1 -type d -name "data*"` || exit 1
for XX_DIR_PATH in ${XX_DIR_PATHS}
do
  XX_PATHS=`find "${XX_DIR_PATH}" -name "*.tfl"` || exit 1
  for XX_PATH in ${XX_PATHS}
  do
    echo "Checking if need uncompressing ${XX_PATH}"
    XX_DIR=`dirname "${XX_PATH}"`
    XX_FILE=`basename "${XX_PATH}"`
    XX_NAME=`echo ${XX_FILE} | awk -F . '{print $1}'`
    if [ ! -f "${XX_DIR}/${XX_NAME}.luaq" ]; then
      #echo ".. Uncompresing ${XX_PATH}"
      #zlibtool "${XX_PATH}" "${XX_DIR}/${XX_NAME}.luaq" 2 >/dev/null
      echo ".. Decrypting ${XX_PATH}"
      xortool "${XX_PATH}" "${XX_DIR}/${XX_NAME}.luaq" 2 >/dev/null
    fi
  done
done

#parse text files
XX_PATHS=`find "${XX_PWD}/data2/text" -maxdepth 1 -name "*.idx"` || exit 1
for XX_PATH in ${XX_PATHS}
do
  echo "Checking if need parsing ${XX_PATH}"
  XX_DIR=`dirname "${XX_PATH}"`
  XX_FILE=`basename "${XX_PATH}"`
  XX_NAME=`echo ${XX_FILE} | awk -F . '{print $1}'`
  if [[ ! -f "${XX_DIR}/${XX_NAME}.csv" || ! -s "${XX_DIR}/${XX_NAME}.csv" ]]; then
    echo ".. Parsing ${XX_PATH}"
    java texttool `cygpath -w "${XX_PATH}"` `cygpath -w "${XX_DIR}/${XX_NAME}.en"` > "${XX_DIR}/${XX_NAME}.csv"
    if [ ! -s "${XX_DIR}/${XX_NAME}.csv" ]; then
      rm -f "${XX_DIR}/${XX_NAME}.csv"
    fi
  fi
done

#create text archive
if [ ! -f "${XX_APK_NAME}_text.zip" ]; then
  XX_TEXT_FILES=`find -name "*.csv" -printf '%p '`
  zip -u -j ${XX_APK_NAME}_text.zip ${XX_TEXT_FILES}
fi

#decompile program files
XX_PATHS=`find -name "*.luaq"` || exit 1
for XX_PATH in ${XX_PATHS}
do
  echo "Checking if need decompiling ${XX_PATH}"
  XX_DIR=`dirname "${XX_PATH}"`
  XX_FILE=`basename "${XX_PATH}"`
  XX_NAME=`echo ${XX_FILE} | awk -F . '{print $1}'`
  if [[ ! -f "${XX_DIR}/${XX_NAME}.lua" || ! -s "${XX_DIR}/${XX_NAME}.lua" ]]; then
    echo ".. Decompiling ${XX_PATH}"
    java unluac/Main `cygpath -w "${XX_PATH}"` > "${XX_DIR}/${XX_NAME}.lua"
    if [ ! -s "${XX_DIR}/${XX_NAME}.lua" ]; then
      rm -f "${XX_DIR}/${XX_NAME}.lua"
    fi
  fi
done

#create program archive
if [ ! -f "${XX_APK_NAME}_lua.zip" ]; then
  XX_LUA_FILES=`find -name "*.lua" -printf '%p '`
  zip -u -j ${XX_APK_NAME}_lua.zip ${XX_LUA_FILES}
fi
