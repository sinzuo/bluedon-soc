/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    WmiInc.h
 * @brief   SOC, Linux/Windows
 *
 *
 * @version 1.0
 * @author  蓝盾开发二部
 * @date    2011年4月
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _WMIINC_H
#define _WMIINC_H

#include <string>
#include <list>
using namespace std;

#define MAX_LINE_LEN 4096

typedef list<long> Objlist;

#pragma warning( disable: 4819 ) 
#pragma warning( disable: 4996 ) 


/**
 * @brief   记录操作系统信息
 * @class   CWmiOperatingSystem
 * @author
 * @date    2011年4月
 */
class CWmiOperatingSystem
{
public:
	string BootDevice;
	string BuildNumber;
	string BuildType;
	string Caption;
	string CodeSet;
	string CountryCode;
	string CreationClassName;
	string CSCreationClassName;
	string CSDVersion;
	string CSName;
	string CurrentTimeZone;
	string DataExecutionPrevention_32BitApplications;
	string DataExecutionPrevention_Available;
	string DataExecutionPrevention_Drivers;
	string DataExecutionPrevention_SupportPolicy;
	string Debug;
	string Description;
	string Distributed;
	string EncryptionLevel;
	string ForegroundApplicationBoost;
	string FreePhysicalMemory;
	string FreeSpaceInPagingFiles;
	string FreeVirtualMemory;
	string InstallDate;
	string LargeSystemCache;
	string LastBootUpTime;
	string LocalDateTime;
	string Locale;
	string Manufacturer;
	string MaxNumberOfProcesses;
	string MaxProcessMemorySize;
	string Name;
	string NumberOfLicensedUsers;
	string NumberOfProcesses;
	string NumberOfUsers;
	string Organization;
	string OSLanguage;
	string OSProductSuite;
	string OSType;
	string OtherTypeDescription;
	string PAEEnabled;
	string PlusProductID;
	string PlusVersionNumber;
	string Primary;
	string ProductType;
	string QuantumLength;
	string QuantumType;
	string RegisteredUser;
	string SerialNumber;
	int ServicePackMajorVersion;
	int ServicePackMinorVersion;
	string SizeStoredInPagingFiles;
	string Status;
	string SuiteMask;
	string SystemDevice;
	string SystemDirectory;
	string SystemDrive;
	string TotalSwapSpaceSize;
	string TotalVirtualMemorySize;
	string TotalVisibleMemorySize;
	string Version;
	string WindowsDirectory;
	
	CWmiOperatingSystem()
	{
		BootDevice = "";
		BuildNumber = "";
		BuildType = "";
		Caption = "";
		CodeSet = "";
		CountryCode = "";
		CreationClassName = "";
		CSCreationClassName = "";
		CSDVersion = "";
		CSName = "";
		CurrentTimeZone = "";
		DataExecutionPrevention_32BitApplications = "";
		DataExecutionPrevention_Available = "";
		DataExecutionPrevention_Drivers = "";
		DataExecutionPrevention_SupportPolicy = "";
		Debug = "";
		Description = "";
		Distributed = "";
		EncryptionLevel = "";
		ForegroundApplicationBoost = "";
		FreePhysicalMemory = "";
		FreeSpaceInPagingFiles = "";
		FreeVirtualMemory = "";
		InstallDate = "";
		LargeSystemCache = "";
		LastBootUpTime = "";
		LocalDateTime = "";
		Locale = "";
		Manufacturer = "";
		MaxNumberOfProcesses = "";
		MaxProcessMemorySize = "";
		Name = "";
		NumberOfLicensedUsers = "";
		NumberOfProcesses = "";
		NumberOfUsers = "";
		Organization = "";
		OSLanguage = "";
		OSProductSuite = "";
		OSType = "";
		OtherTypeDescription = "";
		PAEEnabled = "";
		PlusProductID = "";
		PlusVersionNumber = "";
		Primary = "";
		ProductType = "";
		QuantumLength = "";
		QuantumType = "";
		RegisteredUser = "";
		SerialNumber = "";
		ServicePackMajorVersion = 0;
		ServicePackMinorVersion = 0;
		SizeStoredInPagingFiles = "";
		Status = "";
		SuiteMask = "";
		SystemDevice = "";
		SystemDirectory = "";
		SystemDrive = "";
		TotalSwapSpaceSize = "";
		TotalVirtualMemorySize = "";
		TotalVisibleMemorySize = "";
		Version = "";
		WindowsDirectory = "";
	}

};


