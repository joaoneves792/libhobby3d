OBJS= MS3DFile.o optimizations.o Shader.o MS3DFileIO.o Textures.o

CC=g++
CXXFLAGS=-c -Wall -O3 -fPIC #-DGLES
#CXXFLAGS=-c -ggdb -g3  -Wall -fPIC #-DGLES
LDFLAGS=-O3 -shared -Wl,-soname,libms3d.so.1
LIBS=-lGL -lGLEW -ljpeg -lpng 

all: $(OBJS)
	$(CC) $(LDFLAGS) -o libms3d.so.1.0.1 $(OBJS)

optimizer:
	$(CC) -o optimizer optimizer.cpp -lms3d $(LIBS)

install:
	cp libms3d.so.1.0.1 /usr/lib
	ln -sf /usr/lib/libms3d.so.1.0.1 /usr/lib/libms3d.so
	ln -sf /usr/lib/libms3d.so.1.0.1 /usr/lib/libms3d.so.1
	mkdir -p /usr/include/ms3d
	cp MS3DFile.h Textures.h Shader.h /usr/include/ms3d/

clean:
	-rm $(OBJS)
	-rm libms3d.so.1.0.1
	-rm optimizer
