#----------------------------------------

HIPCC    = hipcc
HIPFLAGS = --offload-arch=gfx90a

#----------------------------------------

bandwidth: bandwidth.o
	$(HIPCC) $(HIPFLAGS) bandwidth.o -o bandwidth

bandwidth.o: bandwidth.cpp
	$(HIPCC) $(HIPFLAGS) -c bandwidth.cpp

.PHONY: clean

clean:
	rm -f bandwidth *.o
