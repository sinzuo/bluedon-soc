/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 * 
 * @file    xxx.h
 * @brief   xxx
 * 
 *
 * @version 1.0 
 * @author  xxx
 * @date    2011年09月13日
 *
 * 修订说明：最初版本
 **************************************************/

#include "common/bd_basic.h"
#include "bd_snmp.h"
#include "utils/bd_common.h"
//#include "bd_util.h"
//#include "bd_common.h"
//#include "bd_log_macros.h"
#include <string>
#ifdef WIN32
#pragma warning(disable:4819)
#endif

//net-snmp inlcude
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/utilities.h>
using namespace std;

//////////////////////////////////////////////////////////////////////////
_TAGSNMPOID::_TAGSNMPOID()
{
    memset(szOID, 0, MAX_OID_LEN);
    next_value = NULL;
}

_TAGSNMPOID::_TAGSNMPOID(const char *pszOID)
{
    memset(szOID, 0, MAX_OID_LEN);
    __strncpy(szOID, sizeof(szOID), pszOID);

    next_value = NULL;
}

_TAGSNMPOID & _TAGSNMPOID::operator = (const char *pszOID)
{
    memset(szOID, 0, MAX_OID_LEN);
    __strncpy(szOID, sizeof(szOID), pszOID);
    return *this;
}

_TAGSNMPOID & _TAGSNMPOID::operator = (const _TAGSNMPOID &objoid)
{
    memset(szOID, 0, MAX_OID_LEN);
    __strncpy(szOID, sizeof(szOID), objoid.szOID);
    return *this;
}

int _TAGSNMPOID::Equal(const _TAGSNMPOID &objoid)
{
    int nRet = 0;
    int nLen1 = strlen(szOID);
    int nLen2 = strlen(objoid.szOID);
    if(nLen1 == nLen2)
    {
        nRet = strcmp(szOID, objoid.szOID);
    }
    else if(nLen1 > nLen2)
    {
        nRet = 1;
    }
    else
    {
        nRet = -1;
    }

    return nRet;
}

int _TAGSNMPOID::GetLastNode()
{
    char szNode[12] = {0};
    int nLen = strlen(szOID);
    while(nLen >= 0)
    {
        if(szOID[nLen] == '.')
        {
            break;
        }
        nLen--;
    }

    __strncpy(szNode, sizeof(szNode), szOID + nLen + 1);
    return atoi(szNode);
}

void _TAGSNMPOID::CopyOID(const unsigned long *objoid, int nOidLen)
{
    char szBuf[16] = {0};
    memset(szOID, 0, MAX_OID_LEN);
    for(int i = 0; i < nOidLen; i++)
    {
        __snprintf(szBuf, sizeof(szBuf), ".%ld", objoid[i]);
        __strncat(szOID, sizeof(szOID), szBuf);
    }
}

void _TAGSNMPOID::Print()
{
    printf("OID: %s\n", szOID);
}

//////////////////////////////////////////////////////////////////////////
_TAGSNMPOIDLIST::_TAGSNMPOIDLIST()
{
    lpOID = NULL;
}

//向列表尾部添加值节点
LPSNMPOID _TAGSNMPOIDLIST::AddTail(const char *pOID)
{
    SNMPOID snmpOID = pOID;

    return AddTail(&snmpOID);
}

//向列表尾部添加值节点
LPSNMPOID _TAGSNMPOIDLIST::AddTail(LPSNMPOID value)
{
    LPSNMPOID lpTempOID, lpNewNode;
    if(lpOID == NULL)
    {
        lpOID = new SNMPOID;
        *lpOID = *value;
        lpNewNode = lpOID;
    }
    else
    {
        lpTempOID = lpOID;
        while(lpTempOID->next_value != NULL)
        {
            lpTempOID = lpTempOID->next_value;
        }

        lpNewNode = new SNMPOID;
        *lpNewNode = *value;
        if(lpNewNode != NULL)
        {
            lpTempOID->next_value = lpNewNode;
        }
    }

    return lpNewNode;
}

//打印
int _TAGSNMPOIDLIST::Print()
{
    LPSNMPOID lpTempOID = lpOID;
    while(lpTempOID != NULL)
    {
        lpTempOID->Print();
        lpTempOID = lpTempOID->next_value;
    }

    return 0;
}

