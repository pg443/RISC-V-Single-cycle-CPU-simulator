#include "Core.h"
#include "Control.h"
unsigned int get_bit(unsigned int instruction, int start, int num_of_bits);
void ControlUnit(Control* ctrl, unsigned int instruction);
void RegisterUnit(DecodeStage *dcd, Register *regs, unsigned int instruction);
void ALU(int alucontrol, ExecStage* execs, int func3);
void MemoryUnit(Control* ctrl, MemStage *mem, Data_Memory *data_mem);
void WriteBack(MemStage *mem, DecodeStage *dcd, Control *ctrl, Register *regs);
int ImmGen(unsigned int instruction);
int ALU_control(int ALUOp, int func3, int func7);



Core::Core(const string &fname, ofstream *out) : out(out), 
						clk(0), 
						PC(0),
						instr_mem(new Instruction_Memory(fname))
{

}




/*
	TODO - Add more functions and modify tick() to simulate single-cycle RISC-V architecture
*/
bool Core::tick()
{
	/*
		Step One: Serving pending instructions
	*/
	if (pending_queue.size() > 0)
	{
		serve_pending_instrs();
	}
	
	/*
		Step Two: Where simulation happens
	*/
	if (PC <= instr_mem->last_addr())
	{
		// Get Instruction
		Instruction &instruction = instr_mem->get_instruction(PC);

		// Increment PC
		// TODO, PC should be incremented or decremented based on instruction
		/*
			Step Three: Simulator related
		*/
		instruction.begin_exe = clk;
		
		// Single-cycle always takes one clock cycle to complete
		instruction.end_exe = clk + 1; 


		Control ctrl;
		DecodeStage dcd;
		ExecStage exec_sig;
		MemStage mem;
		// printf("%s\n",instruction.raw_instr);
		ControlUnit(&ctrl, instruction.instruction);
		printf("Control Unit generated => ");

		RegisterUnit(&dcd, &regs, instruction.instruction);
		printf("Decode stage pass => ");


		exec_sig.src1 = dcd.ReadData1;

		/* ALU source selection */
		if (ctrl.ALUSrc == 0){
			exec_sig.src2 = dcd.ReadData2;
		}else{
			exec_sig.src2 = ImmGen(instruction.instruction);
		}

		unsigned int func3 = get_bit(instruction.instruction, 12, 3);
		unsigned int func7 = get_bit(instruction.instruction, 25, 7);
		int alucontrol = ALU_control(ctrl.ALU_Op, func3, func7);
		ALU(alucontrol, &exec_sig, func3);
		printf("Exec stage pass => ");

		if (ctrl.Branch && exec_sig.zero){
			PC += ImmGen(instruction.instruction);
			printf("Branching\t");
		}else{
			PC += 4;

			mem.wData = dcd.ReadData2;
			mem.Address = exec_sig.result;
			MemoryUnit(&ctrl, &mem, &data_mem);
			printf("Memory stage pass => ");

			WriteBack(&mem, &dcd, &ctrl, &regs);
			printf("WB stage pass\n");
		}

		pending_queue.push_back(instruction);
	}

	clk++;

	/*
		Step Four: Should we shut down simulator
	*/
	if (pending_queue.size() == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void Core::serve_pending_instrs()
{
	list<Instruction>::iterator instr = pending_queue.begin();

	if (instr->end_exe <= clk)
	{
		printStats(instr);
		
		pending_queue.erase(instr);	
	}
}

void Core::printStats(list<Instruction>::iterator &ite)
{
	*out << ite->raw_instr << " => ";
	*out << "Core ID: " << id << "; ";
	*out << "Begin Exe: " << ite->begin_exe << "; ";
	*out << "End Exe: " << ite->end_exe << endl;
}


void ControlUnit(Control *ctrl, unsigned int instruction)
{
	int opcode = get_bit(instruction, 0, 7);
	switch (opcode){
		case 3:		//ld
			ctrl->ALUSrc = 1; 
			ctrl->MemRead = 1;
			ctrl->MemWrite = 0;
			ctrl->MemtoReg = 0;
			ctrl->ALU_Op = 0;
			ctrl->Branch = 0;
			ctrl->RegWrite = 1;
			break;
		case 19:		//Immediate
			ctrl->ALUSrc = 1;
			ctrl->MemRead = 0;
			ctrl->MemWrite = 0;
			ctrl->MemtoReg = 1;
			ctrl->ALU_Op = 2;
			ctrl->Branch = 0;
			ctrl->RegWrite = 1;
			break;
		case 35:		//sd
			ctrl->ALUSrc = 0;
			ctrl->MemRead = 0;
			ctrl->MemWrite = 1;
			ctrl->MemtoReg = 1;
			ctrl->ALU_Op = 0;
			ctrl->Branch = 0;
			ctrl->RegWrite = 0;
			break;
		case 51:		//R-type
			ctrl->ALUSrc = 0;
			ctrl->MemRead = 0;
			ctrl->MemWrite = 0;
			ctrl->MemtoReg = 1;
			ctrl->ALU_Op = 2;
			ctrl->Branch = 0;
			ctrl->RegWrite = 1;
			break;
		case 99:		//SB-type
			ctrl->ALUSrc = 0;
			ctrl->MemRead = 0;
			ctrl->MemWrite = 0;
			ctrl->MemtoReg = 0;
			ctrl->ALU_Op = 1;
			ctrl->Branch = 1;
			ctrl->RegWrite = 0;
			break;
		case 111:		//UJ-type, jal
			ctrl->ALUSrc = 1;
			ctrl->MemRead = 0;
			ctrl->MemWrite = 0;
			ctrl->MemtoReg = 0;
			ctrl->ALU_Op = 0;
			ctrl->Branch = 0;
			ctrl->RegWrite = 1;
			break;
		case 103: 		//jalr		// IMPORTANT IMPLEMENTING THIS ONE
			ctrl->ALUSrc = 0;
			ctrl->MemRead = 1;
			ctrl->MemWrite = 1;
			ctrl->MemtoReg = 1;
			ctrl->ALU_Op = 0;
			ctrl->Branch = 0;
			ctrl->RegWrite = 1;
			break;
	}
}

void RegisterUnit(DecodeStage *dcd, Register *regs, unsigned int instruction){
	dcd->ReadReg1 = get_bit(instruction, 15, 5);
	dcd->ReadReg2 = get_bit(instruction, 20, 5);
	dcd->WriteReg = get_bit(instruction, 7, 5);
	dcd->ReadData1 = regs->reg_read_data(dcd->ReadReg1);
	dcd->ReadData2 = regs->reg_read_data(dcd->ReadReg2);
	
}



unsigned int get_bit(unsigned int instruction, int start, int num_of_bits)
{
	if (start!=0){
		instruction = instruction>>start;
	}
	if (num_of_bits==1){
		instruction &= 0x1;
	}else if (num_of_bits==3){
		instruction &= 0x7;
	}else if (num_of_bits==4){
		instruction &= 0xf;
	}else if (num_of_bits==5){
		instruction &= 0x1f;
	}else if (num_of_bits==6){
		instruction &= 0x3f;
	}else if (num_of_bits==7){
		instruction &= 0x7f;
	}else if (num_of_bits==8){
		instruction &= 0xff;
	}else if (num_of_bits==10){
		instruction &= 0x3ff;
	}
	return instruction;
}


#define ADD 2
#define SUB 6
#define AND_OP 0
#define OR_OP 1
#define XOR_OP 3
#define SLL 4
#define SRR 5
int ALU_control(int ALUOp, int func3, int func7){
    int AluC;
    switch (ALUOp)
    {
        case 0:
            AluC = ADD;
            break;
        case 1:
            AluC = SUB;
            break;
        case 2:
            switch (func3)
            {
                case 0:
                    if (func7==0){
                        AluC = ADD;
                    }else if(func7 == 32){
                        AluC = SUB;
                    }
                    break;
                case 1:
                    AluC = SLL;
                    break;
                case 4:
                    AluC = XOR_OP;
                    break;
                case 5:
                    AluC = SRR;
                    break;
                case 6:
                    AluC = OR_OP;
                    break;
                case 7:
                    AluC = AND_OP;
                    break;
            }
            break;
    }
    return AluC;
}

void ALU(int alucontrol, ExecStage* execs, int func3){
	switch (alucontrol){
		case 0:
			execs->result = execs->src1 && execs->src2;
			break;
		case 1:
			execs->result = execs->src1 || execs->src2;
			break;
		case 2:
			execs->result = execs->src1 + execs->src2;
			break;
		case 3:
			execs->result = execs->src1 ^ execs->src2;
			break;
		case 4:
			execs->result = execs->src1 << execs->src2;
			break;
		case 5:
			execs->result = execs->src1 >> execs->src2;
			break;
		case 6:
			execs->result = execs->src1 - execs->src2;
			switch(func3){
				case 0:
					if (execs->result == 0){
						execs->zero = 1;
					}
					break;
				case 1:
					if (execs->result != 0){
						execs->zero = 1;
					}
					break;
				case 2:
					if (execs->result < 0){
						execs->zero = 1;
					}
					break;
				case 3:
					if (execs->result >= 0){
						execs->zero = 1;
					}
					break;
			}
			break;		
	}
}

void MemoryUnit(Control* ctrl, MemStage *mem, Data_Memory *data_mem){
	if (ctrl->MemWrite == 1){
		data_mem->write_data(mem->Address, mem->wData);
	}else if (ctrl->MemRead == 1){
		mem->rData = data_mem->get_data(mem->Address);
	}
}

void WriteBack(MemStage *mem, DecodeStage *dcd, Control *ctrl, Register *regs){
	if (ctrl->RegWrite == 1){
		if (ctrl->MemtoReg == 0){
			regs->reg_write_data(dcd->WriteReg, mem->rData);
		}else{
			regs->reg_write_data(dcd->WriteReg, mem->Address);
		}
	}
}

int ImmGen(unsigned int instruction){
	int opcode = get_bit(instruction, 0, 7);
	int imm = 0;
	if (opcode == 19){			//I-type
		imm |= get_bit(instruction, 20, 12);
		if (imm&0x800){
			imm |= 0xffffffff<<12;
		}
	}else if(opcode == 35){		//sd
		imm |= get_bit(instruction, 7, 5);
		imm |= (get_bit(instruction, 25, 7)<<5);
		if (imm&0x800){
			imm |= 0xffffffff<<12;
		}
	}else if(opcode == 99){		//SB-type
		imm |= (get_bit(instruction, 8, 4)<<1);			//Extracting 
		imm |= (get_bit(instruction, 25, 6)<<(1+4));
		imm |= (get_bit(instruction, 7, 1)<<(1+4+6));
		imm |= (get_bit(instruction, 31, 1)<<(1+4+6+1));
		if (imm&0x1000){
	    imm |= 0xffffffff<<13;
		}
	}
	printf("Immediate Value:  %d\n", imm);
	return imm;
}
