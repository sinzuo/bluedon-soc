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

#ifndef _BD_SNMP_LIB_H
#define _BD_SNMP_LIB_H

#ifdef WIN32
#pragma warning(disable:4996)
#ifdef BD_SNMP_EXPORTS
#define BD_SNMPEXE_EXPORTS __declspec(dllexport)
#else
#define BD_SNMPEXE_EXPORTS __declspec(dllimport)
#endif // BD_SNMP_EXPORTS
#else
#define BD_SNMPEXE_EXPORTS
#endif // WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <ctype.h>
#include "common/bd_basic.h"

typedef struct variable_list netsnmp_variable_list;

#define SNMP_STRING_VALUE_LENGTH              1024

#define SNMP_SUCCESS                        0        //成功
#define SNMP_ERROR                         -1        //失败
#define SNMP_TIMEOUT					   -2		 //超时

enum NETSNMPVERSION
{
	NETSNMP_VERSION_1	   = 0,	// 1
	NETSNMP_VERSION_2c     = 1,	// 2c
	NETSNMP_VERSION_2u     = 2,	// 2u
	NETSNMP_VERSION_3      = 3	// 3
};

enum SNMPVALUETYPE
{
	SNMP_UNKOWN_TYPE                  = 0,        //未知的数据类型
	SNMP_VALUE_STR                    = 1,        //字符串
	SNMP_VALUE_INTEGER                = 2,        //整数
	//SNMP_VALUE_HEX_STR                = 3,        //16进制字符串
	SNMP_VALUE_IP_ADDRESS             = 4,        //IP地址
	SNMP_VALUE_OID                    = 5,        //OID
	SNMP_VALUE_TIMETICKS              = 6         //TIMETICK
};

enum SECURITYLEVEL
{
    NOAUTH_NOPRIV	       = 1,	// 不认证不加密
    AUTH_NOPRIV            = 2,	// 认证不加密
    AUTH_PRIV              = 3	// 认证加密
};

enum SWITCH_TYPE
{
    SWITCH_TYPE_ZERO            = 0,            //交换机类型为0
    SWITCH_TYPE_H3C             = 1,            //交换机类型为H3C
    SWITCH_TYPE_CISCO           = 2,            //交换机类型为思科
    SWITCH_TYPE_ALCATEL         = 3,            //交换机类型为阿尔卡特
    SWITCH_TYPE_HUAWEI          = 4,              //交换机类型为华为
	SWITCH_TYPE_QIMING          = 5,              //交换机类型为启明
    SWITCH_TYPE_HUAWEI_2011     = 6,            //交换机类型为华为
    SWITCH_TYPE_HUAWEI_2011_AC  = 7,            //交换机类型为华为的AC类型，如AC6605, AC6005, AC6003, ACU2,
                                                //AR100,AR120,AR150,AR160,AR200,AR1200,AR2200,AR3200,AR3600
    SWITCH_TPYE_H3C_25506       = 8,             //交换类型为H3C
    SWITCH_TPYE_H3C_2011        = 9,             //交换类型为H3C
    SWITCH_TYPE_NSG_32328       = 10,              //设备类型为360_NSG
    SWITCH_TYPE_ICG_37157       = 11,           //设备类型为网康ICG
    SWITCH_TYPE_SANGFOR         = 12,            //设备类型为深信服服务器
    SWITCH_TYPE_TOPSEC_NGFW     = 13,            //设备类型为天融信下一代防火墙
    SWITCH_TYPE_YUNNAN_CENTOS   = 14,            //设备类型为云南保山香料烟项目的centos数据服务器
    SWITCH_TYPE_SECAV           = 15             //设备类型为网神secAV3600防毒墙
};

