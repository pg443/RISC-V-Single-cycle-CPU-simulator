#ifndef __REGISTER_H__
#define __REGISTER_H__

#include <map>

using namespace std;

class Register
{
public:
	unsigned int reg_read_data(unsigned int rindex);
	void reg_write_data(unsigned int rindex, unsigned int data);
	
	Register();
        ~Register();
private:
	// map<unsigned int, unsigned int> registers;
	uint64_t *regis;

};

#endif
