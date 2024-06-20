#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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
int print_reg = 0;
unsigned int address1;
unsigned int address2;
bool instruction_limit = false;
map<unsigned int, int> Reg;
map<unsigned int, int> Memory;
vector<string> text;

int findColon(string str);
void PrintinReg(int pc);
void PrintinMem(unsigned int addr1, unsigned int addr2);
void instrExec(string instr, unsigned int pc, unsigned int n, unsigned int cnt);
void ExecRformat(string instr, unsigned int pc, unsigned int n, unsigned int cnt);
void ExecIformat(string instr, unsigned int pc, unsigned int n, unsigned int cnt);
void ExecJformat(string instr, unsigned int pc, unsigned int n, unsigned int cnt);
string Dectobinary(unsigned int decimal);
void store_memory(unsigned pc, unsigned value);

void ExecRformat(string instr, unsigned int pc, unsigned int n, unsigned int cnt){
    unsigned int rs = stoul(instr.substr(6,5), nullptr, 2);
    unsigned int rt = stoul(instr.substr(11,5), nullptr, 2);
    unsigned int rd = stoul(instr.substr(16,5), nullptr, 2);
    unsigned int shamt = stoul(instr.substr(21,5), nullptr, 2);
    unsigned int funct = stoul(instr.substr(26), nullptr, 2);
    if(funct == 33){
        Reg[rd] = Reg[rs] + Reg[rt];
    }else if(funct == 36){
        Reg[rd] = Reg[rs] & Reg[rt];
    }else if(funct == 8){
        unsigned int instrPos = (Reg[rs] - 0x400000)/4;
        if(print_reg == 1){
            PrintinReg(Reg[rs]);
            if(address1 != 0){
                PrintinMem(address1, address2);
            }
        }
        if(instrPos < text.size()){
            instrExec(text[instrPos], Reg[rs], n, cnt + 1);
        }
        
    }else if(funct == 39){
        Reg[rd] = ~(Reg[rs] | Reg[rt]);
    }else if(funct == 37){
        Reg[rd] = Reg[rs] | Reg[rt];
    }else if(funct == 35){
        Reg[rd] = Reg[rs] - Reg[rt];
    }else if(funct == 43){
        if(rs < rt){
            Reg[rd] = 1;
        }else{
            Reg[rd] = 0;
        }
    }else if(funct == 0){
        Reg[rd] = Reg[rt] << shamt;
    }else if(funct == 2){
        Reg[rd] = Reg[rt] >> shamt;
    }
    if(funct != 8){
        unsigned int index = (pc + 4 - 0x400000)/4;
        if(print_reg == 1){
            PrintinReg(pc+4);
            if(address1 != 0){
                PrintinMem(address1, address2);
            }
        }
        if(index < text.size() ){    
            instrExec(text[index], pc + 4, n, cnt + 1);
        }
    }
    
}

void ExecIformat(string instr, unsigned int pc, unsigned int n, unsigned int cnt){
    unsigned int op = stoul(instr.substr(0,6),nullptr, 2);
    unsigned int rs = stoul(instr.substr(6,5),nullptr, 2);
    unsigned int rt = stoul(instr.substr(11,5),nullptr, 2);
    unsigned int unsigned_imm = stoul(instr.substr(16),nullptr, 2);
    int imm = 0;
    if(instr[16] == '1'){
        imm = stoi(instr.substr(16),nullptr, 2) | 0xFFFF0000;
    }else{
        imm = stoi(instr.substr(16),nullptr, 2) | 0x00000000;
    }
    if(op == 9){    
        int sum = Reg[rs] + imm;
        Reg[rt] = sum;
    }else if(op == 0xc){
        Reg[rt] = Reg[rs] & unsigned_imm;
    }else if(op == 4){
        if(Reg[rs] == Reg[rt]){
            unsigned int target = pc + 4 + 4*imm;
            unsigned int backtoaddress = (target - 0x400000)/4;
            pc = target;
            if(print_reg == 1){
                PrintinReg(pc);
                if(address1 != 0){
                    PrintinMem(address1, address2);
                }
            }
            if(backtoaddress < text.size()){
                instrExec(text[backtoaddress], pc, n, cnt + 1);
            }
        }else{
            unsigned int index = (pc + 4 - 0x400000)/4;
            if(print_reg){
                PrintinReg(pc);
                if(address1 != 0){
                    PrintinMem(address1, address2);
                }
            }
            if(index < text.size()){
                instrExec(text[index], pc + 4, n, cnt);
            }
        }
    }else if(op == 5){
        if(Reg[rs] != Reg[rt]){
            unsigned int target = pc + 4 + 4*imm;
            unsigned int backtoaddress = (target - 0x400000)/4;
            pc = target;
            if(print_reg == 1){
                PrintinReg(pc);
                if(address1 != 0){
                    PrintinMem(address1, address2);
                }
            }
            if(backtoaddress < text.size()){
                instrExec(text[backtoaddress], pc, n, cnt + 1);
            }
        }else{
            unsigned int index = (pc + 4 - 0x400000)/4;
            if(print_reg){
                PrintinReg(pc);
                if(address1 != 0){
                    PrintinMem(address1, address2);
                }
            }
            if(index < text.size()){
                instrExec(text[index], pc + 4, n, cnt + 1);
            }
        }
        
    }else if(op == 0xf){
        Reg[rt] = imm << 16;
    }else if(op == 0x23){
        Reg[rt] = (Memory[Reg[rs]+imm] << 24) + (Memory[Reg[rs]+imm+1] << 16) + (Memory[Reg[rs]+imm+2] << 8) + (Memory[Reg[rs]+imm+3]);
    }else if(op == 0x20){
        string byte = bitset<8> (Memory[Reg[rs]+imm]).to_string();
        int four_byte = 0;
        if(byte[0] == '1'){
            four_byte = stoi(byte,nullptr, 2) | 0xFFFF0000;
        }else{
            four_byte = stoi(byte,nullptr, 2) | 0x00000000;
        }
        Reg[rt] = four_byte;
    }else if(op == 0xd){
        Reg[rt] = Reg[rs] | unsigned_imm;
    }else if(op == 0xb){
        if(Reg[rs] < imm){
            Reg[rt] = 1;
        }else{
            Reg[rt] = 0;
        }
    }else if(op == 0x2b){
        string four_byte = Dectobinary(Reg[rt]);
        for(int i = 0;i < 4; i++){
            Memory[Reg[rs]+imm+i] = stoi(four_byte.substr(8*i, 8), nullptr, 2);
        }
    }else if(op == 0x28){
        string store_byte = bitset<32> (Reg[rt]).to_string();
        int byte_data = stoi(store_byte.substr(24,8),nullptr,2);
        Memory[Reg[rs]+imm] = byte_data;
    }
    if(op != 5 && op != 4){
        unsigned int index = (pc + 4 - 0x400000)/4;
        if(print_reg){
            PrintinReg(pc+4);
            if(address1 != 0){
                PrintinMem(address1, address2);
            }
        }
        if(index < text.size()){
            instrExec(text[index], pc + 4, n, cnt + 1);
        }
    }
    
}

