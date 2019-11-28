#include <stdio.h>
#include "bd_snmp_interface.h"
#include "bd_snmp.h"
#include <vector>
#include <iostream>
//CScmpV1

using namespace std;

//删除字符串的字符
void StringDelteChar(string &value, char deletechar)
{
    string::iterator it;
    printf("=======start delete char =========\n");
    for (it = value.begin(); it != value.end(); ) {
        if (*it == deletechar) {
            value.erase(it);
        }
        else {
            ++it;
        }
    }
}

//获取字符串转化成数字的字节，比如 “79K”转成“79*1024”
int StringBytesToLong(string str_value, int &value)
{
    char bytes_units = str_value[str_value.length()-1];
    value = atol(str_value.substr(0,str_value.length()-1).c_str());
    switch(bytes_units)
    {
        case 'T':
            value *= 1024;
        case 'G':
            value *= 1024;
        case 'M':
            value *= 1024;
        case 'K':
            value *= 1024;
        case 'B':
            value *= 1;
            break;
        default:
            value = 0;
            return -1;
    }

    return 0;
}

CSnmpV1::CSnmpV1(const char *pszHost, const char *pszCommunity, const char *pszUser,const char *pszProtol,const char *pszPassPhrase,const long lVersion )
{
    cnet = new CNetSnmp(pszHost, pszCommunity, pszUser, pszProtol, pszPassPhrase, lVersion);
	
}

CSnmpV1::CSnmpV1(const char *pszHost, const char *pszCommunity, const long lVersion, const int seculevel, const char *pszUser, const char *pszProtol, const char *pszPassPhrase, const char *privProtol, const char *privPass)
{

    //cnet = new CNetSnmp(pszHost, pszCommunity, pszUser, pszProtol, pszPassPhrase, lVersion);
   cnet = new CNetSnmp(pszHost, pszCommunity, \
                       lVersion, seculevel,\
                       pszUser, pszProtol, \
                       pszPassPhrase, privProtol, privPass);

}

CSnmpV1::~CSnmpV1()
{
	delete cnet;
    cnet = NULL;
}

int CSnmpV1::SNMP_GetSysInfo(string &sysinfo)
{
    int ret = 0;
    SNMPVALUE sysvalue; //临时变量
    SNMPOID hostoid(".1.3.6.1.2.1.25.1.1.0");   //设备有没有实现host-mib，判断是不是主机
    SNMPOID sysdescoid(".1.3.6.1.2.1.1.1.0");   //系统描述
    SNMPOID sysnameoid(".1.3.6.1.2.1.1.5.0");   //用户名称

    string sysdes = "null";
    string sysname = "null";

    if (cnet->Get_v3(hostoid, &sysvalue) == SNMP_SUCCESS)
    {
        if (cnet->Get_v3(sysdescoid, &sysvalue) == SNMP_SUCCESS)
        {
            string tmpstr = sysvalue.GetValueToString();
            if(tmpstr.find("Windows") != string::npos)
                sysdes = "Windows";
            else if(tmpstr.find("Linux") != string::npos)
                sysdes = "Linux";
            else
                sysdes = "null";
        } else
        {
            ret = -1;
            goto END;
        }

        SNMPVALUE sysnamevalue;  //放置上面的数据遗留问题
        if (cnet->Get_v3(sysnameoid, &sysnamevalue) == SNMP_SUCCESS)
        {
            sysname = sysnamevalue.GetValueToString();
        } else
        {
            ret = -1;
            goto END;
        }
    } else
    {
        if (cnet->Get_v3(sysnameoid, &sysvalue) == SNMP_SUCCESS)
            sysname = sysvalue.GetValueToString();
    }

END:
    sysinfo = sysdes + "~" + sysname;
    return ret;
}

int CSnmpV1::SNMP_GetSwitchType(SNMPVALUE sysvalue)
{
    int switch_type = SWITCH_TYPE_ZERO;
    if( sysvalue.GetValueToString().find("H3C") != string::npos ) {

        if( sysvalue.GetValueToString().find("S5048") != string::npos
            || sysvalue.GetValueToString().find("S7500") != string::npos
            || sysvalue.GetValueToString().find("S3600") != string::npos
            || sysvalue.GetValueToString().find("MSR30-40") != string::npos
            || sysvalue.GetValueToString().find("MSR36-40") != string::npos)
        {
            switch_type = SWITCH_TPYE_H3C_25506;
        }
        else if (sysvalue.GetValueToString().find("S9300") != string::npos) {
            switch_type = SWITCH_TPYE_H3C_2011;
        }
        else {
            switch_type = SWITCH_TYPE_H3C;
        }

        goto switch_return;
    }
    else if( sysvalue.GetValueToString().find("Cisco") != string::npos )
    {
        switch_type = SWITCH_TYPE_CISCO;
        goto switch_return;
    }
    else if( sysvalue.GetValueToString().find("Alcatel") != string::npos )
    {
        switch_type = SWITCH_TYPE_ALCATEL;
        goto switch_return;
    }
    else if( sysvalue.GetValueToString().find("Huawei") != string::npos ) {

        if( sysvalue.GetValueToString().find("S5720") != string::npos ||
            sysvalue.GetValueToString().find("S6720S") != string::npos ) {
            switch_type = SWITCH_TYPE_HUAWEI_2011;
        }
        if (sysvalue.GetValueToString().find("AC6") != string::npos ||
            sysvalue.GetValueToString().find("ACU2") != string::npos ||
            sysvalue.GetValueToString().find("AR1") != string::npos ||
            sysvalue.GetValueToString().find("AR2") != string::npos ||
            sysvalue.GetValueToString().find("AR3") != string::npos ){

            switch_type = SWITCH_TYPE_HUAWEI_2011_AC;

        }
        else{
            switch_type = SWITCH_TYPE_HUAWEI;
        }

        goto switch_return;
    }
    else if( sysvalue.GetValueToString().find("Linux (none)") != string::npos ||
             sysvalue.GetValueToString().find("VSOS-V0302R") != string::npos )
    {
        switch_type = SWITCH_TYPE_QIMING;
        goto switch_return;
    }
    else if( sysvalue.GetValueToString().find("NSG") != string::npos ) {

        if( sysvalue.GetValueToString().find("3500") != string::npos ) {
            switch_type = SWITCH_TYPE_NSG_32328;
        }
        else {
            switch_type = SWITCH_TYPE_NSG_32328;
        }
        goto switch_return;
    }
    else if (sysvalue.GetValueToString().find("Linux NS") != string::npos) {

        if (sysvalue.GetValueToString().find("NSOS") != string::npos) {
            switch_type = SWITCH_TYPE_ICG_37157;
        }
        else {
            switch_type = SWITCH_TYPE_ICG_37157;
        }
        goto switch_return;
    }
    else if (sysvalue.GetValueToString().find("Linux Sangfor") != string::npos) {
        switch_type = SWITCH_TYPE_SANGFOR;
        goto switch_return;
    }
    else if (sysvalue.GetValueToString().find("Topsec  NGFW") != string::npos) {
        switch_type = SWITCH_TYPE_TOPSEC_NGFW;
        goto switch_return;
    }
    else if (sysvalue.GetValueToString().find("Linux sag") != string::npos) {
        switch_type = SWITCH_TYPE_SECAV;
        goto switch_return;
    }
    /*else if (sysvalue.GetValueToString().find("Linux CENTOS") != string::npos) {
        switch_type = SWITCH_TYPE_YUNNAN_CENTOS;
        goto switch_return;
    }*/

    switch_return:
        return switch_type;
}

