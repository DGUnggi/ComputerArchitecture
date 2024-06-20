#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <map>
#include <tuple>
#include <bitset>
#include <typeinfo>
#include <algorithm>
#include <cstring>
using namespace std;

#define text_init_addr 0x00400000
#define data_init_addr 0x10000000
// For output & condition
int always_branch = 2;
int print_reg;
unsigned int address1;
unsigned int address2;
bool instruction_limit;
map<unsigned int, int> Reg;
map<unsigned int, int> Memory;
vector<string> text;
unsigned int End = 1;
unsigned int print_pipe = 0;
unsigned int NumHazard = 0;
unsigned int ATP1 = 0;
unsigned int ATP2 = 0;
unsigned int ANTP = 0;
unsigned int ProgramOver = 0;
// 

typedef struct pc{
    unsigned int NextPc;    
}Program_Counter;

typedef struct if_id{
    unsigned int instr;
    unsigned int IsAvailable;
    unsigned int NextPc;
}IF_ID;

typedef struct id_ex{
    unsigned int brch;
    unsigned int jump;
    unsigned int MemRead;
    unsigned int MemWrite;
    unsigned int MemtoReg;
    unsigned int RegWrite;
    unsigned int opcode;
    unsigned int IsAvailable;
    unsigned int NextPc;
    unsigned int RegisterRd;
    unsigned int RegisterRs;
    unsigned int RegisterRt;
    int rs;
    int rt;
    int imm;
    unsigned int unsigned_imm;
    unsigned int target;
    unsigned int funct;
    unsigned int shamt;
    unsigned int JumpAddress;
    unsigned int BR_TARGET;
}ID_EX;

typedef struct ex_mem{
    unsigned int brch;
    unsigned int jump;
    unsigned int MemRead;
    unsigned int MemWrite;
    unsigned int MemtoReg;
    unsigned int RegWrite;
    unsigned int NextPc;
    unsigned int IsAvailable;
    unsigned int opcode;
    int ALU_OUT;
    unsigned int BR_TARGET;
    unsigned int RegisterRd;
    unsigned int RegisterRs;
    unsigned int RegisterRt;
    int rs;
    int rt;
}EX_MEM;

typedef struct mem_wb{
    unsigned int NextPc;
    unsigned int brch;
    unsigned int jump;
    unsigned int MemRead;
    unsigned int MemWrite;
    unsigned int MemtoReg;
    unsigned int RegWrite;
    unsigned int IsAvailable;
    unsigned int RegNum;
    unsigned int opcode;
    int ALU_OUT;
    int MEM_OUT;
    unsigned int RegisterRd;
    unsigned int RegisterRs;
    unsigned int RegisterRt;
    unsigned int BR_TARGET;
    int rs;
    int rt;
}MEM_WB;

// Function Declare
int findColon(string str);
void PrintinReg(int pc);
void PrintinMem(unsigned int addr1, unsigned int addr2);
string Dectobinary(unsigned int decimal);
void store_memory(unsigned pc, unsigned value);
void ID(pc* Program_Counter, if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb* MEM_WB, unsigned int pc);
void IF(pc* Program_Counter, if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb* MEM_WB, unsigned int num_limit, unsigned int cnt);
void EX(if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb MEM_WB, unsigned int pc);
void MEM(if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb* MEM_WB, unsigned int pc);
void WB(if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb* MEM_WB,   unsigned int pc);
void Stall(pc* PC, id_ex* ID_EX, unsigned int nextpc);
void Flush_JUMP(pc* PC, if_id* IF_ID, unsigned int nextpc);
void Flush_ATP1(pc* PC, if_id* IF_ID, unsigned int nextpc);
void Flush_ATP2(pc* PC, if_id* IF_ID, id_ex* ID_EX, unsigned int nextpc);
void Flush_ANTP(pc* PC, if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, unsigned int nextpc);
void ExecJformat(unsigned int pc, id_ex* ID_EX, ex_mem* EX_MEM);
void ExecRformat(unsigned int op, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb MEM_WB);
void ExecIformat(unsigned int op, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb MEM_WB);
//

void Stall(pc* PC, id_ex* ID_EX, unsigned int nextpc){
    ID_EX->IsAvailable = 0;
    ID_EX->MemRead = 0;
};


void Flush_JUMP(pc* PC, if_id* IF_ID, unsigned int nextpc){
    IF_ID->IsAvailable = 0;
    PC->NextPc = nextpc;
    End = 1;
}
void Flush_ATP1(pc* PC, if_id* IF_ID, unsigned int nextpc){
    IF_ID->IsAvailable = 0;
    PC->NextPc = nextpc;
    End = 1;    
}
void Flush_ATP2(pc* PC, if_id* IF_ID, id_ex* ID_EX, unsigned int nextpc){
    IF_ID->IsAvailable = 0;
    ID_EX->IsAvailable = 0;
    PC->NextPc = nextpc;
    End = 1;
}
void Flush_ANTP(pc* PC, if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, unsigned int nextpc){
    IF_ID->IsAvailable = 0;
    ID_EX->IsAvailable = 0;
    EX_MEM->IsAvailable = 0;
    PC->NextPc = nextpc;
    End = 1;
}


