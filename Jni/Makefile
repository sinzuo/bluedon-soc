
top_srcdir = ..
include $(top_srcdir)/Make.rules

TARGETS    = libbdsocjni.so

CXXFLAGS  += -fPIC

SRCS       =$(wildcard *.cpp \
                       $(top_srcdir)/utils/*.cpp \
                       $(top_srcdir)/crypt/*.cpp)
OBJS       =$(patsubst %.cpp, %.o, $(SRCS)) 
#SRCS_ND    =$(notdir $(SRCS))
#OBJS_ND    =$(patsubst %.cpp, %.o, $(SRCS_ND)) 

INCLUDE    =-I. \
            -I$(top_srcdir) \
            -I$(top_srcdir)/../$(ACE_INCLUDE) \
            -I$(top_srcdir)/../$(POCO_INCLUDE) \
            -I$(top_srcdir)/../$(LOG4XX_INCLUDE) \
            -I$(top_srcdir)/../$(GEOIP_INCLUDE) \
            -I$(top_srcdir)/../$(PBUF_INCLUDE) \
            -I$(JAVA_HOME)/include \
            -I$(JAVA_HOME)/include/linux

LDLIBS     =$(ACE_LIBS) \
            $(POCO_LIBS) \
            $(LOG4XX_LIBS) \
            $(GEOIP_LIBS)


LDFLAGS    =-L. \
            -L$(top_srcdir)/../$(ACE_LDFLAGS) \
            -L$(top_srcdir)/../$(LOG4XX_LDFLAGS) \
            -L$(top_srcdir)/../$(POCO_LDFLAGS) \
            -Wl,-rpath=$(top_srcdir)/../$(POCO_LDFLAGS) \
            -L$(top_srcdir)/../$(GEOIP_LDFLAGS) \
            #-Wl,-rpath=$(top_srcdir)/../$(GEOIP_LDFLAGS) \
            #-Wl,-rpath=$(GEOIP_LDFLAGS)





all: $(TARGETS)

$(TARGETS): $(OBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LDLIBS) $(LDFLAGS) $^
#	-@cp -fv $(TARGETS) ../../bin/
#	-@cp -fv $(TARGETS) /usr/local/socserver/lib/

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE)

clean:
	-@rm -fv $(OBJS)







