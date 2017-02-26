#!/bin/bash

sudo apt-get install gcc g++ python python-dev mercurial python-setuptools git qt4-dev-tools libqt4-dev cmake libc6-dev libc6-dev-i386 g++-multilib gdb valgrind gsl-bin flex bison libfl-dev tcpdump sqlite sqlite3 libsqlite3-dev libxml2 libxml2-dev libgtk2.0-0 libgtk2.0-dev vtun lxc uncrustify texlive texlive-extra-utils texlive-latex-extra texlive-font-utils texlive-lang-portuguese dvipng doxygen graphviz imagemagick python-sphinx dia python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev ipython libboost-signals-dev libboost-filesystem-dev openmpi-bin openmpi-common openmpi-doc libopenmpi-dev

hg clone http://code.nsnam.org/ns-3-allinone

cd ns-3-allinone

./download.py -n ns-3.26

./build.py --enable-examples --enable-tests

cd ns-3.26

./test.py -c core

./waf --run hello-simulator
