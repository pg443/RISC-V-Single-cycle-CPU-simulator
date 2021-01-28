#ifndef __CONTROL_H__
#define __CONTROL_H__

using namespace std;

class Control
{
public:
	
	unsigned int ALUSrc;
	unsigned int MemRead;
	unsigned int MemWrite;
	unsigned int MemtoReg;
	unsigned int ALU_Op;
	unsigned int Branch;
	bool RegWrite;

	Control(){}
};

#endif