/**
 * @brief   记录处理器信息
 * @class   CWmiProcessor
 * @author
 * @date    2011年4月
 */
class CWmiProcessor
{
public:
	int AddressWidth;
	string Architecture;
	string Availability;
	string Caption;
	string ConfigManagerErrorCode;
	string ConfigManagerUserConfig;
	string CpuStatus;
	string CreationClassName;
	int CurrentClockSpeed;
	string CurrentVoltage;
	string DataWidth;
	string Description;
	string DeviceID;
	string ErrorCleared;
	string ErrorDescription;
	string ExtClock;
	string Family;
	string InstallDate;
	string L2CacheSize;
	string L2CacheSpeed;
	string LastErrorCode;
	string Level;
	string LoadPercentage;
	string Manufacturer;
	string MaxClockSpeed;
	string Name;
	string OtherFamilyDescription;
	string PNPDeviceID;
	string PowerManagementCapabilities;
	string PowerManagementSupported;
	string ProcessorId;
	string ProcessorType;
	string Revision;
	string Role;
	string SocketDesignation;
	string Status;
	string StatusInfo;
	string Stepping;
	string SystemCreationClassName;
	string SystemName;
	string UniqueId;
	string UpgradeMethod;
	string Version;
	string VoltageCaps;

	CWmiProcessor()
	{
		AddressWidth = 0;
		Architecture = "";
		Availability = "";
		Caption = "";
		ConfigManagerErrorCode = "";
		ConfigManagerUserConfig = "";
		CpuStatus = "";
		CreationClassName = "";
		CurrentClockSpeed = 0;
		CurrentVoltage = "";
		DataWidth = "";
		Description = "";
		DeviceID = "";
		ErrorCleared = "";
		ErrorDescription = "";
		ExtClock = "";
		Family = "";
		InstallDate = "";
		L2CacheSize = "";
		L2CacheSpeed = "";
		LastErrorCode = "";
		Level = "";
		LoadPercentage = "";
		Manufacturer = "";
		MaxClockSpeed = "";
		Name = "";
		OtherFamilyDescription = "";
		PNPDeviceID = "";
		PowerManagementCapabilities = "";
		PowerManagementSupported = "";
		ProcessorId = "";
		ProcessorType = "";
		Revision = "";
		Role = "";
		SocketDesignation = "";
		Status = "";
		StatusInfo = "";
		Stepping = "";
		SystemCreationClassName = "";
		SystemName = "";
		UniqueId = "";
		UpgradeMethod = "";
		Version = "";
		VoltageCaps = "";
	}
};


/**
 * @brief   记录计算内存信息
 * @class   CWmiComputerSystem
 * @author
 * @date    2011年4月
 */
class CWmiComputerSystem
{
public:
	string AdminPasswordStatus;
	string AutomaticResetBootOption;
	string AutomaticResetCapability;
	string BootOptionOnLimit;
	string BootOptionOnWatchDog;
	string BootROMSupported;
	string BootupState;
	string Caption;
	string ChassisBootupState;
	string CreationClassName;
	string CurrentTimeZone;
	string DaylightInEffect;
	string Description;
	string DNSHostName;
	string Domain;
	string DomainRole;
	string EnableDaylightSavingsTime;
	string FrontPanelResetStatus;
	string InfraredSupported;
	string InitialLoadInfo;
	string InstallDate;
	string KeyboardPasswordStatus;
	string LastLoadInfo;
	string Manufacturer;
	string Model;
	string Name;
	string NameFormat;
	string NetworkServerModeEnabled;
	string NumberOfProcessors;
	string OEMLogoBitmap;
	string OEMStringArray;
	string PartOfDomain;
	string PauseAfterReset;
	string PowerManagementCapabilities;
	string PowerManagementSupported;
	string PowerOnPasswordStatus;
	string PowerState;
	string PowerSupplyState;
	string PrimaryOwnerContact;
	string PrimaryOwnerName;
	string ResetCapability;
	string ResetCount;
	string ResetLimit;
	string Roles;
	string Status;
	string SupportContactDescription;
	string SystemStartupDelay;
	string SystemStartupOptions;
	string SystemStartupSetting;
	string SystemType;
	string ThermalState;
	string TotalPhysicalMemory;
	string UserName;
	string WakeUpType;
	string Workgroup;

