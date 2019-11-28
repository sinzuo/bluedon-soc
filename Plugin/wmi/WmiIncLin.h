/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    WmiIncLin.h
 * @brief   SOC, Linux
 *
 *
 * @version 1.0
 * @author  蓝盾开发二部
 * @date    2011年4月
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _WMIINCLIN_H
#define _WMIINCLIN_H

#include "WmiInc.h"
#include "iconv.h"
//#include "utils/bd_util.h"
#include "ace/OS.h" 

#include <stdio.h>
#include <errno.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <list>
#include <iostream>

using namespace std;

#define MAX_LINE_LEN 4096

typedef list<long> Objlist;

#pragma warning( disable: 4819 ) 
#pragma warning( disable: 4996 ) 

#include "WmiInc.h"

///////////////////////////////////////////////////////////////
//	类    名 : CWMI
//	功    能 : WMI操作实现类
//	作    者 : 
//	时    间 : 2011年4月
//	备    注 : 
///////////////////////////////////////////////////////////////
class CWMI
{
public:
	CWMI()
	{
		m_desIp = "";		
		m_userName = "";
		m_password = "";
	}

	~CWMI() 
	{
	}
    
///////////////////////////////////////////////////////////////
//	函 数 名 : WmiInit
//	函数功能 : 初始化Wmi操作
//	处理过程 :
//	备    注 : 创建完CWMI对象后，需要调用该函数进行初始化工作
//	作    者 : 
//	时    间 : 2011年4月
//	返 回 值 : 成功时返回0，失败时返回1
//	参数说明 : 
///////////////////////////////////////////////////////////////
	int WmiInit(string desIP="", string userName="", string userPsword="")
	{
		m_desIp = desIP;
		m_userName = userName;
		m_password = userPsword;
	
		return 0;   // successfully completed.
	}

///////////////////////////////////////////////////////////////
//	函 数 名 : WmiOperatingSystem
//	函数功能 : 获取操作系统信息
//	处理过程 :
//	备    注 : 以下方法与本方法使用方式类似
//	作    者 : 
//	时    间 : 2011年4月
//	返 回 值 : 成功时返回0，失败时返回1
//	参数说明 : CWmiOperatingSystem操作系统信息对象
///////////////////////////////////////////////////////////////
	int WmiOperatingSystem(CWmiOperatingSystem& os, string &strSrcRes)
	{
		 
		FILE *fp = NULL;
		
		if(1 == WmiExecQuery("Win32_OperatingSystem",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					if(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						string src = line;
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_OperatingSystem pclose pipe error\n");
							}

							return 1;   
						}
						strSrcRes = src;
						string newline = replace_all(src,"!!","!(null)!");
						char buf[62][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
							         %[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									 %[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									 %[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									 %[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									 %[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									 %[^!]!%[^!]",
									 buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],
									 buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],
									 buf[20],buf[21],buf[22],buf[23],buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],
									 buf[30],buf[31],buf[32],buf[33],buf[34],buf[35],buf[36],buf[37],buf[38],buf[39],
									 buf[40],buf[41],buf[42],buf[43],buf[44],buf[45],buf[46],buf[47],buf[48],buf[49],
									 buf[50],buf[51],buf[52],buf[53],buf[54],buf[55],buf[56],buf[57],buf[58],buf[59],
									 buf[60],buf[61]);
						
						os.BootDevice = buf[0];
						os.BuildNumber = buf[1];
						os.BuildType = buf[2];
						os.Caption = buf[3];
						os.CodeSet = buf[4];
						os.CountryCode = buf[5];
						os.CreationClassName = buf[6];
						os.CSCreationClassName = buf[7];
						os.CSDVersion = buf[8];
						os.CSName = buf[9];
						os.CurrentTimeZone = buf[10];
						os.DataExecutionPrevention_32BitApplications = buf[11];
						os.DataExecutionPrevention_Available = buf[12];
						os.DataExecutionPrevention_Drivers = buf[13];
						os.DataExecutionPrevention_SupportPolicy = buf[14];
						os.Debug = buf[15];
						os.Description = buf[16];
						os.Distributed = buf[17];
						os.EncryptionLevel = buf[18];
						os.ForegroundApplicationBoost = buf[19];
						os.FreePhysicalMemory = buf[20];
						os.FreeSpaceInPagingFiles = buf[21];
						os.FreeVirtualMemory = buf[22];
						os.InstallDate = buf[23];
						os.LargeSystemCache = buf[24];
						os.LastBootUpTime = buf[25];
						os.LocalDateTime = buf[26];
						os.Locale = buf[27];
						os.Manufacturer = buf[28];
						os.MaxNumberOfProcesses = buf[29];
						os.MaxProcessMemorySize = buf[30];
						os.Name = buf[31];
						os.NumberOfLicensedUsers = buf[32];
						os.NumberOfProcesses = buf[33];
						os.NumberOfUsers = buf[34];
						os.Organization = buf[35];
						os.OSLanguage = buf[36];
						os.OSProductSuite = buf[37];
						os.OSType = buf[38];
						os.OtherTypeDescription = buf[39];
						os.PAEEnabled = buf[40];
						os.PlusProductID = buf[41];
						os.PlusVersionNumber = buf[42];
						os.Primary = buf[43];
						os.ProductType = buf[44];
						os.QuantumLength = buf[45];
						os.QuantumType = buf[46];
						os.RegisteredUser = buf[47];
						os.SerialNumber = buf[48];
						os.ServicePackMajorVersion = atoi(buf[49]);
						os.ServicePackMinorVersion = atoi(buf[50]);
						os.SizeStoredInPagingFiles = buf[51];
						os.Status = buf[52];
						os.SuiteMask = buf[53];
						os.SystemDevice = buf[54];
						os.SystemDirectory = buf[55];
						os.SystemDrive = buf[56];
						os.TotalSwapSpaceSize = buf[57];
						os.TotalVirtualMemorySize = buf[58];
						os.TotalVisibleMemorySize = buf[59];
						os.Version = buf[60];
						os.WindowsDirectory = buf[61];

					}

					if(line)
					{
						free(line);
					}

					if(-1 ==  pclose(fp))
					{
						printf("Win32_OperatingSystem pclose pipe error\n");
					}
					return 0;
				}
			}
			
		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("Win32_OperatingSystem pclose pipe error\n");
		}

