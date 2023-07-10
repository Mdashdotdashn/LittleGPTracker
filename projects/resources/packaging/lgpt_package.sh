#!/bin/bash
BUILD="$(grep -oP 'BUILD_COUNT [^"]*"\K[^"]*' ../sources/Application/Model/Project.h)"
VERSION=1.3o_$BUILD
PACKAGE=LGPT-$VERSION.zip

collect_resources() { #1PLATFORM #2lgpt.*-exe
  # if [[ ! -f $2 ]]; then
  if [[ -n $(find -name "$2") ]]; then
    echo "" # Found it
  else
    echo "-->file $2 not found<---"
    return;
  fi
  PACKAGE=LGPT-$1-$VERSION.zip
  echo "-=-=Packaging $PACKAGE=-=-"
  CONTENTS="./resources/$1/*"
  CONTENTS+=" $(find -iname $2)"
  if [ "$1" == "PSP" ]; then # PSP files go in the root folder
    zip -9 $PACKAGE -j $CONTENTS
  else # all the others go in the bin
    mkdir bin ; cp $CONTENTS bin
    zip -9 $PACKAGE bin/* && rm -r bin/
  fi
  cd ./resources/packaging 
  CONTENTS="$(find -name README.txt)"
  CONTENTS+=" samplelib/*wav lgpt_ALPHA/* lgpt_ALPHA/samples/*"
  zip -9 ../../$PACKAGE $CONTENTS && cd -
}

rm -f buildPSP/lgpt.elf
collect_resources PSP EBOOT.PBP
collect_resources DEB lgpt.deb-exe
collect_resources STEAM lgpt.steam-exe
collect_resources RS97 lgpt.dge
collect_resources BITTBOY lgpt.elf
collect_resources W32 lgpt.exe