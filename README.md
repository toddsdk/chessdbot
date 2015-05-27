ChessD Bot
==========

A Free Chess Engine, intended to be used by children and teenagers learning how
to play Chess.


Compiling
=========

To compile the source code of ChessD Bot, just type in a terminal:

make


Playing
=======

To play on the terminal, just type:

chessdbot

To choose a level ranging from 1% to 100% (i.e. 25%), type either with or without the "%":

chessdbot -l 25%

To play using a Chess interface (i.e. xboard), just type:

xboard -fcp "./bin/chessdbot -l 25"


Running ChessD Bot with ChessD Server
=====================================

To run a chessdbot as a user of a ChessD Server you have to use the bots.py
program (./bin/bots.py), but first you need to configure it by editing the file
config.xml . You can see the commented examples inside of config.xml, to learn
how to configure bots.py.
With bots.py configured, just type the following commands to get it running:

cd bin/
bots.py -c ../config.xml

Contact
=======

C3SL - Center for Scientific Computing and Free Software
e-mail: xadrez-devel@c3sl.ufpr.br
