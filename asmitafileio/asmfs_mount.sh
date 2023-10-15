#!/usr/bin/env bash
asmfshost=$ASMFS_HOST
asmfsuser=$ASMFS_USER

if [ -z "$ASMFS_HOST" ] 
then
	echo "Variable ASMFS_HOST not set so existing"
	exit
else
	echo "host is  $ASMFS_HOST"
fi


if [ -z "$ASMFS_USER" ] 
then
	echo "Variable ASMFS_USER not set so existing"
	exit
else
	echo "host is  $ASMFS_USER"
fi


./asmfs $ASMFS_HOST $ASMFS_USER  rootdir mountdir

