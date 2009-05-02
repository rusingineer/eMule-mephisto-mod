// modified by Stulle
#include "stdafx.h"

#include "SystemInfo.h"

CSystemInfo::CSystemInfo(){
	CSysInfo::Init();
}

CSystemInfo::~CSystemInfo(){

}

int CSystemInfo::GetCpuUsage(){
	return cpu.GetUsage( &sys, &upTime );
}

uint32 CSystemInfo::GetMemoryUsage(){
	return CSysInfo::GetProcessMemoryUsageInt();
}

int CSystemInfo::GetGlobalCpuUsage(){
	return sys;
}

uint32 CSystemInfo::GetGlobalMemoryUsage(){
	return (uint32)(CSysInfo::GetTotalPhys()-CSysInfo::GetAvailPhys());
}