OBJS= ms3d.o Tex.o shader.o GLM.o Lights.o Shadows.o Text.o Body.o 
LOW_OBJS=../Low3d/*.o

PYTHON_WRAPPER=hobby3d_python_wrap.o
JAVA_WRAPPER=jms3d_wrap.o 

JAVA_OBJS=ms3d.class Tex.class shader.class GLM.class Lights.class Shadows.class Text.class Body.class

CC=g++
CXXFLAGS = -O3 -fPIC -c 
PYTHON_CXXFLAGS = $(shell pkg-config --cflags python3) 
LDFLAGS  = -shared -I../Low3d/ -lGL -lGLEW -ljpeg -lpng
PYTHON_LDFLAGS = $(shell pkg-config --libs python3) 
JAVA_LDFLAGS = -I/usr/lib/jvm/java-8-openjdk/include -I/usr/lib/jvm/java-8-openjdk/include/linux

all: cpp python
	

cpp: $(OBJS)
	$(CC) $(OBJS) $(LOW_OBJS) -o libhobby3d.so $(LDFLAGS) 

python: swig-Python $(OBJS) $(PYTHON_WRAPPER)
	$(CC) $(OBJS) $(LOW_OBJS) $(PYTHON_WRAPPER) -o _pyhobby3d.so $(LDFLAGS) 
	cat constants.py >> pyhobby3d.py


hobby3d_python_wrap.o : hobby3d_python_wrap.cxx
	$(CC) $(CXXFLAGS) $(PYTHON_CXXFLAGS) $^ -o $@ $(PYTHON_LDFLAGS) $(LDFLAGS)

jms3d_wrap.o : jms3d_wrap.cxx
	$(CC) $(CXXFLAGS) jms3d_wrap.cxx -o jms3d_wrap.o $(JAVA_LDFLAGS) $(LDFLAGS)

swig-Python:
	swig -c++ -python -o hobby3d_python_wrap.cxx pyhobby3d.i
	
swig-Java:
	mkdir -p java/src/warpenguin/joao/ms3d
	swig -c++ -java -package warpenguin.joao.ms3d -outdir java/src/warpenguin/joao/ms3d -o jms3d_wrap.cxx  jms3d.i

clean:
	-rm $(OBJS)      
	-rm ./*.java
	-rm -r ./java	
	-rm hobby3d_python_wrap.cxx        
	-rm hobby3d.py                 
	-rm *.so