void ExecJformat(string instr, unsigned int pc, unsigned int n, unsigned int cnt){
    unsigned int target = stoul(instr.substr(6), nullptr, 2);
    if(stoul(instr.substr(0,6), nullptr, 2) == 2){
        string four_pc = Dectobinary(pc+4);
        string pc_four = four_pc.substr(0,4);
        unsigned int PC = (stoul(pc_four, nullptr, 2)) << 28;
        unsigned int target_addr = (target << 2) + PC;
        unsigned int targetPos = (target_addr - 0x400000)/4;
        if(print_reg){
            PrintinReg(target_addr);
            if(address1 != 0){
                PrintinMem(address1, address2);
            }
        }
        if(targetPos < text.size()){
            instrExec(text[targetPos], target_addr, n, cnt + 1);
        }
    }else{
        Reg[31] = pc + 4;
        string four_pc = Dectobinary(pc+4);
        string pc_four = four_pc.substr(0,4);
        unsigned int PC = (stoul(pc_four, nullptr, 2)) << 28;
        unsigned int target_addr = (target << 2) + PC;
        unsigned int targetPos = (target_addr - 0x400000)/4;
        if(print_reg){
            PrintinReg(target_addr);
            if(address1 != 0){
                PrintinMem(address1, address2);
            }
        }
        if(targetPos < text.size()){
            instrExec(text[targetPos], target_addr, n, cnt + 1);
        }
    }
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

void instrExec(string instr, unsigned int pc, unsigned int num_limit, unsigned int cnt){
    string op = instr.substr(0,6);
    unsigned opcode = stoul(op, nullptr, 2);
    if(instruction_limit){
        if(cnt <= num_limit){
            if(opcode == 0){
                ExecRformat(instr, pc, num_limit, cnt);
            }else if(opcode == 2 || opcode == 3){
                ExecJformat(instr, pc, num_limit, cnt);
            }else{
                ExecIformat(instr, pc, num_limit, cnt);
            }
        }
    }
    else{
        if(opcode == 0){
            ExecRformat(instr, pc, num_limit, cnt);
        }else if(opcode == 2 || opcode == 3){
            ExecJformat(instr, pc, num_limit, cnt);
        }else{
            ExecIformat(instr, pc, num_limit, cnt);
        }
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

        if(strcmp(argv[i], "-n") == 0){
            num_instruction = stoul(argv[i+1]);
            instruction_limit = true;
            continue;
        }
    }

    for(int i = 0; i<32; i++){
        Reg[i] = 0;
    }
    
    for(int id = 0 ; id < (address2 - address1); id++){
        Memory[address1 + id] = 0;
    }
    store_memory(text_init_addr, 0);
    store_memory(data_init_addr, 0);

    
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
        cout << str << endl;
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
        instrExec(text[0], PC, n, 1);
    }else{
        if(print_reg == 1 && address1 != 0){
            PrintinMem(address1,address2);
        }
    }
    
    // d가 안 들어온 경우
    if(print_reg == 0){      // -d 입력이 없는 경우에는 명령어를 다 실행한 후 출력 
        PrintinReg(PC+4*n);
        if(address1 != 0){
            PrintinMem(address1,address2);
        }
    }
    
    
    

    return 0;
}
