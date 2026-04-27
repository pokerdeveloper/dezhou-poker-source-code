#!/bin/bash

####################
cd dz
#make clean
rm -f build/*.so
make -j2
cp build/*.so ../build/
cd ..

####################
####################
cd cowboy
#make clean
rm -f build/*.so
make -j2
cp build/*.so ../build/
cd ..

####################

####################
cd pineapple
#make clean
rm -f build/*.so
make -j2
cp build/*.so ../build/
cd ..

####################