int CSnmpV1::SNMP_GetNetworkList(NetWorkInfoList &lis)
{
	//lis.clear();
	SNMPOID if_oid(".1.3.6.1.2.1.2.2");//if_table
	SNMPTABLE tb;

	int ret = -1;
    ret = cnet->GetTable_v3(if_oid,&tb);
	if(ret != 0)
	{
		const char* errMsg = cnet->GetError();
	    if ( !errMsg )
        {
		    errMsg = "Unknown Error!";
	        printf( "failed ret=%d : %s \n", ret,errMsg );
		}	
		return -1;
	}
    /*//ifdesc_list.Print();
	printf("==>count= %d\n",tb.count);
	long line_count = tb.count;
	int fields_count = 22;
    int  index_ifout = 15;
    if (line_count%fields_count != 0)
    {
        fields_count = 18;
        index_ifout = 14;
    }
	int net_count = line_count / fields_count;//有几个网卡
	
	int  index_desc = 1;
	int  index_mac = 5;
	int  index_opStat = 7;
	int  index_ifin = 9;
				
	SNMPTABLEROW *row  = NULL;
	for(int i =1 ;i <= net_count ;i++ )
	{
		NetworkInfo info ;
		
		row = tb[index_desc*net_count + i];
		printf("%d ==> %s \n",index_desc*net_count + i,row->column->column_value.szStrValue);
		if (row != NULL)
		    __strncpy(info.if_name,sizeof(info.if_name),row->column->column_value.szStrValue);
		
		row = tb[index_mac*net_count + i];
		//printf("%d ==> %s \n",index_mac*net_count + i,row->column->column_value.szStrValue);
		if (row != NULL)
		    __strncpy(info.if_mac,sizeof(info.if_mac),row->column->column_value.szStrValue);
		
		row = tb[index_opStat*net_count + i];
		//printf("%d ==> %d \n",index_opStat*net_count + i,row->column->column_value.nInterValue);
		if (row != NULL)
		    info.if_status = row->column->column_value.nInterValue;
		
		row = tb[index_ifin*net_count + i];
        //printf("%d ==> %ld \n",index_ifin*net_count + i,row->column->column_value.nInterValue);
		if (row != NULL)
		    info.if_inOctet = row->column->column_value.nInterValue;
		
		row = tb[index_ifout*net_count + i];
		//printf("%d ==> %ld \n",index_ifout*net_count + i,row->column->column_value.nInterValue);
		if (row != NULL)
		    info.if_outOctet = row->column->column_value.nInterValue;
		
		lis.push_back(info);
    }*/

    int  col_desc = 2;
    int  col_mac = 6;
    int  col_opStat = 8;
    int  col_ifin = 10;
    int  col_ifout = 16;

    int net_count = tb.nRows; //有几个网卡
    int nTbOidLen = strlen(tb.table_oid.szOID);
    printf("&&&&&&&& net_count==> %d \n", net_count);

    SNMPTABLEROW *row  = NULL;
    for(int i = 1; i <= net_count; i++ )
    {
        NetworkInfo info ;
        bool isEmpty = true; //判断该条是否为空
        unsigned int nRowIndex = 0;

        //获取行对应的oid索引
        row = tb[i];
        if(strlen(row->rowoid.szOID) <= nTbOidLen) continue;
        char indexOid[64] = {0};
        tb.GetTailOID(tb.table_oid, row->rowoid, indexOid);
        if(indexOid[0] == '1' && indexOid[1] == '.')
        {
            char *nextdot = strchr(indexOid+2, '.');
            if(nextdot) indexOid[nextdot - indexOid] = '\0';

            nRowIndex = atoi(indexOid + 2);
        } else
            continue;

        //printf("&&&&&&&& i: %d  rol_desc ==> %d \n", i, nRowIndex);
        if(nRowIndex == 0) continue;

        row = tb.GetRowByCoordinate(nRowIndex, col_desc);
        if (row != NULL){
             __strncpy(info.if_name,sizeof(info.if_name),row->column->column_value.szStrValue);
             isEmpty = false;
        }

        row = tb.GetRowByCoordinate(nRowIndex, col_mac);
        if (row != NULL){
            __strncpy(info.if_mac,sizeof(info.if_mac),row->column->column_value.szStrValue);
            isEmpty = false;
        }

        row = tb.GetRowByCoordinate(nRowIndex, col_opStat);
        if (row != NULL){
            info.if_status = row->column->column_value.nInterValue;
            isEmpty = false;
        }

        row = tb.GetRowByCoordinate(nRowIndex, col_ifin);
        if (row != NULL){
            info.if_inOctet = row->column->column_value.nInterValue;
            isEmpty = false;
        }

        row = tb.GetRowByCoordinate(nRowIndex, col_ifout);
        if (row != NULL){
            info.if_outOctet = row->column->column_value.nInterValue;
            isEmpty = false;
        }

        if(!isEmpty)
            lis.push_back(info);
    }

	if (row != NULL) row->Free();

	return 0;
}

