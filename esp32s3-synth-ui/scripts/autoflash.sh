#!/bin/bash
help()
{
   # Display Help
   echo "Flash script."
   echo
   echo "-f is uf2 file folder"
   echo "Default output path is /build/autput"
   echo
   echo "-p is port"
   echo "Default output path is /dev/cu.usbmodem1114101"
   echo
   echo "-b is baud reat"
   echo "Default baud rate is 460800"
   echo
}

path=./build/autput
exec_path="$(cd "$(dirname "$0")" && pwd)"
port=/dev/cu.usbmodem1114101
baud=460800

while getopts ":hn:f:p:b:" option; do
   case $option in
      h) # display Help
         help
         exit;;
      f)
        path=$exec_path/$OPTARG;;
      p)
       port=$OPTARG;;
      b) 
        baud=$OPTARG;;

   esac
done

esptool.py --chip esp32s3 --baud $baud --port $port --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 80m --flash_size 2MB 0x0 $path/bootloader.bin 0x8000 $path/partition-table.bin 0x10000 $path/esp3ss3-synth-ui.bin

