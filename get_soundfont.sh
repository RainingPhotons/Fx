#!/bin/bash

ARCHIVE=GeneralUser_GS_1.471.zip
SF2=winter_lights_2020.sf2

wget -nc https://www.dropbox.com/s/4x27l49kxcwamp5/${ARCHIVE}
unzip -p ${ARCHIVE} "GeneralUser GS 1.471/GeneralUser GS v1.471.sf2" > ${SF2}