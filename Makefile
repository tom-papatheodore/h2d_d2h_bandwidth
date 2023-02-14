#----------------------------------------
# Variables set in setup_environment.sh
#   - name_of_system
#   - HIPFLAGS
#----------------------------------------

ifndef name_of_system
$(error name_of_system is not set)
else
$(info Current system: $(name_of_system))
endif

#----------------------------------------

HIPCC = hipcc

HIPFLAGS +=

#----------------------------------------

bandwidth: bandwidth.o
	$(HIPCC) $(HIPFLAGS) bandwidth.o -o bandwidth

bandwidth.o: bandwidth.cpp
	$(HIPCC) $(HIPFLAGS) -c bandwidth.cpp

.PHONY: clean

clean:
	rm -f bandwidth *.o
