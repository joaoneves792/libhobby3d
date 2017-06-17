OBJS= ms3d.o Tex.o shader.o GLM.o Lights.o Shadows.o Text.o Body.o ms3d_wrap.o jms3d_wrap.o 
LOW_OBJS=../Low3d/*.o
JAVA_OBJS=ms3d.class Tex.class shader.class GLM.class Lights.class Shadows.class Text.class Body.class

CC=g++
CXXFLAGS = -O3 -fPIC -c 
PYTHON_CXXFLAGS = $(shell pkg-config --cflags python3) 
LDFLAGS  = -shared -I../Low3d/ -lGL -lGLEW -ljpeg -lpng
PYTHON_LDFLAGS = $(shell pkg-config --libs python3) 
JAVA_LDFLAGS = -I/usr/lib/jvm/java-8-openjdk/include -I/usr/lib/jvm/java-8-openjdk/include/linux

all: swig $(OBJS)
	$(CC) $(OBJS) $(LOW_OBJS) -o _ms3d.so $(LDFLAGS) 
	cat constants.py >> ms3d.py

#Really dumb rule, but swig generates a .cxx and make doesnt know what to do with it!
ms3d_wrap.o : ms3d_wrap.cxx
	$(CC) $(CXXFLAGS) $(PYTHON_CXXFLAGS) ms3d_wrap.cxx -o ms3d_wrap.o $(PYTHON_LDFLAGS) $(LDFLAGS)

jms3d_wrap.o : jms3d_wrap.cxx
	$(CC) $(CXXFLAGS) jms3d_wrap.cxx -o jms3d_wrap.o $(JAVA_LDFLAGS) $(LDFLAGS)

swig:
	swig -c++ -python ms3d.i
	mkdir -p java/src/warpenguin/joao/ms3d
	swig -c++ -java -package warpenguin.joao.ms3d -outdir java/src/warpenguin/joao/ms3d -o jms3d_wrap.cxx  jms3d.i

clean:
	-rm $(OBJS)      
	-rm ./*.java
	-rm -r ./java	
	-rm ms3d_wrap.cxx        
	-rm ms3d.py                 
	-rm _ms3d.so
