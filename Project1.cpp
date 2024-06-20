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
using namespace std;

#define text_init_addr 0x00400000
#define data_init_addr 0x10000000

map<string, string> label_address;
map<string, string> text_address;
map<string, tuple<int, int>> instructions;

vector<string> word_to_hex;
vector<string> text_to_hex;

string bintohex(string bin);
int findbracket(string address);
int HexorDec(string str);
string DecToHex(unsigned int dec);
void label_addr(vector<vector<string>> word);
void text_addr(vector<vector<string>> text);
string check_format(vector<string> inst, int PC);
void store_word(vector<vector<string>> word);
void store_text(vector<vector<string>> text);
string rformat(vector<string> txt);
string iformat(vector<string> txt,int PC);
string jformat(vector<string> txt);


string bintohex(string bin){
	bitset<32> bit(bin);
	stringstream ss;
	ss << hex << bit.to_ulong();
	return ss.str();
}

int findbracket(string address){
	for(int i = 0; i<address.length();i++){
		if(address[i] == '('){
			return i;
		}
	}
	return 0;
} 
int HexorDec(string str){
	if(str[1] == 'x'){
		return 1;
	}
	else{
		return 0;
	}
}

string DecToHex(unsigned int dec){
	ostringstream ss;
	ss << hex << dec;
	string result = "0x"; 
	result += ss.str();
	return result;
}
void label_addr(vector<vector<string>> word){
	unsigned int numofdata = 0;
	for(int i = 0; i<word.size();i++){	
		if(word[i][0][word[i][0].length()-1] == ':'){
			label_address.insert({word[i][0].substr(0,word[i][0].length()-1),DecToHex(data_init_addr+4*numofdata)});
		}
		if(word.size() > 1){
			numofdata += 1;
		}
	}
	
}

void text_addr(vector<vector<string>> text){
	unsigned int numofdata = 0;
	for(int i = 0; i<text.size(); i++){
		if(text[i][0][text[i][0].length()-1] == ':'){
			text_address.insert({text[i][0].substr(0,text[i][0].length()-1),DecToHex(text_init_addr+4*numofdata)});
			if(text[i].size()>1){
				if(text[i][1] == "la"){
					numofdata += 1;
				}
			}  
		}else if( text[i][0] == "la" || text[i][1] == "la"){
			if(label_address[text[i][text[i].size()-1]].substr(6,4) != "0000"){
				numofdata += 1;
			}
		}
		if(text[i].size() > 1){
			numofdata += 1;
		}
	}
}

string check_format(vector<string> inst, int PC){
	if(get<0>(instructions[inst[0]]) == 0){
		return(rformat(inst));
	}else if(get<0>(instructions[inst[0]]) == 2 || get<0>(instructions[inst[0]]) == 3){
		return(jformat(inst));
	}
	else{
		return(iformat(inst, PC));
	}
	
}

void store_word(vector<vector<string>> word){
	for(int i = 0; i<word.size();i++){
		if (word[i][0] == ".word"){
			if(word[i][1].substr(0,2) != "0x"){
				word_to_hex.push_back(DecToHex(static_cast<unsigned int>(stoul(word[i][1]))));
			}
			else{
				string val = word[i][1].substr(2,word[i][1].length()-2);
				transform(val.begin(), val.end(), val.begin(), ::tolower);
				word_to_hex.push_back("0x"+val);
			}
		}
		else if(word[i][1] == ".word"){
			if(word[i][2].substr(0,2) != "0x"){
				word_to_hex.push_back(DecToHex(static_cast<unsigned int>(stoul(word[i][2]))));
			}
			else{
				string val = word[i][2].substr(2,word[i][2].length()-2);
				transform(val.begin(), val.end(), val.begin(), ::tolower);
				word_to_hex.push_back("0x"+val);
			}
		}
	}
}