int	CSnmpV1::SNMP_GetSWInstalledList(SWInstallInfoList &lis)
{
	//lis.clear();

	SNMPOID swinstall_oid(".1.3.6.1.2.1.25.6.3");//swinstall_table
	SNMPTABLE tb;

	int ret = -1;
    ret = cnet->GetTable_v3(swinstall_oid,&tb);
	if(ret != 0)
	{
		const char* errMsg = cnet->GetError();
	    if ( !errMsg )
        {
		    errMsg = "Unknown Error!";
	        printf( "failed ret=%d : %s \n", ret,errMsg );
		}	
		return -1;
	}

    /*printf("==>count= %d\n",tb.count);
	long line_count = tb.count;
	int fields_count = 5;
	int sw_count = line_count / fields_count;//有几个已安装的软件

	int  index_name = 1;
	int  index_type = 3;
	int  index_date = 4;

	SNMPTABLEROW *row  = NULL;
	for(int i =1 ;i <= sw_count ;i++ )
	{
		SWInstallInfo info;
		
		row = tb[index_name*sw_count + i];
		//printf("%d name==> %s \n",index_name*sw_count + i,row->column->column_value.szStrValue);

		if (row != NULL)
		    __strncpy(info.sw_name,sizeof(info.sw_name),row->column->column_value.szStrValue);
		
		row = tb[index_type*sw_count + i];
		//printf("%d type==> %d \n",index_type*sw_count + i,row->column->column_value.nInterValue);
		if (row != NULL)
		    info.sw_type = row->column->column_value.nInterValue;

		row = tb[index_date*sw_count + i];
		//printf("%d date==> %s\n",index_date*sw_count + i,row->column->column_value.szStrValue);
		if (row != NULL)
		    __strncpy(info.sw_date,sizeof(info.sw_date),row->column->column_value.szStrValue);
		//printf("##########\n");
		lis.push_back(info);
    }*/

    int  col_name = 2;
    int  col_type = 4;
    int  col_date = 5;

    int net_count = tb.nRows; //有几个已安装的软件
    int nTbOidLen = strlen(tb.table_oid.szOID);
    printf("&&&&&&&& net_count==> %d \n", net_count);

    SNMPTABLEROW *row  = NULL;
    for(int i = 1; i <= net_count; i++ )
    {
        SWInstallInfo info ;
        bool isEmpty = true; //判断该条是否为空
        unsigned int nRowIndex = 0;

        //获取行对应的oid索引
        row = tb[i];
        if(strlen(row->rowoid.szOID) <= nTbOidLen) continue;
        char indexOid[64] = {0};
        tb.GetTailOID(tb.table_oid, row->rowoid, indexOid);
        if(indexOid[0] == '1' && indexOid[1] == '.')
        {
            char *nextdot = strchr(indexOid+2, '.');
            if(nextdot) indexOid[nextdot - indexOid] = '\0';

            nRowIndex = atoi(indexOid + 2);
        } else
            continue;

        //printf("&&&&&&&& i: %d  rol_desc ==> %d \n", i, nRowIndex);
        if(nRowIndex == 0) continue;

        row = tb.GetRowByCoordinate(nRowIndex, col_name);
        if (row != NULL){
             __strncpy(info.sw_name,sizeof(info.sw_name),row->column->column_value.szStrValue);
             isEmpty = false;
        }

        row = tb.GetRowByCoordinate(nRowIndex, col_type);
        if (row != NULL){
            info.sw_type = row->column->column_value.nInterValue;
            isEmpty = false;
        }

        row = tb.GetRowByCoordinate(nRowIndex, col_date);
        if (row != NULL){
            __strncpy(info.sw_date,sizeof(info.sw_date),row->column->column_value.szStrValue);

            isEmpty = false;
        }

        if(!isEmpty)
            lis.push_back(info);
    }

	if (row != NULL) row->Free();		
	return 0;
}

int CSnmpV1::SNMP_GetSWRunStatusList(ProcessInfoList &lis)
{
	//lis.clear();

	SNMPOID swrun_oid(".1.3.6.1.2.1.25.4.2");//swrun_table
	SNMPTABLE tb;

	int ret = -1;
    ret = cnet->GetTable_v3(swrun_oid,&tb);
	if(ret != 0)
	{
		const char* errMsg = cnet->GetError();
	    if ( !errMsg )
        {
		    errMsg = "Unknown Error!";
	        printf( "failed ret=%d : %s \n", ret,errMsg );
		}	
		return -1;
	}

	printf("==>count= %d\n",tb.count);
	long line_count = tb.count;
	int fields_count = 7;
	int process_count = line_count / fields_count;//有几个进程

	int  index_name = 1;
	int  index_path = 3;
	int  index_status = 6;

	SNMPTABLEROW *row  = NULL;
	for(int i =1 ;i <= process_count ;i++ )
	{
		ProcessInfo info;
		
		row = tb[index_name*process_count + i];
		//printf("%d name==> %s \n",index_name*process_count + i,row->column->column_value.szStrValue);
		if (row != NULL)
		    __strncpy(info.process_name,sizeof(info.process_name),row->column->column_value.szStrValue);
		
		row = tb[index_path*process_count + i];
		//printf("%d path==> %s \n",index_path*process_count + i,row->column->column_value.szStrValue);
		if (row != NULL)
		    __strncpy(info.process_path,sizeof(info.process_path),row->column->column_value.szStrValue);
		
		row = tb[index_status*process_count + i];
		//printf("%d status==> %d \n",index_status*process_count + i,row->column->column_value.nInterValue);
		if (row != NULL)
		    info.process_status = row->column->column_value.nInterValue;
		//printf("##########\n");
		lis.push_back(info);
	}
	if (row != NULL) row->Free();
	
	return 0;
}


