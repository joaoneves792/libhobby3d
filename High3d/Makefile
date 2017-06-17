OBJS= ms3d.o Tex.o shader.o GLM.o Lights.o Shadows.o Text.o Body.o 
LOW_OBJS=../Low3d/*.o

PYTHON_WRAPPER=hobby3d_python_wrap.o
JAVA_WRAPPER=hobby3d_java_wrap.o 

JAVA_PACKAGE=./java/src/warpenguin/hobby3d/

CC=g++
JC=javac
CLASSPATH=./java/src/warpenguin/hobby3d/
CXXFLAGS = -O3 -fPIC -c
PYTHON_CXXFLAGS = $(shell pkg-config --cflags python3) 
LDFLAGS= -shared -I../Low3d/ -lGL -lGLEW -ljpeg -lpng
PYTHON_LDFLAGS = $(shell pkg-config --libs python3) 
JAVA_LDFLAGS = -I/usr/lib/jvm/java-8-openjdk/include -I/usr/lib/jvm/java-8-openjdk/include/linux

all: cpp python java
	

cpp: $(OBJS)
	$(CC) $(OBJS) $(LOW_OBJS) -o libhobby3d.so $(LDFLAGS) 



#######################################
# 	JAVA
#######################################
swig-Java:
	mkdir -p $(JAVA_PACKAGE)
	swig -c++ -java -package warpenguin.hobby3d -outdir $(JAVA_PACKAGE) -o hobby3d_java_wrap.cxx  jhobby3d.i

java: swig-Java $(OBJS) $(JAVA_WRAPPER)
	$(CC) $(JAVA_WRAPPER) $(OBJS) $(LOW_OBJS) -o libjhobby3d.so $(LDFLAGS)
	$(JC) -cp $(CLASSPATH) $(JAVA_PACKAGE)*.java
	cd ./java/src && jar cvf jhobby3d.jar warpenguin/hobby3d/*.class
	mv ./java/src/jhobby3d.jar .

hobby3d_java_wrap.o : hobby3d_java_wrap.cxx
	$(CC) $(CXXFLAGS)  $^ -o $@ $(JAVA_LDFLAGS) 
	#$(LDFLAGS)


#######################################
# 	PYTHON
#######################################
swig-Python:
	swig -c++ -python -o hobby3d_python_wrap.cxx pyhobby3d.i
	
python: swig-Python $(OBJS) $(PYTHON_WRAPPER)
	$(CC) $(OBJS) $(LOW_OBJS) $(PYTHON_WRAPPER) -o _pyhobby3d.so $(LDFLAGS) 
	cat constants.py >> pyhobby3d.py

hobby3d_python_wrap.o : hobby3d_python_wrap.cxx
	$(CC) $(CXXFLAGS) $(PYTHON_CXXFLAGS) $^ -o $@ $(PYTHON_LDFLAGS)

#######################################
# 	CLEAN
#######################################
clean:
	-rm $(OBJS)
	-rm ./*.o	
	-rm ./*.java
	-rm -r ./java
	-rm ./jhobby3d.jar	
	-rm hobby3d_python_wrap.cxx
	-rm hobby3d_java_wrap.cxx	
	-rm pyhobby3d.py 
	-rm *.so
