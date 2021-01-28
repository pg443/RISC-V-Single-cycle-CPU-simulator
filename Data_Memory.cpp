#include "Data_Memory.h"

uint64_t Data_Memory::get_data(long addr){
	uint64_t ret_data = 0;

	for (int i=0; i<8; i++){
		ret_data |= (data_memory.find(addr+i)->second<<i*8);
	}
	return ret_data;
}
void Data_Memory::write_data(long addr, uint64_t data){
	for (int i=0; i<8; i++){
		data_memory.insert(pair<long, uint8_t>(addr+i, data&0xff));
		data = data>>8;
	}
}
