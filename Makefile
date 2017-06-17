

#Compile the low-level (core) part of the library
low:
	make -C ./Low3d/

#Produce a shared library for use from c++ programs
cpp: low
	make -C ./High3d/ cpp
	-cp ./High3d/libhobby3d.so .

#Produce a python module (and a respective shared library)
#(you need to copy both to your python project enviromnent)
python: low
	make -C ./High3d/ python
	-cp ./High3d/_pyhobby3d.so .
	-cp ./High3d/pyhobby3d.py .

#Produce a jar with with java classes (and a respective native shared library)
#(you need to copy both to your environment, set -Djava.library.path acoordingly
#and call System.loadLibrary("jhobby3d"); in your code)
java: low
	make -C ./High3d/ java
	-cp ./High3d/libjhobby3d.so .
	-cp ./High3d/jhobby3d.jar .

#Produce shared libraries for all languages
all: low cpp python java


clean:
	make -C ./Low3d/ clean
	make -C ./High3d/ clean
	-rm ./*.so
	-rm ./*.py
	-rm ./*.jar
