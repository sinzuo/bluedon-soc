
#include"AgentCheckOptions.h"
#include <curl/curl.h>
#include <stdio.h>
#include  "unistd.h"


typedef vector<tag_check_task_config_t>::const_iterator TASK_ITOR_T ;

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	return size*nmemb;
}

int main(int avgc, char *argv[]) {

    CAgentCheckOptions * optionInstance = CAgentCheckOptions::instance();
	optionInstance->CommondParse(avgc, argv);
	optionInstance->Init();

    unsigned int unCheckInteval = optionInstance->GetConfig()->unIntevalSecond;

    while(true) {
		
	    TASK_ITOR_T iter_beg = (optionInstance->GetConfig())->vecTaskCfg.begin();
	    TASK_ITOR_T iter_end = (optionInstance->GetConfig())->vecTaskCfg.end();

	    for (; iter_beg != iter_end; iter_beg++ ) {
		    if ( strlen( iter_beg->chCheckName ) == 0 
			   || strlen( iter_beg->chCommand )  == 0 ) {
	            continue;   // invalid task
		    }
		    if ( strncmp(iter_beg->chtype,CHECK_TYPE_EXE,1) == 0) {
		        char cmd[128] = { 0 };				
			    sprintf(cmd, "ps -ef | grep %s | grep -v grep | grep -v %s", iter_beg->chCheckName, argv[0]);
			    FILE* rfile = popen(cmd, "r");
			    if (rfile != NULL) {
				    char buf[512] = { 0 };
				    fgets(buf, 512, rfile);
				    pclose(rfile);
				    if ( strlen(buf) == 0 ) {
						printf(" checking result:[%s] [no]\n",iter_beg->chCheckName);
						optionInstance->CommandExecute(iter_beg->chCommand);
		            } else {
						printf(" checking result:[%s] [yes]\n",iter_beg->chCheckName);
		            }
		        }
            }

			if(strncmp(iter_beg->chtype,CHECK_TYPE_URL,1) == 0) {
                CURL *curl;
		        CURLcode res;
				curl = curl_easy_init();
				if( curl ) {
					curl_easy_setopt( curl, CURLOPT_URL, iter_beg->chCheckName);
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
					curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
					curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
					res = curl_easy_perform( curl );
				
					if(res== CURLE_OK) {
						printf(" checking result:[%s] [yes]\n",iter_beg->chCheckName);
					} else {
						printf(" checking result:[%s] [no] [%d]\n",iter_beg->chCheckName,res);
						optionInstance->CommandExecute(iter_beg->chCommand);
					}
					curl_easy_cleanup( curl );
				}
				else
				{
					cout<<"error: curl_easy_init failed..."<<endl;
					/* continue */
				}
            } 
        }
        sleep( unCheckInteval);
    }	
    return 0;	
}