void store_text(vector<vector<string>> text){
	int PC = text_init_addr;
	for(int i=0; i<text.size();i++){
		vector<string> only_instruction;
		if(text[i].size() == 1){
			continue;
		}
		if(text[i][0][text[i][0].size()-1] == ':' && text[i].size() != 1){
			for(int j = 1; j<text[i].size();j++){
				if(label_address.count(text[i][j])){
					only_instruction.push_back(label_address[text[i][j]]);
				}
				else if(text_address.count(text[i][j])){
					only_instruction.push_back(text_address[text[i][j]]);
				}else{
					only_instruction.push_back(text[i][j]);
				}
			}
			if(only_instruction[0] == "la"){
				vector<string> la_1 = {"lui", only_instruction[1],only_instruction[2].substr(0,6)};
				text_to_hex.push_back(check_format(la_1, PC));
				if(only_instruction[2].substr(6,4) != "0000"){
					PC += 4;
					vector<string> la_2 = {"ori", only_instruction[1], only_instruction[1], "0x"+only_instruction[2].substr(6,4)};
					text_to_hex.push_back(check_format(la_2, PC));
				}
			}
			else{
				text_to_hex.push_back(check_format(only_instruction, PC));
			}
		}else{
			for(int j = 0; j<text[i].size();j++){
				if(label_address.count(text[i][j])){
					only_instruction.push_back(label_address[text[i][j]]);
				}
				else if(text_address.count(text[i][j])){
					only_instruction.push_back(text_address[text[i][j]]);
				}else{
					only_instruction.push_back(text[i][j]);
				}
				
			}

			if(only_instruction[0] == "la"){
				vector<string> la_1 = {"lui", only_instruction[1], only_instruction[2].substr(0,6)};
				text_to_hex.push_back(check_format(la_1, PC));
				if(only_instruction[2].substr(6,4) != "0000"){	
					PC += 4;
					vector<string> la_2 = {"ori", only_instruction[1], only_instruction[1], "0x"+only_instruction[2].substr(6,4)};
					text_to_hex.push_back(check_format(la_2, PC));
				}
			}
			else{
				text_to_hex.push_back(check_format(only_instruction, PC));
			}
		}
		PC += 4;
		
		
	}
}


