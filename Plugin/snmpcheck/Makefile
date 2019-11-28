#不能以utf-8 rom格式保存, 否则makefile无法编译
top_srcdir	= ../..
include $(top_srcdir)/Make.rules

TARGETS    = bdsnmpcheck.so

CXXFLAGS  += -fPIC

SRCS       =$(wildcard *.cpp \
                       $(top_srcdir)/utils/*.cpp \
					   $(top_srcdir)/exception/*.cpp \
                       $(top_srcdir)/Plugin/share/netbase/*.cpp \
                       $(top_srcdir)/Plugin/share/snmp/*.cpp)
#$(top_srcdir)/Plugin/share/task/*.cpp \
					   
OBJS       =$(patsubst %.cpp, %.o, $(SRCS))
#SRCS_ND    =$(notdir $(SRCS))
#OBJS_ND    =$(patsubst %.cpp, %.o, $(SRCS_ND)) 


INCLUDE    =-I. \
            -I$(top_srcdir) \
            -I$(top_srcdir)/$(PLUGIN_SHARE_INCLUDE) \
            -I$(top_srcdir)/../$(ACE_INCLUDE) \
            -I$(top_srcdir)/../$(PBUF_INCLUDE) \
            -I$(top_srcdir)/../$(POCO_INCLUDE) \
			-I$(top_srcdir)/../$(SNMP_INCLUDE) \
            -I$(top_srcdir)/../$(LOG4XX_INCLUDE)

LDLIBS     = 
LDFLAGS    =-L.


############################################################

all: $(TARGETS)

$(TARGETS): $(OBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LDLIBS) $(LDFLAGS) $^
	-@cp -fv $(TARGETS) $(top_srcdir)/../lib/

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE)


############################################################

info:
	@echo $(CXX)
	@echo $(CXXFLAGs)
	@echo $(TARGETS)
	@echo $(INCLUDE)
	@echo $(SRCS)
	@echo $(OBJS)
	@echo $(LDFLAGS)

clean:
	-@rm -fv $(OBJS)


install:
	-@cp -fv $(TARGETS) $(top_srcdir)/../lib/

