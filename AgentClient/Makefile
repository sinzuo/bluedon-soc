
top_srcdir = ..
include $(top_srcdir)/Make.rules

TARGETS    = agent_client


SRCS       =$(wildcard *.cpp \
                       $(top_srcdir)/config/*.cpp \
                       $(top_srcdir)/utils/*.cpp \
                       $(top_srcdir)/crypt/*.cpp \
                       $(top_srcdir)/probuf/*.cpp)
OBJS       =$(patsubst %.cpp, %.o, $(SRCS)) 
#SRCS_ND    =$(notdir $(SRCS))
#OBJS_ND    =$(patsubst %.cpp, %.o, $(SRCS_ND)) 

INCLUDE    =-I. \
            -I$(top_srcdir) \
            -I$(top_srcdir)/../$(PBUF_INCLUDE) \
			-I$(top_srcdir)/../$(ACE_INCLUDE) \
			-I$(top_srcdir)/../$(POCO_INCLUDE) \
            -I$(top_srcdir)/../$(ZMQ_INCLUDE)   \
			-I$(top_srcdir)/../$(LOG4XX_INCLUDE) 

LDLIBS     = $(PBUF_LIBS) \
			 $(ACE_LIBS) \
			 $(POCO_LIBS) \
			 $(LOG4XX_LIBS) \
             $(ZMQ_LIBS) 

LDFLAGS    =-L. \
			-L$(top_srcdir)/../$(ACE_LDFLAGS) \
            -L$(top_srcdir)/../$(PBUF_LDFLAGS) \
			-L$(top_srcdir)/../$(POCO_LDFLAGS) \
            -Wl,-rpath=$(top_srcdir)/../$(POCO_LDFLAGS) \
			-L$(top_srcdir)/../$(LOG4XX_LDFLAGS) \
            -L$(top_srcdir)/../$(ZMQ_LDFLAGS)  \
			-L$(top_srcdir)/../$(CURL_LDFLAGS)



all: $(TARGETS)

$(TARGETS): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^  $(LDLIBS) $(LDFLAGS)
	-@cp -fv $(TARGETS) ../../bin/

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE)

clean:
	-@rm -fv $(OBJS)







