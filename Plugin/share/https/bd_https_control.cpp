#include "bd_https_control.h"
#include "crypt/bd_crypto_md5.h"
#include "utils/bd_common.h"

#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/NodeIterator.h"
#include "Poco/DOM/NodeFilter.h"
#include "Poco/DOM/Node.h"
#include "Poco/DOM/NamedNodeMap.h"
#include "Poco/DOM/AutoPtr.h"
#include "Poco/XML/XMLException.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace netprotocol;
using namespace Poco;


#define IDSRULSDIR "/usr/local/socserver/fileserver/idsrules"

bd_https_control::bd_https_control(const string& serverurl):m_serverurl(serverurl)
{
}

string bd_https_control::checkresult(string content,string flags) throw (bd_net_exception)
{
	if(content.empty())
	{
		throw bd_net_exception((flags+" error: xml内容为空!").c_str());
	}
    printf("&&&&&&&&&&&Content:[%s]\n", content.c_str());

	XML::DOMParser parser;
	parser.setFeature(XML::DOMParser::FEATURE_FILTER_WHITESPACE, true);
	parser.setFeature(XML::XMLReader::FEATURE_NAMESPACES, false);
	//XML::InputSource source(content);
	XML::Document * pDoc     =NULL;
	XML::Element  * pElement =NULL;
	XML::Node     * pNode    =NULL;
	string strResult;
	
	try
	{	
        //pDoc = parser.parse(&source);
        pDoc = parser.parseMemory(content.c_str(), content.length());
		if(pDoc==NULL) return "false";
		pElement=pDoc->documentElement();		
		if(pElement==NULL) {
			pDoc->release();
			throw XML::XMLException((flags+" error: xml解析失败!").c_str());
		}
		string strNodePath = string("/Action/")+flags;
		//printf("%s\n",strNodePath.c_str());
		pNode = pElement->getNodeByPath(strNodePath.c_str()); //get a element from second level 
		if(pNode==NULL) {
			pDoc->release();
			throw XML::XMLException((flags+" error: xml节点获取失败: "+strNodePath).c_str());
		}
	    XML::Element * pNodeBase=dynamic_cast<XML::Element*>(pNode); 
	    if( pNodeBase->hasAttribute("result")) {
		    strResult = pNodeBase->getAttribute("result");
	    } else {
			throw XML::XMLException((flags+" error: xml无节点属性result!").c_str());
	    }

		if(strResult.compare("1") == 0 ) {
			throw XML::XMLException((flags+" error: 远程主机返回的xml结果为1").c_str());
		}
	
	} catch (XML::XMLException& exc) {
		if(pDoc!=NULL) pDoc->release();
		throw bd_net_exception(exc.displayText().c_str());
	}
	
	pDoc->release();
    //本期版本不需要加密
	//bd_crypto_md5 md5;
	//string res = md5.encodeUC16(strResult.c_str(),strResult.length());

	return strResult;
}


int bd_https_control::saveresult(string &content, string &filename) 
{
	if (content.empty())
	{
		return -1;
	}
  
	DIR *base_dir = opendir(IDSRULSDIR);
    if(!base_dir){	
		if ( mkdir(IDSRULSDIR,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH) < 0 ) return -1;
    }

	string allpathfile = std::string(IDSRULSDIR) + "/" + filename;
	FILE *pf = 0;
	pf = fopen(allpathfile.c_str(), "w+");
	if(pf == 0)
	{
		//打开文件失败
		return false;
	}

	if(!feof(pf))
	{
		fwrite(content.c_str(),content.size(),1,pf);
	}
	fclose(pf);
	return 0;
}

