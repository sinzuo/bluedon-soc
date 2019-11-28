top_srcdir = ../..
include $(top_srcdir)/Make.rules

TARGETS    = bddocker.so

CXXFLAGS  += -fPIC

SRCS       =$(wildcard *.cpp \
                       $(top_srcdir)/utils/*.cpp \
                       $(top_srcdir)/exception/*.cpp \
                       $(top_srcdir)/crypt/*.cpp \
                       $(top_srcdir)/Plugin/share/netbase/*.cpp \
					   $(top_srcdir)/Plugin/share/https/*.cpp )
					   
OBJS       =$(patsubst %.cpp, %.o, $(SRCS))
#SRCS_ND    =$(notdir $(SRCS))
#OBJS_ND    =$(patsubst %.cpp, %.o, $(SRCS_ND)) 


INCLUDE    =-I. \
            -I$(top_srcdir) \
            -I$(top_srcdir)/$(PLUGIN_SHARE_INCLUDE) \
            -I$(top_srcdir)/../$(ACE_INCLUDE) \
            -I$(top_srcdir)/../$(POCO_INCLUDE) \
            -I$(top_srcdir)/../$(LOG4XX_INCLUDE) \
	    -I$(top_srcdir)/../$(PBUF_INCLUDE) \
			-I$(top_srcdir)/../$(JSON_INCLUDE)

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