	CWmiComputerSystem()
	{
		AdminPasswordStatus = "";
		AutomaticResetBootOption = "";
		AutomaticResetCapability = "";
		BootOptionOnLimit = "";
		BootOptionOnWatchDog = "";
		BootROMSupported = "";
		BootupState = "";
		Caption = "";
		ChassisBootupState = "";
		CreationClassName = "";
		CurrentTimeZone = "";
		DaylightInEffect = "";
		Description = "";
		DNSHostName = "";
		Domain = "";
		DomainRole = "";
		EnableDaylightSavingsTime = "";
		FrontPanelResetStatus = "";
		InfraredSupported = "";
		InitialLoadInfo = "";
		InstallDate = "";
		KeyboardPasswordStatus = "";
		LastLoadInfo = "";
		Manufacturer = "";
		Model = "";
		Name = "";
		NameFormat = "";
		NetworkServerModeEnabled = "";
		NumberOfProcessors = "";
		OEMLogoBitmap = "";
		OEMStringArray = "";
		PartOfDomain = "";
		PauseAfterReset = "";
		PowerManagementCapabilities = "";
		PowerManagementSupported = "";
		PowerOnPasswordStatus = "";
		PowerState = "";
		PowerSupplyState = "";
		PrimaryOwnerContact = "";
		PrimaryOwnerName = "";
		ResetCapability = "";
		ResetCount = "";
		ResetLimit = "";
		Roles = "";
		Status = "";
		SupportContactDescription = "";
		SystemStartupDelay = "";
		SystemStartupOptions = "";
		SystemStartupSetting = "";
		SystemType = "";
		ThermalState = "";
		TotalPhysicalMemory = "";
		UserName = "";
		WakeUpType = "";
		Workgroup = "";
	}
};


/**
 * @brief   记录1394控制器信息
 * @class   CWmi1394Controller
 * @author
 * @date    2011年4月
 */
class CWmi1394Controller
{
public:
	string Caption;
	string CreationClassName;
	string Description;
	string DeviceID;
	string Manufacturer;
	int MaxNumberControlled;
	string Name;
	string PNPDeviceID;
	string Status;

	CWmi1394Controller()
	{
		Caption = "";
		CreationClassName = "";
		Description = "";
		DeviceID = "";
		Manufacturer = "";
		MaxNumberControlled = 0;
		Name = "";
		PNPDeviceID = "";
		Status = "";
	}
};


/**
 * @brief   记录Bios信息
 * @class   CWmiBios
 * @author
 * @date    2011年4月
 */
class CWmiBios
{
public:
	string BiosCharacteristics;
	string BIOSVersion;
	string BuildNumber;
	string Caption;
	string CodeSet;
	string CurrentLanguage;
	string Description;
	string IdentificationCode;
	string InstallableLanguages;
	string InstallDate;
	string LanguageEdition;
	string ListOfLanguages;
	string Manufacturer;
	string Name;
	string OtherTargetOS;
	string PrimaryBIOS;
	string ReleaseDate;
	string SerialNumber;
	string SMBIOSBIOSVersion;
	string SMBIOSMajorVersion;
	string SMBIOSMinorVersion;
	string SMBIOSPresent;
	string SoftwareElementID;
	string SoftwareElementState;
	string Status;
	string TargetOperatingSystem;
	string Version;

	CWmiBios()
	{
		BiosCharacteristics = "";
		BIOSVersion = "";
		BuildNumber = "";
		Caption = "";
		CodeSet = "";
		CurrentLanguage = "";
		Description = "";
		IdentificationCode = "";
		InstallableLanguages = "";
		InstallDate = "";
		LanguageEdition = "";
		ListOfLanguages = "";
		Manufacturer = "";
		Name = "";
		OtherTargetOS = "";
		PrimaryBIOS = "";
		ReleaseDate = "";
		SerialNumber = "";
		SMBIOSBIOSVersion = "";
		SMBIOSMajorVersion = "";
		SMBIOSMinorVersion = "";
		SMBIOSPresent = "";
		SoftwareElementID = "";
		SoftwareElementState = "";
		Status = "";
		TargetOperatingSystem = "";
		Version = "";
	}
};


/**
 * @brief   记录显卡信息
 * @class   CWmiVideoController
 * @author
 * @date    2011年4月
 */
