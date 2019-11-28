#ifndef __MY_APP_H__
#define __MY_APP_H__

#include "gsad_base.h"
#include "gsad_omp.h"

char* start_task(const char * taskid);
char * Initialise(const char* guest, const char* passwd);
char * get_tasks();
char * get_task(const char* taskid);
char * stop_task(const char* taskid);
char * resume_task(const char* taskid);
char* get_report(const char* report_id);
char *del_task(const char* taskid);
char * create_task(const char* taskid );
char *quick_start(const char * host);
#endif
