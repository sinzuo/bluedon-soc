
top_srcdir = ..
include $(top_srcdir)/Make.rules

TARGETS    = sinzuotest


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
            -I$(top_srcdir)/../$(LOG4XX_INCLUDE) \
            -I$(top_srcdir)/../$(ZMQ_INCLUDE) \
            -I$(top_srcdir)/../$(SNMP_INCLUDE) \
            -I$(top_srcdir)/../$(JSON_INCLUDE) \
            -I$(top_srcdir)/../$(SSH2_INCLUDE) \
            -I$(top_srcdir)/../$(CURL_INCLUDE) 

LDLIBS     =$(ACE_LIBS) -lssl -lcrypto \
            $(POCO_LIBS) \
            $(PBUF_LIBS) \
            $(LOG4XX_LIBS) \
            $(ZMQ_LIBS) \
            $(SNMP_LIBS) \
            $(JSON_LIBS) \
            $(CURL_LIBS) \
            $(SSH2_LIBS) \
	    $(SOC_WEAK_LIBS) \
	    $(PTHREAD_LIBS) 

LDFLAGS    =-L. \
            -L$(top_srcdir)/../$(ACE_LDFLAGS) \
            -L$(top_srcdir)/../$(POCO_LDFLAGS) \
            -Wl,-rpath=$(top_srcdir)/../$(POCO_LDFLAGS) \
            -L$(top_srcdir)/../$(PBUF_LDFLAGS) \
            -L$(top_srcdir)/../$(LOG4XX_LDFLAGS) \
            -Wl,-rpath=$(top_srcdir)/../$(APR-UTIL_LDFLAGS) \
            -L$(top_srcdir)/../$(ZMQ_LDFLAGS) \
            -L$(top_srcdir)/../$(SNMP_LDFLAGS) \
            -L$(top_srcdir)/../$(JSON_LDFLAGS) \
            -L$(top_srcdir)/../$(CURL_LDFLAGS) \
            -L$(top_srcdir)/../$(SSH2_LDFLAGS) \
            -L$(SOC_WEAK_LDFLAGS) 


all: $(TARGETS)

$(TARGETS): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@  $^ $(LDLIBS) $(LDFLAGS)
	-@cp -fv $(TARGETS) ../../bin/

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE)

clean:
	-@rm -fv $(OBJS)







