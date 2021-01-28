#ifndef __DATA_MEMORY_H_
#define __DATA_MEMORY_H_

#include <map>

using namespace std;

class Data_Memory
{
public:
	Data_Memory(){};

	uint64_t get_data(long addr);
	void write_data(long addr, uint64_t data);

	// Return the address of the last instruction
	long last_addr() 
	{ 
		map<long, uint8_t>::iterator ite = data_memory.end();
		ite--;
		return ite->second;
	}

private:
	map<long, uint8_t> data_memory;

};

#endif
