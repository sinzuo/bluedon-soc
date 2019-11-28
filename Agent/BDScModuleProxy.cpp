

#include "BDScModuleApp.h"
#include <iostream>
#include <cstdlib>
using namespace std;

CBDScModuleApp *g_pApp = NULL;

int main(int avgc, char **argv) {
	
    g_pApp->CommondParse(avgc, argv); 

	if(g_pApp->Initlize() < 0) {
		exit(-1);
	}
        
    if(g_pApp->RunProcess() != 0) {
		AGENTLOG_DEBUG_S(program run failed!);
		exit(-1);		
    }
    
    return 0;	
}