class CWmiVideoController
{
public:
	string AcceleratorCapabilities;
	string AdapterCompatibility;
	string AdapterDACType;
	int AdapterRAM;
	string Availability;
	string CapabilityDescriptions;
	string Caption;
	string ColorTableEntries;
	string ConfigManagerErrorCode;
	string ConfigManagerUserConfig;
	string CreationClassName;
	string CurrentBitsPerPixel;
	string CurrentHorizontalResolution;
	string CurrentNumberOfColors;
	string CurrentNumberOfColumns;
	string CurrentNumberOfRows;
	string CurrentRefreshRate;
	string CurrentScanMode;
	string CurrentVerticalResolution;
	string Description;
	string DeviceID;
	string DeviceSpecificPens;
	string DitherType;
	string DriverDate;
	string DriverVersion;
	string ErrorCleared;
	string ErrorDescription;
	string ICMIntent;
	string ICMMethod;
	string InfFilename;
	string InfSection;
	string InstallDate;
	string InstalledDisplayDrivers;
	string LastErrorCode;
	int MaxMemorySupported;
	int MaxNumberControlled;
	int MaxRefreshRate;
	int MinRefreshRate;
	string Monochrome;
	string Name;
	string NumberOfColorPlanes;
	string NumberOfVideoPages;
	string PNPDeviceID;
	string PowerManagementCapabilities;
	string PowerManagementSupported;
	string ProtocolSupported;
	string ReservedSystemPaletteEntries;
	string SpecificationVersion;
	string Status;
	string StatusInfo;
	string SystemCreationClassName;
	string SystemName;
	string SystemPaletteEntries;
	string TimeOfLastReset;
	string VideoArchitecture;
	string VideoMemoryType;
	string VideoMode;
	string VideoModeDescription;
	string VideoProcessor;

	CWmiVideoController()
	{
		AcceleratorCapabilities = "";
		AdapterCompatibility = "";
		AdapterDACType = "";
		AdapterRAM = 0;
		Availability = "";
		CapabilityDescriptions = "";
		Caption = "";
		ColorTableEntries = "";
		ConfigManagerErrorCode = "";
		ConfigManagerUserConfig = "";
		CreationClassName = "";
		CurrentBitsPerPixel = "";
		CurrentHorizontalResolution = "";
		CurrentNumberOfColors = "";
		CurrentNumberOfColumns = "";
		CurrentNumberOfRows = "";
		CurrentRefreshRate = "";
		CurrentScanMode = "";
		CurrentVerticalResolution = "";
		Description = "";
		DeviceID = "";
		DeviceSpecificPens = "";
		DitherType = "";
		DriverDate = "";
		DriverVersion = "";
		ErrorCleared = "";
		ErrorDescription = "";
		ICMIntent = "";
		ICMMethod = "";
		InfFilename = "";
		InfSection = "";
		InstallDate = "";
		InstalledDisplayDrivers = "";
		LastErrorCode = "";
		MaxMemorySupported = 0;
		MaxNumberControlled = 0;
		MaxRefreshRate = 0;
		MinRefreshRate = 0;
		Monochrome = "";
		Name = "";
		NumberOfColorPlanes = "";
		NumberOfVideoPages = "";
		PNPDeviceID = "";
		PowerManagementCapabilities = "";
		PowerManagementSupported = "";
		ProtocolSupported = "";
		ReservedSystemPaletteEntries = "";
		SpecificationVersion = "";
		Status = "";
		StatusInfo = "";
		SystemCreationClassName = "";
		SystemName = "";
		SystemPaletteEntries = "";
		TimeOfLastReset = "";
		VideoArchitecture = "";
		VideoMemoryType = "";
		VideoMode = "";
		VideoModeDescription = "";
		VideoProcessor = "";
	}
};



/**
 * @brief   记录总线信息
 * @class   CWmiBus
 * @author
 * @date    2011年4月
 */
class CWmiBus
{
public:
	string Availability;
	int BusNum;
	int BusType;
	string Caption;
	string ConfigManagerErrorCode;
	string ConfigManagerUserConfig;
	string CreationClassName;
	string Description;
	string DeviceID;
	string ErrorCleared;
	string ErrorDescription;
	string InstallDate;
	string LastErrorCode;
	string Name;
	string PNPDeviceID;
	string PowerManagementCapabilities;
	string PowerManagementSupported;
	string Status;
	string StatusInfo;
	string SystemCreationClassName;
	string SystemName;

