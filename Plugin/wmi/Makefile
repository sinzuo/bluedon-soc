top_srcdir = ../..
include $(top_srcdir)/Make.rules

TARGETS    = bdwmi.so

CXXFLAGS  += -fPIC


SRCS       =$(wildcard *.cpp \
                       $(top_srcdir)/utils/*.cpp)
OBJS       =$(patsubst %.cpp, %.o, $(SRCS))
#SRCS_ND    =$(notdir $(SRCS))
#OBJS_ND    =$(patsubst %.cpp, %.o, $(SRCS_ND)) 

INCLUDE    =-I. \
            -I$(top_srcdir) \
            -I$(top_srcdir)/../$(ACE_INCLUDE) \
            -I$(top_srcdir)/../$(POCO_INCLUDE) \
	    -I$(top_srcdir)/../$(PBUF_INCLUDE) \
            -I$(top_srcdir)/../$(LOG4XX_INCLUDE)

LDLIBS     =
LDFLAGS    =-L.



all: $(TARGETS)

$(TARGETS): $(OBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LDLIBS) $(LDFLAGS) $^
	-@cp -fv $(TARGETS) $(top_srcdir)/../lib/

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE)

clean:
	-@rm -fv $(OBJS)
