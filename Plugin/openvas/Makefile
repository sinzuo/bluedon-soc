top_srcdir = ../..
include $(top_srcdir)/Make.rules

TARGETS    = openvas.so

CXXFLAGS  += -fPIC

SRCS       =$(wildcard *.cpp \
                       $(top_srcdir)/utils/*.cpp \
                       $(top_srcdir)/exception/*.cpp \
                       $(top_srcdir)/crypt/*.cpp \
                       $(top_srcdir)/Plugin/share/https/*.cpp \
                       $(top_srcdir)/Plugin/share/netbase/*.cpp \
                       $(top_srcdir)/Plugin/share/task/*.cpp)
#                       $(top_srcdir)/Plugin/share/snmp/*.cpp)

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
			-I$(top_srcdir)/../$(CURL_INCLUDE) \
		    -I./src \
		    -I/usr/lib64/glib-2.0/include \
		    -I/usr/include/glib-2.0 \
		    -I/usr/lib/x86_64-linux-gnu/glib-2.0/include \
            -I/usr/include/libxml2  \
            -I/usr/local/include/openvas \
		    -I/usr/local/include


LDLIBS     =-lgnutls \
		    -lglib-2.0 \
		    -lopenvas_base \
		    -lopenvas_misc \
		    -lxslt \
		    -lexslt \
		    -lxml2 \
		    -lopenvas_omp


LDFLAGS    =-L. \
		    -L./src \
			-L/usr/local/lib64

all: $(TARGETS)

$(TARGETS): $(OBJS)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(LDLIBS) $(LDFLAGS) $^
	-@cp -fv $(TARGETS) $(top_srcdir)/../lib/

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE)

clean:
	-@rm -fv $(OBJS)