//释放
int _TAGSNMPOIDLIST::Free()
{
    LPSNMPOID lpTempOID = NULL;;
    while(lpOID != NULL)
    {
        lpTempOID = lpOID;
        lpOID = lpOID->next_value;
        delete lpTempOID;
        lpTempOID = NULL;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
_TAGSNMPVALUE::_TAGSNMPVALUE()
{
    nValueType = SNMP_UNKOWN_TYPE;
    nInterValue = 0;
    memset(szStrValue, 0 , SNMP_STRING_VALUE_LENGTH);
    next_value = NULL;
}

//是否是数值
bool _TAGSNMPVALUE::IsDigital()
{
    return nValueType == SNMP_VALUE_INTEGER ? true : false;
}

//是否有效类型值
bool _TAGSNMPVALUE::IsValidValue()
{
    return nValueType != SNMP_UNKOWN_TYPE ? true : false;
}

//设置整形值
void _TAGSNMPVALUE::SetIntValue(const char *pszOID, int nValue)
{
    snmpOID = pszOID;
    nValueType = SNMP_VALUE_INTEGER;
    nInterValue = nValue;
}

//设置字符串值
void _TAGSNMPVALUE::SetStringValue(const char *pszOID, const char *pszValue)
{
    this->snmpOID = pszOID;
    this->nValueType = SNMP_VALUE_STR;
    __strncpy(this->szStrValue, SNMP_STRING_VALUE_LENGTH, pszValue);
}

//获取值并转换为字符串
std::string _TAGSNMPVALUE::GetValueToString()
{
    char value[SNMP_STRING_VALUE_LENGTH] = {0};
    switch(nValueType)
    {
    case SNMP_VALUE_STR:
        __snprintf(value, SNMP_STRING_VALUE_LENGTH, "%s", szStrValue);
        break;
    case SNMP_VALUE_INTEGER:
        __snprintf(value, SNMP_STRING_VALUE_LENGTH, "%lld", nInterValue);
        break;
    case SNMP_VALUE_IP_ADDRESS:
        __snprintf(value, SNMP_STRING_VALUE_LENGTH, "%d.%d.%d.%d", IP[0], IP[1], IP[2], IP[3]);
        break;
    case SNMP_VALUE_OID:
        __snprintf(value, SNMP_STRING_VALUE_LENGTH, "%s", snmpValueOID.szOID);
        break;
    case SNMP_VALUE_TIMETICKS:
        __snprintf(value, SNMP_STRING_VALUE_LENGTH, "%d %02d:%02d:%02d.%02d", Timeticks[0], Timeticks[1], Timeticks[2], Timeticks[3], Timeticks[4]);
        break;
    default:
        break;
    }

	if (strlen(value) == 0)
		return " ";
	return std::string(value);
}

//=重载
_TAGSNMPVALUE & _TAGSNMPVALUE::operator = (const _TAGSNMPVALUE &value)
{
    snmpOID = value.snmpOID;
    nValueType = value.nValueType;
    switch(value.nValueType)
    {
    case SNMP_VALUE_INTEGER:
        {
            nInterValue = value.nInterValue;
            break;
        }
    case SNMP_VALUE_STR:
        {
            __strncpy(this->szStrValue, SNMP_STRING_VALUE_LENGTH, value.szStrValue);
            break;
        }
    case SNMP_VALUE_IP_ADDRESS:
        {
            IP[0] = value.IP[0];
            IP[1] = value.IP[1];
            IP[2] = value.IP[2];
            IP[3] = value.IP[3];
            break;
        }
    case SNMP_VALUE_OID:
        {
            this->snmpValueOID = value.snmpValueOID;
            break;
        }
    case SNMP_VALUE_TIMETICKS:
        {
            for(int i = 0; i < 5; i++)
            {
                Timeticks[i] = value.Timeticks[i];
            }
            break;
        }
    default:
        break;
    }

    return *this;
}

//打印值
void _TAGSNMPVALUE::Print()
{
    switch(nValueType)
    {
    case SNMP_VALUE_STR:
        {
            printf("OID: %s\tSTRING:%s\n", snmpOID.szOID, szStrValue);
            break;
        }
    case SNMP_VALUE_INTEGER:
        {
            printf("OID: %s\tINTEGER:%lld\n", snmpOID.szOID, nInterValue);
            break;
        }
    case SNMP_VALUE_IP_ADDRESS:
        {
            printf("OID: %s\tIP ADDRESS:%d.%d.%d.%d\n", snmpOID.szOID, IP[0], IP[1], IP[2], IP[3]);
            break;
        }
    case SNMP_VALUE_OID:
        {
            printf("OID: %s\tOID:%s\n", snmpOID.szOID, snmpValueOID.szOID);
            break;
        }
    case SNMP_VALUE_TIMETICKS:
        {
            printf("OID: %s\tTIMETICKS:%d %02d:%02d:%02d.%02d\n", snmpOID.szOID, Timeticks[0], Timeticks[1], Timeticks[2], Timeticks[3], Timeticks[4]);
            break;
        }
    default:
        {
            printf("OID: %s\t未知的数据类型:%d\n", snmpOID.szOID, nValueType);
        }
    }
}

//保存结果
void _TAGSNMPVALUE::SaveResult(std::string &result)
{
	char tmpstr[2048]= {0};
	switch(nValueType)
	{
	case SNMP_VALUE_STR:
		{
			snprintf(tmpstr, sizeof(tmpstr), "OID: %s\tSTRING:%s\n", snmpOID.szOID, szStrValue);
			break;
		}
	case SNMP_VALUE_INTEGER:
		{
			snprintf(tmpstr, sizeof(tmpstr), "OID: %s\tINTEGER:%lld\n", snmpOID.szOID, nInterValue);
			break;
		}
	case SNMP_VALUE_IP_ADDRESS:
		{
			snprintf(tmpstr, sizeof(tmpstr), "OID: %s\tIP ADDRESS:%d.%d.%d.%d\n", snmpOID.szOID, IP[0], IP[1], IP[2], IP[3]);
			break;
		}
	case SNMP_VALUE_OID:
		{
			snprintf(tmpstr, sizeof(tmpstr), "OID: %s\tOID:%s\n", snmpOID.szOID, snmpValueOID.szOID);
			break;
		}
	case SNMP_VALUE_TIMETICKS:
		{
			snprintf(tmpstr, sizeof(tmpstr), "OID: %s\tTIMETICKS:%d %02d:%02d:%02d.%02d\n", snmpOID.szOID, Timeticks[0], Timeticks[1], Timeticks[2], Timeticks[3], Timeticks[4]);
			break;
		}
	default:
		{
			snprintf(tmpstr, sizeof(tmpstr), "OID: %s\t未知的数据类型:%d\n", snmpOID.szOID, nValueType);
		}
	}
	result = tmpstr;
}



//////////////////////////////////////////////////////////////////////////
_TAGSNMPVALUELIST::_TAGSNMPVALUELIST()
{
    lpValue = NULL;
    nCount = 0;
}

//向列表尾部添加值节点
LPSNMPVALUE _TAGSNMPVALUELIST::AddTail(LPSNMPVALUE value)
{
    LPSNMPVALUE lpTempValue, lpNewNode;
    if(lpValue == NULL)
    {
        lpValue = new SNMPVALUE;
        *lpValue = *value;
        lpNewNode = lpValue;
        nCount++;
    }
    else
    {
        lpTempValue = lpValue;
        while(lpTempValue->next_value != NULL)
        {
            lpTempValue = lpTempValue->next_value;
        }

        lpNewNode = new SNMPVALUE;
        *lpNewNode = *value;
        if(lpNewNode != NULL)
        {
            lpTempValue->next_value = lpNewNode;
            nCount++;
        }
    }

    return lpNewNode;
}

//打印
int _TAGSNMPVALUELIST::Print()
{
    LPSNMPVALUE lpTempValue = lpValue;
    while(lpTempValue != NULL)
    {
        lpTempValue->Print();
        lpTempValue = lpTempValue->next_value;
    }

    return 0;
}

int _TAGSNMPVALUELIST::Count()
{
    return nCount;
}

//释放
int _TAGSNMPVALUELIST::Free()
{
    LPSNMPVALUE lpTempValue;
    while(lpValue != NULL)
    {
        lpTempValue = lpValue;
        lpValue = lpValue->next_value;
        delete lpTempValue;
        lpTempValue = NULL;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
_TAGSNMPTABLEROW::_TAGSNMPTABLEROW()
{
    fields      = 0;
    column      = NULL;
    next_value  = NULL;
}

//按下标读取
SNMPTABLECOLUMN * _TAGSNMPTABLEROW::operator [] (int nCol)
{
    if(nCol > fields || nCol <= 0)
    {
        return NULL;
    }
    LPSNMPTABLECOLUMN lpTemp = column;
    for(int i = 1; i < nCol && lpTemp != NULL; i++)
    {
        if (lpTemp->next_value)
        {
			lpTemp = lpTemp->next_value;
        }
		else
			return NULL;
		
    }
    return lpTemp;
}

//添加列
LPSNMPTABLECOLUMN _TAGSNMPTABLEROW::AddColumn(SNMPVALUE value)
{
    LPSNMPTABLECOLUMN tempColumn = column, tempColumn2 = NULL;
    LPSNMPTABLECOLUMN newColumn = NULL;
    if(tempColumn == NULL)
    {
        column = new SNMPTABLECOLUMN;
        column->column_value = value;
        column->next_value = NULL;
        fields++;
        return column;
    }

    while(tempColumn)
    {
        int nRet = tempColumn->column_value.snmpOID.Equal(value.snmpOID);
        if(nRet == 0)
        {
            return tempColumn;
        }
        else if(nRet == 1)
        {
            break;
        }
        tempColumn2 = tempColumn;
        tempColumn = tempColumn->next_value;
    }

    newColumn = new SNMPTABLECOLUMN;
    newColumn->column_value= value;
    newColumn->next_value = NULL;

    if(tempColumn2 == NULL)
    {
        newColumn->next_value = tempColumn;
        column = newColumn;
    }
    else if(tempColumn == NULL)
    {
        tempColumn2->next_value = newColumn;
    }
    else
    {
        tempColumn2->next_value = newColumn;
        newColumn->next_value = tempColumn;
    }

    fields++;
    return newColumn;
}

//打印
void _TAGSNMPTABLEROW::Print()
{
    LPSNMPTABLECOLUMN tempColumn = column;
    while(tempColumn)
    {
        tempColumn->column_value.Print();
        tempColumn = tempColumn->next_value;
    }
}

//保存结果
void _TAGSNMPTABLEROW::SaveResult(std::string &result)
{
	LPSNMPTABLECOLUMN tempColumn = column;
	while(tempColumn)
	{
		std::string tmpresult;
		tempColumn->column_value.SaveResult(tmpresult);
		result = result + tmpresult + "\n";
		tempColumn = tempColumn->next_value;
	}
}

//释放
void _TAGSNMPTABLEROW::Free()
{
    LPSNMPTABLECOLUMN tempColumn = NULL;
    while(column)
    {
        tempColumn = column;
        column = column->next_value;
        delete tempColumn;
        tempColumn = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
_TAGSNMPTABLE::_TAGSNMPTABLE()
{
    count = 0;
    nRows = 0;
    row = NULL;
}

_TAGSNMPTABLE::_TAGSNMPTABLE(SNMPOID tboid)
{
    count = 0;
    nRows = 0;
    table_oid = tboid;
    row = NULL;
}

_TAGSNMPTABLE::~_TAGSNMPTABLE()
{
    Free();;
}

//按下标读取行
SNMPTABLEROW * _TAGSNMPTABLE::operator [] (int nRow)
{
    if(nRow > count || nRow <= 0)
    {
        return NULL;
    }
    LPSNMPTABLEROW lpTemp = row;
    for(int i = 1; i < nRow && lpTemp != NULL; i++)
    {
        lpTemp = lpTemp->next_value;
    }
    return lpTemp;
}

//根据行列坐标读取行
SNMPTABLEROW *_TAGSNMPTABLE::GetRowByCoordinate(int nRow, int nCol)
{
    //printf("&&&&&--- nCol:%d, nRow:%d\n", nCol, nRow);

    if(nRow <= 0 || nCol <= 0)
    {
        return NULL;
    }

    LPSNMPTABLEROW lpTemp = row;
    for(; lpTemp != NULL; lpTemp = lpTemp->next_value)
    {
        char tmpOid[50] = {0};
        GetTailOID(table_oid, lpTemp->rowoid, tmpOid);

        char *nextdot = strchr(tmpOid, '.');
        if(nextdot == NULL) return NULL;

        int len = nextdot - tmpOid;
        nextdot[0] = '\0';
        int nColnumber = atoi(tmpOid); //列号

        nextdot = strchr(tmpOid + len + 1, '.');
        int nRownumber = 0;  //行号
        if(nextdot != NULL)
        {
            nextdot[0] = '\0';
            nRownumber = atoi(tmpOid + len + 1);
        }
        else
            nRownumber = atoi(tmpOid + len + 1);

        //printf("&&&&&--- cmpOid:%s, tmpOid:%s\n", cmpOid, tmpOid);
        if(nColnumber == nCol && nRownumber == nRow)
        {
            break;
        }

    } //end for..

    return lpTemp;
}

int _TAGSNMPTABLE::GetTailOID(SNMPOID tableoid, SNMPOID srcoid, char *dstoid)
{
	size_t nLen = strlen(tableoid.szOID);

	int count = 0;
	size_t i = 0;
	while( i < strlen(srcoid.szOID) - nLen )
	{
		if(srcoid.szOID[nLen+i] == '.')
			count++;
		i++;
		if ( count == 2)
		{
			break;
		}
			
	}
    __strncpy(dstoid, 50, srcoid.szOID + nLen + i); //尾oid不是以"."开始的

//   printf("&&&&&&&&&&&&&&&&&&&& GetTailOID:%s\n",dstoid);
	return 0;

}

int CmpOID(char *oid1, char *oid2)
{
	int Node1[50] = {0};
	int Node2[50] = {0};
	int nLen1 = strlen(oid1);
	int nLen2 = strlen(oid2);
	char *tmpnode;
	int i = 0;
	int nodelen1 = 0;
	int begin = 0;
	while(i < nLen1)
	{
		if(oid1[i] == '.')
		{
			tmpnode = oid1 + begin;
			oid1[i] = '\n';
			Node1[nodelen1] = atoi(tmpnode);
			begin = i + 1;
			nodelen1++;
		}
		i++;
	}

	i = 0;
	int nodelen2 = 0;
	begin = 0;
	while(i < nLen2)
	{
		if(oid2[i] == '.')
		{
			tmpnode = oid2 + begin;
			oid2[i] = '\n';
			Node2[nodelen2] = atoi(tmpnode);
			begin = i + 1;
			nodelen2++;
		}
		i++;
	}

	i = 0;
	while(i < nodelen1 && i < nodelen2)
	{
		if (Node1[i] == Node2[i])
		{
			i++;
		}
		else if (Node1[i] > Node2[i])
		{
			return 1;
		}
		else
		{
			return -1;
		}
		return 0;
	}
	return 0;
}
//添加行
LPSNMPTABLEROW _TAGSNMPTABLE::AddRow(SNMPOID rowoid, SNMPOID tableoid)
{
    LPSNMPTABLEROW tempRow = row, tempRow2 = NULL;
    LPSNMPTABLEROW newRow = NULL;    

    char node2[50] = {0};
    GetTailOID(tableoid, rowoid, node2);
    //printf("&&&&&&&&&&&&&&&&&&&&node2_1:%s\n",node2);
    if(node2[0] == '1' && node2[1] == '.') nRows++;

    if(tempRow == NULL)
    {
        row = new SNMPTABLEROW;
        row->rowoid = rowoid;
        row->next_value = NULL;
        count++;
        return row;
    }

    while(tempRow)
    {
/*
        int node1 = tempRow->rowoid.GetLastNode();
        int node2 = rowoid.GetLastNode();
*/
		char node1[50] = {0};
		GetTailOID(tableoid,tempRow->rowoid,node1);
//		printf("&&&&&&&&&&&&&&&&&&&&node1:%s\n",node1);
        memset(node2, 0, sizeof(node2));
        GetTailOID(tableoid, rowoid, node2); //由于CmpOID会修改node2, 因此需要重新获取一次

        if( !strcmp(node1, node2))
        {
//			printf("&&&&&&&&&&&&&&&&&&&&return:%s\n",tempRow->rowoid.szOID);           
			return tempRow;
        }
        else if( CmpOID(node1, node2) > 0)
        {
            break;
        }
        tempRow2 = tempRow;
        tempRow = tempRow->next_value;
    }

    newRow = new SNMPTABLEROW;
    newRow->rowoid = rowoid;
    newRow->next_value = NULL;

    if(tempRow2 == NULL)
    {
        newRow->next_value = tempRow;
        row = newRow;
    }
    else if(tempRow == NULL)
    {
        tempRow2->next_value = newRow;
    }
    else
    {
        tempRow2->next_value = newRow;
        newRow->next_value = tempRow;
    }

    count++;
//	printf("&&&&&&&&&&&&&&&&&&&&return:%s\n",newRow->rowoid.szOID);  
    return newRow;
}

//添加值
int _TAGSNMPTABLE::AddValue(SNMPVALUE value, SNMPOID tableoid)
{
    SNMPOID rowoid = value.snmpOID;
    LPSNMPTABLEROW tempRow = row;
    tempRow = AddRow(rowoid, tableoid);
    tempRow->AddColumn(value);

    return 0;
}

//释放
void _TAGSNMPTABLE::Free()
{
    LPSNMPTABLEROW tempRow = NULL;
    while(row)
    {
        tempRow = row;
        row = row->next_value;
        tempRow->Free();
        delete tempRow;
        tempRow = NULL;
    }
}

//打印
void _TAGSNMPTABLE::Print()
{
    LPSNMPTABLEROW tempRow = row;
    int i = 1;
    while(tempRow)
    {
        //tempRow->rowoid.PrintOID();
        printf("第%d行\t共%d列\n", i++, tempRow->fields);
        tempRow->Print();
        printf("\n");
        tempRow = tempRow->next_value;
    }
}

//结果写文件
void _TAGSNMPTABLE::SaveResult(std::string &result)
{
	LPSNMPTABLEROW tempRow = row;
	int i = 1;
	char tmpstr[2048] = {0};

	while(tempRow)
	{
		//tempRow->rowoid.PrintOID();
		std::string tmpres;
		snprintf(tmpstr, sizeof(tmpstr), "第%d行\t共%d列\n", i++, tempRow->fields);
		tempRow->SaveResult(tmpres);
		result = result + tmpstr + tmpres + "\n";
		tempRow = tempRow->next_value;
	}
}

//////////////////////////////////////////////////////////////////////////

//无参构造函数
CNetSnmp::CNetSnmp()
{
    memset(m_szHost, 0, sizeof(m_szHost));
    memset(m_szCommunity, 0, sizeof(m_szCommunity));
    memset(m_szUser, 0, sizeof(m_szUser));
    memset(m_szProtol, 0, sizeof(m_szProtol));
    memset(m_szPassPhrase, 0, sizeof(m_szPassPhrase));
    memset(m_szErrorInfo, 0, sizeof(m_szErrorInfo));
    memset(m_privProtol, 0, sizeof(m_privProtol));
    memset(m_privPassPhrase, 0, sizeof(m_privPassPhrase));
    m_nTimeOut = 0;
    m_lVersion = 1;
    m_secuLevel = AUTH_NOPRIV;
}

//构造函数
CNetSnmp::CNetSnmp(const char *pszHost, const char *pszCommunity, const char *pszUser, const char *pszProtol, const char *pszPassPhrase, const long lVersion, const int nTimeOut)
{
    SetAttribute(pszHost, pszCommunity, pszUser, pszProtol, pszPassPhrase, lVersion, nTimeOut);
}

CNetSnmp::CNetSnmp(const char *pszHost, const char *pszCommunity, const long lVersion,const int seculevel, const char *pszUser, const char *pszProtol, const char *pszPassPhrase, const char *p_privProtol, const char *p_privPass, const int nTimeOut)
{
    SetAttribute_v3(pszHost, pszCommunity, lVersion, seculevel, pszUser, pszProtol, pszPassPhrase, p_privProtol, p_privPass, nTimeOut);
}
CNetSnmp::~CNetSnmp()
{
    memset(m_szHost, 0, sizeof(m_szHost));
    memset(m_szCommunity, 0, sizeof(m_szCommunity));
    memset(m_szUser, 0, sizeof(m_szUser));
    memset(m_szProtol, 0, sizeof(m_szProtol));
    memset(m_szPassPhrase, 0, sizeof(m_szPassPhrase));
    memset(m_szErrorInfo, 0, sizeof(m_szErrorInfo));
    memset(m_privProtol, 0, sizeof(m_privProtol));
    memset(m_privPassPhrase, 0, sizeof(m_privPassPhrase));
    m_nTimeOut = 0;
    m_lVersion = 1;
    m_secuLevel = AUTH_NOPRIV;
}

int CNetSnmp::Get(const SNMPOID snmpOID, LPSNMPVALUE lpValue)
{
    return GetValue(snmpOID, lpValue, SNMP_MSG_GET);
}

int CNetSnmp::GetNext(const SNMPOID snmpOID, LPSNMPVALUE lpValue)
{
    return GetValue(snmpOID, lpValue, SNMP_MSG_GETNEXT);
}

int CNetSnmp::Set(const SNMPOID snmpOID, SNMPVALUE value)
{
    char szBuf[1024] = {0};

    netsnmp_session session;
	void *pSession; //snmp会话
    netsnmp_pdu *pdu;                       //请求PDU
    netsnmp_pdu *response;                  //返回PDU
    oid anOID[MAX_OID_LEN];                 //OID
    size_t anOID_len;                       //OID length
    int status = 0;
    int nReturn = 0;

    //Initialize a "session" that defines who we're going to talk to
    snmp_sess_init(&session);
    //session.peername = __strdup(m_szHost);
    session.peername = m_szHost;
    session.version = m_lVersion;
    if(m_nTimeOut > 0)
    {
        session.timeout = long(m_nTimeOut * 1000000 / 6);
    }

    if(m_lVersion == SNMP_VERSION_1 || m_lVersion == SNMP_VERSION_2c)
    {
        session.community = (u_char *)m_szCommunity;
        session.community_len = strlen(m_szCommunity);
    }
    else if(m_lVersion == SNMP_VERSION_3)
    {
	session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
        session.peername = m_szHost;
		session.securityNameLen = strlen(session.securityName);
		session.securityAuthProto = usmHMACMD5AuthProtocol;
		session.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);
		session.securityAuthKeyLen = USM_AUTH_KU_LEN;
		if (generate_Ku(session.securityAuthProto,
			session.securityAuthProtoLen,
			(u_char *)m_szPassPhrase, strlen(m_szPassPhrase),
			session.securityAuthKey,
			&session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
				char pszerror[1024] = {0};
				snprintf(pszerror, sizeof(pszerror), \
					"Error generating a key (Ku) from the supplied \
					authentication pass phrase.user:%s, passwd:%s", m_szUser, m_szPassPhrase);
                SetError(pszerror);
				return SNMP_ERROR;
        }
    }
    else
    {
        SetError("未知的版本号.");
        return SNMP_ERROR;
    }

    init_snmp("bluedonsoc");

    //Open the session
    pSession = snmp_sess_open(&session);
    if(!pSession)
    {
        SetError("连接会话失败.");
        return SNMP_ERROR;
    }

    //解析OID
    anOID_len = MAX_OID_LEN;
    if (!snmp_parse_oid(snmpOID.szOID, anOID, &anOID_len))
    {
        SetError("解析OID失败.");
        return SNMP_ERROR;
    }

    pdu = snmp_pdu_create(SNMP_MSG_SET);
    read_objid(snmpOID.szOID, anOID, &anOID_len);
    char cType = ConvertValueType(value.nValueType);
    char *pszValue = (char *)value.GetValueToString().c_str();
    if(snmp_add_var(pdu, anOID, anOID_len, cType, pszValue))
    {
        __snprintf(szBuf, sizeof(szBuf), "添加值失败, 类型: %d 值: %s", value.nValueType, pszValue);
        SetError(szBuf);
        snmp_free_pdu(pdu);
        snmp_sess_close(pSession);
        return SNMP_ERROR;
    }

    nReturn = SNMP_SUCCESS;

    status = snmp_sess_synch_response(pSession, pdu, &response);
    if(status == STAT_SUCCESS)
    {
        if(response->errstat == SNMP_ERR_NOERROR)
        {
            nReturn = SNMP_SUCCESS;
        }
        else
        {
            if(response->errstat == SNMP_ERR_NOSUCHNAME)
            {
                SetError("End of MIB.");
                nReturn = SNMP_ERROR;
            }
            else
            {
                __snprintf(szBuf, sizeof(szBuf), "Error in packet. Reason: %s", snmp_errstring(response->errstat));
                SetError(szBuf);
                nReturn = SNMP_ERROR;
            }
        }
    }
    else if(status == STAT_TIMEOUT)
    {
        __snprintf(szBuf, sizeof(szBuf), "Timeout: No Response from: %s", session.peername);
        SetError(szBuf);
        nReturn = SNMP_TIMEOUT;
    }
    else
    {
        SetError("未知错误");
        nReturn = SNMP_ERROR;
    }

    //free the response.
    snmp_free_pdu(response);
    //close the session.
    snmp_sess_close(pSession);

    return nReturn;
}

int CNetSnmp::Set(const SNMPOID snmpOID, const char type, const char *value)
{
    char szBuf[1024] = {0};

    netsnmp_session session;
	void *pSession; //snmp会话
    netsnmp_pdu *pdu;                       //请求PDU
    netsnmp_pdu *response;                  //返回PDU
    oid anOID[MAX_OID_LEN];                 //OID
    size_t anOID_len;                       //OID length
    int status = 0;
    int nReturn = 0;

    //Initialize a "session" that defines who we're going to talk to
    snmp_sess_init(&session);
    session.peername = m_szHost;
    session.version = m_lVersion;
    if(m_nTimeOut > 0)
    {
        session.timeout = long(m_nTimeOut * 1000000 / 6);
    }

    if(m_lVersion == SNMP_VERSION_1 || m_lVersion == SNMP_VERSION_2c)
    {
        session.community = (u_char *)m_szCommunity;
        session.community_len = strlen(m_szCommunity);
    }
    else if(m_lVersion == SNMP_VERSION_3)
    {
	session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
		session.securityName = __strdup(m_szUser);
		session.securityNameLen = strlen(session.securityName);
		session.securityAuthProto = usmHMACMD5AuthProtocol;
		session.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);
		session.securityAuthKeyLen = USM_AUTH_KU_LEN;
		if (generate_Ku(session.securityAuthProto,
			session.securityAuthProtoLen,
			(u_char *)m_szPassPhrase, strlen(m_szPassPhrase),
			session.securityAuthKey,
			&session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
				char pszerror[1024] = {0};
				snprintf(pszerror, sizeof(pszerror), \
					"Error generating a key (Ku) from the supplied \
					authentication pass phrase.user:%s, passwd:%s", m_szUser, m_szPassPhrase);
				SetError(pszerror);
				return SNMP_ERROR;
        }
    }
    else
    {
        SetError("未知的版本号.");
        return SNMP_ERROR;
    }

    init_snmp("bluedonsoc");

    //Open the session
    pSession = snmp_sess_open(&session);
    if(!pSession)
    {
        SetError("连接会话失败.");
        return SNMP_ERROR;
    }

    //解析OID
    anOID_len = MAX_OID_LEN;
    if (!snmp_parse_oid(snmpOID.szOID, anOID, &anOID_len))
    {
        SetError("解析OID失败.");
        return SNMP_ERROR;
    }

    pdu = snmp_pdu_create(SNMP_MSG_SET);
    read_objid(snmpOID.szOID, anOID, &anOID_len);
    if(snmp_add_var(pdu, anOID, anOID_len, type, value))
    {
        __snprintf(szBuf, sizeof(szBuf), "添加值失败, 类型: %c 值: %s", type, value);
        SetError(szBuf);
        snmp_free_pdu(pdu);
        snmp_sess_close(pSession);
        return SNMP_ERROR;
    }

    nReturn = SNMP_SUCCESS;

    status = snmp_sess_synch_response(pSession, pdu, &response);
    if(status == STAT_SUCCESS)
    {
        if(response->errstat == SNMP_ERR_NOERROR)
        {
            nReturn = SNMP_SUCCESS;
        }
        else
        {
            if(response->errstat == SNMP_ERR_NOSUCHNAME)
            {
                SetError("End of MIB.");
                nReturn = SNMP_ERROR;
            }
            else
            {
                __snprintf(szBuf, sizeof(szBuf), "Error in packet. Reason: %s", snmp_errstring(response->errstat));
                SetError(szBuf);
                nReturn = SNMP_ERROR;
            }
        }
    }
    else if(status == STAT_TIMEOUT)
    {
        __snprintf(szBuf, sizeof(szBuf), "Timeout: No Response from: %s", session.peername);
        SetError(szBuf);
        nReturn = SNMP_TIMEOUT;
    }
    else
    {
        SetError("未知错误");
        nReturn = SNMP_ERROR;
    }

    //free the response.
    snmp_free_pdu(response);
    //close the session.
    snmp_sess_close(pSession);

    return nReturn;
}

int CNetSnmp::Walk(const SNMPOID snmpOID, LPSNMPVALUELIST lpValueList, int nMaxRep)
{
    if(lpValueList == NULL)
        return -1;

    char szBuf[1024] = {0};

    netsnmp_session session;
	void *pSession; 		//snmp会话
    netsnmp_pdu *pdu;                       //请求PDU
    netsnmp_pdu *response;                  //返回PDU
    oid anOID[MAX_OID_LEN];                 //OID
    size_t anOID_len;                       //OID length
    oid rootOID[MAX_OID_LEN];
    size_t rootLen;
    netsnmp_variable_list *vars;
    int status                  = 0;
    LPSNMPVALUE lpTempValue     = NULL;
    int nReturn                 = SNMP_SUCCESS;


    //Initialize a "session" that defines who we're going to talk to
    printf("----test----,neip:%s, community:%s, usernm:%s, passwd:%s, version:%ld\n",m_szHost, m_szCommunity, m_szUser, m_szPassPhrase, m_lVersion);
//    BD_LOGINF("----test----,neip:%s, community:%s, usernm:%s, passwd:%s, version:%d",m_szHost, m_szCommunity, m_szUser, m_szPassPhrase, m_lVersion);
	snmp_sess_init(&session);
    session.peername = m_szHost;
    session.version = m_lVersion;
    if(m_nTimeOut > 0)
    {
        session.timeout = long(m_nTimeOut * 1000000 / 6);
    }

    if(m_lVersion == SNMP_VERSION_1 || m_lVersion == SNMP_VERSION_2c)
    {
        session.community = (u_char *)m_szCommunity;
        session.community_len = strlen(m_szCommunity);
    }
    else if(m_lVersion == SNMP_VERSION_3)
    {
	session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
		session.securityName = __strdup(m_szUser);
		session.securityNameLen = strlen(session.securityName);
		session.securityAuthProto = usmHMACMD5AuthProtocol;
		session.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);
		session.securityAuthKeyLen = USM_AUTH_KU_LEN;
		if (generate_Ku(session.securityAuthProto,
			session.securityAuthProtoLen,
			(u_char *)m_szPassPhrase, strlen(m_szPassPhrase),
			session.securityAuthKey,
			&session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
				char pszerror[1024] = {0};
				snprintf(pszerror, sizeof(pszerror), \
					"Error generating a key (Ku) from the supplied \
					authentication pass phrase.user:%s, passwd:%s", m_szUser, m_szPassPhrase);
				SetError(pszerror);
				return SNMP_ERROR;
        }
    }
    else
    {
        //BD_LOGINF("---1---");
	SetError("未知的版本号.");
        return SNMP_ERROR;
    }

    init_snmp("bluedonsoc");

    //Open the session
    pSession = snmp_sess_open(&session);
    if(!pSession)
    {
	//BD_LOGINF("---2---");
        SetError("连接会话失败.");
        return SNMP_ERROR;
    }

    //解析OID
    rootLen = MAX_OID_LEN;
    if (!snmp_parse_oid(snmpOID.szOID, rootOID, &rootLen))
    {
	//BD_LOGINF("---3---");
        SetError("解析OID失败.");
        return SNMP_ERROR;
    }

    int nOver = 1;
    nReturn = SNMP_SUCCESS;
    memmove(anOID, rootOID, rootLen * sizeof(oid));
    anOID_len = rootLen;

    while(nOver)
    {
        pdu = snmp_pdu_create(SNMP_MSG_GETBULK);
        pdu->non_repeaters = 0;
        pdu->max_repetitions = nMaxRep;

        snmp_add_null_var(pdu, anOID, anOID_len);

        status = snmp_sess_synch_response(pSession, pdu, &response);
        if(status == STAT_SUCCESS)
        {
            if (response->errstat == SNMP_ERR_NOERROR)
            {
                for(vars = response->variables; vars; vars = vars->next_variable)
                {
                    if(vars->name_length < rootLen || memcmp(rootOID, vars->name, rootLen * sizeof(oid)) != 0)
                    {
                        // not this sub tree
                        nOver = 0;
                        continue;
                    }

                    SNMPVALUE value;
                    int nRet = AnalysisValue(vars, &value);
                    if(nRet == SNMP_SUCCESS)
                    {
                        lpTempValue = lpValueList->AddTail(&value);
                        if(lpTempValue == NULL)
                        {
                            SetError("向列表添加获取的值失败.");
                            nReturn = SNMP_ERROR;
                            break;
                        }
                        lpTempValue = NULL;
                    }
					printf("print lpTempValue\n");
					if (!lpTempValue)
					{
						printf("lpTempValue is NULL,oid:%s, ip:%s, community:%s\n",snmpOID.szOID,m_szHost,m_szCommunity);
					}
					else
						lpTempValue->Print();

                    //判断是否应该结束
                    if((vars->type != SNMP_ENDOFMIBVIEW) && (vars->type != SNMP_NOSUCHOBJECT) && (vars->type != SNMP_NOSUCHINSTANCE))
                    {
                        if(snmp_oid_compare(anOID, anOID_len, vars->name, vars->name_length) >= 0)
                        {
                            nOver = 0;
                        }

                        if(vars->next_variable == NULL)
                        {
                            memmove(anOID, vars->name, vars->name_length * sizeof(oid));
                            anOID_len = vars->name_length;
                        }
                    }
                    else
                    {
                        nOver = 0;
                    }
                }
            }
            else
            {
                nOver = 0;
                if(response->errstat == SNMP_ERR_NOSUCHNAME)
                {
                    SetError("End of MIB.");
                    nReturn = SNMP_ERROR;
                }
                else
                {
			//BD_LOGINF("---4---");
                    __snprintf(szBuf, sizeof(szBuf), "Error in packet. Reason: %s", snmp_errstring(response->errstat));
                    SetError(szBuf);
                    nReturn = SNMP_ERROR;
                }
            }
        }
        else if(status == STAT_TIMEOUT)
        {
		//BD_LOGINF("---5---");
            nOver = 0;
            __snprintf(szBuf, sizeof(szBuf), "Timeout: No Response from: %s", session.peername);
            SetError(szBuf);
            nReturn = SNMP_TIMEOUT;
        }
        else
        {
		//BD_LOGINF("---6---");
            nOver = 0;
            SetError("未知错误");
            nReturn = SNMP_ERROR;
        }
        //free the response.
        snmp_free_pdu(response);
    }

    //close the session.
    snmp_sess_close(pSession);

    return nReturn;
}

int CNetSnmp::GetTable(const SNMPOID snmpOID, LPSNMPTABLE lpTable, int nMaxRep)
{
    if(lpTable == NULL) lpTable = new SNMPTABLE;

    SNMPVALUELIST   list;
    LPSNMPVALUE     lpTempValue = NULL;

    int nRet = Walk(snmpOID, &list, nMaxRep);
    if(nRet == SNMP_SUCCESS)
    {
        printf("snmpwalk return success\n");
		printf("begin print walk list\n");
		list.Print();
		lpTempValue = list.lpValue;
        lpTable->table_oid = snmpOID;
        while(lpTempValue)
        {
            lpTable->AddValue(*lpTempValue,snmpOID);
            lpTempValue = lpTempValue->next_value;
        }
    }
	printf("snmpwalk return %d,errorinfo:%s oid:%s\n",nRet,m_szErrorInfo,snmpOID.szOID);
	list.Print();
    list.Free();

    return nRet;

}

int CNetSnmp::SetTable(LPSNMPVALUELIST lpValueList)
{
    if(lpValueList == NULL) return SNMP_SUCCESS;

    char szBuf[1024] = {0};

    netsnmp_session session;
	void *pSession; //snmp会话
    netsnmp_pdu *pdu;                       //请求PDU
    netsnmp_pdu *response;                  //返回PDU
    int status = 0;
    int nReturn = 0;

    //Initialize a "session" that defines who we're going to talk to
    snmp_sess_init(&session);
    session.peername = m_szHost;
    session.version = m_lVersion;
    if(m_nTimeOut > 0)
    {
        session.timeout = long(m_nTimeOut * 1000000 / 6);
    }

    if(m_lVersion == SNMP_VERSION_1 || m_lVersion == SNMP_VERSION_2c)
    {
        session.community = (u_char *)m_szCommunity;
        session.community_len = strlen(m_szCommunity);
    }
    else if(m_lVersion == SNMP_VERSION_3)
    {
	session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
		session.securityName = __strdup(m_szUser);
		session.securityNameLen = strlen(session.securityName);
		session.securityAuthProto = usmHMACMD5AuthProtocol;
		session.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);
		session.securityAuthKeyLen = USM_AUTH_KU_LEN;
		if (generate_Ku(session.securityAuthProto,
			session.securityAuthProtoLen,
			(u_char *)m_szPassPhrase, strlen(m_szPassPhrase),
			session.securityAuthKey,
			&session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
				char pszerror[1024] = {0};
				snprintf(pszerror, sizeof(pszerror), \
					"Error generating a key (Ku) from the supplied \
					authentication pass phrase.user:%s, passwd:%s", m_szUser, m_szPassPhrase);
				SetError(pszerror);
				return SNMP_ERROR;
        }
    }
    else
    {
        SetError("未知的版本号.");
        return SNMP_ERROR;
    }

    init_snmp("bluedonsoc");

    //Open the session
    pSession = snmp_sess_open(&session);
    if(!pSession)
    {
        SetError("连接会话失败.");
        return SNMP_ERROR;
    }

    pdu = snmp_pdu_create(SNMP_MSG_SET);

    //解析并添加值到PDU
    LPSNMPVALUE lpTempValue = lpValueList->lpValue;
    while(lpTempValue)
    {
        //解析OID
        oid tempOID[MAX_OID_LEN];                   //OID
        size_t oid_len = MAX_OID_LEN;               //OID length

        if (!snmp_parse_oid(lpTempValue->snmpOID.szOID, tempOID, &oid_len))
        {
            __snprintf(szBuf, sizeof(szBuf), "解析OID失败, OID: %s", lpTempValue->snmpOID.szOID);
            SetError(szBuf);
            snmp_free_pdu(pdu);
            snmp_sess_close(pSession);
            return SNMP_ERROR;
        }

        char cType = ConvertValueType(lpTempValue->nValueType);
        char *pszValue = (char *)lpTempValue->GetValueToString().c_str();
        if(snmp_add_var(pdu, tempOID, oid_len, cType, pszValue))
        {
            __snprintf(szBuf, sizeof(szBuf), "添加值失败, OID: %s, 类型: %d, 值: %s", lpTempValue->snmpOID.szOID, lpTempValue->nValueType, pszValue);
            SetError(szBuf);
            snmp_free_pdu(pdu);
            snmp_sess_close(pSession);
            return SNMP_ERROR;
        }

        lpTempValue = lpTempValue->next_value;
    }

    nReturn = SNMP_SUCCESS;

    status = snmp_sess_synch_response(pSession, pdu, &response);
    if(status == STAT_SUCCESS)
    {
        if(response->errstat == SNMP_ERR_NOERROR)
        {
            nReturn = SNMP_SUCCESS;
        }
        else
        {
            if(response->errstat == SNMP_ERR_NOSUCHNAME)
            {
                SetError("End of MIB.");
                nReturn = SNMP_ERROR;
            }
            else
            {
                __snprintf(szBuf, sizeof(szBuf), "Error in packet. Reason: %s", snmp_errstring(response->errstat));
                SetError(szBuf);
                nReturn = SNMP_ERROR;
            }
        }
    }
    else if(status == STAT_TIMEOUT)
    {
        __snprintf(szBuf, sizeof(szBuf), "Timeout: No Response from: %s", session.peername);
        SetError(szBuf);
        nReturn = SNMP_TIMEOUT;
    }
    else
    {
        SetError("未知错误.");
        nReturn = SNMP_ERROR;
    }

    //free the response.
    snmp_free_pdu(response);
    //close the session.
    snmp_sess_close(pSession);

    return nReturn;

}

int CNetSnmp::GetValue(const SNMPOID snmpOID, LPSNMPVALUE lpValue, int nCommand)
{
    if(lpValue == NULL) lpValue = new SNMPVALUE;

    char szBuf[1024] = {0};

    netsnmp_session session;
	void *pSession; //snmp会话
    netsnmp_pdu *pdu;                       //请求PDU
    netsnmp_pdu *response;                  //返回PDU
    oid anOID[MAX_OID_LEN];                 //OID
    size_t anOID_len;                       //OID length
    netsnmp_variable_list *vars;
    int status = 0;
    int nReturn = 0;

    //Initialize a "session" that defines who we're going to talk to
    snmp_sess_init(&session);
    session.peername = m_szHost;
    session.version = m_lVersion;
    if(m_nTimeOut > 0)
    {
        session.timeout = long(m_nTimeOut * 1000000 / 6);
    }

    if(m_lVersion == SNMP_VERSION_1 || m_lVersion == SNMP_VERSION_2c)
    {
        session.community = (u_char *)m_szCommunity;
        session.community_len = strlen(m_szCommunity);
    }
    else if(m_lVersion == SNMP_VERSION_3)
    {
            session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
            session.securityName = __strdup(m_szUser);
            session.securityNameLen = strlen(session.securityName);
            session.securityAuthProto = usmHMACMD5AuthProtocol;
            session.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);
            session.securityAuthKeyLen = USM_AUTH_KU_LEN;
            if (generate_Ku(session.securityAuthProto,
                session.securityAuthProtoLen,
                (u_char *)m_szPassPhrase, strlen(m_szPassPhrase),
                session.securityAuthKey,
                &session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
                    char pszerror[1024] = {0};
                    snprintf(pszerror, sizeof(pszerror), \
                        "Error generating a key (Ku) from the supplied \
                        authentication pass phrase.user:%s, passwd:%s", m_szUser, m_szPassPhrase);
                    SetError(pszerror);
                    return SNMP_ERROR;
            }


    }
    else
    {
        SetError("未知的版本号.");
        return SNMP_ERROR;
    }

    //SOCK_STARTUP;

    init_snmp("bluedonsoc");

    //Open the session
    pSession = snmp_sess_open(&session);
    if(!pSession)
    {
        SetError("连接会话失败.");
        //SOCK_CLEANUP;
        return SNMP_ERROR;
    }

    //解析OID
    anOID_len = MAX_OID_LEN;
    if (!snmp_parse_oid(snmpOID.szOID, anOID, &anOID_len))
    {
        SetError("解析OID失败.");
        //SOCK_CLEANUP;
        return SNMP_ERROR;
    }

    pdu = snmp_pdu_create(nCommand);
    read_objid(snmpOID.szOID, anOID, &anOID_len);
    snmp_add_null_var(pdu, anOID, anOID_len);

    nReturn = SNMP_SUCCESS;

    status = snmp_sess_synch_response(pSession, pdu, &response);
    if(status == STAT_SUCCESS)
    {
        if(response->errstat == SNMP_ERR_NOERROR)
        {
            for(vars = response->variables; vars; vars = vars->next_variable)
            {
                //判断是否应该结束
                if((vars->type == SNMP_ENDOFMIBVIEW) || (vars->type == SNMP_NOSUCHOBJECT) || (vars->type == SNMP_NOSUCHINSTANCE))
                {
                    __snprintf(szBuf, sizeof(szBuf), "不存在此OID: %s", snmpOID.szOID);
                    SetError(szBuf);
                    nReturn = SNMP_ERROR;
                    break;
                }

                int nRet = AnalysisValue(vars, lpValue);
                if(nRet == SNMP_ERROR)
                {
                    nReturn = SNMP_ERROR;
                    break;
                }
            }
        }
        else
        {
            if(response->errstat == SNMP_ERR_NOSUCHNAME)
            {
                SetError("End of MIB.");
                nReturn = SNMP_ERROR;
            }
            else
            {
                __snprintf(szBuf, sizeof(szBuf), "Error in packet. Reason: %s", snmp_errstring(response->errstat));
                SetError(szBuf);
                nReturn = SNMP_ERROR;
            }
        }
    }
    else if(status == STAT_TIMEOUT)
    {
        __snprintf(szBuf, sizeof(szBuf), "Timeout: No Response from: %s", session.peername);
        SetError(szBuf);
        nReturn = SNMP_TIMEOUT;
    }
    else
    {
        SetError("未知错误");
        nReturn = SNMP_ERROR;
    }

    //free the response.
    snmp_free_pdu(response);
    //close the session.
    snmp_sess_close(pSession);

    //SOCK_CLEANUP;

    return nReturn;;

}

int CNetSnmp::Gets(const LPSNMPOIDLIST lpOIDList, LPSNMPVALUELIST lpValueList)
{
    if(lpValueList == NULL) lpValueList = new SNMPVALUELIST;

    char szBuf[1024] = {0};
    LPSNMPVALUE lpTempValue = NULL;

    netsnmp_session session;
	void *pSession; //snmp会话
    netsnmp_pdu *pdu;                       //请求PDU
    netsnmp_pdu *response;                  //返回PDU
    netsnmp_variable_list *vars;
    int status = 0;
    int nReturn = 0;

    //Initialize a "session" that defines who we're going to talk to
    snmp_sess_init(&session);
    session.peername = m_szHost;
    session.version = m_lVersion;
    if(m_nTimeOut > 0)
    {
        session.timeout = long(m_nTimeOut * 1000000 / 6);
    }

    if(m_lVersion == SNMP_VERSION_1 || m_lVersion == SNMP_VERSION_2c)
    {
        session.community = (u_char *)m_szCommunity;
        session.community_len = strlen(m_szCommunity);
    }
    else if(m_lVersion == SNMP_VERSION_3)
    {
	session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
		session.securityName = __strdup(m_szUser);
		session.securityNameLen = strlen(session.securityName);
		session.securityAuthProto = usmHMACMD5AuthProtocol;
		session.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);
		session.securityAuthKeyLen = USM_AUTH_KU_LEN;
		if (generate_Ku(session.securityAuthProto,
			session.securityAuthProtoLen,
			(u_char *)m_szPassPhrase, strlen(m_szPassPhrase),
			session.securityAuthKey,
			&session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
				char pszerror[1024] = {0};
				snprintf(pszerror, sizeof(pszerror), \
					"Error generating a key (Ku) from the supplied \
					authentication pass phrase.user:%s, passwd:%s", m_szUser, m_szPassPhrase);
				SetError(pszerror);
				return SNMP_ERROR;
        }
    }
    else
    {
        SetError("未知的版本号.");
        return SNMP_ERROR;
    }

    init_snmp("bluedonsoc");

    //Open the session
    pSession = snmp_sess_open(&session);
    if(!pSession)
    {
        SetError("连接会话失败.");
        return SNMP_ERROR;
    }

    pdu = snmp_pdu_create(SNMP_MSG_GET);

    //解析并添加值到PDU
    LPSNMPOID lpTempOID = lpOIDList->lpOID;
    while(lpTempOID)
    {
        //解析OID
        oid tempOID[MAX_OID_LEN];                   //OID
        size_t oid_len = MAX_OID_LEN;               //OID length

        if (!snmp_parse_oid(lpTempOID->szOID, tempOID, &oid_len))
        {
            __snprintf(szBuf, sizeof(szBuf), "解析OID失败, OID: %s", lpTempOID->szOID);
            SetError(szBuf);
            snmp_free_pdu(pdu);
            snmp_sess_close(pSession);
            return SNMP_ERROR;
        }
        read_objid(lpTempOID->szOID, tempOID, &oid_len);
        snmp_add_null_var(pdu, tempOID, oid_len);

        lpTempOID = lpTempOID->next_value;
    }

    nReturn = SNMP_SUCCESS;

    status = snmp_sess_synch_response(pSession, pdu, &response);
    if(status == STAT_SUCCESS)
    {
        if(response->errstat == SNMP_ERR_NOERROR)
        {
            for(vars = response->variables; vars; vars = vars->next_variable)
            {
                //判断是否应该结束
                if((vars->type == SNMP_ENDOFMIBVIEW) || (vars->type == SNMP_NOSUCHOBJECT) || (vars->type == SNMP_NOSUCHINSTANCE))
                {
                    SNMPOID snmpOID;
                    snmpOID.CopyOID(vars->name, vars->name_length);
                    __snprintf(szBuf, sizeof(szBuf), "不存在此OID: %s", snmpOID.szOID);
                    SetError(szBuf);
                    nReturn = SNMP_ERROR;
                    break;
                }

                SNMPVALUE value;
                int nRet = AnalysisValue(vars, &value);
                if(nRet == SNMP_SUCCESS)
                {
                    lpTempValue = lpValueList->AddTail(&value);
                    if(lpTempValue == NULL)
                    {
                        SetError("向列表添加获取的值失败.");
                        nReturn = SNMP_ERROR;
                        break;
                    }
                    lpTempValue = NULL;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            if(response->errstat == SNMP_ERR_NOSUCHNAME)
            {
                SetError("End of MIB.");
                nReturn = SNMP_ERROR;
            }
            else
            {
                __snprintf(szBuf, sizeof(szBuf), "Error in packet. Reason: %s", snmp_errstring(response->errstat));
                SetError(szBuf);
                nReturn = SNMP_ERROR;
            }
        }
    }
    else if(status == STAT_TIMEOUT)
    {
        __snprintf(szBuf, sizeof(szBuf), "Timeout: No Response from: %s", session.peername);
        SetError(szBuf);
        nReturn = SNMP_TIMEOUT;
    }
    else
    {
        SetError("未知错误");
        nReturn = SNMP_ERROR;
    }

    //free the response.
    snmp_free_pdu(response);
    //close the session.
    snmp_sess_close(pSession);

    return nReturn;;

}

void CNetSnmp::SetAttribute(const char *pszHost, const char *pszCommunity, const char *pszUser, const char *pszProtol, const char *pszPassPhrase, const long lVersion, const int nTimeOut)
{
    memset(m_szHost, 0, sizeof(m_szHost));
    if(pszHost != NULL)
        __strncpy(m_szHost, sizeof(m_szHost), pszHost);

    memset(m_szCommunity, 0, sizeof(m_szCommunity));
    if(pszCommunity != NULL)
        __strncpy(m_szCommunity, sizeof(m_szCommunity), pszCommunity);

    memset(m_szUser, 0, sizeof(m_szUser));
    if(pszUser != NULL)
        __strncpy(m_szUser, sizeof(m_szUser), pszUser);

    memset(m_szProtol, 0, sizeof(m_szProtol));
    if(pszProtol != NULL)
        __strncpy(m_szProtol, sizeof(m_szProtol), pszProtol);

    memset(m_szPassPhrase, 0, sizeof(m_szPassPhrase));
    if(pszPassPhrase != NULL)
        __strncpy(m_szPassPhrase, sizeof(m_szPassPhrase), pszPassPhrase);

    memset(m_szErrorInfo, 0, sizeof(m_szErrorInfo));

    m_lVersion = lVersion;

    m_nTimeOut = nTimeOut;
}

void CNetSnmp::SetAttribute_v3(const char *pszHost, const char *pszCommunity, const long lVersion,const int seculevel, const char *pszUser, const char *pszProtol, const char *pszPassPhrase, const char *p_privProtol, const char *p_privPass, const int nTimeOut)
{
    memset(m_szHost, 0, sizeof(m_szHost));
    if(pszHost != NULL)
        __strncpy(m_szHost, sizeof(m_szHost), pszHost);

    memset(m_szCommunity, 0, sizeof(m_szCommunity));
    if(pszCommunity != NULL)
        __strncpy(m_szCommunity, sizeof(m_szCommunity), pszCommunity);

    memset(m_szUser, 0, sizeof(m_szUser));
    if(pszUser != NULL)
        __strncpy(m_szUser, sizeof(m_szUser), pszUser);

    memset(m_szProtol, 0, sizeof(m_szProtol));
    if(pszProtol != NULL)
        __strncpy(m_szProtol, sizeof(m_szProtol), pszProtol);

    memset(m_szPassPhrase, 0, sizeof(m_szPassPhrase));
    if(pszPassPhrase != NULL)
        __strncpy(m_szPassPhrase, sizeof(m_szPassPhrase), pszPassPhrase);

    memset(m_privProtol, 0, sizeof(m_privProtol));
    if(p_privProtol != NULL)
        __strncpy(m_privProtol, sizeof(m_privProtol), p_privProtol);

    memset(m_privPassPhrase, 0, sizeof(m_privPassPhrase));
    if(p_privPass != NULL)
        __strncpy(m_privPassPhrase, sizeof(m_privPassPhrase), p_privPass);

    memset(m_szErrorInfo, 0, sizeof(m_szErrorInfo));

    m_lVersion = lVersion;

    m_nTimeOut = nTimeOut;

    m_secuLevel = seculevel;


}
int CNetSnmp::AnalysisValue(netsnmp_variable_list *value, LPSNMPVALUE lpValue)
{
    int nReturn         = SNMP_SUCCESS;
    char szBuf[128]     = {0};

    if(value->type == ASN_OCTET_STR)
    {
/*
        //test
        struct tree    *subtree = get_tree_head();
        int nOldValueBoolean = netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT);
        int nOldValueInt = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT);
        netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT, true);
        netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT, NETSNMP_OID_OUTPUT_NONE);
        char         *buf = NULL;
        size_t          buf_len = 256, out_len = 0;
        buf = (char *) calloc(buf_len, 1);
        int             buf_overflow = 0;
        subtree = netsnmp_sprint_realloc_objid_tree((u_char **)&buf, &buf_len, &out_len, 0, &buf_overflow, value->name, value->name_length);
        sprint_realloc_octet_string2((u_char **)&buf, &buf_len, &out_len, 0, value, subtree->enums, subtree->hint, NULL);

        lpValue->snmpOID.CopyOID(value->name, value->name_length);
        lpValue->nValueType = SNMP_VALUE_STR;
        memcpy(lpValue->szStrValue, buf, out_len);
        //fprintf(stdout, "%s\n", buf);

        netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT, nOldValueBoolean);
        netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT, nOldValueInt);
		free(buf);
*/
		lpValue->snmpOID.CopyOID(value->name, value->name_length);
		lpValue->nValueType = SNMP_VALUE_STR;

		size_t buf_len = 1024, out_len = 0;
		char* buf = (char *) calloc(buf_len, 1);
		int allow_recalloc = 1;
		sprint_realloc_variable((u_char **)&buf, &buf_len, &out_len, allow_recalloc,
			value->name, value->name_length, value);
		std::string strtype, realvalue;
		bd_split(buf, "STRING:", &strtype, &realvalue);
		bd_trim(&realvalue);
        if( string(buf).find("Hex-STRING") != string::npos) {
            //lpValue->nValueType = SNMP_VALUE_STR;
            HexStringToString(realvalue);
            //fprintf(stdout, "buf:[%s][%d]\n", realvalue.c_str(),realvalue.length());
        }
		memcpy(lpValue->szStrValue, realvalue.c_str(), realvalue.size());
		lpValue->nValueType = SNMP_VALUE_STR;
		fprintf(stdout, "buf:[%s]\n", realvalue.c_str());
		free(buf);
    }
    else if(value->type == ASN_INTEGER || value->type == ASN_GAUGE)
    {
        lpValue->snmpOID.CopyOID(value->name, value->name_length);
        lpValue->nValueType = SNMP_VALUE_INTEGER;
        lpValue->nInterValue = *value->val.integer;
    }
    else if(value->type == ASN_IPADDRESS)
    {
        lpValue->snmpOID.CopyOID(value->name, value->name_length);
        lpValue->nValueType = SNMP_VALUE_IP_ADDRESS;
        lpValue->IP[0] = value->val.string[0];
        lpValue->IP[1] = value->val.string[1];
        lpValue->IP[2] = value->val.string[2];
        lpValue->IP[3] = value->val.string[3];
    }
    else if(value->type == ASN_OBJECT_ID)
    {
        lpValue->snmpOID.CopyOID(value->name, value->name_length);
        lpValue->nValueType = SNMP_VALUE_OID;
        lpValue->snmpValueOID.CopyOID((oid *)value->val.objid, value->val_len / sizeof(oid));
    }
    else if(value->type == ASN_TIMETICKS)
    {
        lpValue->snmpOID.CopyOID(value->name, value->name_length);
        lpValue->nValueType = SNMP_VALUE_TIMETICKS;
        int timeticks = *(u_long *) (value->val.integer);

        int centisecs, seconds, minutes, hours, days;
        centisecs = timeticks % 100;
        timeticks /= 100;
        days = timeticks / (60 * 60 * 24);
        timeticks %= (60 * 60 * 24);

        hours = timeticks / (60 * 60);
        timeticks %= (60 * 60);

        minutes = timeticks / 60;
        seconds = timeticks % 60;

        lpValue->Timeticks[0] = days;
        lpValue->Timeticks[1] = hours;
        lpValue->Timeticks[2] = minutes;
        lpValue->Timeticks[3] = seconds;
        lpValue->Timeticks[4] = centisecs;
    }
    else if(value->type == ASN_COUNTER)
    {
        lpValue->snmpOID.CopyOID(value->name, value->name_length);
        lpValue->nValueType = SNMP_VALUE_INTEGER;
        lpValue->nInterValue = *value->val.integer;
        //printf("count32\n");
    }
    else
    {
        lpValue->snmpOID.CopyOID(value->name, value->name_length);
        __snprintf(szBuf, sizeof(szBuf), "未知的数据类型:%d", value->type);
        SetError(szBuf);
//		BD_LOGERR("Unknow the value type of OID: [%s]", lpValue->snmpOID.szOID);
        nReturn = SNMP_ERROR;
    }

    return nReturn;
}

char CNetSnmp::ConvertValueType(int nType)
{
    char cType = '0';
    switch(nType)
    {
    case SNMP_VALUE_STR:
        cType = 's';
        break;
    case SNMP_VALUE_INTEGER:
        cType = 'i';
        break;
    case SNMP_VALUE_IP_ADDRESS:
        cType = 'a';
        break;
    default:
        break;
    }

    return cType;
}

int CNetSnmp::sprint_realloc_octet_string2(u_char ** buf, size_t * buf_len, size_t * out_len, int allow_realloc, const netsnmp_variable_list * var, const struct enum_list *enums, const char *hint, const char *units)
{

    size_t          saved_out_len = *out_len;
    const char     *saved_hint = hint;
    int             hex = 0, x = 0;
    u_char         *cp;
    int             output_format, len_needed;

    if ((var->type != ASN_OCTET_STR) && 
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
            const char      str[] = "Wrong Type (should be OCTET STRING): ";
            if (snmp_cstrcat
                (buf, buf_len, out_len, allow_realloc, str)) {
                    return sprint_realloc_by_type(buf, buf_len, out_len,
                        allow_realloc, var, NULL, NULL,
                        NULL);
            } else {
                return 0;
            }
    }


    if (hint) {
        int             repeat, width = 1;
        long            value;
        char            code = 'd', separ = 0, term = 0, ch, intbuf[16];
        u_char         *ecp;

        if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
            if (!snmp_cstrcat(buf, buf_len, out_len, allow_realloc, "STRING: ")) {
                return 0;
            }
        }
        cp = var->val.string;
        ecp = cp + var->val_len;

        while (cp < ecp) {
            repeat = 1;
            if (*hint) {
                if (*hint == '*') {
                    repeat = *cp++;
                    hint++;
                }
                width = 0;
                while ('0' <= *hint && *hint <= '9')
                    width = (width * 10) + (*hint++ - '0');
                code = *hint++;
                if ((ch = *hint) && ch != '*' && (ch < '0' || ch > '9')
                    && (width != 0
                    || (ch != 'x' && ch != 'd' && ch != 'o')))
                    separ = *hint++;
                else
                    separ = 0;
                if ((ch = *hint) && ch != '*' && (ch < '0' || ch > '9')
                    && (width != 0
                    || (ch != 'x' && ch != 'd' && ch != 'o')))
                    term = *hint++;
                else
                    term = 0;
                if (width == 0)  // Handle malformed hint strings 
                    width = 1;
            }

            while (repeat && cp < ecp) {
                value = 0;
                if (code != 'a' && code != 't') {
                    for (x = 0; x < width; x++) {
                        value = value * 256 + *cp++;
                    }
                }
                switch (code) {
                case 'x':
                    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                        NETSNMP_DS_LIB_2DIGIT_HEX_OUTPUT)
                        && value < 16) {
                            __snprintf(intbuf, sizeof(intbuf), "%02lX", value);
                    } else {
                        __snprintf(intbuf, sizeof(intbuf), "%02lX", value);
                    }
                    if (!snmp_cstrcat
                        (buf, buf_len, out_len, allow_realloc, intbuf)) {
                            return 0;
                    }
                    break;
                case 'd':
                    __snprintf(intbuf, sizeof(intbuf), "%ld", value);
                    if (!snmp_cstrcat
                        (buf, buf_len, out_len, allow_realloc, intbuf)) {
                            return 0;
                    }
                    break;
                case 'o':
                    __snprintf(intbuf, sizeof(intbuf), "%lo", value);
                    if (!snmp_cstrcat
                        (buf, buf_len, out_len, allow_realloc, intbuf)) {
                            return 0;
                    }
                    break;
                case 't': // new in rfc 3411 
                case 'a':
                    // A string hint gives the max size - we may not need this much 
                    len_needed = SNMP_MIN( width, ecp-cp );
                    while ((*out_len + len_needed + 1) >= *buf_len) {
                        if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                            return 0;
                        }
                    }
                    for (x = 0; x < width && cp < ecp; x++) {
                        *(*buf + *out_len) = *cp++;
                        (*out_len)++;
                    }
                    *(*buf + *out_len) = '\0';
                    break;
                default:
                    *out_len = saved_out_len;
                    if (snmp_cstrcat(buf, buf_len, out_len, allow_realloc,
                        "(Bad hint ignored: ")
                        && snmp_cstrcat(buf, buf_len, out_len,
                        allow_realloc, saved_hint)
                        && snmp_cstrcat(buf, buf_len, out_len,
                        allow_realloc, ") ")) {
                            return sprint_realloc_octet_string2(buf, buf_len,
                                out_len,
                                allow_realloc,
                                var, enums,
                                NULL, NULL);
                    } else {
                        return 0;
                    }
                }

                if (cp < ecp && separ) {
                    while ((*out_len + 1) >= *buf_len) {
                        if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                            return 0;
                        }
                    }
                    *(*buf + *out_len) = separ;
                    (*out_len)++;
                    *(*buf + *out_len) = '\0';
                }
                repeat--;
            }

            if (term && cp < ecp) {
                while ((*out_len + 1) >= *buf_len) {
                    if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                        return 0;
                    }
                }
                *(*buf + *out_len) = term;
                (*out_len)++;
                *(*buf + *out_len) = '\0';
            }
        }

        if (units) {
            return (snmp_cstrcat
                (buf, buf_len, out_len, allow_realloc, " ")
                && snmp_cstrcat(buf, buf_len, out_len, allow_realloc, units));
        }
        if ((*out_len >= *buf_len) &&
            !(allow_realloc && snmp_realloc(buf, buf_len))) {
                return 0;
        }
        *(*buf + *out_len) = '\0';

        return 1;
    }

    output_format = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_STRING_OUTPUT_FORMAT);
    if (0 == output_format) {
        output_format = NETSNMP_STRING_OUTPUT_GUESS;
    }
    switch (output_format) {
    case NETSNMP_STRING_OUTPUT_GUESS:
        hex = 0;
        for (cp = var->val.string, x = 0; x < (int) var->val_len; x++, cp++) {
            if (!isprint(*cp) && !isspace(*cp)) {
                hex = 1;
            }
        }
        break;

    case NETSNMP_STRING_OUTPUT_ASCII:
        hex = 0;
        break;

    case NETSNMP_STRING_OUTPUT_HEX:
        hex = 1;
        break;
    }

    if (var->val_len == 0) {
        return snmp_cstrcat(buf, buf_len, out_len, allow_realloc, "\"\"");
    }

    if (hex) {
        if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
            if (!snmp_cstrcat(buf, buf_len, out_len, allow_realloc, "\"")) {
                return 0;
            }
        } else {
            if (!snmp_cstrcat
                (buf, buf_len, out_len, allow_realloc, "Hex-STRING: ")) {
                    return 0;
            }
        }

        //if (!sprint_realloc_hexstring(buf, buf_len, out_len, allow_realloc,
        //                              var->val.string, var->val_len)) {
        //    return 0;
        //}

        if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
            if (!snmp_cstrcat(buf, buf_len, out_len, allow_realloc, "\"")) {
                return 0;
            }
        }
    } else {
        if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
            if (!snmp_cstrcat(buf, buf_len, out_len, allow_realloc,
                "STRING: ")) {
                    return 0;
            }
        }
        if (!snmp_cstrcat
            (buf, buf_len, out_len, allow_realloc, "\"")) {
                return 0;
        }
        if (!sprint_realloc_asciistring
            (buf, buf_len, out_len, allow_realloc, var->val.string,
            var->val_len)) {
                return 0;
        }
        if (!snmp_cstrcat(buf, buf_len, out_len, allow_realloc, "\"")) {
            return 0;
        }
    }

    if (units) {
        return (snmp_cstrcat(buf, buf_len, out_len, allow_realloc, " ")
            && snmp_cstrcat(buf, buf_len, out_len, allow_realloc, units));
    }
    return 1;
}