	CWmiBus()
	{
		Availability = "";
		BusNum = 0;
		BusType = 0;
		Caption = "";
		ConfigManagerErrorCode = "";
		ConfigManagerUserConfig = "";
		CreationClassName = "";
		Description = "";
		DeviceID = "";
		ErrorCleared = "";
		ErrorDescription = "";
		InstallDate = "";
		LastErrorCode = "";
		Name = "";
		PNPDeviceID = "";
		PowerManagementCapabilities = "";
		PowerManagementSupported = "";
		Status = "";
		StatusInfo = "";
		SystemCreationClassName = "";
		SystemName = "";
	}
};


/**
 * @brief   记录系统槽位信息
 * @class   CWmiSystemSlot
 * @author
 * @date    2011年4月
 */
class CWmiSystemSlot
{
public:
	string ConnectorPinout;
	//int ConnectorType[];
	string CreationClassName;
	int CurrentUsage;
	string Description;
	string Model;
	string Name;
	int Number;
	string OtherIdentifyingInfo;
	string PartNumber;
	string PurposeDescription;
	string SerialNumber;
	string SKU;
	string SlotDesignation;
	string Status;
	string Tag;
	string Version;
};


/**
 * @brief   记录端口信息
 * @class   CWmiParallelPort
 * @author
 * @date    2011年4月
 */
class CWmiParallelPort
{
public:
//	int Capabilities[];
//	string CapabilityDescriptions[];
	string Caption;
	string CreationClassName;
	string Description;
	string DeviceID;
	string Name;
	string PNPDeviceID;
	string Status;

	CWmiParallelPort()
	{
		Caption = "";
		CreationClassName = "";
		Description = "";
		DeviceID = "";
		Name = "";
		PNPDeviceID = "";
		Status = "";
	}
};


/**
 * @brief   记录系统进程信息
 * @class   CWmiProcess
 * @author
 * @date    2011年4月
 */
class CWmiProcess
{
public:
	string Caption;
	string CommandLine;
	string CreationClassName;
	string CreationDate;
	string CSCreationClassName;
	string CSName;
	string Description;
	string ExecutablePath;
	string ExecutionState;
	string Handle;
	string HandleCount;
	string InstallDate;
	string KernelModeTime;
	string MaximumWorkingSetSize;
	string MinimumWorkingSetSize;
	string Name;
	string OSCreationClassName;
	string OSName;
	string OtherOperationCount;
	string OtherTransferCount;
	string PageFaults;
	string PageFileUsage;
	string ParentProcessId;
	string PeakPageFileUsage;
	string PeakVirtualSize;
	string PeakWorkingSetSize;
	int Priority;
	string PrivatePageCount;
	int ProcessId;
	string QuotaNonPagedPoolUsage;
	string QuotaPagedPoolUsage;
	string QuotaPeakNonPagedPoolUsage;
	string QuotaPeakPagedPoolUsage;
	string ReadOperationCount;
	string ReadTransferCount;
	string SessionId;
	string Status;
	string TerminationDate;
	int ThreadCount;
	string UserModeTime;
	string VirtualSize;
	string WindowsVersion;
	string WorkingSetSize;
	string WriteOperationCount;
	string WriteTransferCount;

	CWmiProcess()
	{
		Caption = "";
		CommandLine = "";
		CreationClassName = "";
		CreationDate = "";
		CSCreationClassName = "";
		CSName = "";
		Description = "";
		ExecutablePath = "";
		ExecutionState = "";
		Handle = "";
		HandleCount = "";
		InstallDate = "";
		KernelModeTime = "";
		MaximumWorkingSetSize = "";
		MinimumWorkingSetSize = "";
		Name = "";
		OSCreationClassName = "";
		OSName = "";
		OtherOperationCount = "";
		OtherTransferCount = "";
		PageFaults = "";
		PageFileUsage = "";
		ParentProcessId = "";
		PeakPageFileUsage = "";
		PeakVirtualSize = "";
		PeakWorkingSetSize = "";
		Priority = 0;
		PrivatePageCount = "";
		ProcessId = 0;
		QuotaNonPagedPoolUsage = "";
		QuotaPagedPoolUsage = "";
		QuotaPeakNonPagedPoolUsage = "";
		QuotaPeakPagedPoolUsage = "";
		ReadOperationCount = "";
		ReadTransferCount = "";
		SessionId = "";
		Status = "";
		TerminationDate = "";
		ThreadCount = 0;
		UserModeTime = "";
		VirtualSize = "";
		WindowsVersion = "";
		WorkingSetSize = "";
		WriteOperationCount = "";
		WriteTransferCount = "";
	}
};