#ifndef __cplusplus
extern "C"{
#endif

	//OID字符串结构体
	typedef struct BD_SNMPEXE_EXPORTS _TAGSNMPOID
	{
		char    		szOID[256];
		_TAGSNMPOID   	*next_value;

		_TAGSNMPOID();
		_TAGSNMPOID(const char *pszOID);

		_TAGSNMPOID & operator = (const char *pszOID);

		_TAGSNMPOID & operator = (const _TAGSNMPOID &objoid);

		//比较大小 -1 小于 0 相等 1 大于
		int Equal(const _TAGSNMPOID &objoid);

		//获取最后一级OID
		int GetLastNode();

		int GetLastWord(int len, char *lastword, int wordsize);

		//判断OID是否为空
		bool IsEmpty()
		{
			if(strlen(szOID) == 0)
			{
				return true;
			}

			return false;
		}

		//复制SNMPOID 值
		void CopyOID(const unsigned long *objoid, int nOidLen);

		//打印OID
		void Print();
	}SNMPOID, *LPSNMPOID;

	//OID列表
	typedef struct BD_SNMPEXE_EXPORTS _TAGSNMPOIDLIST
	{
		LPSNMPOID     lpOID;

		_TAGSNMPOIDLIST();

		//向列表尾部添加值节点
		LPSNMPOID AddTail(const char *pOID);

		//向列表尾部添加值节点
		LPSNMPOID AddTail(LPSNMPOID value);

		//打印
		int Print();

		//释放
		int Free();
	}SNMPOIDLIST, *LPSNMPOIDLIST;

	//SNMP值信息结构体
	typedef struct BD_SNMPEXE_EXPORTS _TAGSNMPVALUE
	{
		int             nValueType;
		long long       nInterValue;
		char            szStrValue[SNMP_STRING_VALUE_LENGTH];
		int             IP[4];
		int             Timeticks[5];
		SNMPOID         snmpValueOID;

		SNMPOID         snmpOID;
		_TAGSNMPVALUE   *next_value;

		_TAGSNMPVALUE();

		//是否是数值
		bool IsDigital();

		//是否有效类型值
		bool IsValidValue();

		//设置整形值
		void SetIntValue(const char *pszOID, int nValue);

		//设置字符串值
		void SetStringValue(const char *pszOID, const char *pszValue);

		//获取值并转换为字符串
		std::string GetValueToString();

		//=重载
		_TAGSNMPVALUE & operator = (const _TAGSNMPVALUE &value);

		//打印值
		void Print();

		//保存结果
		void SaveResult(std::string &result);

	}SNMPVALUE, *LPSNMPVALUE;

	//SNMP值列表
	typedef struct BD_SNMPEXE_EXPORTS _TAGSNMPVALUELIST
	{
		_TAGSNMPVALUELIST();

		//向列表尾部添加值节点
		LPSNMPVALUE AddTail(LPSNMPVALUE value);

		//打印
		int Print();

		int Count();
		//释放
		int Free();
		LPSNMPVALUE     lpValue;
	private:
		int nCount;

	}SNMPVALUELIST, *LPSNMPVALUELIST;

	//SNMP表列结构体
	typedef struct BD_SNMPEXE_EXPORTS _TAGSNMPTABLECOLUMN
	{
		SNMPVALUE               column_value;
		_TAGSNMPTABLECOLUMN     *next_value;

		_TAGSNMPTABLECOLUMN()
		{
			next_value = NULL;
		}

		void Print()
		{
			column_value.Print();
		}
	}SNMPTABLECOLUMN, *LPSNMPTABLECOLUMN;

	//SNMP表行结构体
	typedef struct BD_SNMPEXE_EXPORTS _TAGSNMPTABLEROW
	{
		int fields;
		SNMPOID rowoid;
		LPSNMPTABLECOLUMN   column;
		_TAGSNMPTABLEROW    *next_value;

		_TAGSNMPTABLEROW();

		//按下标读取
		SNMPTABLECOLUMN * operator [] (int nCol);

		//添加列
		LPSNMPTABLECOLUMN AddColumn(SNMPVALUE value);

		//打印
		void Print();
		//释放
		void Free();

		//保存结果
		void SaveResult(std::string &result);

	}SNMPTABLEROW, *LPSNMPTABLEROW;

	//SNMP表结构体
	typedef struct BD_SNMPEXE_EXPORTS _TAGSNMPTABLE
	{
        int count;  //数据总条数
        int nRows;  //表的行数, 并非行的最大oid索引值
		SNMPOID table_oid;
		LPSNMPTABLEROW row;

		_TAGSNMPTABLE();
		_TAGSNMPTABLE(SNMPOID tboid);

		~_TAGSNMPTABLE();

		//按下标读取行
		SNMPTABLEROW * operator [] (int nRow);

        //根据行列坐标读取行
        SNMPTABLEROW * GetRowByCoordinate(int nRow, int nCol);

		//添加行
		LPSNMPTABLEROW AddRow(SNMPOID rowoid, SNMPOID tableoid);

		//添加值
		int AddValue(SNMPVALUE value,SNMPOID snmpOID);

        //获取尾Oid
        int GetTailOID(SNMPOID tableoid, SNMPOID srcoid, char *dstoid);

		//释放
		void Free();

		//打印
		void Print();

		//保存结果
		void SaveResult(std::string &result);

	}SNMPTABLE, *LPSNMPTABLE;

	//CNetSNmp类
	class BD_SNMPEXE_EXPORTS CNetSnmp
	{
	public:
		//构造函数
		CNetSnmp();

		CNetSnmp(const char *pszHost, const char *pszCommunity, const char *pszUser, const char *pszProtol, const char *pszPassPhrase, const long lVersion, const int nTimeOut = 0);

        CNetSnmp(const char *pszHost, const char *pszCommunity, const long lVersion,const int seculevel, const char *pszUser, const char *pszProtol, const char *pszPassPhrase, const char *p_privProtol, const char *p_privPass, const int nTimeOut = 0);
		//析构函数
		~CNetSnmp();

		//设置属性
		//lVersion: SNMP_VERSION_1 SNMP_VERSION_2c  SNMP_VERSION_3
		//SNMP_VERSION_1 SNMP_VERSION_2c : pszHost  pszCommunity
		//SNMP_VERSION_3 : pszHost pszUser pszProtol  pszPassPhrase
		void SetAttribute(const char *pszHost, const char *pszCommunity, const char *pszUser, const char *pszProtol, const char *pszPassPhrase, const long lVersion, const int nTimeOut = 0);
        void SetAttribute_v3(const char *pszHost, const char *pszCommunity, const long lVersion,const int seculevel, const char *pszUser, const char *pszProtol, const char *pszPassPhrase, const char *p_privProtol, const char *p_privPass, const int nTimeOut = 0);
		//获取
		int Get(const SNMPOID snmpOID, LPSNMPVALUE lpValue);

		int Gets(const LPSNMPOIDLIST lpOIDList, LPSNMPVALUELIST lpValueList);

		//获取下一个
		int GetNext(const SNMPOID snmpOID, LPSNMPVALUE lpValue);

		//设置指定OID的值
		int Set(const SNMPOID snmpOID, SNMPVALUE value);

		//设置指定OID的值
		int Set(const SNMPOID snmpOID, const char type, const char *value);

		//遍历指定OID下的所有值
		int Walk(const SNMPOID snmpOID, LPSNMPVALUELIST lpValueList, int nMaxRep = 50);

		//获取表
		int GetTable(const SNMPOID snmpOID, LPSNMPTABLE lpTable, int nMaxRep = 50);

		//设置表值或者一批OID值
		int SetTable(LPSNMPVALUELIST lpValueList);

		//将netsnmp_variable_list 解析为LPSNMPVALUE类型
		int AnalysisValue(netsnmp_variable_list *value, LPSNMPVALUE lpValue);

		//值类型转换
		char ConvertValueType(int nType);

		//输出字符串
		int sprint_realloc_octet_string2(unsigned char ** buf, size_t * buf_len, size_t * out_len, int allow_realloc, const netsnmp_variable_list * var, const struct enum_list *enums, const char *hint, const char *units);

		//发送trap消息
		int SendTrap(const SNMPOID snmpOID, const char type, const char* trapvalue);

		//设置错误信息
		int SetError(const char *pszError);

		//获取错误信息
		const char *GetError();


        int Get_v3(const SNMPOID snmpOID, LPSNMPVALUE lpValue);
		int Walk_v3(const SNMPOID snmpOID, LPSNMPVALUELIST lpValueList, int nMaxRep = 50);
		int GetTable_v3(const SNMPOID snmpOID, LPSNMPTABLE lpTable, int nMaxRep = 50);

	private:
		//command:SNMP_MSG_GET SNMP_MSG_GETNEXT
        int GetValue_v3(const SNMPOID snmpOID, LPSNMPVALUE lpValue, int nCommand);
        int GetValue(const SNMPOID snmpOID, LPSNMPVALUE lpValue, int nCommand);


	private:
		char    m_szHost[128];                //主机IP地址
		char    m_szCommunity[32];          //共同体名
		char    m_szUser[32];                //v3用户名
		char    m_szProtol[32];              //v3协议
		char    m_szPassPhrase[32];         //v3密码
		char    m_szErrorInfo[1024];        //错误信息
		int     m_nTimeOut;                 //会话超时澹:秒
		long    m_lVersion;                 //版本
        int     m_secuLevel;          //安全等级
        char    m_privProtol[32];              //加密协议
        char    m_privPassPhrase[32];         //加密密码


	};

#ifndef __cplusplus
}
#endif  // extern "C"
#endif      // _BD_SNMP_LIB_H

