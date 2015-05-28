CXXFLAGS=-Wall -O3 -g
SRC= $(wildcard *.cc)
CPPOBJECTS=$(SRC:.cc=.o)
BINARIES=infodisplay

RGB_INCDIR=matrix/include
RGB_LIBDIR=matrix/lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a

NRF_INCDIR=nrflib/
NRF_LIBDIR=nrflib/
NRF_LIBRARY_NAME=nrflib
NRF_LIBRARY=$(NRF_LIBDIR)/lib$(NRF_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -L$(NRF_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

all : infodisplay

infodisplay : $(CPPOBJECTS) $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) $(CPPOBJECTS) -o $@ $(LDFLAGS)

$(RGB_LIBRARY):
	$(MAKE) -C $(RGB_LIBDIR)
	
$(NRF_LIBRARY):
	$(MAKE) -C $(NRF_LIBDIR)

%.o : %.cc
	$(CXX) -I$(RGB_INCDIR) -I$(NRF_INCDIR) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(CPPOBJECTS) $(BINARIES)
	$(MAKE) -C $(RGB_LIBDIR) clean
	$(MAKE) -C $(NRF_LIBDIR) clean