void PrintinReg(int pc){
    cout << "Current register values:" << endl;
    cout << "------------------------------------" << endl;
    printf("PC: 0x%x\n", pc);
    cout << "Registers:" << endl;
    for(int i = 0; i < 32; i++){
        printf("R%d: 0x%x\n", i, Reg[i]);
    }
}
void PrintinPipe(unsigned int DO_IF, unsigned int DO_ID, unsigned int DO_EX, unsigned int DO_MEM, unsigned int DO_WB, unsigned int IFPC, unsigned int IDPC, unsigned int EXPC, unsigned int MEMPC, unsigned int WBPC){
    cout << "Current pipeline PC State:" << endl;
    cout << "{";
        if(DO_IF){
            printf("0x%x", IFPC);
        }
        cout << "|";
        if(DO_ID){
            printf("0x%x", IDPC);
        }
        cout << "|";
        if(DO_EX){
            printf("0x%x", EXPC);
        }
        cout << "|";
        if(DO_MEM){
            printf("0x%x", MEMPC);
        }
        cout << "|";
        if(DO_WB){
            printf("0x%x", WBPC);
        }
        cout << "}\n" << endl;
}
void PrintinMem(unsigned int addr1, unsigned int addr2){
    printf("Memory content [0x%x..0x%x]:\n", addr1, addr2);
    cout << "------------------------------------" << endl;
    unsigned int iteration = (addr2 - addr1)/4;
    unsigned int remainder = (addr2 - addr1) % 4;
    if((addr2 - addr1) == 0){
        printf("0x%x: 0x%x\n", addr1, Memory[addr1]);
    }else if(remainder == 0){
        for(int i = 0; i<iteration+1; i++){
            printf("0x%x: 0x%x\n", (addr1 + 4*i), ((Memory[addr1 + 4*i] << 24) + (Memory[addr1 + 4*i+1] << 16) + (Memory[addr1 + 4*i+2] << 8) + Memory[addr1 + 4*i + 3]));
        }
    }else{
        for(int i = 0; i<iteration; i++){
            printf("0x%x: 0x%x\n", (addr1 + 4*i), ((Memory[addr1 + 4*i] << 24) + (Memory[addr1 + 4*i+1] << 16) + (Memory[addr1 + 4*i+2] << 8) + Memory[addr1 + 4*i + 3]));
        }

        int sum = 0;
        for(int j = 0; j < remainder+1; j++){
            sum += (Memory[addr1+4*iteration+j] << (8*(remainder - j)));
        }
        printf("0x%x: 0x%x\n", (addr1 + 4*iteration), sum);
        
    }
}
int findColon(string str){
    for(int i = 0; i < str.size(); i++){
        if(str[i] == ':'){
            return i;
        }
    }
    return 0;
}
string Dectobinary(unsigned int decimal){
    string bin = bitset<32> (decimal).to_string();
    return bin;
}

