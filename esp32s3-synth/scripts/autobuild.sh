#!/bin/bash

help()
{
   # Display Help
   echo "Build script."
   echo
   echo "-n is device id"
   echo "Default device id is ZERO"
   echo
   echo "-o is output path"
   echo "Default output path is /build/autput"
   echo
   echo "-l is console log level"
   echo "Default is NONE. replace with DEBUG if needed"
   echo 
}

id=ZERO
path=./autput
exec_path="$(cd "$(dirname "$0")" && pwd)"
log=NONE 

while getopts ":hn:o:l:" option; do
   case $option in
      h) # display Help
         help
         exit;;
      n) 
        id=$OPTARG;;
      o)
        path=$exec_path/$OPTARG;;
      l)
        log=$OPTARG;;
      *)
        echo invalid flag
        exit;;

   esac
done

echo "Builing binary"
echo "Device id $id"
echo "Output Path $path"

cd "$(dirname "$0")" || exit

cd ../

sed -i '' "s/CONFIG_DEVICE_ID=.*/CONFIG_DEVICE_ID=\"$id\"/" sdkconfig
sed -i '' "s/CONFIG_DEVICE_ID=.*/CONFIG_DEVICE_ID=\"$id\"/" sdkconfig.defaults
sed -i '' "s/CONFIG_LOG_DEFAULT_LEVEL_.*/CONFIG_LOG_DEFAULT_LEVEL_$log=y/" sdkconfig
sed -i '' "s/CONFIG_LOG_DEFAULT_LEVEL_.*/CONFIG_LOG_DEFAULT_LEVEL_$log=y/" sdkconfig.defaults

# rm -rf build
mkdir -p build
cd build


cmake -G Ninja -DPYTHON_DEPS_CHECKED=1 -DESP_PLATFORM=1 -DCCACHE_ENABLE=0 -DIDF_TARGET=esp32c3   ..
# -DPYTHON=~/.espressif/python_env/idf5.3_py3.12_env/bin/python 

ninja all

mkdir -p $path
cp ./esp-metalbox-switch-clean.bin $path/$id.bin
cp ./bootloader/bootloader.bin $path/bootloader.bin
cp ./partition_table/partition-table.bin $path/partition-table.bin

