#!/bin/bash

install_path=/usr/bin

if [ -d $install_path ]; then
	echo
else
	`sudo mkdir $install_path`;
fi;

`sudo cp -r ../runtime/X86Lib2/* $install_path`


`sudo cp COStreamC $install_path/COStreamC`



