CXXFLAGS=-Wall -O3 -g -Wextra -Wno-unused-parameter -D_FILE_OFFSET_BITS=64
OBJECTS=main.o
BINARIES=main

OPTIONAL_OBJECTS=video-viewer.o
OPTIONAL_BINARIES=video-viewer

# Where our library resides. You mostly only need to change the
# RGB_LIB_DISTRIBUTION, this is where the library is checked out.
RGB_LIB_DISTRIBUTION=../rpi-rgb-led-matrix
RGB_INCDIR=$(RGB_LIB_DISTRIBUTION)/include
RGB_LIBDIR=$(RGB_LIB_DISTRIBUTION)/lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

# Imagemagic flags, only needed if actually compiled.
MAGICK_CXXFLAGS?=`GraphicsMagick++-config --cppflags --cxxflags`
MAGICK_LDFLAGS?=`GraphicsMagick++-config --ldflags --libs`
AV_CXXFLAGS=`pkg-config --cflags  libavcodec libavformat libswscale libavutil`

all : $(BINARIES)

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR)

main: main.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) main.o -o $@ $(LDFLAGS) $(MAGICK_LDFLAGS)

test: drawTest.o $(RGB_LIBRARY)
	$(CXX) $(CXXFLAGS) drawTest.o -o $@ $(LDFLAGS) $(MAGICK_LDFLAGS)

%.o : %.cpp
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) -c -o $@ $<

# We're using a couple of deprecated functions. Pull request to update this to
# the latest libraries is welcome.
video-viewer.o: video-viewer.cc
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) -Wno-deprecated-declarations -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(BINARIES) $(OPTIONAL_OBJECTS) $(OPTIONAL_BINARIES)

FORCE:
.PHONY: FORCE