int CSnmpV1::SNMP_GetRamUsage(double *value)
{
	*value = 0;
    SNMPVALUE sysvalue; //临时变量
    SNMPOID sysdescoid(".1.3.6.1.2.1.1.1.0"); //系统描述 判断交换机类型，
    SNMPOID hostoid(".1.3.6.1.2.1.25.1.1.0");   //设备有没有实现host-mib，判断是不是主机
    bool is_host = false;
    int  switch_type = SWITCH_TYPE_ZERO;
    if (cnet->Get_v3(hostoid, &sysvalue) == SNMP_SUCCESS)
    {
        is_host = true;
    }

    /*
    if(false == is_host)  //不是主机的话，判断时候思科或者华为的交换机，从而获取内存占用率
    {
        if (cnet->Get_v3(sysdescoid, &sysvalue) == SNMP_SUCCESS)
        {
            //获取交换机的型号
            switch_type = SNMP_GetSwitchType(sysvalue);
        }
    }*/

    if (cnet->Get_v3(sysdescoid, &sysvalue) == SNMP_SUCCESS)
    {
        //获取交换机的型号
        switch_type = SNMP_GetSwitchType(sysvalue);
    }

    if( switch_type == SWITCH_TYPE_CISCO )  //思科交换机
    {
        SNMPOID ciscoMemFreeoid(".1.3.6.1.4.1.9.9.48.1.1.1.6.1");//空闲内存
        SNMPOID ciscoMemUsedoid(".1.3.6.1.4.1.9.9.48.1.1.1.5.1");//已使用内存
        int  FreeMemory = 0;
        int  UsedMemory = 0;
        if (cnet->Get_v3(ciscoMemFreeoid, &sysvalue) == SNMP_SUCCESS)
        {
           FreeMemory = sysvalue.nInterValue;
        }
        if (cnet->Get_v3(ciscoMemUsedoid, &sysvalue) == SNMP_SUCCESS)
        {
           UsedMemory = sysvalue.nInterValue;
        }
        *value = (double)UsedMemory/(UsedMemory+FreeMemory);
        return 0;
    }
    else if( switch_type == SWITCH_TPYE_H3C_25506 )
    {
        SNMPOID h3cMemoid(".1.3.6.1.4.1.25506.2.6.1.1.1.1.8");
        unsigned long total = 0;
        int count = 0;
        if (SNMP_GetTotalOfListValue(h3cMemoid, total, count) == 0)
        {
            if (count == 0) {
                return 0;
            }
            *value = (double)total/count/100;
            return 0;
        }
        else
            return -1;
    }
    else if( switch_type == SWITCH_TPYE_H3C_2011
             || switch_type == SWITCH_TYPE_H3C
             || switch_type == SWITCH_TYPE_HUAWEI_2011
             || switch_type == SWITCH_TYPE_HUAWEI )
    {
        SNMPOID huaMemSizeoid(".1.3.6.1.4.1.2011.6.3.5.1.1.2"); //总共内存空间
        SNMPOID huaMemUsedoid(".1.3.6.1.4.1.2011.6.3.5.1.1.4"); //已使用的内存
        unsigned long huaMemSize = 0;
        unsigned long huaUsedSize = 0;
        int totalcount = 0;
        int usedcount = 0;
        SNMP_GetTotalOfListValue(huaMemSizeoid,huaMemSize,totalcount);
        SNMP_GetTotalOfListValue(huaMemUsedoid,huaUsedSize,usedcount);
        if (totalcount == usedcount && huaMemSize != 0)
        {
            *value = (double)huaUsedSize/huaMemSize;
			return 0;
        }
        else
            return -1;
    }
    else if (switch_type == SWITCH_TYPE_HUAWEI_2011_AC){
        SNMPOID huacpuoid("1.3.6.1.4.1.2011.5.25.31.1.1.1.1.7");
        long unsigned total = 0;
        int count = 0;
        if (SNMP_GetTotalOfListValue(huacpuoid,total,count) == 0)
        {
            if (count == 0) {
                return 0;
            }
            *value = (double)total/count/100;
            return 0;
        }
        else
            return -1;
    }
    else if(switch_type == SWITCH_TYPE_QIMING)
    {
        SNMPOID qmMemUsageoid("1.3.6.1.4.1.15227.1.3.1.1.2.0");
        string usages;
        if (cnet->Get_v3(qmMemUsageoid, &sysvalue) == SNMP_SUCCESS)
        {
            unsigned int i = 1;
            usages = sysvalue.GetValueToString();
            if(usages.find(':') != string::npos)
                i = usages.find(':') + 1;
            usages = usages.substr(i, usages.find('%'));
        }
        printf("-------usages:%s-------\n", usages.c_str());
        *value = (double)atoi(usages.c_str())/100;
        return 0;
    }
    else if( switch_type == SWITCH_TYPE_NSG_32328 ) {
        SNMPOID nsgcpuoid(".1.3.6.1.4.1.32328.6.1.10.3.2.0");
        if (cnet->Get_v3(nsgcpuoid, &sysvalue) == SNMP_SUCCESS)
        {
           *value = (double)sysvalue.nInterValue/100;
           return 0;
        }
        else
           return -1;
    }
    else if (switch_type == SWITCH_TYPE_ICG_37157) {
        SNMPOID icgcpupoid("1.3.6.1.4.1.37157.1.4.1.0");
        string strUsage;
        if (cnet->Get_v3(icgcpupoid, &sysvalue) == SNMP_SUCCESS) {
            /*
            unsigned int i = 1;
            strUsage = sysvalue.GetValueToString();
            if (strUsage.find_first_of('\"') != string::npos)
            {
                i = strUsage.find('\"') + 1;
                strUsage = strUsage.substr(i, strUsage.find('%'));
                *value = (double)atoi(strUsage.c_str())/100;
            }*/
            strUsage = sysvalue.GetValueToString();
            StringDelteChar(strUsage, '\"');
            StringDelteChar(strUsage,'%');
            *value = (double)atoi(strUsage.c_str())/100;
            return 0;
        }
        else
            return -1;
    }
    else if (switch_type == SWITCH_TYPE_TOPSEC_NGFW) {
        SNMPOID topsecramoid(".1.3.6.1.4.1.14331.5.5.1.4.6.0");
        if (cnet->Get_v3(topsecramoid, &sysvalue) == SNMP_SUCCESS) {
            *value = (double)sysvalue.nInterValue/100;
            return 0;
        }
        else
            return -1;
    }
    else if (switch_type == SWITCH_TYPE_YUNNAN_CENTOS) {
        SNMPOID yunnancentosramoid(".1.3.6.1.2.1.25.5.1.1.2");
        if (cnet->Get_v3(yunnancentosramoid, &sysvalue) == SNMP_SUCCESS) {
            *value = (double)sysvalue.nInterValue/100;
            return 0;
        }
        else
            return -1;
    }
    else if (switch_type == SWITCH_TYPE_SECAV) {
        SNMPOID secavramoid(".1.3.6.1.4.1.40094.1.6");
        if (cnet->Get_v3(secavramoid, &sysvalue) == SNMP_SUCCESS) {
            *value = (double)sysvalue.nInterValue/100;
            return 0;
        }
        else
            return -1;
    }

    if(true == is_host)
    {
        SNMPOID oid(".1.3.6.1.2.1.25.2.3.1");//Storage
        SNMPVALUELIST lis;

        int ret = -1;
        ret = cnet->Walk_v3(oid,&lis);//用table不好做
        if(ret != SNMP_SUCCESS)
        {
            const char* errMsg = cnet->GetError();
            if ( !errMsg )
            {
                errMsg = "Unknown Error!";
                fprintf( stderr,"failed ret=%d : %s \n", ret,errMsg );
            }
            return -1;
        }

        int storage_count = 0;//磁盘分区和内存的总个数
        char *oid_ram_used = NULL;//已使用的物理内存的oid
        char *oid_ram_total = NULL;//总的的物理内存的oid

        LPSNMPVALUE t = lis.lpValue;
        while(NULL != t)
        {
            if(t->nValueType == SNMP_VALUE_OID)
            {
                storage_count++;
                if(strcmp(".1.3.6.1.2.1.25.2.1.2",t->GetValueToString().c_str())==0)//物理内存
                {
                    //printf("oid oid==> %s \n",t->snmpOID.szOID);
                    char *ptr;
                    char *p;
                    char *pt;
                    ptr = strtok_r(t->snmpOID.szOID, ".", &p);
                    while(ptr != NULL)
                    {
                        //printf("ptr=%s\n",ptr);
                        pt = ptr;
                        ptr = strtok_r(NULL, ".", &p);
                    }
                    //printf("pt ==%s \n",pt);
                    char str_used[32] = ".1.3.6.1.2.1.25.2.3.1.6.";
                    char str_total[32] = ".1.3.6.1.2.1.25.2.3.1.5.";
                    oid_ram_used = strcat(str_used,pt);
                    oid_ram_total = strcat(str_total,pt);
                    break;
                }
            }
            t = t->next_value;
        }

        if( NULL==oid_ram_used || NULL==oid_ram_total )
        {
            fprintf( stderr,"can not find Physical Memory ... \n");
            return -2;
        }
        //printf("storage_count = %d oid_ram_total=%s oid_ram_used=%s \n",storage_count,oid_ram_total,oid_ram_used);


        int totalRam_value = 0;
        int usedRam_value = 0;
        while(NULL != lis.lpValue)
        {
            if(0==strcmp(lis.lpValue->snmpOID.szOID,oid_ram_total))
            {
                //printf("ram_total value==> %lld \n",lis.lpValue->nInterValue);
                totalRam_value = lis.lpValue->nInterValue;
            }
            if(0==strcmp(lis.lpValue->snmpOID.szOID,oid_ram_used))
            {
                //printf("ram_used value==> %lld \n",lis.lpValue->nInterValue);
                usedRam_value = lis.lpValue->nInterValue;
            }
            lis.lpValue = lis.lpValue->next_value;
        }

        if( 0==totalRam_value ||  0==usedRam_value)
        {
            fprintf( stderr,"error: can not find totalRam or  usedRam... \n");
            return -3;
        }
        //printf(" usedRam_value = %d ,totalRam_value=%d \n",usedRam_value,totalRam_value);
        *value = (double)usedRam_value/totalRam_value;
        lis.Free();
    }
	return 0;
}

