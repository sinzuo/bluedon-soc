
#include "bd_task.h"
#include "Poco/Types.h"
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/NodeIterator.h"
#include "Poco/DOM/NodeFilter.h"
#include "Poco/DOM/Node.h"
#include "Poco/DOM/NamedNodeMap.h"
#include "Poco/DOM/AutoPtr.h"
#include "Poco/XML/XMLException.h"
#include <iostream>

using namespace Poco;
using namespace netprotocol;

string CTask::commond_shutdown()
{
    string result;
    if(m_vecProtoParam.size() == 3)//关机指令(参数为:用户名，密码，URL)三种
    {
       
       bd_https_control ctrl(m_vecProtoParam[2].c_str());  
       try{
          ctrl.authentication(m_vecProtoParam[0].c_str(),m_vecProtoParam[1].c_str());
          result  = ctrl.shutdown();
          }catch(const bd_net_exception& ex){
           cerr<<ex.what()<<endl;
           return ex.what();
         }
    } else result = "Parameters Error";
    return result;
}

string CTask::commond_reboot()
{
    string result;    
    if(m_vecProtoParam.size() == 3)//重启指令(参数为:用户名，密码，URL)三种
    {  
       bd_https_control ctrl(m_vecProtoParam[2].c_str());  
       try{
          ctrl.authentication(m_vecProtoParam[0].c_str(),m_vecProtoParam[1].c_str());
          result  = ctrl.reboot();
          }catch(const bd_net_exception& ex){
           cerr<<ex.what()<<endl;
           return ex.what();
         }      
    } else result = "Parameters Error";
    return result;
}

string CTask::commond_factory_recover()
{
    string result;
    if(m_vecProtoParam.size() == 3)//恢复出厂指令(参数为:用户名，密码，URL)三种
    {
       bd_https_control ctrl(m_vecProtoParam[2].c_str());
       try{
          ctrl.authentication(m_vecProtoParam[0].c_str(), m_vecProtoParam[1].c_str());
          result  = ctrl.factory_recover();
          }catch(const bd_net_exception& ex){
           cerr<<ex.what()<<endl;
           return ex.what();
         }
    } else result = "Parameters Error";
    return result;
}

string CTask::commond_restart_service()
{
    string result;    
    if(m_vecProtoParam.size() == 4)//重启服务指令(参数为:用户名，密码，URL，服务名称)四种
    {
       bd_https_control ctrl(m_vecProtoParam[2].c_str());  
       try{
          ctrl.authentication(m_vecProtoParam[0].c_str(),m_vecProtoParam[1].c_str());
          result  = ctrl.restart_service(m_vecProtoParam[3].c_str());
          }catch(const bd_net_exception& ex){
           cerr<<ex.what()<<endl;
           return ex.what();
         }       
    } else result = "Parameters Error";
    return result;
}

string CTask::commond_stop_service()
{
    string result;    
    if(m_vecProtoParam.size() == 4)//停止服务指令(参数为:用户名，密码，URL，服务名称)四种
    {
        try{
          bd_https_control ctrl(m_vecProtoParam[2].c_str());  
          ctrl.authentication(m_vecProtoParam[0].c_str(),m_vecProtoParam[1].c_str());
          result  = ctrl.stop_service(m_vecProtoParam[3].c_str());
          }catch(const bd_net_exception& ex){
           cerr<<ex.what()<<endl;
           return ex.what();
         }
    } else result = "Parameters Error";
    return result;
}

string CTask::commond_syntime()
{
    string result;    
    if(m_vecProtoParam.size() == 4)//同步时间指令(参数为:用户名，密码，URL，服务器ip)四种
    {
        
          bd_https_control ctrl(m_vecProtoParam[2].c_str());  
       try{
          ctrl.authentication(m_vecProtoParam[0].c_str(),m_vecProtoParam[1].c_str());
          result  = ctrl.syntime(m_vecProtoParam[3].c_str());
          }catch(const bd_net_exception& ex){
           cerr<<ex.what()<<endl;
           return ex.what();
         }       
    }  else result = "Parameters Error";
    return result;
}



bool CTask::getPingState(const string &strIP, int ntimeout, int *timecost) {
	bd_ping bdPing;
	bool bConnect = bdPing.ping(strIP, ntimeout, timecost);
    return bConnect;
}

