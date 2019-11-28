#ifndef __BD_JSON_INTERFACE__
#define __BD_JSON_INTERFACE__

#include <vector>
#include <string>

using namespace std;

typedef struct _containers_info
{
    string Id;
    string Image;
    string ImageID;
    string Names;
    string State;
    double cpu_percent;
    double mem_percent;
    string IPAddress;
    string MAC;
    string Command;
    string NetworkMode;
    string Created;
    string StartedAt;
    string FinishedAt;
    int cpu_count;
    double mem_limit;

}containers_info;

int json_parse_result(string neip, vector<containers_info> &vec);

int inspect_result(string retStr, string retStr_new, vector<containers_info>::iterator iter);

int container_json(string retStr, vector<containers_info>::iterator iter);


#endif  //__BD_JSON_INTERFACE__

