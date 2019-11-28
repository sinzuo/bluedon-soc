#include <cstdio>
#include <vector>

#include "json_interface.h"
#include "json/json.h"

using namespace std;

int json_parse_result(string retStr, vector<containers_info> &vec)
{

    try{
        containers_info con_info;
        Json::Reader reader;
        Json::Value root;
        vec.clear();
        Json::Value::Members::iterator iter;
        Json::Value::Members members;

        if(!reader.parse(retStr, root))
        {
            printf("fail to parse.");
            return -1;
        }
        for (int i = 0; i < root.size(); i++)
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

            if (root[i]["HostConfig"]["NetworkMode"].isString())
            {
                con_info.NetworkMode = root[i]["HostConfig"]["NetworkMode"].asString();
            }
            if(!con_info.NetworkMode.empty() && root[i]["NetworkSettings"]["Networks"][con_info.NetworkMode]["IPAddress"].isString())
            {
               con_info.IPAddress = root[i]["NetworkSettings"]["Networks"][con_info.NetworkMode]["IPAddress"].asString();
            }
            if(!con_info.NetworkMode.empty() && root[i]["NetworkSettings"]["Networks"][con_info.NetworkMode]["MacAddress"].isString())
            {
               con_info.MAC = root[i]["NetworkSettings"]["Networks"][con_info.NetworkMode]["MacAddress"].asString();
            }


            con_info.State = root[i]["State"].asString();

            vec.push_back(con_info);
        }
    }
    catch(exception &ex)
    {
        return -1;
    }

    //Driver = root["DriverStatus"];
    return 0;


}


int inspect_result(string retStr,  string retStr_new, vector<containers_info>::iterator iter)
{

    try{
        Json::Reader reader;
        Json::Value root;
        Json::Reader reader_new;
        Json::Value root_new;
        if(!reader.parse(retStr, root))
        {
            printf("fail to parse.");
            return -1;
        }
        if(!reader_new.parse(retStr_new, root_new))
        {
            printf("fail to parse.");
            return -1;
        }
        //计算cpu占用
        double cpu_total_usage = root_new["cpu_stats"]["cpu_usage"]["total_usage"].asDouble() - root["cpu_stats"]["cpu_usage"]["total_usage"].asDouble();
        double cpu_system_uasge = root_new["cpu_stats"]["system_cpu_usage"].asDouble() - root["cpu_stats"]["system_cpu_usage"].asDouble();
        int cpu_num = root["cpu_stats"]["cpu_usage"]["percpu_usage"].size();
        iter->cpu_count = cpu_num;
        iter->cpu_percent =cpu_total_usage/cpu_system_uasge * cpu_num * 100;
        //计算mem占用
        double mem_usage = root_new["memory_stats"]["usage"].asDouble();
        double mem_limit = root_new["memory_stats"]["limit"].asDouble();
        iter->mem_percent = mem_usage/mem_limit * 100;


        iter->mem_limit = mem_limit;
    }
    catch(exception &ex)
    {
        return -1;
    }

    return 0;

}



int container_json(string retStr, vector<containers_info>::iterator iter)
{

    try{
        Json::Reader reader;
        Json::Value root;
        if(!reader.parse(retStr, root))
        {
            printf("fail to parse.");
            return -1;
        }

        //获取容器时间
        iter->Created = root["Created"].asString();
        iter->StartedAt = root["State"]["StartedAt"].asString();
        iter->FinishedAt = root["State"]["FinishedAt"].asString();

    }
    catch(exception &ex)
    {
        return -1;
    }

    return 0;

}


