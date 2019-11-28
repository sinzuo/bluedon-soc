
top_srcdir = ..
include $(top_srcdir)/Make.rules

SUBDIRS =   controller  filelog  inspector  inspecUrl  monitor  \
          netflow  nmapscan  ping    snmpcheck  snmptrap  syslog threatint \
		  topu wmi  weakpasswd docker_monitor sftp

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
		echo "making $@ in $$subdir"; \
		( cd $$subdir && $(MAKE) -f Makefile $@ ) || exit 1; \
	done

