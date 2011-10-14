#!/bin/sh

aclocal
autoheader
automake
autoconf
./configure "$@"