/**
 * @brief   记录硬盘信息
 * @class   CWmiDiskDrive
 * @author
 * @date    2011年4月
 */
class CWmiDiskDrive
{
public:
	string Availability;
	int BytesPerSector;
	string Capabilities;
	string CapabilityDescriptions;
	string Caption;
	string CompressionMethod;
	string ConfigManagerErrorCode;
	string ConfigManagerUserConfig;
	string CreationClassName;
	string DefaultBlockSize;
	string Description;
	string DeviceID;
	string ErrorCleared;
	string ErrorDescription;
	string ErrorMethodology;
	string Index;
	string InstallDate;
	string InterfaceType;
	string LastErrorCode;
	string Manufacturer;
	string MaxBlockSize;
	string MaxMediaSize;
	string MediaLoaded;
	string MediaType;
	string MinBlockSize;
	string Model;
	string Name;
	string NeedsCleaning;
	string NumberOfMediaSupported;
	int Partitions;
	string PNPDeviceID;
	string PowerManagementCapabilities;
	string PowerManagementSupported;
	string SCSIBus;
	string SCSILogicalUnit;
	string SCSIPort;
	string SCSITargetId;
	string SectorsPerTrack;
	string Signature;
	string Size;
	string Status;
	string StatusInfo;
	string SystemCreationClassName;
	string SystemName;
	string TotalCylinders;
	string TotalHeads;
	string TotalSectors;
	string TotalTracks;
	string TracksPerCylinder;

	CWmiDiskDrive()
	{
		Availability = "";
		BytesPerSector = 0;
		Capabilities = "";
		CapabilityDescriptions = "";
		Caption = "";
		CompressionMethod = "";
		ConfigManagerErrorCode = "";
		ConfigManagerUserConfig = "";
		CreationClassName = "";
		DefaultBlockSize = "";
		Description = "";
		DeviceID = "";
		ErrorCleared = "";
		ErrorDescription = "";
		ErrorMethodology = "";
		Index = "";
		InstallDate = "";
		InterfaceType = "";
		LastErrorCode = "";
		Manufacturer = "";
		MaxBlockSize = "";
		MaxMediaSize = "";
		MediaLoaded = "";
		MediaType = "";
		MinBlockSize = "";
		Model = "";
		Name = "";
		NeedsCleaning = "";
		NumberOfMediaSupported = "";
		Partitions = 0;
		PNPDeviceID = "";
		PowerManagementCapabilities = "";
		PowerManagementSupported = "";
		SCSIBus = "";
		SCSILogicalUnit = "";
		SCSIPort = "";
		SCSITargetId = "";
		SectorsPerTrack = "";
		Signature = "";
		Size = "";
		Status = "";
		StatusInfo = "";
		SystemCreationClassName = "";
		SystemName = "";
		TotalCylinders = "";
		TotalHeads = "";
		TotalSectors = "";
		TotalTracks = "";
		TracksPerCylinder = "";
	}
};


/**
 * @brief   记录网络适配器信息
 * @class   CWmiNetAdpater
 * @author
 * @date    2011年4月
 */
class CWmiNetAdpater
{
public:
	string AdapterType;
	string AdapterTypeId;
	string AutoSense;
	int Availability;
	string Caption;
	string ConfigManagerErrorCode;
	string ConfigManagerUserConfig;
	string CreationClassName;
	string Description;
	int DeviceID;
	string ErrorCleared;
	string ErrorDescription;
	int Index;
	string InstallDate;
	string Installed;
	string InterfaceIndex;
	string LastErrorCode;
	string MACAddress;
	string Manufacturer;
	string MaxNumberControlled;
	string MaxSpeed;
	string Name;
	string NetConnectionID;
	string NetConnectionStatus;
	string NetworkAddresses;
	string PermanentAddress;
	string PNPDeviceID;
	string PowerManagementCapabilities;
	string PowerManagementSupported;
	string ProductName;
	string ServiceName;
	string Speed;
	string Status;
	string StatusInfo;
	string SystemCreationClassName;
	string SystemName;
	string TimeOfLastReset;