void store_memory(unsigned pc, unsigned value){
    Memory[pc] = value;
}
// Instruction을 Fetch하는 것은 일단 무조건 하는 걸로 구현하고 다음에 바꾸자 !!
void Pipeline(pc* Program_Counter, if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb* MEM_WB, unsigned int num_limit, unsigned int cnt, unsigned int cycle){
    string Fetchinstr = text[(Program_Counter->NextPc-0x400000)/4];
    unsigned int pc = Program_Counter->NextPc;
    mem_wb MeM_WB = *MEM_WB;
    unsigned int IF_PC = pc;
    unsigned int ID_PC = IF_ID->NextPc;
    unsigned int EX_PC = ID_EX->NextPc;
    unsigned int MEM_PC = EX_MEM->NextPc;
    unsigned int WB_PC = MEM_WB->NextPc;
    unsigned int IF_isavailable = End;
    unsigned int ID_isavailable = IF_ID->IsAvailable;
    unsigned int EX_isavailable = ID_EX->IsAvailable;
    unsigned int MEM_isavailable = EX_MEM->IsAvailable;
    unsigned int WB_isavailable = MEM_WB->IsAvailable;
    
    // Stage Execution 
    WB(IF_ID, ID_EX, EX_MEM, MEM_WB, MEM_WB->NextPc);
    MEM(IF_ID, ID_EX, EX_MEM, MEM_WB, EX_MEM->NextPc);
    EX(IF_ID, ID_EX, EX_MEM, MeM_WB, ID_EX->NextPc);
    ID(Program_Counter, IF_ID, ID_EX, EX_MEM, MEM_WB, IF_ID->NextPc);
    IF(Program_Counter, IF_ID, ID_EX, EX_MEM, MEM_WB, num_limit, cnt);
    
    
    // Decision for next IF PC;
    unsigned int address;
    unsigned int index;
    // 여기는 다음 Fetch할 instruction의 PC 값 구하는 부분
    if(ATP2 == 1){ // Always Take prediction and Failed to predict -> add 2 Flush in ID and EX
        ATP2--;
        Flush_ATP2(Program_Counter, IF_ID, ID_EX, MEM_WB->NextPc + 4);
    }else if(ANTP == 1){ // Always not take prediction and Failed to predict -> add 3 Flush in ID, EX, MEM
        ANTP--;
        Flush_ANTP(Program_Counter, IF_ID, ID_EX, EX_MEM, MEM_WB->BR_TARGET);
    }else if(ID_EX->jump){ // Jump always incur 1 Flush in ID stage
        ID_EX->jump = 0;
        Flush_JUMP(Program_Counter, IF_ID, ID_EX->JumpAddress);
    }else if(ATP1 == 1){ // Always Take prediction add 1 Flush in ID stage
        ATP1--;
        Flush_ATP1(Program_Counter, IF_ID, ID_EX->BR_TARGET);
    }else if(NumHazard > 0){
        NumHazard--;
    }else if(End == 0){
        Program_Counter->NextPc = pc;
    }else{ // Hazard not occured
        Program_Counter->NextPc = pc + 4;
    }
    
    // Condition for Program Over
    index = (Program_Counter->NextPc - 0x400000)/4;
    if(index == text.size()){
         // 다음 실행할 명령어가 초과했다면, 이제 더 이상 PC 값은 증가 X, 그리고 레지스터나 메모리출력에서 (PC - 4) 해서 출력하기 
        End = 0;
    }
    if(print_pipe){
        printf("====== Cycle %d ======\n", cycle);
        PrintinPipe(IF_isavailable, ID_isavailable, EX_isavailable, MEM_isavailable, WB_isavailable, IF_PC, ID_PC, EX_PC, MEM_PC, WB_PC);
    }
    if(print_reg == 1){
        if(print_pipe == 0){
            printf("====== Cycle %d ======\n", cycle);
        }
        if(index == text.size()){
            PrintinReg(IF_PC);
        }else{
            PrintinReg(IF_PC + 4);
        }
        
        if(address1 != 0){
            PrintinMem(address1, address2);
        }
    }
    if((instruction_limit == 1 && ProgramOver == num_limit) || ((IF_ID->IsAvailable || ID_EX->IsAvailable || EX_MEM->IsAvailable || MEM_WB->IsAvailable || End) == 0)){
        
        if(print_reg == 0 && print_pipe == 0 && instruction_limit == 1){
            printf("===== Completion cycle: %d",cycle);
            cout<< " =====\n" << endl;
            PrintinPipe(IF_isavailable, ID_isavailable, EX_isavailable, MEM_isavailable, WB_isavailable, IF_PC, ID_PC, EX_PC, MEM_PC, WB_PC);
            printf("\n");
            PrintinReg(IF_PC);
        }else{
            WB_isavailable = 0;
            printf("===== Completion cycle: %d",cycle);
            cout<< " =====\n" << endl;
            PrintinPipe(IF_isavailable, ID_isavailable, EX_isavailable, MEM_isavailable, WB_isavailable, IF_PC, ID_PC, EX_PC, MEM_PC, WB_PC);
            printf("\n");
            PrintinReg(IF_PC);
        }
        if(print_reg == 0){
            if(address1 != 0){
                PrintinMem(address1, address2);
            }
        }
        
    
    }else{
        Pipeline(Program_Counter, IF_ID, ID_EX, EX_MEM, MEM_WB, num_limit, cnt + 1, cycle + 1); 
    }  
   
}
void IF(pc* Program_Counter, if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb* MEM_WB, unsigned int num_limit, unsigned int cnt){
    if(End == 0){
        IF_ID->IsAvailable = 0;
        return;
    }
    if(NumHazard == 0){
        IF_ID->IsAvailable = 1;
        IF_ID->instr = stoul(text[(Program_Counter->NextPc-0x400000)/4], nullptr, 2);
        IF_ID->NextPc = Program_Counter->NextPc;
    }
    /*
    if(cnt <= num_limit){
        IF_ID->NextPc = Program_Counter->NextPc;
    }
    */
    
    
    
    /*
    if(instruction_limit){
        if(cnt == num_limit){
            End = 0;
        }
    }
    */
    
}
// 여기서 Control signal 계속 뒤로 전달해줘야함!
void ID(pc* Program_Counter, if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb* MEM_WB, unsigned int pc){
    if(IF_ID->IsAvailable == 0){
        ID_EX->brch = 0;
        ID_EX->jump = 0;
        ID_EX->MemRead = 0;
        ID_EX->MemWrite = 0;
        ID_EX->MemtoReg = 0;
        ID_EX->RegWrite = 0;
        ID_EX->IsAvailable = 0;
        ID_EX->NextPc = 0;
        ID_EX->RegisterRd = 33;
        ID_EX->RegisterRs = 33;
        ID_EX->RegisterRt = 33;
        return;
    }
    ID_EX->IsAvailable = IF_ID->IsAvailable;
    string instr = Dectobinary(IF_ID->instr);
    string op = instr.substr(0,6);
    
    // R format
    unsigned int opcode = stoul(op, nullptr, 2);
    unsigned int rs = stoul(instr.substr(6,5), nullptr, 2);
    unsigned int rt = stoul(instr.substr(11,5), nullptr, 2);
    unsigned int rd = stoul(instr.substr(16,5), nullptr, 2);
    unsigned int shamt = stoul(instr.substr(21,5), nullptr, 2);
    unsigned int funct = stoul(instr.substr(26), nullptr, 2);

    if(ID_EX->MemRead){
        if(opcode == 0 && funct != 8){ // JR을 제외한 R format
            if(ID_EX->RegisterRd == rs || ID_EX->RegisterRd == rt){
                NumHazard = 1;
                Stall(Program_Counter, ID_EX, pc);
                return;
            }
        }else if(opcode == 9 || opcode == 0xc || opcode == 0xd || opcode == 0xb || opcode == 0x23 || opcode == 0x20){ // store, branch를 제외한 I format
            if(ID_EX->RegisterRd == rs){
                NumHazard = 1;
                Stall(Program_Counter, ID_EX, pc);
                return;
            }
        }
    }

    ID_EX->opcode = opcode;
    ID_EX->NextPc = pc;
    ID_EX->rs = Reg[rs];
    ID_EX->rt = Reg[rt];
    ID_EX->RegisterRs = rs;
    ID_EX->RegisterRt = rt;
    
    unsigned int Rs = 0;
    unsigned int Rt = 0;
    unsigned int IF_ID_funct = 0;
    unsigned int IF_ID_opcode = 0;
    string IF_ID_instr;

    if(End == 1){
        IF_ID_instr = text[(Program_Counter->NextPc-0x400000)/4];
        Rs = stoul(IF_ID_instr.substr(6,5), nullptr, 2);
        Rt = stoul(IF_ID_instr.substr(11,5), nullptr, 2);
        IF_ID_opcode = stoul(IF_ID_instr.substr(0,6), nullptr, 2);
        IF_ID_funct = stoul(IF_ID_instr.substr(26), nullptr, 2);
    }

    // I format
    unsigned int unsigned_imm = stoul(instr.substr(16),nullptr, 2);
    int imm = 0;
    if(instr[16] == '1'){
        imm = stoi(instr.substr(16),nullptr, 2) | 0xFFFF0000;
    }else{
        imm = stoi(instr.substr(16),nullptr, 2) | 0x00000000;
    }
    ID_EX->imm = imm;
    // J format
    unsigned int target = stoul(instr.substr(6), nullptr, 2);
    if(opcode == 2){ // jump
        ID_EX->jump = 1;
        string four_pc = Dectobinary(pc);
        string pc_four = four_pc.substr(0,4);
        unsigned int PC = (stoul(pc_four, nullptr, 2)) << 28;
        unsigned int target_addr = (target << 2) + PC;
        ID_EX->JumpAddress = target_addr;
    }else if(opcode == 3){ // jump
        ID_EX->jump = 1;
        ID_EX->rs = pc + 4 ;
        string four_pc = Dectobinary(pc);
        string pc_four = four_pc.substr(0,4);
        unsigned int PC = (stoul(pc_four, nullptr, 2)) << 28;
        unsigned int target_addr = (target << 2) + PC;
        ID_EX->JumpAddress = target_addr;
    }else if(opcode == 0 && funct == 8){ // JR
        int Reg_Rs = Reg[rs];
        if(EX_MEM->RegisterRd != 0 && EX_MEM->RegisterRd == rs){
            Reg_Rs = EX_MEM->ALU_OUT;
        }
        ID_EX->jump = 1;
        ID_EX->JumpAddress = Reg_Rs;
    }else if(opcode == 4 || opcode == 5){
        if(always_branch == 1){
            ATP1 = 1;
            unsigned int target = pc + 4 + 4*imm;
            ID_EX->BR_TARGET = target;
        }
        
    }else{ // jump를 제외한 instruction
        ID_EX->IsAvailable = 1;
        ID_EX->imm = imm;
        ID_EX->unsigned_imm = unsigned_imm;
        ID_EX->NextPc = pc;
        ID_EX->RegisterRd = rd;
        ID_EX->RegisterRs = rs;
        ID_EX->RegisterRt = rt;
        ID_EX->rs = Reg[rs];
        ID_EX->rt = Reg[rt];
        ID_EX->opcode = opcode;
        ID_EX->shamt = shamt;
        ID_EX->funct = funct;
    }

    if(opcode == 0 && funct != 8){ // JR을 제외한 R Format
        ID_EX->brch = 0;
        ID_EX->MemRead = 0;
        ID_EX->MemWrite = 0;   
        ID_EX->RegWrite = 1;
        ID_EX->MemtoReg = 0;
        ID_EX->jump = 0;
        ID_EX->RegisterRd = rd;
    }else if(opcode == 2 || (opcode == 0 && funct == 8)){ // JR과 J
        ID_EX->brch = 0;
        ID_EX->MemRead = 0;
        ID_EX->MemWrite = 0;
        ID_EX->RegWrite = 0;
        ID_EX->MemtoReg = 0;
        ID_EX->jump = 1;
        ID_EX->RegisterRd = 33;
    }else if(opcode == 3){ // Jal 
        ID_EX->brch = 0;
        ID_EX->MemRead = 0;
        ID_EX->MemWrite = 0;
        ID_EX->RegWrite = 1;
        ID_EX->MemtoReg = 0;
        ID_EX->jump = 1;
        ID_EX->RegisterRd = 31;
    }else if(opcode == 4 || opcode == 5){ // Beq, Bne
        ID_EX->MemRead = 0;
        ID_EX->MemWrite = 0;
        ID_EX->RegWrite = 0;
        ID_EX->MemtoReg = 0;
        ID_EX->jump = 0;
        ID_EX->RegisterRd = 33;
    }else if(opcode == 0x2b || opcode == 0x28){ // Store
        ID_EX->brch = 0;
        ID_EX->MemRead = 0;
        ID_EX->MemWrite = 1;
        ID_EX->RegWrite = 0;
        ID_EX->MemtoReg = 0;
        ID_EX->jump = 0;
        ID_EX->RegisterRd = 33;
    }else if(opcode == 0x23 || opcode == 0x20){ // Load
        ID_EX->brch = 0;
        ID_EX->MemRead = 1;
        ID_EX->MemWrite = 0;
        ID_EX->RegWrite = 1;
        ID_EX->MemtoReg = 1;
        ID_EX->jump = 0;
        ID_EX->RegisterRd = rt;
    }else{ // branch, store, load를 제외한 I format
        ID_EX->brch = 0;
        ID_EX->MemRead = 0;
        ID_EX->MemWrite = 0;
        ID_EX->RegWrite = 1;
        ID_EX->MemtoReg = 0;
        ID_EX->jump = 0;
        ID_EX->RegisterRd = rt;
    }
    
    if(End == 0){
        IF_ID->IsAvailable = 0;
    }
    
}
// 여기서 EX_MEM->instr 값을 지정해줘야함
void EX(if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb MEM_WB, unsigned int pc){
    if(ID_EX->IsAvailable == 0){
        EX_MEM->brch = 0;
        EX_MEM->jump = 0;
        EX_MEM->MemRead = 0;
        EX_MEM->MemWrite = 0;
        EX_MEM->MemtoReg = 0;
        EX_MEM->RegWrite = 0;
        EX_MEM->IsAvailable = 0;
        EX_MEM->NextPc = 0;
        EX_MEM->RegisterRd = 33;
        EX_MEM->RegisterRs= 33;
        EX_MEM->RegisterRt = 33;
        return;
    }
    unsigned int opcode = ID_EX->opcode;
    
    if(opcode == 0){
        if(ID_EX->funct == 8){
            ExecJformat(pc, ID_EX, EX_MEM);
        }
        else{
            ExecRformat(opcode, ID_EX, EX_MEM, MEM_WB);
        }
    }else if(opcode == 3){
        ExecJformat(pc, ID_EX, EX_MEM);
    }else{
        ExecIformat(opcode, ID_EX, EX_MEM, MEM_WB);
    }

    EX_MEM->MemRead = ID_EX->MemRead;
    EX_MEM->MemWrite = ID_EX->MemWrite;
    EX_MEM->RegWrite = ID_EX->RegWrite;
    EX_MEM->MemtoReg = ID_EX->MemtoReg;
    EX_MEM->jump = ID_EX->jump;
    EX_MEM->RegisterRd = ID_EX->RegisterRd;
    EX_MEM->IsAvailable = ID_EX->IsAvailable;
    EX_MEM->opcode = opcode;
    EX_MEM->NextPc = pc;
    if(IF_ID->IsAvailable == 0){
        ID_EX->IsAvailable = 0;
    }
}
void ExecRformat(unsigned int op, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb MEM_WB){
    int alu_out;
    int rs = ID_EX->rs;
    int rt = ID_EX->rt;

    int Forward1 = 0;
    int Forward2 = 0;
    if(EX_MEM->RegWrite && EX_MEM->RegisterRd != 0){
        if(EX_MEM->RegisterRd == ID_EX->RegisterRs){
            Forward1 = 0b10;
        }
        if(EX_MEM->RegisterRd == ID_EX->RegisterRt){
            Forward2 = 0b10;
        }
    }
    if(MEM_WB.RegWrite && MEM_WB.RegisterRd != 0){
        if(EX_MEM->RegisterRd != ID_EX->RegisterRs && MEM_WB.RegisterRd == ID_EX->RegisterRs){
            Forward1 = 0b01;
        }
        if(EX_MEM->RegisterRd != ID_EX->RegisterRt && MEM_WB.RegisterRd == ID_EX->RegisterRt){
            Forward2 = 0b01;
        }
    }
    if(Forward1 == 0b01){
        if(MEM_WB.MemtoReg){
            rs = MEM_WB.MEM_OUT;
        }else{
            rs = MEM_WB.ALU_OUT;
        }
    }else if(Forward1 == 0b10){
        rs = EX_MEM->ALU_OUT;
    }
    if(Forward2 == 0b01){
        if(MEM_WB.MemtoReg){
            rt = MEM_WB.MEM_OUT;
        }else{
            rt = MEM_WB.ALU_OUT;
        }
    }else if(Forward2 == 0b10){
        rt = EX_MEM->ALU_OUT;
    }
    
    if(ID_EX->funct == 0x21){ 
        alu_out = rs + rt;
    }else if(ID_EX->funct == 0x24){
        alu_out = rs & rt;
    }else if(ID_EX->funct == 39){
        alu_out = ~(rs | rt);
    }else if(ID_EX->funct == 37){
        alu_out = rs | rt;
    }else if(ID_EX->funct == 35){
        alu_out = rs - rt;
    }else if(ID_EX->funct == 43){
        if(rs < rt){
            alu_out = 1;
        }else{
            alu_out = 0;
        }
    }else if(ID_EX->funct == 0){
        alu_out = rt << ID_EX->shamt;
    }else if(ID_EX->funct == 2){
        alu_out = rt >> ID_EX->shamt;
    }
    EX_MEM->ALU_OUT = alu_out;
}