		return 1;   
	}	

	int WmiProcessor(CWmiProcessor& po, string &strSrcRes)
	{
		
		FILE *fp = NULL;

		if(1 == WmiExecQuery("Win32_Processor",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					if(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						string src = line;
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_Processor pclose pipe error\n");
							}

							return 1;   
						}
						
						strSrcRes = src;
						string newline = replace_all(src,"!!","!(null)!");
						char buf[44][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!\
									%[^!]!%[^!]!%[^!]!%[^!]",
									buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],
									buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],
									buf[20],buf[21],buf[22],buf[23],buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],
									buf[30],buf[31],buf[32],buf[33],buf[34],buf[35],buf[36],buf[37],buf[38],buf[39],
									buf[40],buf[41],buf[42],buf[43]);
						
						po.AddressWidth = atoi(buf[0]);
						po.Architecture = buf[1];
						po.Availability = buf[2];
						po.Caption = buf[3];
						po.ConfigManagerErrorCode = buf[4];
						po.ConfigManagerUserConfig = buf[5];
						po.CpuStatus = buf[6];
						po.CreationClassName = buf[7];
						po.CurrentClockSpeed = atoi(buf[8]);
						po.CurrentVoltage = buf[9];
						po.DataWidth = buf[10];
						po.Description = buf[11];
						po.DeviceID = buf[12];
						po.ErrorCleared = buf[13];
						po.ErrorDescription = buf[14];
						po.ExtClock = buf[15];
						po.Family = buf[16];
						po.InstallDate = buf[17];
						po.L2CacheSize = buf[18];
						po.L2CacheSpeed = buf[19];
						po.LastErrorCode = buf[20];
						po.Level = buf[21];
						po.LoadPercentage = buf[22];
						po.Manufacturer = buf[23];
						po.MaxClockSpeed = buf[24];
						po.Name = buf[25];
						po.OtherFamilyDescription = buf[26];
						po.PNPDeviceID = buf[27];
						po.PowerManagementCapabilities = buf[28];
						po.PowerManagementSupported = buf[29];
						po.ProcessorId = buf[30];
						po.ProcessorType = buf[31];
						po.Revision = buf[32];
						po.Role = buf[33];
						po.SocketDesignation = buf[34];
						po.Status = buf[35];
						po.StatusInfo = buf[36];
						po.Stepping = buf[37];
						po.SystemCreationClassName = buf[38];
						po.SystemName = buf[39];
						po.UniqueId = buf[40];
						po.UpgradeMethod = buf[41];
						po.Version = buf[42];
						po.VoltageCaps = buf[43];

					}

					if(line)
					{
						free(line);
					}

					if(-1 ==  pclose(fp))
					{
						printf("Win32_Processor pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("Win32_Processor pclose pipe error\n");
		}

		return 1;   
	}

	int WmiBios(CWmiBios& bo, string &strSrcRes)
	{
		FILE *fp = NULL;

		if(1 == WmiExecQuery("Win32_BIOS",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					if(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						string src = line;
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_BIOS pclose pipe error\n");
							}

							return 1;   
						}
						strSrcRes = src;
						string newline = replace_all(src,"!!","!(null)!");
	
						char *outnewline = NULL;

						char buf[27][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]",
									buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],
									buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],
									buf[20],buf[21],buf[22],buf[23],buf[24],buf[25],buf[26]);
						
						bo.BiosCharacteristics = buf[0];
						bo.BIOSVersion = buf[1];
						bo.BuildNumber = buf[2];
						bo.Caption = buf[3];
						bo.CodeSet = buf[4];
						bo.CurrentLanguage = buf[5];
						bo.Description = buf[6];
						bo.IdentificationCode = buf[7];
						bo.InstallableLanguages = buf[8];
						bo.InstallDate = buf[9];
						bo.LanguageEdition = buf[10];
						bo.ListOfLanguages = buf[11];
						bo.Manufacturer = buf[12];
						bo.Name = buf[13];
						bo.OtherTargetOS = buf[14];
						bo.PrimaryBIOS = buf[15];
						bo.ReleaseDate = buf[16];
						bo.SerialNumber = buf[17];
						bo.SMBIOSBIOSVersion = buf[18];
						bo.SMBIOSMajorVersion = buf[19];
						bo.SMBIOSMinorVersion = buf[20];
						bo.SMBIOSPresent = buf[21];
						bo.SoftwareElementID = buf[22];
						bo.SoftwareElementState = buf[23];
						bo.Status = buf[24];
						bo.TargetOperatingSystem = buf[25];
						bo.Version = buf[26];
						
					}

					if(line)
					{
						free(line);
					}

					if(-1 ==  pclose(fp))
					{
						printf("Win32_BIOS pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("Win32_BIOS pclose pipe error\n");
		}

		return 1;   

	}

	int WmiComputerSystem(CWmiComputerSystem& cs, string &strSrcRes)
	{		
		FILE *fp = NULL;

		if(1 == WmiExecQuery("Win32_ComputerSystem",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					if(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						
						string src = line;
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_ComputerSystem pclose pipe error\n");
							}

							return 1;   
						}
						strSrcRes = src;
						string newline = replace_all(src,"!!","!(null)!");

						char *outnewline = NULL;

						char buf[55][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]",
									buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],
									buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],
									buf[20],buf[21],buf[22],buf[23],buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],
									buf[30],buf[31],buf[32],buf[33],buf[34],buf[35],buf[36],buf[37],buf[38],buf[39],
									buf[40],buf[41],buf[42],buf[43],buf[44],buf[45],buf[46],buf[47],buf[48],buf[49],
									buf[50],buf[51],buf[52],buf[53],buf[54]);
						
						cs.AdminPasswordStatus = buf[0];
						cs.AutomaticResetBootOption = buf[1];
						cs.AutomaticResetCapability = buf[2];
						cs.BootOptionOnLimit = buf[3];
						cs.BootOptionOnWatchDog = buf[4];
						cs.BootROMSupported = buf[5];
						cs.BootupState = buf[6];
						cs.Caption = buf[7];
						cs.ChassisBootupState = buf[8];
						cs.CreationClassName = buf[9];
						cs.CurrentTimeZone = buf[10];
						cs.DaylightInEffect = buf[11];
						cs.Description = buf[12];
						cs.DNSHostName = buf[13];
						cs.Domain = buf[14];
						cs.DomainRole = buf[15];
						cs.EnableDaylightSavingsTime = buf[16];
						cs.FrontPanelResetStatus = buf[17];
						cs.InfraredSupported = buf[18];
						cs.InitialLoadInfo = buf[19];
						cs.InstallDate = buf[20];
						cs.KeyboardPasswordStatus = buf[21];
						cs.LastLoadInfo = buf[22];
						cs.Manufacturer = buf[23];
						cs.Model = buf[24];
						cs.Name = buf[25];
						cs.NameFormat = buf[26];
						cs.NetworkServerModeEnabled = buf[27];
						cs.NumberOfProcessors = buf[28];
						cs.OEMLogoBitmap = buf[29];
						cs.OEMStringArray = buf[30];
						cs.PartOfDomain = buf[31];
						cs.PauseAfterReset = buf[32];
						cs.PowerManagementCapabilities = buf[33];
						cs.PowerManagementSupported = buf[34];
						cs.PowerOnPasswordStatus = buf[35];
						cs.PowerState = buf[36];
						cs.PowerSupplyState = buf[37];
						cs.PrimaryOwnerContact = buf[38];
						cs.PrimaryOwnerName = buf[39];
						cs.ResetCapability = buf[40];
						cs.ResetCount = buf[41];
						cs.ResetLimit = buf[42];
						cs.Roles = buf[43];
						cs.Status = buf[44];
						cs.SupportContactDescription = buf[45];
						cs.SystemStartupDelay = buf[46];
						cs.SystemStartupOptions = buf[47];
						cs.SystemStartupSetting = buf[48];
						cs.SystemType = buf[49];
						cs.ThermalState = buf[50];
						cs.TotalPhysicalMemory = buf[51];
						cs.UserName = buf[52];
						cs.WakeUpType = buf[53];
						cs.Workgroup = buf[54];
					}

					if(line)
					{
						free(line);
					}

					if(-1 ==  pclose(fp))
					{
						printf("Win32_ComputerSystem pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("Win32_ComputerSystem pclose pipe error\n");
		}

		return 1;   

	}

	int WmiVideoController(CWmiVideoController& vo, string &strSrcRes)
	{
		FILE *fp = NULL;

		if(1 == WmiExecQuery("Win32_VideoController",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					if(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						string src = line;
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_VideoController pclose pipe error\n");
							}

							return 1;   
						}
			            strSrcRes = src;
						string newline = replace_all(src,"!!","!(null)!");

						char *outnewline = NULL;

						char buf[59][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]",
									buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],
									buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],
									buf[20],buf[21],buf[22],buf[23],buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],
									buf[30],buf[31],buf[32],buf[33],buf[34],buf[35],buf[36],buf[37],buf[38],buf[39],
									buf[40],buf[41],buf[42],buf[43],buf[44],buf[45],buf[46],buf[47],buf[48],buf[49],
									buf[50],buf[51],buf[52],buf[53],buf[54],buf[55],buf[56],buf[57],buf[58]);
						
						vo.AcceleratorCapabilities = buf[0];
						vo.AdapterCompatibility = buf[1];
						vo.AdapterDACType = buf[2];
						vo.AdapterRAM = atoi(buf[3]);
						vo.Availability = buf[4];
						vo.CapabilityDescriptions = buf[5];
						vo.Caption = buf[6];
						vo.ColorTableEntries = buf[7];
						vo.ConfigManagerErrorCode = buf[8];
						vo.ConfigManagerUserConfig = buf[9];
						vo.CreationClassName = buf[10];
						vo.CurrentBitsPerPixel = buf[11];
						vo.CurrentHorizontalResolution = buf[12];
						vo.CurrentNumberOfColors = buf[13];
						vo.CurrentNumberOfColumns = buf[14];
						vo.CurrentNumberOfRows = buf[15];
						vo.CurrentRefreshRate = buf[16];
						vo.CurrentScanMode = buf[17];
						vo.CurrentVerticalResolution = buf[18];
						vo.Description = buf[19];
						vo.DeviceID = buf[20];
						vo.DeviceSpecificPens = buf[21];
						vo.DitherType = buf[22];
						vo.DriverDate = buf[23];
						vo.DriverVersion = buf[24];
						vo.ErrorCleared = buf[25];
						vo.ErrorDescription = buf[26];
						vo.ICMIntent = buf[27];
						vo.ICMMethod = buf[28];
						vo.InfFilename = buf[29];
						vo.InfSection = buf[30];
						vo.InstallDate = buf[31];
						vo.InstalledDisplayDrivers = buf[32];
						vo.LastErrorCode = buf[33];
						vo.MaxMemorySupported = atoi(buf[34]);
						vo.MaxNumberControlled = atoi(buf[35]);
						vo.MaxRefreshRate = atoi(buf[36]);
						vo.MinRefreshRate = atoi(buf[37]);
						vo.Monochrome = buf[38];
						vo.Name = buf[39];
						vo.NumberOfColorPlanes = buf[40];
						vo.NumberOfVideoPages = buf[41];
						vo.PNPDeviceID = buf[42];
						vo.PowerManagementCapabilities = buf[43];
						vo.PowerManagementSupported = buf[44];
						vo.ProtocolSupported = buf[45];
						vo.ReservedSystemPaletteEntries = buf[46];
						vo.SpecificationVersion = buf[47];
						vo.Status = buf[48];
						vo.StatusInfo = buf[49];
						vo.SystemCreationClassName = buf[50];
						vo.SystemName = buf[51];
						vo.SystemPaletteEntries = buf[52];
						vo.TimeOfLastReset = buf[53];
						vo.VideoArchitecture = buf[54];
						vo.VideoMemoryType = buf[55];
						vo.VideoMode = buf[56];
						vo.VideoModeDescription = buf[57];
						vo.VideoProcessor = buf[58];

					}

					if(line)
					{
						free(line);
					}

					if(-1 ==  pclose(fp))
					{
						printf("Win32_VideoController pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("Win32_VideoController pclose pipe error\n");
		}

		return 1;   
	}


	int Wmi1394Controller(CWmi1394Controller& c1394, string &strSrcRes)
	{
	//	if(1 == WmiExecQuery(&pEnumerator,"Win32_1394Controller"))
	//	{
	//		return 1;
	//	}

		return 0;   
	}

	int WmiBus(Objlist& buslist, string &strSrcRes)
	{
		FILE *fp = NULL;

		if(1 == WmiExecQuery("Win32_Bus",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					while(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						string src = line;	
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_Bus pclose pipe error\n");
							}

							return 1;   
						}
						strSrcRes = src;
						string newline = replace_all(src,"!!","!(null)!");

						char *outnewline = NULL;

						char buf[21][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]",
									buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],
									buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],buf[20]);
												
						CWmiBus *bus = new CWmiBus(); // 所分配的内存由调用程序进行删除

						bus->Availability = buf[0];
						bus->BusNum = atoi(buf[1]);
						bus->BusType = atoi(buf[2]);
						bus->Caption = buf[3];
						bus->ConfigManagerErrorCode = buf[4];
						bus->ConfigManagerUserConfig = buf[5];
						bus->CreationClassName = buf[6];
						bus->Description = buf[7];
						bus->DeviceID = buf[8];
						bus->ErrorCleared = buf[9];
						bus->ErrorDescription = buf[10];
						bus->InstallDate = buf[11];
						bus->LastErrorCode = buf[12];
						bus->Name = buf[13];
						bus->PNPDeviceID = buf[14];
						bus->PowerManagementCapabilities = buf[15];
						bus->PowerManagementSupported = buf[16];
						bus->Status = buf[17];
						bus->StatusInfo = buf[18];
						bus->SystemCreationClassName = buf[19];
						bus->SystemName = buf[20];

						buslist.push_back((long)bus);		

					}
		
					if(line)
					{
						free(line);
					}

					if(-1 ==  pclose(fp))
					{
						printf("Win32_Bus pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("Win32_Bus pclose pipe error\n");
		}

		return 1;   
	
	}

	int WmiSystemSlot(Objlist& slotlist, string &strSrcRes)
	{
		//if(1 == WmiExecQuery(&pEnumerator,"Win32_SystemSlot"))
		//{
		//	return 1;
		//}

		return 0;   
	}

	int WmiParallelPort(Objlist& para, string &strSrcRes)
	{
	//	if(1 == WmiExecQuery(&pEnumerator,"Win32_ParallelPort"))
	//	{
	//		return 1;
	//	}

		// Get the data from the query

		return 0;   
	}

	int WmiProcess(Objlist& pocesslist, string &strSrcRes)
	{
		FILE *fp = NULL;

		if(1 == WmiExecQuery("Win32_Process",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					while(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						string src = line;
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_Process pclose pipe error\n");
							}

							return 1;   
						}
						strSrcRes = src;
						string newline = replace_all(src,"!!","!(null)!");

						char buf[45][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]",
									buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],
									buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],
									buf[20],buf[21],buf[22],buf[23],buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],
									buf[30],buf[31],buf[32],buf[33],buf[34],buf[35],buf[36],buf[37],buf[38],buf[39],
									buf[40],buf[41],buf[42],buf[43],buf[44]);
						
						CWmiProcess *po = new CWmiProcess(); // 所分配的内存由调用程序进行删除

						po->Caption = buf[0];
						po->CommandLine = buf[1];
						po->CreationClassName = buf[2];
						po->CreationDate = buf[3];
						po->CSCreationClassName = buf[4];
						po->CSName = buf[5];
						po->Description = buf[6];
						po->ExecutablePath = buf[7];
						po->ExecutionState = buf[8];
						po->Handle = buf[9];
						po->HandleCount = buf[10];
						po->InstallDate = buf[11];
						po->KernelModeTime = buf[12];
						po->MaximumWorkingSetSize = buf[13];
						po->MinimumWorkingSetSize = buf[14];
						po->Name = buf[15];
						po->OSCreationClassName = buf[16];
						po->OSName = buf[17];
						po->OtherOperationCount = buf[18];
						po->OtherTransferCount = buf[19];
						po->PageFaults = buf[20];
						po->PageFileUsage = buf[21];
						po->ParentProcessId = buf[22];
						po->PeakPageFileUsage = buf[23];
						po->PeakVirtualSize = buf[24];
						po->PeakWorkingSetSize = buf[25];
						po->Priority = atoi(buf[26]);
						po->PrivatePageCount = buf[27];
						po->ProcessId = atoi(buf[28]);
						po->QuotaNonPagedPoolUsage = buf[29];
						po->QuotaPagedPoolUsage = buf[30];
						po->QuotaPeakNonPagedPoolUsage = buf[31];
						po->QuotaPeakPagedPoolUsage = buf[32];
						po->ReadOperationCount = buf[33];
						po->ReadTransferCount = buf[34];
						po->SessionId = buf[35];
						po->Status = buf[36];
						po->TerminationDate = buf[37];
						po->ThreadCount = atoi(buf[38]);
						po->UserModeTime = buf[39];
						po->VirtualSize = buf[40];
						po->WindowsVersion = buf[41];
						po->WorkingSetSize = buf[42];
						po->WriteOperationCount = buf[43];
						po->WriteTransferCount = buf[44];

						pocesslist.push_back((long)po);	

						
					}

					if(line)
					{
						free(line);
					}


					if(-1 ==  pclose(fp))
					{
						printf("Win32_Process pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("Win32_Process pclose pipe error\n");
		}

		return 1;   
	}

	int WmiDiskDrive(CWmiDiskDrive& diskDrive, string &strSrcRes)
	{
		FILE *fp = NULL;

		if(1 == WmiExecQuery("Win32_DiskDrive",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					if(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						string src = line;
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_DiskDrive pclose pipe error\n");
							}

							return 1;   
						}
                        strSrcRes = src;						
						printf("src:%s\n",src.c_str());
						FILE *fp;
						fp = ACE_OS::fopen("/home/SOC_windows/test.txt","wb+");
						ACE_OS::fwrite(src.c_str(),1,src.length(),fp);
						ACE_OS::fclose(fp);
						string newline = replace_all(src,"!!","!(null)!");

						char buf[49][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]",
									buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],
									buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],
									buf[20],buf[21],buf[22],buf[23],buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],
									buf[30],buf[31],buf[32],buf[33],buf[34],buf[35],buf[36],buf[37],buf[38],buf[39],
									buf[40],buf[41],buf[42],buf[43],buf[44],buf[45],buf[46],buf[47],buf[48]);
						
						diskDrive.Availability = buf[0];
						diskDrive.BytesPerSector = atoi(buf[1]);
						diskDrive.Capabilities = buf[2];
						diskDrive.CapabilityDescriptions = buf[3];
						diskDrive.Caption = buf[4];
						diskDrive.CompressionMethod = buf[5];
						diskDrive.ConfigManagerErrorCode = buf[6];
						diskDrive.ConfigManagerUserConfig = buf[7];
						diskDrive.CreationClassName = buf[8];
						diskDrive.DefaultBlockSize = buf[9];
						diskDrive.Description = buf[10];
						diskDrive.DeviceID = buf[11];
						diskDrive.ErrorCleared = buf[12];
						diskDrive.ErrorDescription = buf[13];
						diskDrive.ErrorMethodology = buf[14];
						diskDrive.Index = buf[15];
						diskDrive.InstallDate = buf[16];
						diskDrive.InterfaceType = buf[17];
						diskDrive.LastErrorCode = buf[18];
						diskDrive.Manufacturer = buf[19];
						diskDrive.MaxBlockSize = buf[20];
						diskDrive.MaxMediaSize = buf[21];
						diskDrive.MediaLoaded = buf[22];
						diskDrive.MediaType = buf[23];
						diskDrive.MinBlockSize = buf[24];
						diskDrive.Model = buf[25];
						diskDrive.Name = buf[26];
						diskDrive.NeedsCleaning = buf[27];
						diskDrive.NumberOfMediaSupported = buf[28];
						diskDrive.Partitions = atoi(buf[29]);
						diskDrive.PNPDeviceID = buf[30];
						diskDrive.PowerManagementCapabilities = buf[31];
						diskDrive.PowerManagementSupported = buf[32];
						diskDrive.SCSIBus = buf[33];
						diskDrive.SCSILogicalUnit = buf[34];
						diskDrive.SCSIPort = buf[35];
						diskDrive.SCSITargetId = buf[36];
						diskDrive.SectorsPerTrack = buf[37];
						diskDrive.Signature = buf[38];
						diskDrive.Size = buf[39];
						diskDrive.Status = buf[40];
						diskDrive.StatusInfo = buf[41];
						diskDrive.SystemCreationClassName = buf[42];
						diskDrive.SystemName = buf[43];
						diskDrive.TotalCylinders = buf[44];
						diskDrive.TotalHeads = buf[45];
						diskDrive.TotalSectors = buf[46];
						diskDrive.TotalTracks = buf[47];
						diskDrive.TracksPerCylinder = buf[48];
						//printf("diskDrive.Manufacturer:%s\n", *diskDrive.SCSIPort.c_str());
					}

					if(line)
					{
						free(line);
					}

					if(-1 ==  pclose(fp))
					{
						printf("Win32_DiskDrive pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("Win32_DiskDrive pclose pipe error\n");
		}

		return 1;   

	}


	int WmiNetAdpater(Objlist& netAdpaterlist, string &strSrcRes)
	{
		FILE *fp = NULL;

		if(1 == WmiExecQuery("Win32_NetworkAdapter",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					while(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						string src = line;
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_NetworkAdapter pclose pipe error\n");
							}

							return 1;   
						}
						strSrcRes = src;
						string newline = replace_all(src,"!!","!(null)!");

						char buf[37][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]",
									buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],
									buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],
									buf[20],buf[21],buf[22],buf[23],buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],
									buf[30],buf[31],buf[32],buf[33],buf[34],buf[35],buf[36]);
						
						CWmiNetAdpater *netAdpater = new CWmiNetAdpater(); // 所分配的内存由调用程序进行删除

						netAdpater->AdapterType = buf[0];
						netAdpater->AdapterTypeId = buf[1];
						netAdpater->AutoSense = buf[2];
						netAdpater->Availability = atoi(buf[3]);
						netAdpater->Caption = buf[4];
						netAdpater->ConfigManagerErrorCode = buf[5];
						netAdpater->ConfigManagerUserConfig = buf[6];
						netAdpater->CreationClassName = buf[7];
						netAdpater->Description = buf[8];
						netAdpater->DeviceID = atoi(buf[9]);
						netAdpater->ErrorCleared = buf[10];
						netAdpater->ErrorDescription = buf[11];
						netAdpater->Index = atoi(buf[12]);
						netAdpater->InstallDate = buf[13];
						netAdpater->Installed = buf[14];
						netAdpater->InterfaceIndex = buf[15];
						netAdpater->LastErrorCode = buf[16];
						netAdpater->MACAddress = buf[17];
						netAdpater->Manufacturer = buf[18];
						netAdpater->MaxNumberControlled = buf[19];
						netAdpater->MaxSpeed = buf[20];
						netAdpater->Name = buf[21];
						netAdpater->NetConnectionID = buf[22];
						netAdpater->NetConnectionStatus = buf[23];
						netAdpater->NetworkAddresses = buf[24];
						netAdpater->PermanentAddress = buf[25];
						netAdpater->PNPDeviceID = buf[26];
						netAdpater->PowerManagementCapabilities = buf[27];
						netAdpater->PowerManagementSupported = buf[28];
						netAdpater->ProductName = buf[29];
						netAdpater->ServiceName = buf[30];
						netAdpater->Speed = buf[31];
						netAdpater->Status = buf[32];
						netAdpater->StatusInfo = buf[33];
						netAdpater->SystemCreationClassName = buf[34];
						netAdpater->SystemName = buf[35];
						netAdpater->TimeOfLastReset = buf[36];

						netAdpaterlist.push_back((long)netAdpater);					

					}

					if(line)
					{
						free(line);
					}

					if(-1 ==  pclose(fp))
					{
						printf("Win32_NetworkAdapter pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("Win32_NetworkAdapter pclose pipe error\n");
		}

		return 1;   
	}

	int WmiNetAdpaterConfig(Objlist& netAdpaterConfiglist, string &strSrcRes)
	{
		FILE *fp = NULL;

		if(1 == WmiExecQuery("Win32_NetworkAdapterConfiguration",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					while(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						string src = line;
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_NetworkAdapterConfiguration pclose pipe error\n");
							}

							return 1;   
						}
			            strSrcRes = src;
						string newline = replace_all(src,"!!","!(null)!");

						char buf[60][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]",
									buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],
									buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],
									buf[20],buf[21],buf[22],buf[23],buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],
									buf[30],buf[31],buf[32],buf[33],buf[34],buf[35],buf[36],buf[37],buf[38],buf[39],
									buf[40],buf[41],buf[42],buf[43],buf[44],buf[45],buf[46],buf[47],buf[48],buf[49],
									buf[50],buf[51],buf[52],buf[53],buf[54],buf[55],buf[56],buf[57],buf[58],buf[59]);
												
						CWmiNetAdpaterConfig *netAdpaterConfig = new CWmiNetAdpaterConfig(); // 所分配的内存由调用程序进行删除
					
						netAdpaterConfig->ArpAlwaysSourceRoute = buf[0];
						netAdpaterConfig->ArpUseEtherSNAP = buf[1];
						netAdpaterConfig->Caption = buf[2];
						netAdpaterConfig->DatabasePath = buf[3];
						netAdpaterConfig->DeadGWDetectEnabled = buf[4];
						netAdpaterConfig->DefaultIPGateway = buf[5];
						netAdpaterConfig->DefaultTOS = buf[6];
						netAdpaterConfig->DefaultTTL = buf[7];
						netAdpaterConfig->Description = buf[8];
						netAdpaterConfig->DHCPEnabled = buf[9];
						netAdpaterConfig->DHCPLeaseExpires = buf[10];
						netAdpaterConfig->DHCPLeaseObtained = buf[11];
						netAdpaterConfig->DHCPServer = buf[12];
						netAdpaterConfig->DNSDomain = buf[13];
						netAdpaterConfig->DNSDomainSuffixSearchOrder = buf[14];
						netAdpaterConfig->DNSEnabledForWINSResolution = buf[15];
						netAdpaterConfig->DNSHostName = buf[16];
						
						//netAdpaterConfig->DNSServerSearchOrder[256] = buf[17]; // 有多项，中间用‘，’号隔开
						char *temp = NULL;
						temp = strtok(buf[17],"(");
						temp = strtok(temp,")"); // 去括号
						int index = 0;
						netAdpaterConfig->DNSServerSearchOrder[index] = strtok(temp,",");
						char *p = NULL;
						while((p=strtok(NULL,",")))
						{
							index++;
							netAdpaterConfig->DNSServerSearchOrder[index] = p;
						}

						netAdpaterConfig->DomainDNSRegistrationEnabled = buf[18];
						netAdpaterConfig->ForwardBufferMemory = buf[19];
						netAdpaterConfig->FullDNSRegistrationEnabled = buf[20];
						netAdpaterConfig->GatewayCostMetric = buf[21];
						netAdpaterConfig->IGMPLevel = buf[22];
						netAdpaterConfig->Index = atoi(buf[23]);

						//netAdpaterConfig->IPAddress[256] = buf[24];  // 有多项，中间用‘，’号隔开
						temp = NULL;
						temp = strtok(buf[24],"(");
						temp = strtok(temp,")"); // 去括号
						index = 0;
						netAdpaterConfig->IPAddress[index] = strtok(temp,",");
						p = NULL;
						while((p=strtok(NULL,",")))
						{
							index++;
							netAdpaterConfig->IPAddress[index] = p;
						}

						netAdpaterConfig->IPConnectionMetric = buf[25];
						netAdpaterConfig->IPEnabled = buf[26];
						netAdpaterConfig->IPFilterSecurityEnabled = buf[27];
						netAdpaterConfig->IPPortSecurityEnabled = buf[28];
						netAdpaterConfig->IPSecPermitIPProtocols = buf[29];
						netAdpaterConfig->IPSecPermitTCPPorts = buf[30];
						netAdpaterConfig->IPSecPermitUDPPorts = buf[31];

						//netAdpaterConfig->IPSubnet[256] = buf[33]; // 有多项，中间用‘，’号隔开
						temp = NULL;
						temp = strtok(buf[33],"(");
						temp = strtok(temp,")"); // 去括号
						index = 0;
						netAdpaterConfig->IPSubnet[index] = strtok(temp,",");
						p = NULL;
						while((p=strtok(NULL,",")))
						{
							index++;
							netAdpaterConfig->IPSubnet[index] = p;
						}

						netAdpaterConfig->IPUseZeroBroadcast = buf[33];
						netAdpaterConfig->IPXAddress = buf[34];
						netAdpaterConfig->IPXEnabled = buf[35];
						netAdpaterConfig->IPXFrameType = buf[36];
						netAdpaterConfig->IPXMediaType = buf[37];
						netAdpaterConfig->IPXNetworkNumber = buf[38];
						netAdpaterConfig->IPXVirtualNetNumber = buf[39];
						netAdpaterConfig->KeepAliveInterval = buf[40];
						netAdpaterConfig->KeepAliveTime = buf[41];
						netAdpaterConfig->MACAddress = buf[42];  
						netAdpaterConfig->MTU = buf[43];
						netAdpaterConfig->NumForwardPackets = buf[44];
						netAdpaterConfig->PMTUBHDetectEnabled = buf[45];
						netAdpaterConfig->PMTUDiscoveryEnabled = buf[46];
						netAdpaterConfig->ServiceName = buf[47];
						netAdpaterConfig->SettingID = buf[48];
						netAdpaterConfig->TcpipNetbiosOptions = buf[49];
						netAdpaterConfig->TcpMaxConnectRetransmissions = buf[50];
						netAdpaterConfig->TcpMaxDataRetransmissions = buf[51];
						netAdpaterConfig->TcpNumConnections = buf[52];
						netAdpaterConfig->TcpUseRFC1122UrgentPointer = buf[53];
						netAdpaterConfig->TcpWindowSize = buf[54];
						netAdpaterConfig->WINSEnableLMHostsLookup = buf[55];
						netAdpaterConfig->WINSHostLookupFile = buf[56];
						netAdpaterConfig->WINSPrimaryServer = buf[57];
						netAdpaterConfig->WINSScopeID = buf[58];
						netAdpaterConfig->WINSSecondaryServer = buf[59];

						netAdpaterConfiglist.push_back((long)netAdpaterConfig);

					}

					if(line)
					{
						free(line);
					}


					if(-1 ==  pclose(fp))
					{
						printf("Win32_NetworkAdapterConfiguration pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("Win32_NetworkAdapterConfiguration pclose pipe error\n");
		}

		return 1;
	
	}

	int WmiService(Objlist& serlist, string &strSrcRes)
	{
		FILE *fp = NULL;

		if(1 == WmiExecQuery("Win32_Service",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					while(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						string src = line;
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_Service pclose pipe error\n");
							}

							return 1;   
						}
						strSrcRes = src;
						string newline = replace_all(src,"!!","!(null)!");

						char buf[25][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]!%[^!]! \
									%[^!]!%[^!]!%[^!]!%[^!]!%[^!]",
									buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],
									buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],
									buf[20],buf[21],buf[22],buf[23],buf[24]);

						CWmiService *ser = new CWmiService(); // 所分配的内存由调用程序进行删除

						ser->AcceptPause = buf[0];
						ser->AcceptStop = buf[1];
						ser->Caption = buf[2];
						ser->CheckPoint = buf[3];
						ser->CreationClassName = buf[4];
						ser->Description = buf[5];
						ser->DesktopInteract = buf[6];
						ser->DisplayName = buf[7];
						ser->ErrorControl = buf[8];
						ser->ExitCode = buf[9];
						ser->InstallDate = buf[10];
						ser->Name = buf[11];
						ser->PathName = buf[12];
						ser->ProcessId = atoi(buf[13]);
						ser->ServiceSpecificExitCode = buf[14];
						ser->ServiceType = buf[15];
						ser->Started = buf[16];
						ser->StartMode = buf[17];
						ser->StartName = buf[18];
						ser->State = buf[19];
						ser->Status = buf[20];
						ser->SystemCreationClassName = buf[21];
						ser->SystemName = buf[22];
						ser->TagId = buf[23];
						ser->WaitHint = buf[24];

						serlist.push_back((long)ser);

					}

					if(line)
					{
						free(line);
					}

					if(-1 ==  pclose(fp))
					{
						printf("Win32_Service pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("Win32_Service pclose pipe error\n");
		}

		return 1;   
	}

	int WmiProduct(Objlist& prolist, string &strSrcRes)
	{
		
		return 1;   
	}

	int WmiPerfLogicalDisk(Objlist& serlist, string &strSrcRes)
	{
		FILE *fp = NULL;

		if(1 == WmiExecQueryWql("select Name,FreeMegabytes from Win32_PerfFormattedData_PerfDisk_LogicalDisk",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					while(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						string src = line;
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_PerfFormattedData_PerfDisk_LogicalDisk pclose pipe error\n");
							}

							return 1;   
						}
					    strSrcRes = src;
						string newline = replace_all(src,"!!","!(null)!");

						char buf[2][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]",buf[0],buf[1]);

						CWmiPerfLogicalDisk *disk = new CWmiPerfLogicalDisk(); // 所分配的内存由调用程序进行删除
						
						disk->FreeMegabytes = atoi(buf[0]);
						disk->Name = buf[1];
						
						serlist.push_back((long)disk);

					}

					if(line)
					{
						free(line);
					}

					if(-1 ==  pclose(fp))
					{
						printf("PerfLogicalDisk pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("PerfLogicalDisk pclose pipe error\n");
		}

		return 1;   
	}

	int WmiPerfMemory(CWmiPerfMemory& memory, string &strSrcRes)
	{
		FILE *fp = NULL;

		if(1 == WmiExecQueryWql("select AvailableMBytes from Win32_PerfFormattedData_PerfOS_Memory",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					if(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						
						memory.AvailableMBytes = line;
					}
					strSrcRes = line;
					if(line)
					{
						free(line);
					}

					if(-1 ==  pclose(fp))
					{
						printf("PerfMemory pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("PerfMemory pclose pipe error\n");
		}

		return 1;   

	}

	int WmiPerfProcessor(Objlist& serlist, string &strSrcRes)
	{
		FILE *fp = NULL;

		if(1 == WmiExecQueryWql("select Name,PercentProcessortime from Win32_PerfFormattedData_PerfOS_Processor",&fp))
		{
			return 1;
		}

		char * line = NULL;
		size_t len = 0;

		// 获取第一行
		if(-1 != getline(&line,&len,fp))
		{
			//printf("receive----%s\n",line);

			// 查询成功
			if(0 == strncmp(line,"CLASS",strlen("CLASS")))
			{
				//  查询属性项
				if(-1 != getline(&line,&len,fp))
				{
					//printf("receive----%s\n",line);

					// 循环获取参数值
					while(getline(&line,&len,fp) != -1)
					{
						//printf("receive----%s\n",line);
						string src = line;
						if (src.length() > 4096)
						{
							if(line)
							{
								free(line);
							}

							if(-1 ==  pclose(fp))
							{
								printf("Win32_PerfFormattedData_PerfOS_Processor pclose pipe error\n");
							}

							return 1;   
						}
						strSrcRes = src;
						string newline = replace_all(src,"!!","!(null)!");

						char buf[2][4096] = {0};
						sscanf(newline.c_str(),"%[^!]!%[^!]",buf[0],buf[1]);

						CWmiPerfProcessor *pro = new CWmiPerfProcessor(); // 所分配的内存由调用程序进行删除

						pro->Name = buf[0];
						pro->PercentProcessortime = buf[1];

						serlist.push_back((long)pro);

					}

					if(line)
					{
						free(line);
					}

					if(-1 ==  pclose(fp))
					{
						printf("PerfProcessor pclose pipe error\n");
					}
					return 0;
				}
			}

		}

		if(line)
		{
			free(line);
		}

		if(-1 ==  pclose(fp))
		{
			printf("PerfProcessor pclose pipe error\n");
		}

		return 1;   
	}

///////////////////////////////////////////////////////////////
//	函 数 名 : WmiExecCommand
//	函数功能 : 执行关闭、重启、注销等动作
//	处理过程 :
//	备    注 : 
//	作    者 : 
//	时    间 : 2011年5月
//	返 回 值 : 成功时返回0，失败时返回1
//	参数说明 : 
///////////////////////////////////////////////////////////////
	int WmiExecCommand(char* commandParm)
	{
	
		FILE * fp = NULL;

		char command[1024] = {0};
		sprintf(command,"wmis -U %s\%%%s //%s \"%s\"",m_userName.c_str(),m_password.c_str(),m_desIp.c_str(),commandParm);
		printf("%s\n",command);

		fp = popen(command,"r");
		if(NULL == fp)
		{
			printf("popen pipe error\n");
			return 1;
		}
		pclose(fp);
		return 0;
		
	}

	int WmiExecQuery(char *szWql,FILE **ppfile)
	{
		FILE * fp = NULL;

		char command[1024] = {0};
		char sql[128] = {0};
		sprintf(sql,"select * from %s",szWql);
		sprintf(command,"wmic -U %s\%%%s //%s \"%s\"",m_userName.c_str(),m_password.c_str(),m_desIp.c_str(),sql);
		printf("%s\n",command);

		fp = popen(command,"r");
		if(NULL == fp)
		{
			printf("popen pipe error\n");
			return 1;
		}
		
		*ppfile = fp;
		
		return 0;
	}

	int WmiExecQueryWql(char *szWql,FILE **ppfile)
	{
		FILE * fp = NULL;

		char command[1024] = {0};
		
		sprintf(command,"wmic -U %s\%%%s //%s \"%s\"",m_userName.c_str(),m_password.c_str(),m_desIp.c_str(),szWql);
		printf("%s\n",command);

		fp = popen(command,"r");
		if(NULL == fp)
		{
			printf("popen pipe error\n");
			return 1;
		}

		*ppfile = fp;

		return 0;
	}

	string& replace_all(string& str,const string& old_value,const string& new_value)   
	{   
		while (true )   {   
			string::size_type pos(0);   
			if ((pos= str.find(old_value))!= string::npos)   
				str.replace(pos,old_value.length(),new_value);   
			else    break ;   
		}   
		str += "!"; // 便于分割，加上!字符串   
		return    str; 
	}   
	string& replace_all_distinct(string& str,const string& old_value,const string& new_value)   
	{   
		for (string::size_type pos(0);pos!= string::npos;pos+= new_value.length())   {   
			if ((pos= str.find(old_value,pos))!= string:: npos)   
				str.replace(pos,old_value.length(),new_value);   
			else break ;   
		}   
		return str;
	}  

protected:
	string	m_desIp;		
	string	m_userName;
	string  m_password;
};

#endif // WMI_H