	CWmiNetAdpater()
	{
		AdapterType = "";
		AdapterTypeId = "";
		AutoSense = "";
		Availability = 0;
		Caption = "";
		ConfigManagerErrorCode = "";
		ConfigManagerUserConfig = "";
		CreationClassName = "";
		Description = "";
		DeviceID = 0;
		ErrorCleared = "";
		ErrorDescription = "";
		Index = 0;
		InstallDate = "";
		Installed = "";
		InterfaceIndex = "";
		LastErrorCode = "";
		MACAddress = "";
		Manufacturer = "";
		MaxNumberControlled = "";
		MaxSpeed = "";
		Name = "";
		NetConnectionID = "";
		NetConnectionStatus = "";
		NetworkAddresses = "";
		PermanentAddress = "";
		PNPDeviceID = "";
		PowerManagementCapabilities = "";
		PowerManagementSupported = "";
		ProductName = "";
		ServiceName = "";
		Speed = "";
		Status = "";
		StatusInfo = "";
		SystemCreationClassName = "";
		SystemName = "";
		TimeOfLastReset = "";
	}
};


/**
 * @brief   记录网络适配器配置信息
 * @class   CWmiNetAdpaterConfig
 * @author
 * @date    2011年4月
 */
class CWmiNetAdpaterConfig
{
public:
	string ArpAlwaysSourceRoute;
	string ArpUseEtherSNAP;
	string Caption;
	string DatabasePath;
	string DeadGWDetectEnabled;
	string DefaultIPGateway;
	string DefaultTOS;
	string DefaultTTL;
	string Description;
	string DHCPEnabled;
	string DHCPLeaseExpires;
	string DHCPLeaseObtained;
	string DHCPServer;
	string DNSDomain;
	string DNSDomainSuffixSearchOrder;
	string DNSEnabledForWINSResolution;
	string DNSHostName;
	string DNSServerSearchOrder[256]; // 有多项，中间用‘，’号隔开
	string DomainDNSRegistrationEnabled;
	string ForwardBufferMemory;
	string FullDNSRegistrationEnabled;
	string GatewayCostMetric;
	string IGMPLevel;
	int Index;
	string IPAddress[256];  // 有多项，中间用‘，’号隔开
	string IPConnectionMetric;
	string IPEnabled;
	string IPFilterSecurityEnabled;
	string IPPortSecurityEnabled;
	string IPSecPermitIPProtocols;
	string IPSecPermitTCPPorts;
	string IPSecPermitUDPPorts;
	string IPSubnet[256]; // 有多项，中间用‘，’号隔开
	string IPUseZeroBroadcast;
	string IPXAddress;
	string IPXEnabled;
	string IPXFrameType;
	string IPXMediaType;
	string IPXNetworkNumber;
	string IPXVirtualNetNumber;
	string KeepAliveInterval;
	string KeepAliveTime;
	string MACAddress;  
	string MTU;
	string NumForwardPackets;
	string PMTUBHDetectEnabled;
	string PMTUDiscoveryEnabled;
	string ServiceName;
	string SettingID;
	string TcpipNetbiosOptions;
	string TcpMaxConnectRetransmissions;
	string TcpMaxDataRetransmissions;
	string TcpNumConnections;
	string TcpUseRFC1122UrgentPointer;
	string TcpWindowSize;
	string WINSEnableLMHostsLookup;
	string WINSHostLookupFile;
	string WINSPrimaryServer;
	string WINSScopeID;
	string WINSSecondaryServer;