void bd_https_control::authentication(const string& username,const string& pwd)throw(bd_net_exception,std::runtime_error)
{
	m_username=username;
	m_password=pwd;
	char poststr[256];
	sprintf(poststr,"cmd=authentication&username=%s&password=%s",
			m_username.c_str(),m_password.c_str());
	string content;
	try
	{
		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		content=https.post_request_page(this->m_serverurl,poststr);
		//printf("%s\n",content.c_str());
		m_sessionkey=checkresult(content,"Authentication");
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
	catch(const std::runtime_error& ex)
	{
		throw;
	}

}

string bd_https_control::reboot()throw(bd_net_exception)
{
	try
	{
		char poststr[200];
		sprintf(poststr,"cmd=reboot&username=%s&password=%s&key=%s",
				m_username.c_str(),m_password.c_str(),m_sessionkey.c_str());
		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		string content=https.post_request_page(this->m_serverurl,poststr);
		//m_result = content;
		m_result = checkresult(content,"Reboot");
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
    return m_result;
}

string bd_https_control::shutdown()throw(bd_net_exception)
{
	try
	{
		char poststr[200];
		sprintf(poststr,"cmd=shutdown&username=%s&password=%s&key=%s",
				m_username.c_str(),m_password.c_str(),m_sessionkey.c_str());
		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		string content=https.post_request_page(this->m_serverurl,poststr);
		m_result = content;
		m_result = checkresult(content,"Shutdown");
		//printf("%s\n",content.c_str());
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
    return m_result;
}

string bd_https_control::restart_service(const string& servicename)throw(bd_net_exception)
{
	try
	{
		char poststr[200];
		sprintf(poststr,"cmd=restartservice&username=%s&password=%s&key=%s&service=%s",
				m_username.c_str(),m_password.c_str(),m_sessionkey.c_str(),servicename.c_str());
		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		string content=https.post_request_page(this->m_serverurl,poststr);
		//m_result = content;
		m_result = checkresult(content,"RestartService");
		//printf("%s\n",content.c_str());
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
    return m_result;
}

string bd_https_control::stop_service(const string& servicename)throw(bd_net_exception)
{
	try
	{
		char poststr[200];
		sprintf(poststr,"cmd=stopservice&username=%s&password=%s&key=%s&service=%s",
				m_username.c_str(),m_password.c_str(),m_sessionkey.c_str(),servicename.c_str());
		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		string content=https.post_request_page(this->m_serverurl,poststr);
		//m_result = content;
		m_result = checkresult(content,"StopService");
		//printf("%s\n",content.c_str());
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
    return m_result;
}

string bd_https_control::syntime(const string& serverip)throw(bd_net_exception)
{
	try
		{
	char poststr[200];
	sprintf(poststr,"cmd=syntime&username=%s&password=%s&key=%s&serverip=%s",
			m_username.c_str(),m_password.c_str(),m_sessionkey.c_str(),serverip.c_str());
	bd_https https;
	if(!m_verify_file.empty())
	{
		https.customverify(m_verify_file);
	}
	string content=https.post_request_page(this->m_serverurl,poststr);
	//m_result = content;
	//printf("%s\n",content.c_str());
	m_result = checkresult(content,"SynTime");

	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
    return m_result;
}

string bd_https_control::factory_recover()throw(bd_net_exception)
{
    try
    {
        char poststr[200];
        sprintf(poststr,"cmd=restore&username=%s&password=%s&key=%s",
                m_username.c_str(),m_password.c_str(),m_sessionkey.c_str());
        bd_https https;
        if(!m_verify_file.empty())
        {
            https.customverify(m_verify_file);
        }
        string content=https.post_request_page(this->m_serverurl,poststr);
        m_result = content;
        m_result = checkresult(content,"Restore");
        //printf("%s\n",content.c_str());
    }
    catch(const bd_net_exception& ex)
    {
        throw;
    }
    return m_result;
}

void bd_https_control::backuppolicy(const string& filetype,
		const string& savefilename)throw(bd_net_exception)
{
	char poststr[200];
	sprintf(poststr,"cmd=backuppolicy&username=%s&password=%s&key=%s&filetype=%s",
			m_username.c_str(),m_password.c_str(),m_sessionkey.c_str(),filetype.c_str());
	try
	{
		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		https.post_request_downloadfile(this->m_serverurl,poststr,savefilename);
		//string content=https.post_request_page(this->m_serverurl,poststr);
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
}

string bd_https_control::uploadpolicy(const string& filetype,
		const string& filename)throw(bd_net_exception,std::logic_error)
{
	string content;
	map<string,string> postdata;
	postdata["cmd"]="uploadpolicy";
	postdata["username"]=m_username;
	postdata["password"]=m_password;
	postdata["key"]=m_sessionkey;
	postdata["filetype"]=filetype;
//	sprintf(poststr,"cmd=uploadpolicy&username=%s&password=%s&key=%s&filetype=%s",
//			m_username.c_str(),m_password.c_str(),m_sessionkey.c_str(),filetype.c_str());
	try
	{
		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		content=https.post_request_uploadfile(this->m_serverurl,postdata,filename,"filename");
		//m_result = content;
		//printf("%s\n",content.c_str());
		m_result = checkresult(content,"UploadPolicy");
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
    return m_result;
}

string bd_https_control::getstatus(const string& statustype)
{
	char poststr[200];
	sprintf(poststr,"cmd=getstatus&username=%s&password=%s&key=%s&statustype=%s",
			m_username.c_str(),m_password.c_str(),m_sessionkey.c_str(),statustype.c_str());
	bd_https https;
	string content;
	try
	{
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		content=https.post_request_page(this->m_serverurl,poststr);
		m_result = content;
		//string content=https.post_request_page("https://172.16.2.32:441/cgi-bin/index.cgi",poststr);
		//printf("%s\n",this->m_serverurl.c_str());
		//printf("%s\n",poststr);
		//printf("%s\n",content.c_str());
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
	return content;
}


void bd_https_control::GetIdsRules()
{
	try
	{
		std::string poststr = "cmd=GetIdsRules&username=";
		poststr = poststr + m_username + "&password=";
		poststr = poststr + m_password + "&key=" + m_sessionkey;

		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		string content=https.post_request_page(this->m_serverurl,poststr);
		m_result = content;

		std::vector< std::string > urlvec;
		bd_split(this->m_serverurl, "/", &urlvec);
		std::string serverip = urlvec[2];
		std::string filename = serverip + ".xml";
		saveresult(content,filename);
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
}

void bd_https_control::SetIdsRules(std::string &rules)
{
	try
	{
		std::string poststr = "cmd=SetIdsRules&username=";
		poststr = poststr + m_username + "&password=";
		poststr = poststr + m_password + "&key=" + m_sessionkey;

		if (!rules.empty())
		{
			poststr = poststr + "&" + rules;
 		}

		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		string content=https.post_request_page(this->m_serverurl,poststr);
		m_result = content;
		checkresult(content,"SetIdsRules");
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
}

string bd_https_control::ipsession(const string& target,const string& action,
		const string& enable,const string& protocol,const string& srcip,
		const string& srcport,const string& desip,const string& desport)
{
	try
	{
		std::string poststr = "cmd=ipsession&username=";
		poststr = poststr + m_username + "&password=";
		poststr = poststr + m_password + "&key=" + m_sessionkey;
		
		if (!target.empty())
		{
			poststr = poststr + "&target=" + target;
		}
		if (!action.empty())
		{
			poststr = poststr + "&action=" + action;
		}
		if (!enable.empty())
		{
			poststr = poststr + "&enable=" + enable;
		}
		if (!protocol.empty())
		{
			poststr = poststr + "&protocol=" + protocol;
		}
		if (!srcip.empty())
		{
			poststr = poststr + "&srcip=" + srcip;
		}
		if (!srcport.empty())
		{
			poststr = poststr + "&srcport=" + srcport;
		}
		if (!desip.empty())
		{
			poststr = poststr + "&desip=" + desip;
		}
		if (!desport.empty())
		{
			poststr = poststr + "&desport=" + desport;
		}
		
		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		string content=https.post_request_page(this->m_serverurl,poststr);
		//m_result = content;
		m_result = checkresult(content,"IpManage");
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
    return m_result;
}

string bd_https_control::macmanage(const struct mac_policy& macpolicy)
{
	try
	{
		std::string poststr = "cmd=macmanage&username=";
		poststr = poststr + m_username + "&password=";
		poststr = poststr + m_password + "&key=" + m_sessionkey;

		if (!macpolicy.target.empty())
		{
			poststr = poststr + "&target=" + macpolicy.target;
		}
		if (!macpolicy.action.empty())
		{
			poststr = poststr + "&action=" + macpolicy.action;
		}
		if (!macpolicy.enable.empty())
		{
			poststr = poststr + "&enable=" + macpolicy.enable;
		}
		if (!macpolicy.protocol.empty())
		{
			poststr = poststr + "&protocol=" + macpolicy.protocol;
		}
		if (!macpolicy.macaddr.empty())
		{
			poststr = poststr + "&macaddr=" + macpolicy.macaddr;
		}
		if (!macpolicy.srcip.empty())
		{
			poststr = poststr + "&srcip=" + macpolicy.srcip;
		}
		if (!macpolicy.srcport.empty())
		{
			poststr = poststr + "&srcport=" + macpolicy.srcport;
		}
		if (!macpolicy.desip.empty())
		{
			poststr = poststr + "&desip=" + macpolicy.desip;
		}
		if (!macpolicy.desport.empty())
		{
			poststr = poststr + "&desport=" + macpolicy.desport;
		}
		if (!macpolicy.packets.empty())
		{
			poststr = poststr + "&packets=" + macpolicy.packets;
		}
		if (!macpolicy.connections.empty())
		{
			poststr = poststr + "&connections=" + macpolicy.connections;
		}
		if (!macpolicy.stimehour.empty())
		{
			poststr = poststr + "&stimehour=" + macpolicy.stimehour;
		}
		if (!macpolicy.stimemin.empty())
		{
			poststr = poststr + "&stimemin=" + macpolicy.stimemin;
		}
		if (!macpolicy.etimehour.empty())
		{
			poststr = poststr + "&etimehour=" + macpolicy.etimehour;
		}
		if (!macpolicy.etimemin.empty())
		{
			poststr = poststr + "&etimemin=" + macpolicy.etimemin;
		}
		if (!macpolicy.exclude.empty())
		{
			poststr = poststr + "&exclude=" + macpolicy.exclude;
		}
		
		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		string content=https.post_request_page(this->m_serverurl,poststr);
		//m_result = content;
		m_result = checkresult(content,"MacManage");
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
    return m_result;
}

string bd_https_control::scannet(const string& beginip,const string& endip)throw(bd_net_exception)
{
	try
	{
		char poststr[200];
		sprintf(poststr,"cmd=scannet&username=%s&password=%s&key=%s&beginip=%s&endip=%s",
				m_username.c_str(),m_password.c_str(),m_sessionkey.c_str(),beginip.c_str(),endip.c_str());
		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		string content=https.post_request_page(this->m_serverurl,poststr);
		//m_result = content;
		//printf("%s\n",content.c_str());
		m_result = checkresult(content,"ScanNet");
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
    return m_result;
}

void bd_https_control::getNELogin(const string& iplist,list<NELogin*> *nelogins)
{
	char poststr[200];
	sprintf(poststr,"cmd=getnelogininfo&username=%s&password=%s&key=%s&iplist=%s",
			m_username.c_str(),m_password.c_str(),m_sessionkey.c_str(),iplist.c_str());
	bd_https https;
	string content;
	try
	{
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		content=https.post_request_page(this->m_serverurl,poststr);
		m_result = content;
		//string content=https.post_request_page("https://172.16.2.32:441/cgi-bin/index.cgi",poststr);
		printf("%s\n",poststr);
		printf("%s\n",content.c_str());

		if(content.empty())
		{
			throw bd_net_exception("content empty");
		}		
/* 
##################### 暂时没有用到该函数，需要时再把该函数中XML处理部分改成Poco Xml #####################
		xmlDocPtr doc;           //定义解析文档指针
		doc=xmlParseMemory(content.c_str(),content.length()+1);
		xmlNodePtr curNode = xmlDocGetRootElement(doc);

		//检查确认当前文档中包含内容
		if (NULL == curNode)
		{
		  fprintf(stderr,"empty document\n");
		  xmlFreeDoc(doc);
		  throw bd_net_exception("empty document");
		}
		curNode = curNode->xmlChildrenNode;
		while (NULL != curNode)
		{
			if (!xmlStrcmp(curNode->name, BAD_CAST "NELogin"))
			{
				NELogin* nelogin=new NELogin();

				xmlNodePtr curChildNode = curNode->xmlChildrenNode;
				while (NULL != curChildNode)
				{
					if (!xmlStrcmp(curChildNode->name, BAD_CAST "Host"))
					{
						xmlChar *szAttr = xmlGetProp (curChildNode, BAD_CAST "ip");
						char *strkey=(char*)szAttr;

						nelogin->host.Ip=strkey;
						xmlNodePtr curPortNode=curChildNode->xmlChildrenNode;
						while(NULL != curPortNode)
						{
							if (xmlStrcmp(curPortNode->name, BAD_CAST "text")!=0)
							{
									char *strkey=(char*)(curPortNode->xmlChildrenNode->content);
									bd_str2int(strkey,&(nelogin->host.Port));
							}
							curPortNode=curPortNode->next;
						}
						xmlFree(szAttr);
					}
					else if(!xmlStrcmp(curChildNode->name, BAD_CAST "ProtoType"))
					{
						char *strkey=(char*)curChildNode->xmlChildrenNode->content;
						nelogin->ProtoType=strkey;
					}
					else if(!xmlStrcmp(curChildNode->name, BAD_CAST "User"))
					{
						xmlChar *szAttr = xmlGetProp (curChildNode, BAD_CAST "name");
						char *strkey=(char*)szAttr;

						nelogin->user.Name=strkey;
						xmlNodePtr curpwdNode=curChildNode->xmlChildrenNode;
						while(NULL != curpwdNode)
						{
							if (xmlStrcmp(curpwdNode->name, BAD_CAST "text")!=0)
							{
								nelogin->user.Password=(char*)curpwdNode->xmlChildrenNode->content;
							}
							curpwdNode=curpwdNode->next;
						}
						xmlFree(szAttr);
					}
					curChildNode=curChildNode->next;
				}
				nelogins->push_back(nelogin);
			}
			curNode=curNode->next;
		}
		xmlFreeDoc(doc);
##################### 暂时没有用到该函数，需要时再把该函数中XML处理部分改成Poco Xml #####################
*/
		//xmlCleanupParser();
		//result->nodesetval->nodeTab->count
		//xmlChar* key=xmlGetProp(result->nodesetval->nodeTab[0],(const xmlChar*)"result");
		//xmlStrcmp (proot->name, BAD_CAST "根节点") != 0
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}

}


void bd_https_control::flowmonitor(const string& filetype,
								   const string& savefilename)throw(bd_net_exception)
{
	char poststr[200];
	sprintf(poststr,"cmd=flowmonitor&username=%s&password=%s&key=%s&filetype=%s",
		m_username.c_str(),m_password.c_str(),m_sessionkey.c_str(),filetype.c_str());
	
	try
	{
		bd_https https;
		if(!m_verify_file.empty())
		{
			https.customverify(m_verify_file);
		}
		https.post_request_downloadfile(this->m_serverurl,poststr,savefilename);
		//string content=https.post_request_page(this->m_serverurl,poststr);
	}
	catch(const bd_net_exception& ex)
	{
		throw;
	}
}

void bd_https_control::set_verify_file(const std::string& certfilename)
{
	this->m_verify_file=certfilename;
}