//计算列表总数
int CSnmpV1::SNMP_GetTotalOfListValue(SNMPOID &oid,unsigned long &total,int &count)
{
    SNMPVALUELIST lis;
    int ret = -1;
    ret = cnet->Walk_v3(oid,&lis);
    if(ret != 0)
    {
        const char* errMsg = cnet->GetError();
        if ( !errMsg )
        {
            errMsg = "Unknown Error!";
            fprintf( stderr,"failed ret=%d : %s \n", ret,errMsg );
        }
        return -1;
    }

//    int count = 0;
//    int total = 0;

    LPSNMPVALUE t = lis.lpValue;
    while(NULL != t)
    {
        if (t->nInterValue != 0)
        {
            total += t->nInterValue;
            count++;
        }
        //t->Print();
        t= t->next_value;
    }
    if(count == 0)
    {
        fprintf(stderr,"can not find cpu info ... \n");
        return -2;
    }
    lis.Free();
    return 0;
}

int CSnmpV1::SNMP_GetCpuUsage(double *value)
{
    SNMPVALUE sysvalue; //临时变量
    SNMPOID sysdescoid(".1.3.6.1.2.1.1.1.0"); //系统描述 判断交换机类型，交换机获取cpu占用率专用oid
    SNMPOID hostoid(".1.3.6.1.2.1.25.1.1.0");   //设备有没有实现host-mib，判断是不是主机
    *value = 0;

    bool is_host = false;
    int  switch_type = SWITCH_TYPE_ZERO;

    if (cnet->Get_v3(hostoid, &sysvalue) == SNMP_SUCCESS)
    {
        is_host = true;
    }

    /*
    if(false == is_host)  //不是主机的话，判断时候思科或者华为的交换机，从而获取cpu占用率
    {
        if (cnet->Get_v3(sysdescoid, &sysvalue) == SNMP_SUCCESS)
        {
            //获取交换机的型号
            switch_type = SNMP_GetSwitchType(sysvalue);
        }
    }*/
    if (cnet->Get_v3(sysdescoid, &sysvalue) == SNMP_SUCCESS)
    {
        //获取交换机的型号
        switch_type = SNMP_GetSwitchType(sysvalue);
    }

    printf("=============SNMP_GetCpuUsage switch_type:%d=====\n",switch_type);

    if( switch_type == SWITCH_TYPE_CISCO )
    {
        SNMPOID ciscocpuoid(".1.3.6.1.4.1.9.2.1.57.0");//前一分钟的cpu占用率
        if (cnet->Get_v3(ciscocpuoid, &sysvalue) == SNMP_SUCCESS)
        {
           *value = (double)sysvalue.nInterValue/100;
           return 0;
        }
        else
           return -1;
    }
    else if( switch_type == SWITCH_TPYE_H3C_25506 )
    {
        SNMPOID h3cCpuoid(".1.3.6.1.4.1.25506.2.6.1.1.1.1.6");
        unsigned long total = 0;
        int count = 0;
        if (SNMP_GetTotalOfListValue(h3cCpuoid, total, count) == 0)
        {
            if (count == 0) {
                return 0;
            }
            *value = (double)total/count/100;
            return 0;
        }
        else
            return -1;
    }
    else if( switch_type == SWITCH_TPYE_H3C_2011
             || switch_type == SWITCH_TYPE_H3C
             || switch_type == SWITCH_TYPE_HUAWEI_2011
             || switch_type == SWITCH_TYPE_HUAWEI )
    {
        SNMPOID huacpuoid(".1.3.6.1.4.1.2011.6.3.4.1.3");
        long unsigned total = 0;
        int count = 0;
        if (SNMP_GetTotalOfListValue(huacpuoid,total,count) == 0)
        {
            if (count == 0) {
                return 0;
            }
            *value = (double)total/count/100;
            return 0;
        }
        else
            return -1;
    }
    else if (switch_type == SWITCH_TYPE_HUAWEI_2011_AC){
        SNMPOID huacpuoid("1.3.6.1.4.1.2011.5.25.31.1.1.1.1.5");
        long unsigned total = 0;
        int count = 0;
        if (SNMP_GetTotalOfListValue(huacpuoid,total,count) == 0)
        {
            if (count == 0) {
                return 0;
            }
            *value = (double)total/count/100;
            return 0;
        }
        else
            return -1;
    }
    else if( switch_type == SWITCH_TYPE_NSG_32328 ) {
        SNMPOID nsgcpuoid(".1.3.6.1.4.1.32328.6.1.10.1.2.0");
        if (cnet->Get_v3(nsgcpuoid, &sysvalue) == SNMP_SUCCESS)
        {
           *value = (double)sysvalue.nInterValue/100;
           return 0;
        }
        else
           return -1;
    }
    else if (switch_type == SWITCH_TYPE_ICG_37157) {
        SNMPOID icgcpupoid("1.3.6.1.4.1.37157.1.4.3.0");
        string strUsage;

        if (cnet->Get_v3(icgcpupoid, &sysvalue) == SNMP_SUCCESS) {
//            unsigned int i = 1;
//            strUsage = sysvalue.GetValueToString();
//            if (strUsage.find_first_of('\"') != string::npos)
//            {
//                i = strUsage.find('\"') + 1;
//                strUsage = strUsage.substr(i, strUsage.find('%'));
//                *value = (double)atoi(strUsage.c_str())/100;
//            }
            strUsage = sysvalue.GetValueToString();
            StringDelteChar(strUsage, '\"');
            StringDelteChar(strUsage,'%');
            *value = (double)atoi(strUsage.c_str())/100;
            return 0;
        }
        else
            return -1;
    }
    else if (switch_type == SWITCH_TYPE_SANGFOR) {
        SNMPOID sangforcpupoid(".1.3.6.1.4.1.35047.1.6.2.0");
        string strUsage;
        if (cnet->Get_v3(sangforcpupoid, &sysvalue) == SNMP_SUCCESS) {
            strUsage = sysvalue.GetValueToString();
            StringDelteChar(strUsage, '\"');
            *value = (double)atof(strUsage.c_str());
//            unsigned int i = 1;
//            strUsage = sysvalue.GetValueToString();
//            if (strUsage.find_first_of('\"') != string::npos)
//            {
//                i = strUsage.find('\"') + 1;
//                strUsage = strUsage.substr(i, strUsage.find_last_of('\"'));
//                *value = (double)atof(strUsage.c_str());
//            }
            return 0;
        }
        else
            return -1;
    }
    else if (switch_type == SWITCH_TYPE_TOPSEC_NGFW) {
        SNMPOID topseccpuoid(".1.3.6.1.4.1.14331.5.5.1.4.5.0");
        if (cnet->Get_v3(topseccpuoid, &sysvalue) == SNMP_SUCCESS) {
            *value = (double)sysvalue.nInterValue/100;
            return 0;
        }
        else
            return -1;
    }
    else if (switch_type == SWITCH_TYPE_YUNNAN_CENTOS) {
        SNMPOID yunnancentoscpuoid(".1.3.6.1.2.1.25.5.1.1.1");
        if (cnet->Get_v3(yunnancentoscpuoid, &sysvalue) == SNMP_SUCCESS) {
            *value = (double)sysvalue.nInterValue/100;
            return 0;
        }
        else
            return -1;
    }
    else if (switch_type == SWITCH_TYPE_SECAV) {
        SNMPOID secavcpuoid(".1.3.6.1.4.1.40094.1.5");
        if (cnet->Get_v3(secavcpuoid, &sysvalue) == SNMP_SUCCESS) {
            *value = (double)sysvalue.nInterValue/100;
            return 0;
        }
        else
            return -1;
    }

    if(true == is_host)
    {
        SNMPOID hostoid(".1.3.6.1.2.1.25.3.3.1.2");//cpu
        unsigned long total = 0;
        int count = 0;
        if (SNMP_GetTotalOfListValue(hostoid,total,count) == 0)
        {
            *value = (double)total/count/100;
            return 0;
        }
        else
            return -1;
    }

	return 0;
}




