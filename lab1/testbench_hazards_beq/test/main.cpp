#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.
#define ADDU 1
#define SUBU 3
#define AND 4
#define OR  5
#define NOR 7
struct IFStruct {
    bitset<32>  PC;
    bool        nop;
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem;
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu
    bool        wrt_enable;
    bool        nop;
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem;
    bool        wrt_enable;
    bool        nop;
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public:
        bitset<32> Reg_data;

     	RF()
    	{
			Registers.resize(32);
			Registers[0] = bitset<32> (0);
        }

        bitset<32> readRF(bitset<5> Reg_addr)
        {
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }

        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }

		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt");
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();
		}

	private:
		vector<bitset<32> >Registers;
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {
			IMem.resize(MemSize);
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{
					IMem[i] = bitset<8>(line);
					i++;
				}
			}
            else cout<<"Unable to open file";
			imem.close();
		}

		bitset<32> readInstr(bitset<32> ReadAddress)
		{
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;
		}

    private:
        vector<bitset<8> > IMem;
};

class DataMem
{
    public:
        bitset<32> ReadData;
        DataMem()
        {
            DMem.resize(MemSize);
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();
        }

        bitset<32> readDataMem(bitset<32> Address)
        {
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;
		}

        void writeDataMem(bitset<32> Address, bitset<32> WriteData)
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));
        }

        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {
                    dmemout << DMem[j]<<endl;
                }

            }
            else cout<<"Unable to open file";
            dmemout.close();
        }

    private:
		vector<bitset<8> > DMem;
};

class ALU
{
      public:
             bitset<32> ALUresult;
             bitset<32> ALUOperation (bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
             {
                    unsigned int result;
                    switch(ALUOP.to_ulong())
                    {
                        case ADDU : result = oprand1.to_ulong() + oprand2.to_ulong(); break;//加减必须转ulong因为bitset没有定义加减运算，或者直接二进制加减
                        case SUBU : result = oprand1.to_ulong() - oprand2.to_ulong(); break;
                        case AND : result = oprand1.to_ulong() & oprand2.to_ulong();  break;//可以不转ulong的，有定义
                        case OR  : result = oprand1.to_ulong() | oprand2.to_ulong();  break;//同上
                        case NOR : result = ~(oprand1.to_ulong() | oprand2.to_ulong()); break;
                    }
                 return ALUresult = bitset<32>(result);
            }
};

unsigned long shiftbits(bitset<32> inst, int start){
    return (inst.to_ulong()>>start);
}

bitset<32> signextend(bitset<16> imm){
    string res;
    if (imm[15]==0){//16位立即数的最高位是0 正数
        res = "0000000000000000"+imm.to_string();
    }
    else{//负数
        res = "1111111111111111"+imm.to_string();
    }
    return (bitset<32> (res));
}

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl;

        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl;

        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl;
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;

        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl;
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl;
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl;
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl;
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl;
    }
    else cout<<"Unable to open file";
    printstate.close();
}