string rformat(vector<string> txt){
	string first = bitset<6>(0).to_string();
	string res;
	if(txt[0] == "jr"){
		unsigned int sec = stoi(txt[1].substr(1,txt[1].length()-1));
		string second = bitset<5>(sec).to_string();
		string third = bitset<15>(0).to_string();
		string fourth = bitset<6>(8).to_string();
		res = first + second + third + fourth;
	}else if(txt[0] == "sll" || txt[0] =="srl"){
		string second = bitset<5>(0).to_string();
		unsigned int rd = stoi(txt[1].substr(1,txt[1].length()-2));
		unsigned int rt = stoi(txt[2].substr(1,txt[2].length()-2));
		unsigned int shamt;
		if(HexorDec(txt[3])){
			shamt = stoi(txt[3],nullptr,16);
		}else{
			shamt = stoi(txt[3]);
		} 
		string third = bitset<5>(rt).to_string();
		string fourth = bitset<5>(rd).to_string();
		string fifth = bitset<5>(shamt).to_string();
		string sixth;
		if(txt[0] == "sll"){
			sixth = bitset<6>(0).to_string();
		}
		else{
			sixth = bitset<6>(2).to_string();
		}
		res = first + second + third + fourth + fifth + sixth;
	}else{
		unsigned int rd = stoi(txt[1].substr(1,txt[1].length()-2));
		unsigned int rs = stoi(txt[2].substr(1,txt[2].length()-2));
		unsigned int rt = stoi(txt[3].substr(1,txt[3].length()-1));
		string second = bitset<5>(rs).to_string();
		string third = bitset<5>(rt).to_string();
		string fourth = bitset<5>(rd).to_string();
		string fifth = bitset<5>(0).to_string();
		unsigned long funct;
		if(HexorDec(to_string(get<1>(instructions[txt[0]])))){
			funct = stoi(to_string(get<1>(instructions[txt[0]])),nullptr,16);
		}
		else{
			funct = stoi(to_string(get<1>(instructions[txt[0]])));
		}
		
		string sixth = bitset<6>(funct).to_string();
		res = first + second + third + fourth + fifth + sixth;
	}
	unsigned long decimal = stoi(res, nullptr, 2);
	string hexform = DecToHex(decimal);
	return hexform;
}
string iformat(vector<string> txt,int PC){
	string res;
	string first;
	unsigned int op;
	if(HexorDec(to_string(get<0>(instructions[txt[0]])))){
		op = stoi(to_string(get<0>(instructions[txt[0]])),nullptr,16);
	}else{
		op = stoi(to_string(get<0>(instructions[txt[0]])));
	}
	first = bitset<6>(op).to_string();
	unsigned int imm;
	unsigned int offset;
	unsigned int rs;
	unsigned int rt;
	string second;
	string third;
	string fourth;
	if(txt[0] == "andi"){
		if(HexorDec(txt[3])){
			imm = stoi(txt[3],nullptr,16);
		}
		else{
			imm = stoi(txt[3]);
		}
		rt = stoi(txt[1].substr(1,txt[1].length()-2));
		rs = stoi(txt[2].substr(1,txt[2].length()-2));
		second = bitset<5>(rs).to_string();
		third = bitset<5>(rt).to_string();
		fourth = bitset<16>(imm).to_string();
        res = first + second + third + fourth;
	}else if(txt[0] == "beq" || txt[0] == "bne"){
		rt = stoi(txt[2].substr(1,txt[2].length()-2));
		rs = stoi(txt[1].substr(1,txt[1].length()-2));
		second = bitset<5>(rs).to_string();
		third = bitset<5>(rt).to_string();
		if(HexorDec(txt[3])){
			offset = (stoi(txt[3],nullptr,16)-PC-4)/4;
		}else{
			offset = (stoi(txt[3])-PC-4)/4;
		}
		fourth = bitset<16>(offset).to_string();
		res = first + second + third + fourth;
	}else if(txt[0] == "lui"){
		second = bitset<5>(0).to_string();
		if(HexorDec(txt[2])){
			imm = stoi(txt[2],nullptr,16);
		}
		else{
			imm = stoi(txt[2]);
		}
		rt = stoi(txt[1].substr(1,txt[1].length()-2));
		third = bitset<5>(rt).to_string();
		fourth = bitset<16>(imm).to_string();
		res = first + second + third + fourth;
	}else if(txt[0] == "lw" || txt[0] == "lb" || txt[0] == "sw" || txt[0] == "sb"){
		int i; 
		i = findbracket(txt[2]);
		rs = stoi(txt[2].substr(i+2,txt[2].length()-(i+3)));
		offset = stoi(txt[2].substr(0,i));
		rt = stoi(txt[1].substr(1,txt[1].length()-2));
		second = bitset<5>(rs).to_string();
		third = bitset<5>(rt).to_string();
		fourth = bitset<16>(offset).to_string();
		res = first + second + third + fourth;
	}else if(txt[0] == "ori" || txt[0] =="sltiu"){
		rt = stoi(txt[1].substr(1,txt[1].length()-2));
		rs = stoi(txt[2].substr(1,txt[2].length()-2));
		second = bitset<5>(rs).to_string();
		third = bitset<5>(rt).to_string();
		if(HexorDec(txt[3])){
			imm = stoi(txt[3],nullptr,16);
		}
		else{
			imm = stoi(txt[3]);
		}
		fourth = bitset<16>(imm).to_string();
		res = first + second + third + fourth;
	}
	else{
		rt = stoi(txt[1].substr(1,txt[1].length()-2));
		rs = stoi(txt[2].substr(1,txt[2].length()-2));
		second = bitset<5>(rs).to_string();
		third = bitset<5>(rt).to_string();
		if(HexorDec(txt[3])){
			imm = stoi(txt[3],nullptr,16);
		}
		else{
			imm = stoi(txt[3]);
		}
		fourth = bitset<16>(imm).to_string();
		res = first + second + third + fourth;
	}
	unsigned long decimal = stoul(res, nullptr, 2);
	string hexform = DecToHex(decimal);
	return hexform;
}

