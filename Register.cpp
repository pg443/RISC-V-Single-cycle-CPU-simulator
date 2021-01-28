#include "Register.h"
#include <iostream>

unsigned int Register::reg_read_data(unsigned int rindex)
{
	return regis[rindex];
}

void Register::reg_write_data(unsigned int rindex, unsigned int data)
{
	if (rindex==0){
		cout<<"\nERROR: Overwriting zero register NOT ALLOWED\n";
	}else{
		regis[rindex] = data;
	}
}

 Register::Register(){
    regis = (uint64_t *)malloc(32 * sizeof(uint64_t));

    regis[0] = 0;
    regis[25] = 0; 
 }

Register::~Register()
{
    free(regis);
}