	CWmiNetAdpaterConfig()
	{
		ArpAlwaysSourceRoute = "";
		ArpUseEtherSNAP = "";
		Caption = "";
		DatabasePath = "";
		DeadGWDetectEnabled = "";
		DefaultIPGateway = "";
		DefaultTOS = "";
		DefaultTTL = "";
		Description = "";
		DHCPEnabled = "";
		DHCPLeaseExpires = "";
		DHCPLeaseObtained = "";
		DHCPServer = "";
		DNSDomain = "";
		DNSDomainSuffixSearchOrder = "";
		DNSEnabledForWINSResolution = "";
		DNSHostName = "";
		//DNSServerSearchOrder = "";
		DomainDNSRegistrationEnabled = "";
		ForwardBufferMemory = "";
		FullDNSRegistrationEnabled = "";
		GatewayCostMetric = "";
		IGMPLevel = "";
		Index = 0;
		//IPAddress = "";
		IPConnectionMetric = "";
		IPEnabled = "";
		IPFilterSecurityEnabled = "";
		IPPortSecurityEnabled = "";
		IPSecPermitIPProtocols = "";
		IPSecPermitTCPPorts = "";
		IPSecPermitUDPPorts = "";
		//IPSubnet = "";
		IPUseZeroBroadcast = "";
		IPXAddress = "";
		IPXEnabled = "";
		IPXFrameType = "";
		IPXMediaType = "";
		IPXNetworkNumber = "";
		IPXVirtualNetNumber = "";
		KeepAliveInterval = "";
		KeepAliveTime = "";
		MACAddress = "";
		MTU = "";
		NumForwardPackets = "";
		PMTUBHDetectEnabled = "";
		PMTUDiscoveryEnabled = "";
		ServiceName = "";
		SettingID = "";
		TcpipNetbiosOptions = "";
		TcpMaxConnectRetransmissions = "";
		TcpMaxDataRetransmissions = "";
		TcpNumConnections = "";
		TcpUseRFC1122UrgentPointer = "";
		TcpWindowSize = "";
		WINSEnableLMHostsLookup = "";
		WINSHostLookupFile = "";
		WINSPrimaryServer = "";
		WINSScopeID = "";
		WINSSecondaryServer = "";
		for(int iii=0;iii<256;iii++)
		{
			DNSServerSearchOrder[iii] = "";
			IPAddress[iii] = "";
			IPSubnet[iii] = "";
		}
	}
};


/**
 * @brief   记录服务信息
 * @class   CWmiService
 * @author
 * @date    2011年4月
 */
class CWmiService
{
public:
	
	string AcceptPause;
	string AcceptStop;
	string Caption;
	string CheckPoint;
	string CreationClassName;
	string Description;
	string DesktopInteract;
	string DisplayName;
	string ErrorControl;
	string ExitCode;
	string InstallDate;
	string Name;
	string PathName;
	int ProcessId;
	string ServiceSpecificExitCode;
	string ServiceType;
	string Started;
	string StartMode;
	string StartName;
	string State;
	string Status;
	string SystemCreationClassName;
	string SystemName;
	string TagId;
	string WaitHint;

	CWmiService()
	{
		AcceptPause = "";
		AcceptStop = "";
		Caption = "";
		CheckPoint = "";
		CreationClassName = "";
		Description = "";
		DesktopInteract = "";
		DisplayName = "";
		ErrorControl = "";
		ExitCode = "";
		InstallDate = "";
		Name = "";
		PathName = "";
		ProcessId = 0;
		ServiceSpecificExitCode = "";
		ServiceType = "";
		Started = "";
		StartMode = "";
		StartName = "";
		State = "";
		Status = "";
		SystemCreationClassName = "";
		SystemName = "";
		TagId = "";
		WaitHint = "";
	}
};


/**
 * @brief   记录软件信息
 * @class   CWmiProduct
 * @author
 * @date    2011年4月
 */
class CWmiProduct
{
public:
	string Caption;
	string Description;
	string IdentifyingNumber;
	string InstallDate;
	string Name;
	string PackageCache;
	string Vendor;
	string Version;

	CWmiProduct()
	{
		Caption = "";
		Description = "";
		IdentifyingNumber = "";
		InstallDate = "";
		Name = "";
		PackageCache = "";
		Vendor = "";
		Version = "";
	}
};


/**
 * @brief   记录实时硬盘性能信息
 * @class   CWmiPerfLogicalDisk
 * @author
 * @date    2011年4月
 */
class CWmiPerfLogicalDisk
{
public:
	string Name;
	int FreeMegabytes;

	CWmiPerfLogicalDisk()
	{
		Name = "";
		FreeMegabytes = 0;
	}

};


/**
 * @brief   记录实时内存性能信息
 * @class   CWmiPerfMemory
 * @author
 * @date    2011年4月
 */
class CWmiPerfMemory
{
public:
	string AvailableMBytes;

	CWmiPerfMemory()
	{
		AvailableMBytes = "";
	}

};


/**
 * @brief   记录实时CPU性能信息
 * @class   CWmiPerfProcessor
 * @author
 * @date    2011年4月
 */
class CWmiPerfProcessor
{
public:
	string Name;
	string PercentProcessortime;

	CWmiPerfProcessor()
	{
		Name = "";
		PercentProcessortime = "";
	}

};

#endif // WMI_H