void ExecJformat(unsigned int pc, id_ex* ID_EX, ex_mem* EX_MEM){
    EX_MEM->ALU_OUT = ID_EX->rs;
    return;
};

void ExecIformat(unsigned int op, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb MEM_WB){
    int alu_out;
    int rs = ID_EX->rs;
    int rt = ID_EX->rt;

    int Forward1 = 0;
    int Forward2 = 0;
    if(EX_MEM->RegWrite && EX_MEM->RegisterRd != 0){
        if(EX_MEM->RegisterRd == ID_EX->RegisterRs){
            Forward1 = 0b10;
        }
        if(EX_MEM->RegisterRd == ID_EX->RegisterRt){
            Forward2 = 0b10;
        }
    }
    if(MEM_WB.RegWrite && MEM_WB.RegisterRd != 0){
        if(EX_MEM->RegisterRd != ID_EX->RegisterRs && MEM_WB.RegisterRd == ID_EX->RegisterRs){
            Forward1 = 0b01;
        }
        if(EX_MEM->RegisterRd != ID_EX->RegisterRt && MEM_WB.RegisterRd == ID_EX->RegisterRt){
            Forward2 = 0b01;
        }
    }
    if(Forward1 == 0b01){
        rs = MEM_WB.ALU_OUT;
    }else if(Forward1 == 0b10){
        rs = EX_MEM->ALU_OUT;
    }

    if(Forward2 == 0b01){
        rt = MEM_WB.ALU_OUT;
    }else if(Forward2 == 0b10){
        rt = EX_MEM->ALU_OUT;
    }

    if(op == 9){    
        alu_out = rs + ID_EX->imm;
    }else if(op == 0xc){
        alu_out = rs & ID_EX->unsigned_imm;
    }else if(op == 4){
        if(rs == rt){
            EX_MEM->brch = 1;
            unsigned int target = ID_EX->NextPc + 4 + 4*(ID_EX->imm);
            EX_MEM->BR_TARGET = target;
        }else{
            EX_MEM->brch = 0;
        }
    }else if(op == 5){
        if(rs != rt){
            EX_MEM->brch = 1;
            unsigned int target = ID_EX->NextPc + 4 + 4*(ID_EX->imm);
            EX_MEM->BR_TARGET = target;
        }else{
            EX_MEM->brch = 0;
        }    
    }else if(op == 0xf){
        alu_out = ID_EX->imm << 16;
    }else if(op == 0x23){
        alu_out = rs + ID_EX->imm;
    }else if(op == 0x20){
        alu_out = rs + ID_EX->imm;
    }else if(op == 0xd){
        alu_out = rs | ID_EX->unsigned_imm;
    }else if(op == 0xb){
        if(rs < ID_EX->imm){
            alu_out = 1;
        }else{
            alu_out = 0;
        }
    }else if(op == 0x2b){
        EX_MEM->rt = rt;
        alu_out = rs + ID_EX->imm;
    }else if(op == 0x28){
        alu_out = rs + ID_EX->imm;
    }
    if(op != 4 & op != 5){
        EX_MEM->ALU_OUT = alu_out;
    }
}