//设置错误信息
int CNetSnmp::SetError(const char *pszError)
{
    memset(m_szErrorInfo, 0, sizeof(m_szErrorInfo));

    if(pszError != NULL)
        __strncpy(m_szErrorInfo, sizeof(m_szErrorInfo), pszError);

    return SNMP_SUCCESS;
}


//获取错误信息
const char *CNetSnmp::GetError()
{
    return m_szErrorInfo;
}

int CNetSnmp::SendTrap(const SNMPOID snmpOID, const char type, const char *value)
{
    //alltype = "=iutaosxdbIUFD";
    //if('i' != type && 'u' != type && 't' != type && 'a' != type && 'o' != type && 's' != type && 
    //	'x' != type && 'd' != type && 'b' != type && 'I' != type && 'U' != type && 'F' != type && 'D' != type)
    //	return -2;

    char szBuf[1024] = {0};
    netsnmp_session session, *ss;
    netsnmp_pdu *pdu;
    int status = 0;
    oid Oid[MAX_OID_LEN];                 //OID
    size_t Oid_Len;                       //OID length

    snmp_sess_init(&session);
    session.version = m_lVersion;
    session.peername = m_szHost;
    session.remote_port = 162;
    session.community = (u_char *)m_szCommunity;
    session.community_len = strlen((char *)session.community);
    session.retries = 3;
    session.timeout = m_nTimeOut;
    session.sessid = 0;

    SOCK_STARTUP;
    ss = snmp_add(&session, netsnmp_transport_open_client("snmptrap", session.peername), NULL, NULL);
    if (NULL == ss)
    {
        SetError("添加目标地址失败.");
        //snmp_sess_perror("snmptrap", &session);
        SOCK_CLEANUP;
        return SNMP_ERROR;
    }

    if(SNMP_VERSION_1 == m_lVersion)
        pdu = snmp_pdu_create(SNMP_MSG_TRAP);
    else
        pdu = snmp_pdu_create(SNMP_MSG_TRAP2);

    //解析OID
    Oid_Len = MAX_OID_LEN;
    if (!snmp_parse_oid(snmpOID.szOID, Oid, &Oid_Len))
    {
        SetError("解析OID失败.");
        snmp_free_pdu(pdu);
        snmp_sess_close(ss);
        SOCK_CLEANUP;
        return SNMP_ERROR;
    }

    read_objid(snmpOID.szOID, Oid, &Oid_Len);

    status = snmp_add_var(pdu, Oid, Oid_Len, type, value);
    if(0 != status)
    {
        __snprintf(szBuf, sizeof(szBuf), "添加值失败, 类型: %c 值: %s", type, value);
        SetError(szBuf);
        //snmp_sess_perror("snmptrap add value error!", &session);
        snmp_free_pdu(pdu);
        snmp_sess_close(ss);
        snmp_shutdown("snmptrap");
        SOCK_CLEANUP;
        return SNMP_ERROR;
    }

    status = snmp_sess_send(ss, pdu);
    if (0 == status)
    {
        SetError("发送失败.");
        //snmp_sess_perror("snmptrap send info error!", &session);
        snmp_free_pdu(pdu);
        snmp_sess_close(ss);
        snmp_shutdown("snmptrap");
        SOCK_CLEANUP;
        return SNMP_ERROR;
    }

    snmp_sess_close(ss);
    snmp_shutdown("snmptrap");
    SOCK_CLEANUP;

    return 0;
}

