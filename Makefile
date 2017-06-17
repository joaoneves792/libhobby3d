
all:
	make -C ./Low3d/
	make -C ./High3d/
	-cp ./High3d/_pyhobby3d.so .
	-cp ./High3d/pyhobby3d.py .
	-cp ./High3d/libhobby3d.so .

clean:
	make -C ./Low3d/ clean
	make -C ./High3d/ clean
	-rm ./*.so
	-rm ./*.py