// instr 0 : R, 1 : I format, 2: Jump , 이때 JR도 jump라고 하자.
// 여기서 load byte의 경우에 sign extension을 해줘야하나?
void MEM(if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb* MEM_WB, unsigned int pc){
    if(EX_MEM->IsAvailable == 0){
        MEM_WB->brch = 0;
        MEM_WB->MemRead = 0;
        MEM_WB->MemWrite = 0;
        MEM_WB->RegWrite = 0;
        MEM_WB->MemtoReg = 0;
        MEM_WB->jump = 0;
        MEM_WB->IsAvailable = 0;
        MEM_WB->NextPc = 0;
        MEM_WB->RegisterRd = 33;
        MEM_WB->RegisterRs= 33;
        MEM_WB->RegisterRt = 33;
        return;
    }
    MEM_WB->brch = EX_MEM->brch;
    MEM_WB->MemRead = EX_MEM->MemRead;
    MEM_WB->MemWrite = EX_MEM->MemWrite;
    MEM_WB->RegWrite = EX_MEM->RegWrite;
    MEM_WB->MemtoReg = EX_MEM->MemtoReg;
    MEM_WB->jump = EX_MEM->jump;
    MEM_WB->RegisterRd = EX_MEM->RegisterRd;
    MEM_WB->IsAvailable = EX_MEM->IsAvailable;
    MEM_WB->NextPc = pc;
    MEM_WB->opcode = EX_MEM->opcode;
    unsigned int opcode = EX_MEM->opcode;

    if(EX_MEM->RegWrite && EX_MEM->MemtoReg == 0){ // R format + Load/store/branch 제외한 I
        MEM_WB->ALU_OUT = EX_MEM->ALU_OUT;

    }else if(EX_MEM->MemRead == 1){ // Load
        int address = EX_MEM->ALU_OUT;
        if(opcode == 0x23){
            MEM_WB->MEM_OUT = (Memory[address] << 24) + (Memory[address + 1] << 16) + (Memory[address + 2] << 8) + (Memory[address + 3]);
        }else{
            string byte = bitset<8> (Memory[address]).to_string();
            int Byte = 0;
            if(byte[0] == '1'){
                Byte = stoi(byte,nullptr, 2) | 0xFFFF0000;
            }else{
                Byte = stoi(byte,nullptr, 2) | 0x00000000;
            }
            MEM_WB->MEM_OUT = Byte;
        }
    }
    else if(EX_MEM->MemWrite == 1){ // Store 
        string four_byte;
        if(MEM_WB->MemRead){ // Load -> Store 인 경우에는 forwarding 해주면 됨 !
            if(MEM_WB->RegisterRd == EX_MEM->RegisterRs){
                four_byte = Dectobinary(MEM_WB->MEM_OUT);
            }else{
                four_byte = Dectobinary(EX_MEM->rt);
            }
        }else{
            four_byte = Dectobinary(EX_MEM->rt);
        }
        
        if(opcode == 0x2b){
            for(int i = 0;i < 4; i++){
                Memory[EX_MEM->ALU_OUT+i] = stoi(four_byte.substr(8*i, 8), nullptr, 2);
            }
        }else if(opcode == 0x28){
            int byte_data = stoi(four_byte.substr(24,8),nullptr,2);
            Memory[EX_MEM->ALU_OUT] = byte_data;
        }
    }else if(opcode == 4){ // Beq
        if(always_branch == 0 && EX_MEM->brch == 1){
            MEM_WB->BR_TARGET = EX_MEM->BR_TARGET;
            ANTP = 1;
            return;
        }else if(always_branch == 1 && EX_MEM->brch == 0){
            ATP2 = 1;
        }
    }else if(opcode == 5){ // Bne
        if(always_branch == 1 && EX_MEM->brch == 0){
            ATP2 = 1;
            return;
        }else if(always_branch == 0 && EX_MEM->brch == 1){
            MEM_WB->BR_TARGET = EX_MEM->BR_TARGET;
            ANTP = 1;
            return;
        }
    }
    if(ID_EX->IsAvailable == 0){
        EX_MEM->IsAvailable = 0;
    }
    
}