int CNetSnmp::GetValue_v3(const SNMPOID snmpOID, LPSNMPVALUE lpValue, int nCommand)
{
    if(lpValue == NULL) lpValue = new SNMPVALUE;

    char szBuf[1024] = {0};

    netsnmp_session session;
    void *pSession; //snmp会话
    netsnmp_pdu *pdu;                       //请求PDU
    netsnmp_pdu *response;                  //返回PDU
    oid anOID[MAX_OID_LEN];                 //OID
    size_t anOID_len;                       //OID length
    netsnmp_variable_list *vars;
    int status = 0;
    int nReturn = 0;

    //Initialize a "session" that defines who we're going to talk to
    snmp_sess_init(&session);
    session.peername = m_szHost;
    session.version = m_lVersion;
    if(m_nTimeOut > 0)
    {
        session.timeout = long(m_nTimeOut * 1000000 / 6);
    }

    if(m_lVersion == SNMP_VERSION_1 || m_lVersion == SNMP_VERSION_2c)
    {
        session.community = (u_char *)m_szCommunity;
        session.community_len = strlen(m_szCommunity);
    }
    else if(m_lVersion == SNMP_VERSION_3)
    {


        switch(m_secuLevel)
        {
            case NOAUTH_NOPRIV:
            {
                session.securityLevel = SNMP_SEC_LEVEL_NOAUTH;
                break;
            }
            case AUTH_NOPRIV:
            {
                session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
                break;
            }
            case AUTH_PRIV:
            {
                session.securityLevel = SNMP_SEC_LEVEL_AUTHPRIV;
                break;
            }
            default:
                session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
        }
        //session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
        session.securityName = __strdup(m_szUser);
        session.securityNameLen = strlen(session.securityName);

        if(strcmp(m_szProtol,"sha") == 0 || strcmp(m_szProtol,"SHA") == 0)
        {
            session.securityAuthProto = usmHMACSHA1AuthProtocol;
            session.securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;

        }
        else
        {
            session.securityAuthProto = usmHMACMD5AuthProtocol;
            session.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);
        }

        session.securityAuthKeyLen = USM_AUTH_KU_LEN;
        if (generate_Ku(session.securityAuthProto,
            session.securityAuthProtoLen,
            (u_char *)m_szPassPhrase, strlen(m_szPassPhrase),
            session.securityAuthKey,
            &session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
                char pszerror[1024] = {0};
                snprintf(pszerror, sizeof(pszerror), \
                    "Error generating a key (Ku) from the supplied \
                    authentication pass phrase.user:%s, passwd:%s", m_szUser, m_szPassPhrase);
                SetError(pszerror);
                return SNMP_ERROR;
        }


        if(strcmp(m_privProtol,"aes") == 0 || strcmp(m_privProtol,"AES") == 0)
        {
//            session.securityPrivProto = usmAES128PrivProtocol;
            session.securityPrivProtoLen = USM_PRIV_PROTO_AES128_LEN;
        }
        else
        {
//            session.securityPrivProto = usmDESPrivProtocol;
            session.securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
        }


        session.securityPrivKeyLen = USM_PRIV_KU_LEN;

        if (generate_Ku(session.securityAuthProto,
                session.securityAuthProtoLen,
                (u_char *)m_privPassPhrase, strlen(m_privPassPhrase),
                session.securityPrivKey,
                &session.securityPrivKeyLen) != SNMPERR_SUCCESS) {
                char pszerror[] = "Error generating a key (Ku) from the supplied privacy pass phrase. \n";
                SetError(pszerror);

                return (-2);
        }

    }
    else
    {
        SetError("未知的版本号.");
        return SNMP_ERROR;
    }

    //SOCK_STARTUP;

    init_snmp("bluedonsoc");

    //Open the session
    pSession = snmp_sess_open(&session);
    if(!pSession)
    {
        SetError("connetc session failed.");
        //SOCK_CLEANUP;
        return SNMP_ERROR;
    }

    //解析OID
    anOID_len = MAX_OID_LEN;
    if (!snmp_parse_oid(snmpOID.szOID, anOID, &anOID_len))
    {
        SetError("analyze OID failed.");
        //SOCK_CLEANUP;
        return SNMP_ERROR;
    }

    pdu = snmp_pdu_create(nCommand);
    read_objid(snmpOID.szOID, anOID, &anOID_len);
    snmp_add_null_var(pdu, anOID, anOID_len);

    nReturn = SNMP_SUCCESS;

    status = snmp_sess_synch_response(pSession, pdu, &response);
    if(status == STAT_SUCCESS)
    {
        if(response->errstat == SNMP_ERR_NOERROR)
        {
            for(vars = response->variables; vars; vars = vars->next_variable)
            {
                //判断是否应该结束
                if((vars->type == SNMP_ENDOFMIBVIEW) || (vars->type == SNMP_NOSUCHOBJECT) || (vars->type == SNMP_NOSUCHINSTANCE))
                {
                    __snprintf(szBuf, sizeof(szBuf), "not exit OID: %s", snmpOID.szOID);
                    SetError(szBuf);
                    nReturn = SNMP_ERROR;
                    break;
                }

                int nRet = AnalysisValue(vars, lpValue);
                if(nRet == SNMP_ERROR)
                {
                    nReturn = SNMP_ERROR;
                    break;
                }
            }
        }
        else
        {
            if(response->errstat == SNMP_ERR_NOSUCHNAME)
            {
                SetError("End of MIB.");
                nReturn = SNMP_ERROR;
            }
            else
            {
                __snprintf(szBuf, sizeof(szBuf), "Error in packet. Reason: %s", snmp_errstring(response->errstat));
                SetError(szBuf);
                nReturn = SNMP_ERROR;
            }
        }
    }
    else if(status == STAT_TIMEOUT)
    {
        __snprintf(szBuf, sizeof(szBuf), "Timeout: No Response from: %s", session.peername);
        SetError(szBuf);
        nReturn = SNMP_TIMEOUT;
    }
    else
    {
        SetError("unknown error.");
        nReturn = SNMP_ERROR;
    }

    //free the response.
    snmp_free_pdu(response);
    //close the session.
    snmp_sess_close(pSession);

    //SOCK_CLEANUP;

    return nReturn;;

}

