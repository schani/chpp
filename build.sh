#!/bin/sh

aclocal
autoconf
automake
./configure "$@"