void WB(if_id* IF_ID, id_ex* ID_EX, ex_mem* EX_MEM, mem_wb* MEM_WB, unsigned int pc){
    if(MEM_WB->IsAvailable == 0){
        return;
    }
    ProgramOver += 1;
    if(MEM_WB->RegWrite){
        int write_data;
        if(MEM_WB->MemtoReg){
            write_data = MEM_WB->MEM_OUT;
        }else{
            write_data = MEM_WB->ALU_OUT;
        }
        Reg[MEM_WB->RegisterRd] = write_data;
    }
    if(EX_MEM->IsAvailable == 0){
        MEM_WB->IsAvailable = 0;
    }
}

int main(int argc, const char* argv[]){
	string filename = argv[argc-1];
    ifstream object_file(filename);
    unsigned int PC = 0x400000;
    print_reg = 0;
	instruction_limit = false;
    address1 = 0;
    address2 = 0;
    unsigned int num_instruction = 0;
    unsigned int n = 0;

    for(int i = 1; i < argc; i++){

        if(strcmp(argv[i], "-atp") == 0){
            always_branch = 1;
            continue;
        }else if(strcmp(argv[i], "-antp") == 0){
            always_branch = 0;
            continue;
        }

        if(strcmp(argv[i], "-m") == 0){
            int split = findColon(argv[i+1]);
            string m = argv[i+1];
            address1 = stoul(m.substr(0,split),nullptr,16);
            address2 = stoul(m.substr(split+1),nullptr,16);
            continue;
        }

        if(strcmp(argv[i], "-d") == 0){
            print_reg = 1;
            continue;
        }

        if(strcmp(argv[i], "-p") == 0){
            print_pipe = 1;
            continue;
        }

        if(strcmp(argv[i], "-n") == 0){
            num_instruction = stoul(argv[i+1]);
            instruction_limit = true;
            continue;
        }
    }

    if(always_branch == 2){
        printf("ERROR : Branch Prediction command is not included !");
        exit(1);
    }

    for(int i = 0; i<32; i++){
        Reg[i] = 0;
    }
    
    for(int id = 0 ; id < (address2 - address1); id++){
        Memory[address1 + id] = 0;
    }
    
    store_memory(text_init_addr, 0);
    store_memory(data_init_addr, 0);
    pc* Program_Counter = new pc();
    Program_Counter->NextPc = 0x400000;
    if_id* IF_ID = new if_id();
    IF_ID->instr = 0x01;
    IF_ID->IsAvailable = 0;
    IF_ID->NextPc = 0;
    id_ex* ID_EX = new id_ex{0,};
    ex_mem* EX_MEM = new ex_mem{0,};
    mem_wb* MEM_WB = new mem_wb{0,};

    string str;
    int i = 0;
    int text_to_data = 0;
    unsigned int text_size;
    unsigned int data_size;
    unsigned int text_count = 0;
    unsigned int data_count = 0;

	while(!object_file.eof()){
		getline(object_file, str);
        if(str == "") continue;
		stringstream ss(str);
        
		if(i == 0){
            text_size = stoul(str, nullptr, 16)/4;
            i++;
            continue;
        }
        else if(i == 1){
            data_size = stoul(str, nullptr, 16)/4;
            i++;
            continue;
        }

        if(text_to_data < text_size){
            string four_byte = bitset<32> (stoul(str, nullptr, 16)).to_string();
            for(int idx = 0; idx <4; idx++){
                Memory[text_init_addr + 4*text_count + idx] = stoul(four_byte.substr(8*idx, 8), nullptr, 2);
            }
            text.push_back(Dectobinary(stoul(str,nullptr,16)));
            text_to_data += 1;
            text_count += 1;
        }
        else{
            string four_byte = bitset<32> (stoul(str, nullptr, 16)).to_string();
            for(int idx = 0; idx <4; idx++){
                Memory[data_init_addr + 4*data_count + idx] = stoul(four_byte.substr(8*idx, 8), nullptr, 2);
            }
            data_count += 1;
        }
	}

    if(i == 0){   // 빈파일이 들어오는 경우
        PrintinReg(PC);
        printf("\n");
        if(address1 != 0){
            PrintinMem(address1, address2);
        }
        exit(1);
    }

    if(instruction_limit){   // -n이 입력된 경우 
        n = num_instruction;
    }
    else{  // -n이 입력되지 않은 경우
        n = text_size;}
    if(n > 0){
        Pipeline(Program_Counter, IF_ID, ID_EX ,EX_MEM, MEM_WB, n, 1, 1);
    }else{
        End = 1;
        if(print_pipe){
            printf("===== Cycle: %d",0);
            cout<< " =====\n" << endl;
            PrintinPipe(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            printf("\n");
        }
        if(print_reg){
            PrintinReg(PC);
        }
        if(address1 != 0){
            PrintinMem(address1, address2);
        }
    }
    
    return 0;
}
