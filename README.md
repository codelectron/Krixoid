Krixoid
=======

Krixoid is a rapid prototyping library for Linux application development. It follows Arduino style syntax to provide easy user interface and rich set of APIs.The library is specifically written for Single board computers running Linux such as Raspberry PI, Beagle bone etc.

Instruction to use:
After downloading the source, your directory structure should contain src and examples directories.

Compile and install Krixoid:

compile the library as follows

$cd src

$make

After it gets compiled successfully, you can install as follows. 

$sudo make install

The example folder contains directory containing sample codes with makefiles. 

Twitter support has been added but it works only GCC 4.7 and after. 
The code used has gnu extensions for C and that is not supported by G++4.6

In order to get it work, uncomment the '#' in the makefile and install G++4.7

$sudo apt-get install g++-4.7 

Also you need to link the path such that it look like this 

/usr/bin/g++ -> /usr/bin/g++-4.7

It can be done using the below command

$ln -sf /usr/bin/g++  /usr/bin/g++-4.7