bool CTask::getSysInfo(string &sysinfo)
{
    sysinfo = "unknown~unknown"; //返回数据的格式
    if( m_vecProtoParam[0].compare(PROTOCOL_SNMP) == 0 ) {  //snmp协议
        if( m_vecProtoParam.size() == 10) {
             CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(), \
                     atol(m_vecProtoParam[3].c_str()),atoi(m_vecProtoParam[4].c_str()),\
                     m_vecProtoParam[5].c_str(),m_vecProtoParam[6].c_str(),\
                     m_vecProtoParam[7].c_str(),m_vecProtoParam[8].c_str(), \
                     m_vecProtoParam[9].c_str());  //snmp v3

             if(snmp.SNMP_GetSysInfo(sysinfo) == 0)
                 return true;
             else
                 printf("----Error: snmp.SNMP_GetSysInfo() error!\n");
        } else
            printf("----Error: m_vecProtoParam.size is %d\n", m_vecProtoParam.size());
    } else {
        cout << "----unsupport for this proto" << endl;
    }

    return false;
}

bool CTask::getCpuUsage( int &nCpu) {
    if( m_vecProtoParam[0].compare(PROTOCOL_BDSEC) == 0 ) {// bdsec
        string content;
        bd_https_control ctrl(m_vecProtoParam[3].c_str());
        try {
            //ctrl.set_verify_file(cert_filepath);
            ctrl.authentication (m_vecProtoParam[1].c_str(),m_vecProtoParam[2].c_str());
            content=ctrl.getstatus("cpu");
            if (content.length() == 0) return false;
        } catch (const bd_net_exception& ex) {
            cerr<<ex.what()<<endl;
            return false;
        }

        map<string,string> map_cpu;
        if ( !getVaFromXml(content, BDSECXML_NODEPATH_CUP, BDSECXML_NODENAME_CPU, map_cpu ))
            return false;
        if(map_cpu.size() == 0) return false;
        map<string,string>::iterator itor_temp = map_cpu.begin();
        for(;itor_temp != map_cpu.end(); itor_temp++) {
            nCpu += atoi(itor_temp->second.c_str());
        }
        nCpu /= map_cpu.size();
    } else if(m_vecProtoParam[0].compare(PROTOCOL_SNMP) == 0){
        if( m_vecProtoParam.size() == 10) {
             CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(), \
                     atol(m_vecProtoParam[3].c_str()),atoi(m_vecProtoParam[4].c_str()),\
                     m_vecProtoParam[5].c_str(),m_vecProtoParam[6].c_str(),\
                     m_vecProtoParam[7].c_str(),m_vecProtoParam[8].c_str(), \
                     m_vecProtoParam[9].c_str());//snmp v3

             //CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(),m_vecProtoParam[3].c_str(),m_vecProtoParam[4].c_str(),m_vecProtoParam[5].c_str(),atol(m_vecProtoParam[6].c_str()));//snmp v3
             double dCpu;
             snmp.SNMP_GetCpuUsage(&dCpu);
             nCpu = (int)(dCpu*100);
        } else
            return false;
    }else if (m_vecProtoParam[0].compare(PROTOCOL_DOCKER) == 0){
        if( m_vecProtoParam.size() == 4 )
        {
            CDocker docker(m_vecProtoParam[1].c_str(),m_vecProtoParam[2].c_str(),\
                    m_vecProtoParam[3].c_str());
            double dCPU;
            docker.DOCKER_GetCpuUsage(&dCPU);
            nCpu = (int)(dCPU*100);
        }
        else
        {return false;}

    }
    else{
        return false;
     }

    return true;
}
bool CTask::getMemUsage( int &nMem) {
    if( m_vecProtoParam[0].compare(PROTOCOL_BDSEC) == 0 ) {// bdsec
        string content;
        bd_https_control ctrl(m_vecProtoParam[3].c_str());
        try {
            //ctrl.set_verify_file(cert_filepath);
            ctrl.authentication(m_vecProtoParam[1].c_str(),m_vecProtoParam[2].c_str());
            content=ctrl.getstatus("memory");
            if (content.length() == 0) return false;
        } catch (const bd_net_exception& ex) {
            cerr<<ex.what()<<endl;
            return false;
        }

        map<string,string> map_memory;
        if ( !getVaFromXml(content, BDSECXML_NODEPATH_MEM, BDSECXML_NODENAME_MEM, map_memory ))
            return false;
        if(map_memory.size() == 0) return false;
        nMem = atoi(map_memory["memory"].c_str());
    } else if(m_vecProtoParam[0].compare(PROTOCOL_SNMP) == 0){
        if( m_vecProtoParam.size() == 10)
        {
            CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(), \
                    atol(m_vecProtoParam[3].c_str()),atoi(m_vecProtoParam[4].c_str()),\
                    m_vecProtoParam[5].c_str(),m_vecProtoParam[6].c_str(),\
                    m_vecProtoParam[7].c_str(),m_vecProtoParam[8].c_str(),\
                    m_vecProtoParam[9].c_str());//snmp v3

            //CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(),m_vecProtoParam[3].c_str(),m_vecProtoParam[4].c_str(),m_vecProtoParam[5].c_str(),atol(m_vecProtoParam[6].c_str()));//snmp v3
            //CSnmpV1 snmp(m_vecProtoParam[1].c_str(),m_vecProtoParam[2].c_str(),0);
            double dMem;
            snmp.SNMP_GetRamUsage(&dMem);
            nMem = (int)(dMem*100);
        } else
            return false;
    } else if (m_vecProtoParam[0].compare(PROTOCOL_DOCKER) == 0){
        if( m_vecProtoParam.size() == 4 )
        {
            CDocker docker(m_vecProtoParam[1].c_str(),m_vecProtoParam[2].c_str(),\
                    m_vecProtoParam[3].c_str());
            double dMem;
            docker.DOCKER_GetRamUsage(&dMem);
            nMem = (int)(dMem*100);
        } else
            return false;
    } else {
        return false;
    }

    return true;
}
bool CTask::getDiskUsage( map<string,string> &mapDiskUsage) {
    if( m_vecProtoParam[0].compare(PROTOCOL_BDSEC) == 0 ) {// bdsec
	    string content;
        bd_https_control ctrl(m_vecProtoParam[3].c_str()); 
        try {
		    //ctrl.set_verify_file(cert_filepath); 
		    ctrl.authentication(m_vecProtoParam[1].c_str(),m_vecProtoParam[2].c_str());
			content=ctrl.getstatus("harddisk");
			if (content.length() == 0) return false;
        } catch (const bd_net_exception& ex) {
            cerr<<ex.what()<<endl;
	        return false;
        }

		map<string,string> map_disk;
		if ( !getVaFromXml(content, BDSECXML_NODEPATH_DISK, BDSECXML_NODENAME_DISK, mapDiskUsage)) 
			return false;
		if(mapDiskUsage.size() == 0)
			return false;
	} else if(m_vecProtoParam[0].compare(PROTOCOL_SNMP) == 0){
        if( m_vecProtoParam.size() == 10)
        {
            CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(), \
                    atol(m_vecProtoParam[3].c_str()),atoi(m_vecProtoParam[4].c_str()),\
                    m_vecProtoParam[5].c_str(),m_vecProtoParam[6].c_str(),\
                    m_vecProtoParam[7].c_str(),m_vecProtoParam[8].c_str(),\
                    m_vecProtoParam[9].c_str());//snmp v3
            //CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(),m_vecProtoParam[3].c_str(),m_vecProtoParam[4].c_str(),m_vecProtoParam[5].c_str(),atol(m_vecProtoParam[6].c_str()));//snmp v3
            //CSnmpV1 snmp(m_vecProtoParam[1].c_str(),m_vecProtoParam[2].c_str(),0);
            DiskInfoList DiskInfos;
            snmp.SNMP_GetDiskUsageList(DiskInfos);
            for(vector<PartitionInfo>::iterator it = DiskInfos.begin(); it != DiskInfos.end(); ++it)
            {
               if(strcmp(it->partition_type,".1.3.6.1.2.1.25.2.1.4") == 0
                  || strcmp(it->partition_type, "getdisktype") == 0)
               {
                   string key = it->partition_name;
                   char value[100];
                   memset(value, 0, 100);
                   sprintf(value,"%f~%d~%d",it->partition_usage*100, it->disk_total*4, it->disk_used*4);
                   fprintf(stdout,"=============getDiskUsage:%s\n",value);

                   string s_value = value;
                   mapDiskUsage.insert(make_pair(key,s_value));
               }

            }
        }
        else
        {return false;}
	}
    else{
        return false; 
     }
	
    return true;
}
bool CTask::getServiceState(map<string,string> &mapServiceState ) {
    if( m_vecProtoParam[0].compare(PROTOCOL_BDSEC) == 0 ) {// bdsec 
	    string content;
        bd_https_control ctrl(m_vecProtoParam[3].c_str()); // https://172.16.2.32:441/cgi-bin/bdsec_daemon
        try {
		    //ctrl.set_verify_file(cert_filepath); 
		    ctrl.authentication(m_vecProtoParam[1].c_str(),m_vecProtoParam[2].c_str());
		    content=ctrl.getstatus("service");
			if (content.length() == 0) return false;
        } catch (const bd_net_exception& ex) {
            cerr<<ex.what()<<endl;
	        return false;
        }

		if ( !getVaFromXml(content, BDSECXML_NODEPATH_SERVICE, BDSECXML_NODENAME_SERVICE, mapServiceState )) 
			return false;
		if(mapServiceState.size() == 0) 
			return false;
	} else if(m_vecProtoParam[0].compare(PROTOCOL_SNMP) == 0){

        if( m_vecProtoParam.size() == 10)
        {
            CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(), \
                    atol(m_vecProtoParam[3].c_str()),atoi(m_vecProtoParam[4].c_str()),\
                    m_vecProtoParam[5].c_str(),m_vecProtoParam[6].c_str(),\
                    m_vecProtoParam[7].c_str(),m_vecProtoParam[8].c_str(),\
                    m_vecProtoParam[9].c_str());//snmp v3
            //CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(),m_vecProtoParam[3].c_str(),m_vecProtoParam[4].c_str(),m_vecProtoParam[5].c_str(),atol(m_vecProtoParam[6].c_str()));//snmp v3
            //CSnmpV1 snmp(m_vecProtoParam[1].c_str(),m_vecProtoParam[2].c_str(),0);
            ProcessInfoList ProcessInfos;
            if(snmp.SNMP_GetSWRunStatusList(ProcessInfos) < 0){return false;}
            for(vector<ProcessInfo>::iterator it = ProcessInfos.begin(); it != ProcessInfos.end(); ++it)
            {
               string key = it->process_name;
               string value = "";
               switch(it->process_status)
               {
                   //1:running  2:runnable 3:notRunnable 4:invalid
                   case 1:
                       value = "running";
                       break;
                   case 2:
                       value = "runnable";
                       break;
                   case 3:
                       value = "notRunnable";
                       break;
                   case 4:
                       value = "invalid";
                       break;
                   default:
                       break;

               }
               mapServiceState.insert(make_pair(key,value));
            }
        }
        else
        {return false;}
	}
    else{
        return false; 
     }

    return true;
}

