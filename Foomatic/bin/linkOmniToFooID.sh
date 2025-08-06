#!/bin/sh
#
# Ryan Harper - raharper@us.ibm.com

# this script will use two files and two scripts to gather information and check to see if any of the
# omni printers have a Foomatic Printer ID, if so, it will then make a symlink to the Omni name based 
# on the printer id number.  For instance, Omni supports the Epson_Stylus_Color_740, ESC740 also has
# a foo id of 62112, so, this script will symlink (ln -sf Epson_Stylus_Color_740.xml 62112.xml) to the
# foo id.  This will let printcont 0.3.23 support omni on printers that already are supported by other
# print drivers, such as stp.  

# hash pairs of PrinterName:FooID
FOOMATIC_DB=`pwd`/foomaticDB
# output dir
OMNI_COMBO_DIR=/usr/src/Omni/Foomatic/foomatic-db/db/combo/omni

cd $OMNI_COMBO_DIR
for each in $(ls )
do
   each=${each%.*}
   FOOID=$(grep -i $each: $FOOMATIC_DB | awk -F\: '{print $2;}')
   if [ "$FOOID" != "" ]; then
      printf "%-50s" "Linking $each to $FOOID.xml ..."
      ln -sf $each.xml $FOOID.xml
      printf "%s\n" "done"
   fi
done