string jformat(vector<string> txt){
	string first;
	if(txt[0] != "jal"){
		first = bitset<6>(2).to_string();
	}else{
		first = bitset<6>(3).to_string();
	}
	unsigned int target;
	if(HexorDec(txt[1])){
		target = stoi(txt[1],nullptr,16)/4;
	}
	else{
		target = stoi(txt[1])/4;
	}

	string res = bitset<26>(target).to_string();	
	res = first + res;
	unsigned int decimal = stoi(res, nullptr, 2);
	string hexform = DecToHex(decimal);
	return hexform;
}


int main(int argc, char* argv[]){
	if(argc != 2){
		printf("Input file is only one assembly file !");
		exit(1);
	}
	instructions.insert({{"addiu",make_tuple(9,0)},{"addu",make_tuple(0,0x21)},{"and",make_tuple(0,0x24)},{"andi",make_tuple(0xc,0)},{"beq",make_tuple(4,0)},{"bne",make_tuple(5,0)},
	{"j",make_tuple(2,0)},{"jal",make_tuple(3,0)},{"jr",make_tuple(0,8)},{"lui",make_tuple(0xf,0)},{"lw",make_tuple(0x23,0)},{"lb",make_tuple(0x20,0)},{"nor",make_tuple(0,0x27)},
	{"or",make_tuple(0,0x25)},{"ori",make_tuple(0xd,0)},{"sltiu",make_tuple(0xb,0)},{"sltu",make_tuple(0,0x2b)},{"sll",make_tuple(0,0)},{"srl",make_tuple(0,2)},
	{"sw",make_tuple(0x2b,0)},{"sb",make_tuple(0x28,0)},{"subu",make_tuple(0,0x23)}});

	string filename = argv[1];
	
	ifstream assembly_file(filename);
	
	if(!assembly_file.is_open()){
		printf("Failed to open the file");
		exit(1);
	}
    vector<vector<string>> word;
	vector<vector<string>> text;

    int decision = 0;
	string str;
	while(!assembly_file.eof()){
		getline(assembly_file,str);
	
		if(str == "") continue;
        
		stringstream ss(str);
		string txt;
		
		vector<string> assembly;
		while(ss>>txt){
			assembly.push_back(txt);
		}
		if(assembly[0] == ".data"){
			decision = 1;
			continue;
		}
		else if(assembly[0] == ".text"){
			decision = 0;
			continue;
		}
		if(decision){
			word.push_back(assembly);
		}
		else{
			text.push_back(assembly);
		}
	}
	// II. Label 혹은 instruction에 해당하는 주소 저장
	label_addr(word);
	text_addr(text);

	// III, IIII. 
	store_word(word);
	store_text(text);

	assembly_file.close();
	/// IIIII.
	ofstream fout;
	fout.open(filename.substr(0,filename.length()-2)+".o",ios::out | ios::binary);

	int word_section_size = 4*word_to_hex.size();
	int text_section_size = 4*text_to_hex.size();

	if(fout.is_open()){
		fout.write(DecToHex(text_section_size).c_str(),DecToHex(text_section_size).size());
		fout.write("\n",1);
		fout.write(DecToHex(word_section_size).c_str(),DecToHex(word_section_size).size());
		fout.write("\n",1);

		for(int j = 0; j < text_to_hex.size() ; j++){
			fout.write(text_to_hex[j].c_str(),text_to_hex[j].size());
			fout.write("\n",1);
		}
		for(int i = 0; i < word_to_hex.size() ; i++){
			fout.write(word_to_hex[i].c_str(),word_to_hex[i].size());
			fout.write("\n",1);
		}
	}
	fout.close();
	
	return 0;



}


