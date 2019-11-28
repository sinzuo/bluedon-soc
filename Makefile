
top_srcdir = .
include $(top_srcdir)/Make.rules

SUBDIRS = Plugin Agent Jni

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
		echo "making $@ in $$subdir"; \
		( cd $$subdir && $(MAKE) -f Makefile $@ ) || exit 1; \
	done

