
top_srcdir = ..
include $(top_srcdir)/Make.rules

TARGETS    = agent_check


SRCS       =$(wildcard *.cpp)
OBJS       =$(patsubst %.cpp, %.o, $(SRCS)) 
#SRCS_ND    =$(notdir $(SRCS))
#OBJS_ND    =$(patsubst %.cpp, %.o, $(SRCS_ND)) 

INCLUDE    =-I. \
            -I$(top_srcdir) \
            -I$(top_srcdir)/../$(POCO_INCLUDE) \
            -I$(top_srcdir)/../$(CURL_INCLUDE)

LDLIBS     = $(POCO_LIBS) \
             $(CURL_LIBS)

LDFLAGS    =-L. \
            -L$(top_srcdir)/../$(POCO_LDFLAGS) \
            -Wl,-rpath=$(top_srcdir)/../$(POCO_LDFLAGS) \
            -L$(top_srcdir)/../$(CURL_LDFLAGS)

all: $(TARGETS)

$(TARGETS): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@  $^ $(LDLIBS) $(LDFLAGS)
	-@cp -fv $(TARGETS) ../../bin/

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE)

clean:
	-@rm -fv $(OBJS)







