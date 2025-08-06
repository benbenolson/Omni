#!/bin/sh

# this script will install the modified xml files into the foomatic database
# directory.  Usually, /usr/share/foomatic, but there may be a conflict with
# /usr/local/share/foomatic.  To avoid conflict and pain and anguish, maybe
# you should either, make sure that Foomatic is using one path or the other,
# (check Makefile in /usr/src/foomatic, change the PREFIX) or do a nasty hack
# like I did, symlink /usr/local/share/foomatic to /usr/share/foomatic.


# OK, now, I suppose I should check to make sure that all the other steps havce
# already been done, so, the following directories should exists
#
# /usr/src/Omni/Foomatic/foomatic-db
# /usr/src/Omni/Foomatic/foomatic-db/db
# /usr/src/Omni/Foomatic/foomatic-db/db/source
# /usr/src/Omni/Foomatic/foomatic-db/db/source/combo
# /usr/src/Omni/Foomatic/foomatic-db/db/source/combo/omni
# /usr/src/Omni/Foomatic/foomatic-db/db/source/driver
# /usr/src/Omni/Foomatic/foomatic-db/db/source/opt
# /usr/src/Omni/Foomatic/foomatic-db/db/source/printer

#OMNI_HOME=/usr/src/Omni
OMNI_HOME=/home/Omni/Omni.C++
#FOOMATIC_HOME=/usr/local/share/foomatic
FOOMATIC_HOME=/usr/share/foomatic

if [ ! -d $OMNI_HOME/Foomatic/foomatic-db ]; then
   echo "Run $OMNI_HOME/Foomatic/Foomatic first to generate the foomatic-db dir"
   exit 1
fi

# now we just copy over all the omni driver xml files into the foomatic dir
OLD_DIR=$(pwd)
cp -aRv $OMNI_HOME/Foomatic/foomatic-db/db/source/driver/* $FOOMATIC_HOME/db/source/driver
cp -aRv $OMNI_HOME/Foomatic/foomatic-db/db/source/opt/* $FOOMATIC_HOME/db/source/opt
cd $OMNI_HOME/Foomatic/foomatic-db/db/source/printer/
for each in $(ls) 
do
   if [ ! -f $FOOMATIC_HOME/db/source/printer/$each ]; then
      cp -v $each $FOOMATIC_HOME/db/source/printer/
   else
      echo "$FOOMATIC_HOME/db/source/printer/$each already exists."
   fi
done
cd $OLD_DIR

cp -aRv $OMNI_HOME/Foomatic/foomatic-db/db/combo/* $FOOMATIC_HOME/db/compiled/combo

# thats it!



