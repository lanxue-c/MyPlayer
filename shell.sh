#!/bin/sh
make distclean
qmake -project
qmake
make

