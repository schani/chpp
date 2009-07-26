#!/bin/sh

aclocal
autoconf
automake
autoheader
./configure "$@"
