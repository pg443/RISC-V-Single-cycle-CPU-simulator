#ifndef __STAGE_SIGNALS_H__
#define __STAGE_SIGNALS_H__

#include <vector>
#include <string>

using namespace std;

class DecodeStage
{
public:
	unsigned int ReadReg1;
	unsigned int ReadReg2;
	unsigned int WriteReg;
	unsigned int WriteData;
	unsigned int ReadData1;
	unsigned int ReadData2;
	DecodeStage(){}
};

class ExecStage
{
public:
	unsigned int src1;
	unsigned int src2;
	unsigned int result;
	bool zero = 0;
	ExecStage(){}
};

class MemStage
{
public:
	long Address;
	unsigned int wData;
	unsigned int rData;
	MemStage(){}
};


#endif