int main()
{
    // instruction
    bitset<32> PC=0;
    bitset<32> instruction;
    bitset<6> opcode;
    bitset<6> funct;
    bitset<16> imm;

    //control signals
    bitset<1> IType;
    bitset<1> JType;
    bitset<1> RType;
    bitset<1> IsBranch;
    bitset<1> IsLoad;
    bitset<1> IsStore;
    bitset<1> WrtEnable;

    // RF signals
    bitset<5> RReg1;
    bitset<5> RReg2;
    bitset<5> WReg;
    bitset<32> WData;

    // ALU signals
    bitset<3>  ALUop;
    bitset<32> ALUin1;
    bitset<32> ALUin2;
    bitset<32> signext;
    bitset<32> ALUOut;

    // DMEM signals
    bitset<32> DMAddr;
    bitset<32> WriteData;
    bitset<1> ReadMem;
    bitset<1> WriteMem;

    // pc signals
    bitset<32> pcplusfour;
    bitset<32> jaddr;
    bitset<32> braddr;
    bitset<1> IsEq;
    //Simultaneous R/W request to the same register -> Write before Read

    RF myRF;
    ALU myALU;
    INSMem myInsMem;
    DataMem myDataMem;
    stateStruct state;

    int cycle=0;//initialization stage
    bool flag = 1;//BNE
    int stall = 0;

    state.IF.PC=0;
    state.IF.nop=false;
    state.ID.nop=true;
    state.EX.nop=true;
    state.MEM.nop=true;
    state.WB.nop=true;
    ofstream file;
    file.open("stateresult.txt");
    file.close();
    while (1) {
        stateStruct newState=state;

        /* --------------------- WB stage --------------------- */
        if(!state.WB.nop){

            if(state.WB.wrt_enable){
                cout<<"state.WB.Wrt_reg_addr: "<<state.WB.Wrt_reg_addr<<" state.WB.Wrt_data: "<<state.WB.Wrt_data<<endl;
                myRF.writeRF(state.WB.Wrt_reg_addr,state.WB.Wrt_data);

            }
        }



        /* --------------------- MEM stage --------------------- */
        if(!state.MEM.nop){
            DMAddr = state.MEM.ALUresult;
            newState.WB.nop = false;
            newState.WB.wrt_enable = state.MEM.wrt_enable;
            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;


            //cout<<""
            cout<<"waht?"<<endl;
            if(state.MEM.rd_mem==0 && state.MEM.wrt_mem==0)// R type
            {
                newState.WB.Wrt_data = state.MEM.ALUresult;
                cout<<" newState.WB.Wrt_data: "<<newState.WB.Wrt_data<<endl;
            }
            // 写回寄存器
            else if(newState.MEM.rd_mem){//是LW
                newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
            }
            else{
                //Mem to Mem
                if(state.WB.Wrt_reg_addr == state.MEM.Rt && state.WB.wrt_enable){
                    state.MEM.Store_data = state.WB.Wrt_data;
                }

                myDataMem.writeDataMem(state.MEM.ALUresult,state.MEM.Store_data);
            }

        }
        else{
            newState.WB.nop=true;
        }


        /* --------------------- EX stage --------------------- */
        if(!state.EX.nop){
            newState.MEM.nop = false;
            //Mem to Ex
            if(state.WB.wrt_enable && state.WB.Wrt_reg_addr==state.EX.Rs){
                state.EX.Read_data1 = state.WB.Wrt_data;
            }
            if(state.WB.wrt_enable && state.WB.Wrt_reg_addr==state.EX.Rt){
                state.EX.Read_data2 = state.WB.Wrt_data;
            }
            //Ex to Ex
            if(state.EX.Rs==state.MEM.Wrt_reg_addr && state.MEM.wrt_enable && !state.MEM.rd_mem){
                state.EX.Read_data1 = state.MEM.ALUresult;
            }
            if(state.EX.Rt==state.MEM.Wrt_reg_addr && state.MEM.wrt_enable && !state.MEM.rd_mem){
                state.EX.Read_data2 = state.MEM.ALUresult;
            }

            ALUin1 = state.EX.Read_data1;
            ALUin2 = (state.EX.is_I_type)?signextend(state.EX.Imm):state.EX.Read_data2;
            newState.MEM.ALUresult = myALU.ALUOperation(ALUop,ALUin1,ALUin2);//这里用了ALUOP会不会有问题？ 应该不会 更新的步骤在下面ID
            newState.MEM.Rs = state.EX.Rs;
            newState.MEM.Rt = state.EX.Rt;
            newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
            newState.MEM.wrt_enable = state.EX.wrt_enable;
            newState.MEM.rd_mem = state.EX.rd_mem;
            newState.MEM.wrt_mem = state.EX.wrt_mem;
            if(state.EX.wrt_mem)
                newState.MEM.Store_data = state.EX.Read_data2;

            cout<<"cycle: "<<cycle<<endl;
            cout<<"MEM.Rs: "<<newState.MEM.Rs<<" MEM.Rt: "<<newState.MEM.Rt<<" MEM.wrt_enable: "<<newState.MEM.wrt_enable<<" Wrt_reg_addr: "<<newState.MEM.Wrt_reg_addr<<endl;

        }
        else{
            newState.MEM.nop=true;
        }



        /* --------------------- ID stage --------------------- */

        if(!state.ID.nop){
            newState.EX.nop=false;
            //newState.ID.Instr = myInsMem(state.IF.PC);
            opcode = bitset<6> (shiftbits(state.ID.Instr,26));
            funct = bitset<6> (shiftbits(state.ID.Instr, 0));
            ALUop = (opcode.to_ulong()==35 || opcode.to_ulong()==43)?(bitset<3>(string("001"))):bitset<3> (shiftbits(state.ID.Instr, 0));
            newState.EX.is_I_type = (opcode.to_ulong()!=0 && opcode.to_ulong()!=2)?1:0;
            newState.EX.Imm = bitset<16> (shiftbits(state.ID.Instr, 0));
            IsBranch = (opcode.to_ulong()==4)?1:0;
            IsLoad = (opcode.to_ulong()==35)?1:0;
            IsStore = (opcode.to_ulong()==43)?1:0;
            RType = (opcode.to_ulong()==0)?1:0;
            IType = (opcode.to_ulong()!=0 && opcode.to_ulong()!=2)?1:0;
            JType = (opcode.to_ulong()==2)?1:0;
            newState.EX.wrt_enable = (IsStore.to_ulong() || IsBranch.to_ulong() || JType.to_ulong())?0:1;
            newState.EX.Rs = bitset<5> (shiftbits(state.ID.Instr, 21));
            newState.EX.Rt = bitset<5> (shiftbits(state.ID.Instr, 16));
            newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
            newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);
            newState.EX.Wrt_reg_addr = (IType.to_ulong())? newState.EX.Rt : bitset<5> (shiftbits(state.ID.Instr, 11));
            newState.EX.rd_mem = (IsLoad.to_ulong())?1:0;
            newState.EX.wrt_mem = (IsStore.to_ulong())?1:0;
            newState.EX.alu_op = (RType.to_ulong()==1 && (funct.to_ulong()==35))?0:1;//判断是不是subu subu是0 其他是1

            signext = signextend (newState.EX.Imm);
            IsEq = (newState.EX.Read_data1.to_ulong()==newState.EX.Read_data2.to_ulong())?1:0;
            /*cout<<"cycle: "<<cycle<<endl;
            cout<<"newState.EX.Read_data1: "<<newState.EX.Read_data1<<endl;
            cout<<"newState.EX.Read_data2: "<<newState.EX.Read_data2<<endl;
            cout<<"newState.EX.Wrt_reg_addr: "<<newState.EX.Wrt_reg_addr<<endl;*/
            //判断BNE
            if(IsBranch==1 && IsEq.to_ulong()==0){//是个BNE
                //cout<<cycle<<endl;
                flag = 0;
                newState.IF.PC = bitset<32>( state.IF.PC.to_ulong() + (bitset<32>((bitset<30> (shiftbits(signext,0))).to_string()+"00")).to_ulong() );
            }


            //Mem to Ex + Stall
            if(state.EX.rd_mem && (state.EX.Wrt_reg_addr==newState.EX.Rs||state.EX.Wrt_reg_addr==newState.EX.Rt) && RType.to_ulong()==1){
                stall = 1;
            }
            if(stall){
                newState.EX.nop = true;
            }


        }
        else{
            newState.EX.nop=true;
        }




        /* --------------------- IF stage --------------------- */
        if(!state.IF.nop){

            instruction = myInsMem.readInstr(state.IF.PC);

            if(instruction.to_string()=="11111111111111111111111111111111"){
            //When a HALT instruction is fetched in IF stage at cycle N,
            //the nop bit of the IF stage in the next clock cycle (cycle N+1) is set to 1
            //and subsequently stays at 1. The nop bit of the ID/RF stage is set to 1 in cycle N+1
            //and subsequently stays at 1. The nop bit of the EX stage is set to 1 in cycle N+2 and subsequently stays at 1.
            //The nop bit of the MEM stage is set to 1 in cycle N+3 and subsequently stays at 1.
            //The nop bit of the WB stage is set to 1 in cycle N+4 and subsequently stays at 1.
                newState.IF.nop = true;
                //newState.ID.nop = true;
                newState.IF.PC = state.IF.PC;
                newState.ID.Instr=instruction;
            }
            else{
                if(flag){
                    newState.IF.PC = bitset<32> (state.IF.PC.to_ulong() + 4);
                    newState.ID.Instr=instruction;
                    newState.ID.nop=false;
                }
                else{
                    newState.ID.Instr=instruction;
                    newState.ID.nop=true;
                    flag = 1;
                }

            }
            if(stall){
                newState.IF = state.IF;
                newState.ID = state.ID;
                stall = 0;
            }

        }
        else{
            newState.IF.nop=state.IF.nop;
            newState.ID.nop=true;
        }







        /* --------------------- update stage --------------------- */
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)//Halt instruction
            break;

        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ...

        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */

        cycle++;
    }

    myRF.outputRF(); // dump RF;
	myDataMem.outputDataMem(); // dump data mem

	return 0;
}