bool CTask::getNetworkUsage(NetWorkInfoList &lis)
{
    if(m_vecProtoParam[0].compare(PROTOCOL_SNMP) == 0){
        if( m_vecProtoParam.size() == 10)
        {
            CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(), \
                    atol(m_vecProtoParam[3].c_str()),atoi(m_vecProtoParam[4].c_str()),\
                    m_vecProtoParam[5].c_str(),m_vecProtoParam[6].c_str(),\
                    m_vecProtoParam[7].c_str(),m_vecProtoParam[8].c_str(),\
                    m_vecProtoParam[9].c_str());//snmp v3
            //CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(),m_vecProtoParam[3].c_str(),m_vecProtoParam[4].c_str(),m_vecProtoParam[5].c_str(),atol(m_vecProtoParam[6].c_str()));//snmp v3
            //CSnmpV1 snmp(m_vecProtoParam[1].c_str(),m_vecProtoParam[2].c_str(),0);
            if(snmp.SNMP_GetNetworkList(lis)){return false;}
        }
        else
        {return false;}
	}
    else{
        return false; 
     }
    return true;
}
bool CTask::getSWInstalledUsage(SWInstallInfoList &lis)
{
    if(m_vecProtoParam[0].compare(PROTOCOL_SNMP) == 0){
        if( m_vecProtoParam.size() == 10)
        {
            CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(), \
                    atol(m_vecProtoParam[3].c_str()),atoi(m_vecProtoParam[4].c_str()),\
                    m_vecProtoParam[5].c_str(),m_vecProtoParam[6].c_str(),\
                    m_vecProtoParam[7].c_str(),m_vecProtoParam[8].c_str(),\
                    m_vecProtoParam[9].c_str());//snmp v3
            //CSnmpV1 snmp(m_vecProtoParam[1].c_str(), m_vecProtoParam[2].c_str(),m_vecProtoParam[3].c_str(),m_vecProtoParam[4].c_str(),m_vecProtoParam[5].c_str(),atol(m_vecProtoParam[6].c_str()));//snmp v3
            //CSnmpV1 snmp(m_vecProtoParam[1].c_str(),m_vecProtoParam[2].c_str(),0);
            if(snmp.SNMP_GetSWInstalledList(lis)){return false;}
        }
        else
        {return false;}

	}
    else{
        return false; 
     }

    return true;
}
bool CTask::getVaFromXml(const string &strXml, const char *chnodePath, const char *chnodeName, map<string, string> &mapResult) {
	XML::DOMParser parser;
	parser.setFeature(XML::DOMParser::FEATURE_FILTER_WHITESPACE, true);
	parser.setFeature(XML::XMLReader::FEATURE_NAMESPACES, false);

	//XML::InputSource source(strXml);
	XML::Document * pDoc     =NULL;
	XML::Element  * pElement =NULL;
	XML::Node     * pNode    =NULL;
	try
	{
        //pDoc = parser.parse(&source);	
        pDoc = parser.parseMemory(strXml.c_str(), strXml.length());
		if(pDoc==NULL) return false;
		pElement=pDoc->documentElement();
		if(pElement==NULL) {
			pDoc->release();
			return false;
		}
		pNode = pElement->getNodeByPath(chnodePath); //get a element from second level 
		if(pNode==NULL) {
			pDoc->release();
			return false;
		}
	} catch (XML::XMLException& exc) {
		cerr << "error:CTask::getVaFromXml failed:" << exc.displayText() << endl;	
		if(pDoc!=NULL) pDoc->release();
		return false;
	}

    try { // 不读取本节点的 sum 值了，直接通过子节点个数来循环
		if(pNode->hasChildNodes()) {
			XML::NodeList* pChildNodeList=pNode->childNodes();
			unsigned long dwCount=pChildNodeList->length();
			for(UInt32 num=0; num<dwCount; num++) {
				XML::Element * pChildNode=dynamic_cast<XML::Element*>(pChildNodeList->item(num)); 
				string mapkey;
				string mapval;
				if( !strncmp( pChildNode->nodeName().c_str(),chnodeName,sizeof(chnodeName))) {			
                    if( strncmp(chnodeName, BDSECXML_NODENAME_CPU, sizeof(BDSECXML_NODENAME_CPU)) == 0) {
						GET_XML_ATTR(pChildNode, Name        ,mapkey)
						GET_XML_ATTR(pChildNode, usedpercent ,mapval)
                    } else if(strncmp( chnodeName, BDSECXML_NODENAME_MEM, sizeof(BDSECXML_NODENAME_MEM)) == 0) {
                        string strMemtotal, strMemused;
						GET_XML_ATTR(pChildNode, memtotal ,strMemtotal)
						GET_XML_ATTR(pChildNode, memused  ,strMemused)
                        int nMemTotal = atoi(strMemtotal.c_str());
						int nMemUsed  = atoi(strMemused.c_str());
						if ( nMemTotal <= 0 || nMemTotal <= 0)return false;
						int nMemUseRate = (int)(100 *(nMemUsed*1.0/nMemTotal));
						if (nMemUseRate == 0) nMemUseRate = 1;  // 当内存使用率较少时，填 1
						char chMemUseRate[3] = {0};
						sprintf(chMemUseRate, "%d", nMemUseRate);
						// 内存xml层次数量比其他少一级，没有key值，这里写死一个key
						mapkey = "memory";  
						mapval = chMemUseRate;		
					}else if(strncmp( chnodeName, BDSECXML_NODENAME_DISK, sizeof(BDSECXML_NODENAME_DISK)) == 0) {
						string strDisktotal, strDiskused;
						GET_XML_ATTR(pChildNode, Name	   ,mapkey)
						GET_XML_ATTR(pChildNode, disktotal ,strDisktotal)
						GET_XML_ATTR(pChildNode, diskused  ,strDiskused)
						int nDiskTotal = atoi(strDisktotal.c_str());
						int nDiskUsed  = atoi(strDiskused.c_str());						
						if ( nDiskTotal <= 0 || nDiskUsed <= 0)return false;
						int nDiskUseRate = (int)(100 *(nDiskUsed*1.0/nDiskTotal));
						if (nDiskUseRate == 0) nDiskUseRate = 1;  // 当磁盘使用率较少时，填 1
						char chDiskUseRate[3] = {0};
						sprintf(chDiskUseRate, "%d", nDiskUseRate);
						mapval = chDiskUseRate;                
					}else if(strncmp( chnodeName, BDSECXML_NODENAME_SERVICE,  sizeof(BDSECXML_NODENAME_SERVICE)) == 0) {
						GET_XML_ATTR(pChildNode, Name	,mapkey)
						GET_XML_ATTR(pChildNode, status ,mapval)
					}		
					mapResult.insert(make_pair(mapkey,mapval));
				}
			}
			pChildNodeList->release();
		} 
	} catch (XML::XMLException& exc) {
	    cerr << "error:CTask::getVaFromXml failed:" << exc.displayText() << endl;	
		if(pDoc!=NULL) pDoc->release();
		return false;
	}	
	pDoc->release();	
	return true;
	
}



