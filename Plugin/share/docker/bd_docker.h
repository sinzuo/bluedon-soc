#ifndef __BD_DOCKER_INTERFACE__
#define __BD_DOCKER_INTERFACE__

#include <vector>
#include <string>

using namespace std;

typedef struct tagDockerRuntimeInfo{
    string Created;
    string StartedAt;
    string FinishedAt;
}DockerRuntimeInfo;

typedef struct _docker_containers_info{
    string Id;
    string Image;
    string ImageID;
    string Names;
    string State;
    string IPAddress;
    string MAC;
    string Command;
    string NetworkMode;
}docker_containers_info;

typedef struct _docker_host_info
{
    string host_ip;
    string host_port;
}docker_host_info;

class CDocker
{
public:
    CDocker(const string &dockerip,const string &hostip, const string &hostport);
    ~CDocker();
    int DOCKER_GetRunTimeList(DockerRuntimeInfo &lis);
    int DOCKER_GetCpuUsage(double *value);
    int DOCKER_GetRamUsage(double *value);

private:
    docker_host_info docker_host;               //宿主信息
    docker_containers_info dock_container;      //容器信息
};

#endif  //__BD_DOCKER_INTERFACE__

