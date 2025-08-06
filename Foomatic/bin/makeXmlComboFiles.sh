#!/bin/sh
# 
# Ryan Harper - raharper@us.ibm.com
#
# This script will take the output xml files from Omni/Foomatic/Foomatic and run them 
# through foomatic/foomatic-combo-xml.  This binary will convert the 3 types of foomatic
# xml files into a combo xml file.  
#
#
# This shell should reside in Omni/Foomatic/bin

# Change this as needed, its the location and name of the foomatic combo compiler
#FOOMATIC_COMPILER=/usr/src/foomatic/foomatic-combo-xml
FOOMATIC_COMPILER=/usr/bin/foomatic-combo-xml

# this is the dir of the output from Omni/Foomatic/Foomatic
#KIT_DIR=/usr/src/Omni/Foomatic/foomatic-db/
KIT_DIR=/home/Omni/Omni.C++/Foomatic/foomatic-db/

# this is your driver name
DRIVERNAME="omni"

# this is where you want the subsequent combo xml files
OUTPUT_DIR=$KIT_DIR/db/combo/$DRIVERNAME

# make a place for the output
mkdir -p $OUTPUT_DIR

# go into the base of the xml files
cd $KIT_DIR/db/source/printer
for each in $(ls *.xml)
do
   # save a copy of the file name
   OUTNAME=$each
   # trim off the .xml
   each=${each%*.xml}
   printf "%-43s" "Compiling $each..."
   # run each printer through this, -p is printername, -d is drivername, and -l is location of the foomatic-db directory
   $FOOMATIC_COMPILER -p $each -d $DRIVERNAME -l $KIT_DIR > $OUTPUT_DIR/$OUTNAME
   printf "%s %s\n" "done" " Writing-> $OUTNAME"
done