int CSnmpV1::SNMP_GetDiskUsageList(DiskInfoList& li)
{
	//li.clear();
    SNMPVALUE sysvalue; //临时变量
    SNMPOID sysdescoid(".1.3.6.1.2.1.1.1.0"); //系统描述 判断交换机类型，交换机获取cpu占用率专用oid
    SNMPOID hostoid(".1.3.6.1.2.1.25.1.1.0");   //设备有没有实现host-mib，判断是不是主机

    bool is_host = false;
    int  switch_type = SWITCH_TYPE_ZERO;

    if (cnet->Get_v3(hostoid, &sysvalue) == SNMP_SUCCESS)
    {
        is_host = true;
    }

    if (cnet->Get_v3(sysdescoid, &sysvalue) == SNMP_SUCCESS)
    {
        //获取交换机的型号
        switch_type = SNMP_GetSwitchType(sysvalue);
    }

    if (switch_type == SWITCH_TYPE_ICG_37157) {
        PartitionInfo info = {0};
        SNMPOID icgcpupoid("1.3.6.1.4.1.37157.1.4.2.0");
        if (cnet->Get_v3(icgcpupoid, &sysvalue) == SNMP_SUCCESS) {
            string strUsage;
            unsigned int i = 1;
            strUsage = sysvalue.GetValueToString();
            if (strUsage.find_first_of('\"') != string::npos)
            {
                i = strUsage.find('\"') + 1;
                strUsage = strUsage.substr(i, strUsage.find('%'));
                info.partition_usage = (double)atoi(strUsage.c_str())/100;
            }
            __strncpy(info.partition_name,sizeof(info.partition_name),"disk");
            __strncpy(info.partition_type,sizeof(info.partition_type),"getdisktype");
            info.disk_total = 0;
            info.disk_used = 0;

            li.push_back(info);
            return 0;
        }
        else
            return -1;
    }
    else if (switch_type == SWITCH_TYPE_SANGFOR) {
        SNMPOID oid(".1.3.6.1.4.1.35047.1.5.1");
        SNMPVALUELIST lis;

        int ret = -1;
        ret = cnet->Walk_v3(oid, &lis);
        if (ret != SNMP_SUCCESS) {
            const char * errMsg = cnet->GetError();
            if (!errMsg) {
                errMsg = "UnKown Error";
                fprintf(stderr, "failed ret = %d : %s \n", ret, errMsg);
            }
            return -1;
        }

        int index = lis.Count();
        if (index == 0) {
            fprintf(stderr, "no storage record..\n");
            return -2;
        }

        printf("===SNMP_GetDiskUsageList get index:%d\n", index);

        for(int i=1 ; i <= index; i++)
        {
            char chIndex[5] = {0};
            sprintf(chIndex,"%d",i);
            LPSNMPVALUE t = lis.lpValue;
            char oid_disk_desc[32]  = ".1.3.6.1.4.1.35047.1.5.1.2.";
            char oid_disk_used[32]  = ".1.3.6.1.4.1.35047.1.5.1.4.";
            char oid_disk_total[32] = ".1.3.6.1.4.1.35047.1.5.1.3.";
            char oid_disk_usage[32] = ".1.3.6.1.4.1.35047.1.5.1.6.";

            strcat(oid_disk_desc, chIndex);
            strcat(oid_disk_usage, chIndex);
            strcat(oid_disk_used, chIndex);
            strcat(oid_disk_total,chIndex);
            printf("oid_disk_used=%s,  oid_disk_total=%s \n  ",oid_disk_used,oid_disk_total);

            PartitionInfo info = {0};
            int nflag = 16;
            int disk_used  = 0;
            int disk_total = 0;
            double disk_usage = 0;
            while(NULL != t)
            {
                if(0==strcmp(t->snmpOID.szOID,oid_disk_desc))
                {
                    printf("disk_desc value==> %s \n",t->szStrValue);
                    string oid_disk_desc_str = t->szStrValue;
                    StringDelteChar(oid_disk_desc_str,'\"');
                    printf("disk_desc value==> %s \n",oid_disk_desc_str.c_str());
                    __strncpy(info.partition_name,sizeof(info.partition_name),oid_disk_desc_str.c_str());
                    printf("disk_desc value==> %s \n",oid_disk_desc_str.c_str());
                    nflag >>= 1;
                }

                if(0==strcmp(t->snmpOID.szOID,oid_disk_total))
                {
                    string str_oid_disk_total = t->szStrValue;
                    StringDelteChar(str_oid_disk_total,'\"');
                    StringBytesToLong(str_oid_disk_total, disk_total);
                    printf("disk_total value ==> %ld \n",disk_total);
//                    disk_total = t->nInterValue;
                    nflag >>= 1;
                }

                if(0==strcmp(t->snmpOID.szOID,oid_disk_used))
                {
                    string str_oid_disk_used = t->szStrValue;
                    StringDelteChar(str_oid_disk_used,'\"');
                    StringBytesToLong(str_oid_disk_used, disk_used);
                    printf("disk_used value ==> %ld \n",disk_used);

//                    disk_used = t->nInterValue;
                    nflag >>= 1;
                }

                if(0==strcmp(t->snmpOID.szOID,oid_disk_usage))
                {
                    string str_oid_disk_usage = t->szStrValue;
                    StringDelteChar(str_oid_disk_usage,'\"');
                    StringDelteChar(str_oid_disk_usage,'%');
                    disk_usage = (double)atof(str_oid_disk_usage.c_str())/100;
                    printf("disk_total value ==> %lf \n",disk_usage);

                    nflag >>= 1;
                }

                if(0!=disk_used && 0!=disk_total && 0!=disk_usage)
                {
                    info.partition_usage = disk_usage;
                    info.disk_total = disk_total;
                    info.disk_used = disk_used;
                    __strncpy(info.partition_type,sizeof(info.partition_type),"getdisktype");
                    disk_used = 0;
                    disk_total = 0;
                    disk_usage = 0;
                }

                if (nflag == 1) // 匹配成功
                {
                    char value[100];
                    memset(value, 0, 100);
                    sprintf(value,"%f~%d~%d",info.partition_usage, info.disk_total, info.disk_used);
                    fprintf(stdout,"=============nflag:%s\n",value);
                    li.push_back(info);
                    break;
                }
                t = t->next_value;
            }

        }
        lis.Free();
        return 0;
    }
    else if (switch_type == SWITCH_TYPE_SECAV) {
        PartitionInfo info = {0};
        SNMPOID icgcpupoid(".1.3.6.1.4.1.40094.1.8");
        if (cnet->Get_v3(icgcpupoid, &sysvalue) == SNMP_SUCCESS) {
            string strUsage;
            unsigned int i = 1;
            strUsage = sysvalue.GetValueToString();
            if (strUsage.find_first_of('\"') != string::npos)
            {
                i = strUsage.find('\"') + 1;
                strUsage = strUsage.substr(i, strUsage.find('%'));
                info.partition_usage = (double)atoi(strUsage.c_str())/100;
            }
            __strncpy(info.partition_name,sizeof(info.partition_name),"disk");
            __strncpy(info.partition_type,sizeof(info.partition_type),"getdisktype");
            info.disk_total = 0;
            info.disk_used = 0;

            li.push_back(info);
            return 0;
        }
        else
            return -1;
    }

    if(true == is_host)
    {

        SNMPOID oid(".1.3.6.1.2.1.25.2.3.1");//hrStorageEntry
        SNMPVALUELIST lis;

        int ret = -1;
        ret = cnet->Walk_v3(oid,&lis);
        if(ret != SNMP_SUCCESS)
        {
            const char* errMsg = cnet->GetError();
            if ( !errMsg )
            {
                errMsg = "Unknown Error!";
                fprintf( stderr,"failed ret=%d : %s \n", ret,errMsg );
            }
            return -1;
        }

        int index = lis.Count();
        if(index == 0)
        {
            fprintf(stderr,"no storage record..\n");
            return -2;
        }

        for(int i=1 ; i <= index; i++)
        {
            char chIndex[5] = {0};
            sprintf(chIndex,"%d",i);
            LPSNMPVALUE t = lis.lpValue;
            char oid_disk_desc[32]  = ".1.3.6.1.2.1.25.2.3.1.3.";
            char oid_disk_type[32]  = ".1.3.6.1.2.1.25.2.3.1.2.";
            char oid_disk_used[32]  = ".1.3.6.1.2.1.25.2.3.1.6.";
            char oid_disk_total[32] = ".1.3.6.1.2.1.25.2.3.1.5.";

            strcat(oid_disk_desc, chIndex);
            strcat(oid_disk_type, chIndex);
            strcat(oid_disk_used, chIndex);
            strcat(oid_disk_total,chIndex);
            printf("oid_disk_used=%s,  oid_disk_total=%s \n  ",oid_disk_used,oid_disk_total);

            PartitionInfo info = {0};
            int nflag = 16;
            int disk_used  = 0;
            int disk_total = 0;
            while(NULL != t)
            {
                if(0==strcmp(t->snmpOID.szOID,oid_disk_desc))
                {
                    //printf("disk_desc value==> %s \n",t->szStrValue);
                    __strncpy(info.partition_name,sizeof(info.partition_name),t->szStrValue);
                    nflag >>= 1;
                }

                if(0==strcmp(t->snmpOID.szOID,oid_disk_type))
                {
                    //printf("disk_type value==> %d \n",t->szStrValue);
                    __strncpy(info.partition_type,sizeof(info.partition_type),t->GetValueToString().c_str());
                    nflag >>= 1;
                }

                if(0==strcmp(t->snmpOID.szOID,oid_disk_total))
                {
                    //printf("disk_total value==> %lld \n",t->nInterValue);
                    disk_total = t->nInterValue;
                    nflag >>= 1;
                }

                if(0==strcmp(t->snmpOID.szOID,oid_disk_used))
                {
                    //printf("disk_used value==> %lld \n",t->nInterValue);
                    disk_used = t->nInterValue;
                    nflag >>= 1;
                }
                if(0!=disk_used && 0!=disk_total)
                {
                    info.partition_usage = disk_used*1.0/disk_total;
                      info.disk_total = disk_total;
                      info.disk_used = disk_used;
                      disk_used = 0;
                      disk_total = 0;
                }

                if (nflag == 1) // 匹配成功
                {
                    li.push_back(info);
                    break;
                }
                t = t->next_value;
            }

        }
        lis.Free();
    }
	return 0;
}