int CNetSnmp::Get_v3(const SNMPOID snmpOID, LPSNMPVALUE lpValue)
{
    return GetValue_v3(snmpOID, lpValue, SNMP_MSG_GET);
}


int CNetSnmp::Walk_v3(const SNMPOID snmpOID, LPSNMPVALUELIST lpValueList, int nMaxRep)
{
    if(lpValueList == NULL)
        return -1;

    char szBuf[1024] = {0};

    netsnmp_session session;
    void *pSession; 		//snmp会话
    netsnmp_pdu *pdu;                       //请求PDU
    netsnmp_pdu *response;                  //返回PDU
    oid anOID[MAX_OID_LEN];                 //OID
    size_t anOID_len;                       //OID length
    oid rootOID[MAX_OID_LEN];
    size_t rootLen;
    netsnmp_variable_list *vars;
    int status                  = 0;
    LPSNMPVALUE lpTempValue     = NULL;
    int nReturn                 = SNMP_SUCCESS;

    printf("----test----,neip:%s, community:%s, usernm:%s, passwd:%s, version:%ld\n",m_szHost, m_szCommunity, m_szUser, m_szPassPhrase, m_lVersion);
    snmp_sess_init(&session);
    session.peername = m_szHost;
    session.version = m_lVersion;
    if(m_nTimeOut > 0)
    {
        session.timeout = long(m_nTimeOut * 1000000 / 6);
    }

    if(m_lVersion == SNMP_VERSION_1 || m_lVersion == SNMP_VERSION_2c)
    {
        session.community = (u_char *)m_szCommunity;
        session.community_len = strlen(m_szCommunity);
    }
    else if(m_lVersion == SNMP_VERSION_3)
    {

        switch(m_secuLevel)
        {
            case NOAUTH_NOPRIV:
            {
                session.securityLevel = SNMP_SEC_LEVEL_NOAUTH;
                break;
            }
            case AUTH_NOPRIV:
            {
                session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
                break;
            }
            case AUTH_PRIV:
            {
                session.securityLevel = SNMP_SEC_LEVEL_AUTHPRIV;
                break;
            }
            default:
                session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
        }

        //session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
        session.securityName = __strdup(m_szUser);
        session.securityNameLen = strlen(session.securityName);
        if(strcmp(m_szProtol, "sha") == 0 || strcmp(m_szProtol, "SHA") == 0)
        {
 //           session.securityAuthProto = usmHMACSHA1AuthProtocol;
            session.securityAuthProtoLen = sizeof(usmHMACSHA1AuthProtocol)/sizeof(oid);
        }
        else
        {
//            session.securityAuthProto = usmHMACMD5AuthProtocol;
            session.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);

        }

        session.securityAuthKeyLen = USM_AUTH_KU_LEN;
        if (generate_Ku(session.securityAuthProto,
            session.securityAuthProtoLen,
            (u_char *)m_szPassPhrase, strlen(m_szPassPhrase),
            session.securityAuthKey,
            &session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
                char pszerror[1024] = {0};
                snprintf(pszerror, sizeof(pszerror), \
                    "Error generating a key (Ku) from the supplied \
                    authentication pass phrase.user:%s, passwd:%s", m_szUser, m_szPassPhrase);
                SetError(pszerror);
                return SNMP_ERROR;
        }

        if(strcmp(m_privProtol, "aes") == 0 || strcmp(m_privProtol, "AES") == 0)
        {
//            session.securityPrivProto = usmAES128PrivProtocol;
            session.securityPrivProtoLen = USM_PRIV_PROTO_AES128_LEN;
        }
        else
        {
//            session.securityPrivProto = usmDESPrivProtocol;
            session.securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
        }


        session.securityPrivKeyLen = USM_PRIV_KU_LEN;

        if (generate_Ku(session.securityAuthProto,
                session.securityAuthProtoLen,
                (u_char *)m_privPassPhrase, strlen(m_privPassPhrase),
                session.securityPrivKey,
                &session.securityPrivKeyLen) != SNMPERR_SUCCESS) {
                char pszerror[] = "Error generating a key (Ku) from the supplied privacy pass phrase. \n";
                SetError(pszerror);

                return (-2);
        }


    }
    else
    {
        //BD_LOGINF("---1---");
        SetError("未知的版本号.");
        return SNMP_ERROR;
    }

    init_snmp("bluedonsoc");

    //Open the session
    pSession = snmp_sess_open(&session);
    if(!pSession)
    {
    //BD_LOGINF("---2---");
        SetError("连接会话失败.");
        return SNMP_ERROR;
    }

    //解析OID
    rootLen = MAX_OID_LEN;
    if (!snmp_parse_oid(snmpOID.szOID, rootOID, &rootLen))
    {
    //BD_LOGINF("---3---");
        SetError("解析OID失败.");
        return SNMP_ERROR;
    }

    int nOver = 1;
    nReturn = SNMP_SUCCESS;
    memmove(anOID, rootOID, rootLen * sizeof(oid));
    anOID_len = rootLen;

    while(nOver)
    {
        pdu = snmp_pdu_create(SNMP_MSG_GETBULK);
        pdu->non_repeaters = 0;
        pdu->max_repetitions = nMaxRep;

        snmp_add_null_var(pdu, anOID, anOID_len);

        status = snmp_sess_synch_response(pSession, pdu, &response);
        if(status == STAT_SUCCESS)
        {
            if (response->errstat == SNMP_ERR_NOERROR)
            {
                for(vars = response->variables; vars; vars = vars->next_variable)
                {
                    if(vars->name_length < rootLen || memcmp(rootOID, vars->name, rootLen * sizeof(oid)) != 0)
                    {
                        // not this sub tree
                        nOver = 0;
                        continue;
                    }

                    SNMPVALUE value;
                    int nRet = AnalysisValue(vars, &value);
                    if(nRet == SNMP_SUCCESS)
                    {
                        lpTempValue = lpValueList->AddTail(&value);
                        if(lpTempValue == NULL)
                        {
                            SetError("向列表添加获取的值失败.");
                            nReturn = SNMP_ERROR;
                            break;
                        }
                        lpTempValue = NULL;
                    }
                    //printf("print lpTempValue\n");
                    if (!lpTempValue)
                    {
                        printf("lpTempValue is NULL,oid:%s, ip:%s, community:%s\n",snmpOID.szOID,m_szHost,m_szCommunity);
                    }
                    else
                        lpTempValue->Print();

                    //判断是否应该结束
                    if((vars->type != SNMP_ENDOFMIBVIEW) && (vars->type != SNMP_NOSUCHOBJECT) && (vars->type != SNMP_NOSUCHINSTANCE))
                    {
                        if(snmp_oid_compare(anOID, anOID_len, vars->name, vars->name_length) >= 0)
                        {
                            nOver = 0;
                        }

                        if(vars->next_variable == NULL)
                        {
                            memmove(anOID, vars->name, vars->name_length * sizeof(oid));
                            anOID_len = vars->name_length;
                        }
                    }
                    else
                    {
                        nOver = 0;
                    }
                }
            }
            else
            {
                nOver = 0;
                if(response->errstat == SNMP_ERR_NOSUCHNAME)
                {
                    SetError("End of MIB.");
                    nReturn = SNMP_ERROR;
                }
                else
                {
                    __snprintf(szBuf, sizeof(szBuf), "Error in packet. Reason: %s", snmp_errstring(response->errstat));
                    SetError(szBuf);
                    nReturn = SNMP_ERROR;
                }
            }
        }
        else if(status == STAT_TIMEOUT)
        {
            nOver = 0;
            __snprintf(szBuf, sizeof(szBuf), "Timeout: No Response from: %s", session.peername);
            SetError(szBuf);
            nReturn = SNMP_TIMEOUT;
        }
        else
        {
            nOver = 0;
            SetError("未知错误");
            nReturn = SNMP_ERROR;
        }
        //free the response.
        snmp_free_pdu(response);
    }

    //close the session.
    snmp_sess_close(pSession);

    return nReturn;
}

int CNetSnmp::GetTable_v3(const SNMPOID snmpOID, LPSNMPTABLE lpTable, int nMaxRep)
{
    if(lpTable == NULL) lpTable = new SNMPTABLE;

    SNMPVALUELIST   list;
    LPSNMPVALUE     lpTempValue = NULL;

    int nRet = Walk_v3(snmpOID, &list, nMaxRep);
    if(nRet == SNMP_SUCCESS)
    {
        printf("snmpwalk return success\n");
        printf("begin print walk list\n");
        list.Print();
        lpTempValue = list.lpValue;
        lpTable->table_oid = snmpOID;
        while(lpTempValue)
        {
            lpTable->AddValue(*lpTempValue,snmpOID);
            lpTempValue = lpTempValue->next_value;
        }
    }
    printf("snmpwalk return %d,errorinfo:%s oid:%s\n",nRet,m_szErrorInfo,snmpOID.szOID);
    list.Print();
    list.Free();

    return nRet;

}

