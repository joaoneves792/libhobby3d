
all:
	make -C ./Low3d/
	make -C ./High3d/
	cp ./High3d/_ms3d.so .
	cp ./High3d/ms3d.py .

clean:
	make -C ./Low3d/ clean
	make -C ./High3d/ clean
	-rm _ms3d.so
	-rm ms3d.py