int CSnmpV1::SNMP_GetDeviceList(DeviceList& li)
{
	//li.clear();
	SNMPOID oid(".1.3.6.1.2.1.25.3.2.1");//hrDeviceEntry
	SNMPVALUELIST lis;

	int ret = -1;
    ret = cnet->Walk_v3(oid,&lis);
	if(ret != 0)
	{
		const char* errMsg = cnet->GetError();
	    if ( !errMsg )
        {
		    errMsg = "Unknown Error!";
	        fprintf( stderr,"failed ret=%d : %s \n", ret,errMsg );
		}
		return -1;
	}

	int index = lis.Count();
	if(index == 0)
	{
		fprintf(stderr,"no storage record..\n");
		return -2;
	}


	for(int i=1 ; i <= index; i++)
	{
		char chIndex[5] = {0};
		sprintf(chIndex,"%d",i);
		LPSNMPVALUE t = lis.lpValue;

		char oid_device_desc[32]  = ".1.3.6.1.2.1.25.3.2.1.3.";
		char oid_device_type[32]  = ".1.3.6.1.2.1.25.3.2.1.2.";

		strcat(oid_device_desc, chIndex);
		strcat(oid_device_type, chIndex);

		int nflag = 4;
		DeviceInfo info = {0};
	    while(NULL != t)
		{
			if(0==strcmp(t->snmpOID.szOID,oid_device_desc))
			{
				//printf("device_name value==> %s \n",t->szStrValue);
				__strncpy(info.device_name,sizeof(info.device_name),t->szStrValue);
				nflag >>= 1;
			}

			if(0==strcmp(t->snmpOID.szOID,oid_device_type))
			{
				//printf("device_type value==> %d \n",t->szStrValue);
				__strncpy(info.device_type,sizeof(info.device_type),t->GetValueToString().c_str());
				nflag >>= 1;
			}

			if (nflag == 1) // 匹配成功
			{
				li.push_back(info);
				break;
			}

			t = t->next_value;
		}
	}
	lis.Free();
	return 0;
}
