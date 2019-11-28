#include <cstdio>
#include <vector>
#include "https/bd_https.h"
#include "bd_docker.h"
#include "json/json.h"

using namespace std;
using namespace netprotocol;

int InitDockerContainer(string retStr, docker_containers_info &con_info,string dockerip)
{

    try{
        Json::Reader reader;
        Json::Value root;

        if(!reader.parse(retStr, root))
        {
            printf("fail to parse.");
            return -1;
        }
        for (int i = 0; i < root.size(); i++)
        {
            string networkMode,ipaddress;
            if (root[i]["HostConfig"]["NetworkMode"].isString())
            {
                networkMode = root[i]["HostConfig"]["NetworkMode"].asString();
            }
            if (networkMode.compare("default") == 0)
            {
                networkMode = "bridge";
            }

            if (!networkMode.empty() && root[i]["NetworkSettings"]["Networks"][networkMode]["IPAddress"].isString())
            {
                ipaddress = root[i]["NetworkSettings"]["Networks"][networkMode]["IPAddress"].asString();
            }

            if (dockerip.compare(ipaddress) == 0)
            {
                con_info.Id = root[i]["Id"].asString();
                con_info.Image = root[i]["Image"].asString();
                con_info.Command = root[i]["Command"].asString();
                con_info.ImageID = root[i]["ImageID"].asString();
                if (root[i]["Names"].type() == Json::arrayValue)
                {
                    int cnt = root[i]["Names"].size();
                    for (int j = 0; j < cnt; j++)
                    {
                        con_info.Names = (root[i]["Names"][j].asString());
                    }
                }
                con_info.NetworkMode = networkMode;
                con_info.IPAddress = ipaddress;
                if(!con_info.NetworkMode.empty() && root[i]["NetworkSettings"]["Networks"][con_info.NetworkMode]["MacAddress"].isString())
                {
                   con_info.MAC = root[i]["NetworkSettings"]["Networks"][con_info.NetworkMode]["MacAddress"].asString();
                }
                con_info.State = root[i]["State"].asString();
                break;
            }
        }
    }
    catch(exception &ex)
    {
        return -1;
    }

    //Driver = root["DriverStatus"];
    return 0;
}

CDocker::CDocker( const string &dockerip, const string &hostip, const string &hostport)
{
    bd_https http;
    docker_host.host_ip = hostip;
    docker_host.host_port = hostport;
    //获取所有容器信息
    string url =  "http://" + docker_host.host_ip + ":" + docker_host.host_port +"/containers/json?all=1";
    string content = http.get_request_page(url);
    InitDockerContainer(content, dock_container, dockerip);
}

CDocker::~CDocker(){

}

int CDocker::DOCKER_GetRunTimeList(DockerRuntimeInfo &lis)
{
    bd_https http;
    string url = "http://"+ docker_host.host_ip + ":" + docker_host.host_port +"/containers/"+ dock_container.Id +"/json";
    string content = http.get_request_page(url);
    try{
        Json::Reader reader;
        Json::Value root;
        if(!reader.parse(content, root))
        {
            printf("fail to parse.");
            return -1;
        }

        //获取容器时间
        lis.Created = root["Created"].asString();
        lis.StartedAt = root["State"]["StartedAt"].asString();
        lis.FinishedAt = root["State"]["FinishedAt"].asString();
    }
    catch(exception &ex)
    {
        return -1;
    }
    return 0;
}

int CDocker::DOCKER_GetCpuUsage(double *value)
{
    bd_https http;
    string url = "http://"+ docker_host.host_ip + ":" + docker_host.host_port +"/containers/"+ dock_container.Id +"/stats?stream=false";
    string content = http.get_request_page(url);
    string content_new = http.get_request_page(url); //计算cpu占用，需要获取两次状态计算差值
    try{
        Json::Reader reader;
        Json::Value root;
        Json::Reader reader_new;
        Json::Value root_new;
        if(!reader.parse(content, root))
        {
            printf("fail to parse.");
            return -1;
        }
        if(!reader_new.parse(content_new, root_new))
        {
            printf("fail to parse.");
            return -1;
        }
        //计算cpu占用
        double cpu_total_usage = root_new["cpu_stats"]["cpu_usage"]["total_usage"].asDouble() - root["cpu_stats"]["cpu_usage"]["total_usage"].asDouble();
        double cpu_system_uasge = root_new["cpu_stats"]["system_cpu_usage"].asDouble() - root["cpu_stats"]["system_cpu_usage"].asDouble();
        int cpu_num = root["cpu_stats"]["cpu_usage"]["percpu_usage"].size();
        *value =cpu_total_usage/cpu_system_uasge * cpu_num * 100;
    }
    catch(exception &ex)
    {
        return -1;
    }

    return 0;
}

int CDocker::DOCKER_GetRamUsage(double *value)
{
    bd_https http;
    string url = "http://"+ docker_host.host_ip + ":" + docker_host.host_port +"/containers/"+ dock_container.Id +"/stats?stream=false";
    string content = http.get_request_page(url);
    try{
        Json::Reader reader;
        Json::Value root;
        if(!reader.parse(content, root))
        {
            printf("fail to parse.");
            return -1;
        }
        //计算mem占用
        double mem_usage = root["memory_stats"]["usage"].asDouble();
        double mem_limit = root["memory_stats"]["limit"].asDouble();
        *value = mem_usage/mem_limit * 100;
    }
    catch(exception &ex)
    {
        return -1;
    }

    return 0;
}

