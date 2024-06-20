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
#include <random>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>


using namespace std;
unsigned int L1_READ_HIT;
unsigned int L1_WRITE_HIT;
unsigned int L1_READ_MISS;
unsigned int L1_WRITE_MISS;
unsigned int L2_READ_HIT;
unsigned int L2_WRITE_HIT;
unsigned int L2_READ_MISS;
unsigned int L2_WRITE_MISS;
unsigned int L1_CLEAN_EVICT;
unsigned int L1_DIRTY_EVICT;
unsigned int L2_CLEAN_EVICT;
unsigned int L2_DIRTY_EVICT;
typedef struct block{
    unsigned int DIRTY;
    unsigned int TAG;
    unsigned int INDEX;
    unsigned int BLOCK_SIZE;
    time_t RECENT_TIME;
    string START_ADDRESS;
}Block;

typedef struct cache{
    unsigned int CAPACITY;
    unsigned int ASSOCIATIVITY;
    unsigned int REPLACEMENT_POLICY;
    map<unsigned int, vector<Block*>> SET;
}Cache;

unsigned int Powerof2(unsigned int number);
string Dectobinary(unsigned int decimal);
void L1_ACCESS(Cache* L1_CACHE, Cache* L2_CACHE, string address, unsigned int BLOCKSIZE, unsigned int mode);
Block* L2_ACCESS(Cache* L1_CACHE, Cache* L2_CACHE, string address, unsigned int BLOCKSIZE, unsigned int mode);
void printBinary(unsigned int n);
void BlockintoL1Cache(Cache* L1_CACHE, Cache* L2_CACHE, Block* BLOCK);
void BlockintoL2Cache(Cache* L1_CACHE, Cache* L2_CACHE, Block* BLOCK);


void printBinary(unsigned int n) {
    printf("%u",n);
    for (int i = sizeof(n) * 8 - 1; i >= 0; i--) {
        putchar((n & (1 << i)) ? '1' : '0');
    }
    cout << "\n";
}
// 메모리 주소의 범위가 얼마나 될지 모르니 일단 크게 잡아놓아보자.. 앞은 0으로 채워지니까
string Dectobinary(unsigned int decimal){
    string bin = bitset<64> (decimal).to_string();
    return bin;
}

// 2's Power
unsigned int Powerof2(unsigned int number){
    unsigned int Totalcount = 0;
    while(number != 1){
        number = number/2;
        Totalcount++;
    }
    return Totalcount;
}

// Can apply to either L1 or L2
void L1_ACCESS(Cache* L1_CACHE, Cache* L2_CACHE, string address, unsigned int BLOCKSIZE, unsigned int mode){
    unsigned int NUMOFSETS = Powerof2((L1_CACHE->CAPACITY/BLOCKSIZE)/L1_CACHE->ASSOCIATIVITY);
    unsigned int NUMOFBLOCKS = Powerof2(BLOCKSIZE);
    unsigned int tag = stoul(address.substr(0, 64-(NUMOFSETS + NUMOFBLOCKS)), nullptr, 2);
    unsigned int index = stoul(address.substr(64-(NUMOFSETS + NUMOFBLOCKS), NUMOFSETS), nullptr, 2);
    
    /*/ DEBUG
    cout << "This is L1 Access !" << endl;
    cout << "Memory Address : " <<  address << endl;
    cout << "Tag : ";
    printBinary(tag);
    cout << "Index : ";
    printBinary(index);
    cout <<"BlockOffset : " << address.substr(64-NUMOFBLOCKS,NUMOFBLOCKS) << endl;
    /*/

    // Find the tag matching order in INDEX
    unsigned int HIT_INDEX;
    bool Hit = false;
    for(int i = 0; i < L1_CACHE->SET[index].size(); i++){
        if((L1_CACHE->SET[index][i]->TAG) == tag){
            Hit = true;
            HIT_INDEX = i;
            break;
        }
    }
    //

    // If Hit, update the block time to decide the eviction block
    if(Hit == true){
        L1_CACHE->SET[index][HIT_INDEX]->RECENT_TIME = time(NULL);
        // Mode
        if(mode ==  0){
            L1_READ_HIT++;
        }
        else{
            L1_CACHE->SET[index][HIT_INDEX]->DIRTY = 1;
            L1_WRITE_HIT++;
        }
    // If miss, add the block into the Cache according to the number of blocks in set
    }else{
        Block* newBlock = new Block();
        newBlock = L2_ACCESS(L1_CACHE, L2_CACHE, address, BLOCKSIZE, mode);
        newBlock->TAG = tag;
        newBlock->INDEX = index;
        newBlock->RECENT_TIME = time(NULL);
        newBlock->DIRTY = 0;
        newBlock->BLOCK_SIZE = BLOCKSIZE;
        newBlock->START_ADDRESS = address.substr(0, 64-(NUMOFBLOCKS));
        // Mode
        if(mode == 0){
            L1_READ_MISS++;
        }
        else{
            newBlock->DIRTY = 1;
            L1_WRITE_MISS++;
        }
        BlockintoL1Cache(L1_CACHE, L2_CACHE, newBlock);
    }
}

Block* L2_ACCESS(Cache* L1_CACHE, Cache* L2_CACHE, string address, unsigned int BLOCKSIZE, unsigned int mode){
    unsigned int NUMOFSETS = Powerof2((L2_CACHE->CAPACITY/BLOCKSIZE)/L2_CACHE->ASSOCIATIVITY);
    unsigned int NUMOFBLOCKS = Powerof2(BLOCKSIZE);
    unsigned int tag = stoul(address.substr(0,64-(NUMOFSETS + NUMOFBLOCKS)), nullptr, 2);
    unsigned int index = stoul(address.substr(64-(NUMOFSETS + NUMOFBLOCKS), NUMOFSETS), nullptr, 2);
    /*/ DEBUG
    cout << "This is L2 Access !" << endl;
    cout << "Memory Address : " << address << endl;
    cout << "TAG : " ;
    printBinary(tag);
    cout << "INDEX :";
    printBinary(index);
    /*/

    // Find the tag matching order in INDEX
    unsigned int HIT_INDEX;
    bool Hit = false;
    for(int i = 0; i < L2_CACHE->SET[index].size(); i++){
        if((L2_CACHE->SET[index][i]->TAG) == tag){
            Hit = true;
            HIT_INDEX = i;
            break;
        }
    }
    //

    // If Hit, update the block time to decide the eviction block
    if(Hit == true){
        L2_CACHE->SET[index][HIT_INDEX]->RECENT_TIME = time(NULL);
        // Mode
        if(mode == 0){               // L1 Miss & L2 Hit (Read)
            L2_READ_HIT++;
        }
        else{                       // L1 Miss & L2 Hit (Write)
            // L2_CACHE->SET[index][HIT_INDEX]->DIRTY = 1;
            L2_WRITE_HIT++;
        }
        return L2_CACHE->SET[index][HIT_INDEX];

    // If miss, add the block into the Cache according to the number of blocks in set
    }else{
        Block* newBlock = new Block();
        newBlock->TAG = tag;
        newBlock->INDEX = index;
        newBlock->BLOCK_SIZE = BLOCKSIZE;
        newBlock->RECENT_TIME = time(NULL);
        newBlock->START_ADDRESS = address.substr(0, 64-(NUMOFBLOCKS));
        // Mode
        if(mode == 0){
            L2_READ_MISS++;
        }
        else{
            newBlock->DIRTY = 1;
            L2_WRITE_MISS++;
        }
        BlockintoL2Cache(L1_CACHE, L2_CACHE, newBlock);
        return newBlock;
    }
}

void BlockintoL1Cache(Cache* L1_CACHE, Cache* L2_CACHE, Block* BLOCK){
    if(L1_CACHE->SET[BLOCK->INDEX].size() < L1_CACHE->ASSOCIATIVITY){
        L1_CACHE->SET[BLOCK->INDEX].push_back(BLOCK);
    }else{
        int IDX;
        if(L1_CACHE->REPLACEMENT_POLICY == 0){
            vector<time_t> value;
            for(int i = 0; i < L1_CACHE->SET[BLOCK->INDEX].size(); i++){
                value.push_back(L1_CACHE->SET[BLOCK->INDEX][i]->RECENT_TIME);
            }
            // Calculate the Least Recently Used Block index
            auto minElementIter = std::min_element(value.begin(), value.end());
            IDX = std::distance(value.begin(), minElementIter);
            if(L1_CACHE->SET[BLOCK->INDEX][IDX]->DIRTY == 1){
                L1_DIRTY_EVICT++;
            }else{
                L1_CLEAN_EVICT++;
            }
            L1_CACHE->SET[BLOCK->INDEX].erase(L1_CACHE->SET[BLOCK->INDEX].begin()+IDX);
            L1_CACHE->SET[BLOCK->INDEX].shrink_to_fit();
            L1_CACHE->SET[BLOCK->INDEX].push_back(BLOCK);
        }else{ // RANDOM EVICT
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dis(0, L1_CACHE->ASSOCIATIVITY-1);
            IDX = dis(gen);
            if(L1_CACHE->SET[BLOCK->INDEX][IDX]->DIRTY == 1){
                L1_DIRTY_EVICT++;
            }else{
                L1_CLEAN_EVICT++;
            }
            L1_CACHE->SET[BLOCK->INDEX].erase(L1_CACHE->SET[BLOCK->INDEX].begin()+IDX);
            L1_CACHE->SET[BLOCK->INDEX].shrink_to_fit();
            L1_CACHE->SET[BLOCK->INDEX].push_back(BLOCK);
        }
        if(L1_CACHE->SET[BLOCK->INDEX][IDX]->DIRTY == 1){
            string INITIAL_ADDRESS = BLOCK->START_ADDRESS;
            unsigned int NUMOFSETS = Powerof2((L2_CACHE->CAPACITY/BLOCK->BLOCK_SIZE)/L2_CACHE->ASSOCIATIVITY);
            unsigned int tag = stoul(INITIAL_ADDRESS.substr(0,INITIAL_ADDRESS.size()-NUMOFSETS), nullptr, 2);
            unsigned int index = stoul(INITIAL_ADDRESS.substr(INITIAL_ADDRESS.size()-NUMOFSETS), nullptr, 2);
            for(int i = 0; i < L1_CACHE->SET[index].size(); i++){
                if((L1_CACHE->SET[index][i]->TAG) == tag){
                    L1_CACHE->SET[BLOCK->INDEX][i]->DIRTY = 1;
                    break;
                }
            }
        }
        


    }
}

void BlockintoL2Cache(Cache* L1_CACHE, Cache* L2_CACHE, Block* BLOCK){
    if(L2_CACHE->SET[BLOCK->INDEX].size() < L2_CACHE->ASSOCIATIVITY){
        L2_CACHE->SET[BLOCK->INDEX].push_back(BLOCK);
    }else{
        unsigned int IDX;
        if(L2_CACHE->REPLACEMENT_POLICY == 0){
            vector<time_t> value;
            for(int i = 0; i < L1_CACHE->SET[BLOCK->INDEX].size(); i++){
                value.push_back(L1_CACHE->SET[BLOCK->INDEX][i]->RECENT_TIME);
            }
            auto minElementIter = std::min_element(value.begin(), value.end());
            // Calculate the Least Recently Used Block index
            IDX = std::distance(value.begin(), minElementIter);
            L2_CACHE->SET[BLOCK->INDEX].erase(L2_CACHE->SET[BLOCK->INDEX].begin()+IDX);
            L2_CACHE->SET[BLOCK->INDEX].shrink_to_fit();
            L2_CACHE->SET[BLOCK->INDEX].push_back(BLOCK);
        }else{ // RANDOM EVICT
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dis(0, L2_CACHE->ASSOCIATIVITY-1);
            IDX = dis(gen);
            L2_CACHE->SET[BLOCK->INDEX].erase(L2_CACHE->SET[BLOCK->INDEX].begin()+IDX);
            L2_CACHE->SET[BLOCK->INDEX].shrink_to_fit();
            L2_CACHE->SET[BLOCK->INDEX].push_back(BLOCK);
        }
        // Check if L1 Cache has same Block 
        string INITIAL_ADDRESS = BLOCK->START_ADDRESS;
        unsigned int NUMOFSETS = Powerof2((L1_CACHE->CAPACITY/BLOCK->BLOCK_SIZE)/L1_CACHE->ASSOCIATIVITY);
        unsigned int tag = stoul(INITIAL_ADDRESS.substr(0,INITIAL_ADDRESS.size()-NUMOFSETS), nullptr, 2);
        unsigned int index = stoul(INITIAL_ADDRESS.substr(INITIAL_ADDRESS.size()-NUMOFSETS), nullptr, 2);
        bool L1_CACHE_EXIT = false;
        unsigned int MATCHING_INDEX = 0;
        for(int i = 0; i < L1_CACHE->SET[index].size(); i++){
            if((L1_CACHE->SET[index][i]->TAG) == tag){
                L1_CACHE_EXIT = true;
                MATCHING_INDEX = i;
                break;
            }
        }
        if(L1_CACHE_EXIT){
            if(L2_CACHE->SET[BLOCK->INDEX][IDX]->DIRTY == 1){
                L2_DIRTY_EVICT++;
                if(L1_CACHE->SET[BLOCK->INDEX][MATCHING_INDEX]->DIRTY == 1){
                    L1_DIRTY_EVICT++;
                }else{
                    L1_CLEAN_EVICT++;
                }
            }else{
                if(L1_CACHE->SET[BLOCK->INDEX][MATCHING_INDEX]->DIRTY == 1){
                    L1_DIRTY_EVICT++;
                    L2_DIRTY_EVICT++;
                }else{
                    L1_CLEAN_EVICT++;
                    L2_CLEAN_EVICT++;
                }
            }

            delete L1_CACHE->SET[BLOCK->INDEX][MATCHING_INDEX];
            L1_CACHE->SET[BLOCK->INDEX].erase(L1_CACHE->SET[BLOCK->INDEX].begin()+MATCHING_INDEX);
            L1_CACHE->SET[BLOCK->INDEX].shrink_to_fit();
        }else{
            if(L2_CACHE->SET[BLOCK->INDEX][IDX]->DIRTY == 1){
                L2_DIRTY_EVICT++;
            }else{
                L2_CLEAN_EVICT++;
            }
        }
    }
}


int main(int argc, const char* argv[]){
	string filename = argv[argc-1];
    ifstream TRACE_FILE(filename);

    // Variable for File processing
    unsigned int L2_capacity;
    unsigned int L2_capacity_KB;
    unsigned int L2_associativity;
    unsigned int L1_capacity;
    unsigned int L1_capacity_KB;
    unsigned int L1_associativity;
    unsigned int block_size;
    unsigned int replacement_policy;
    unsigned int NUM_PARAM = 0;
    unsigned int check = 0;
    //

    // Variable for Caching
    unsigned int read_access = 0;
    unsigned int write_access = 0;  
    L1_READ_HIT = 0;
    L1_WRITE_HIT = 0;
    L1_READ_MISS = 0;
    L1_WRITE_MISS = 0;
    L1_CLEAN_EVICT = 0;
    L1_DIRTY_EVICT = 0;
    L2_CLEAN_EVICT = 0;
    L2_DIRTY_EVICT = 0;

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-c") == 0){
            L2_capacity_KB = stoul(argv[i+1]);
            L2_capacity = stoul(argv[i+1])*pow(2,10);
            L1_capacity_KB = L2_capacity_KB/4;
            L1_capacity = L2_capacity/4;
            NUM_PARAM++;
            continue;
        }

        if(strcmp(argv[i], "-a") == 0){
            L2_associativity = stoul(argv[i+1]);
            L1_associativity = L2_associativity/4;
            if(L2_associativity<=2){
                L1_associativity = L2_associativity;
            }
            NUM_PARAM++;
            continue;
        }

        if(strcmp(argv[i], "-b") == 0){
            block_size = stoul(argv[i+1]);
            NUM_PARAM++;
            continue;
        }

        if(strcmp(argv[i], "-lru") == 0){
            replacement_policy = 0;
            NUM_PARAM++;
            continue;
        }

        if(strcmp(argv[i], "-random") == 0){
            replacement_policy = 1;
            NUM_PARAM++;    
            continue;
        }
    }
    if(NUM_PARAM < 4){
        cout << "ERROR : Parameters are Missing !" << endl;
    }

    unsigned int L1_index = (L1_capacity/block_size)/L1_associativity;
    unsigned int L2_index = (L2_capacity/block_size)/L2_associativity;
    map<unsigned int, vector<Block*>> l1_set;
    for(int i = 0; i < L1_index ; i++){
        vector<Block*> set;
        l1_set[i] = set;
    }
    map<unsigned int, vector<Block*>> l2_set;
    for(int i = 0; i < L2_index ; i++){
        vector<Block*> set;
        l2_set[i] = set;
    }

    Cache* L1CACHE = new Cache{L1_capacity, L1_associativity, replacement_policy, l1_set};
    Cache* L2CACHE = new Cache{L2_capacity, L2_associativity, replacement_policy, l2_set};
    // Trace the file until the end
    string str;
    unsigned int mode;
    int j = 0;
    while(!TRACE_FILE.eof()){
        
		getline(TRACE_FILE, str);
        if(str == "") continue;
		stringstream ss(str);
        char MODE;
        string PHYSICAL_ADDRESS;
        ss >> MODE >> PHYSICAL_ADDRESS;
        
        // Change the Address from Hex to binary
        unsigned int address = stoul(PHYSICAL_ADDRESS, nullptr, 16);
        string BINARY_ADDRESS = Dectobinary(address);
        // Mode Check
        unsigned int mode;
        if(MODE == 'R'){
            mode = 0;
            read_access++;
        }else{
            mode = 1;
            write_access++;
        }
        //
        L1_ACCESS(L1CACHE, L2CACHE, BINARY_ADDRESS, block_size, mode);
        //sleep(1);
        
      
    }
    filename = filename + "_" + to_string(L2_capacity_KB) + "_" + to_string(L2_associativity) + "_" + to_string(block_size);
    FILE *file = fopen(filename.c_str(), "w");
    if (file == NULL) {
        printf("File cannot be opened.\n");
        return 1;
    }

    // 데이터를 파일로 출력
    fprintf(file, "-- General Stats --\n");
    fprintf(file, "L1 Capacity: %d\n", L1_capacity_KB);
    fprintf(file, "L1 way: %d\n", L1_associativity);
    fprintf(file, "L2 Capacity: %d\n", L2_capacity_KB);
    fprintf(file, "L2 way: %d\n", L2_associativity);
    fprintf(file, "Block Size: %d\n", block_size);
    fprintf(file, "Total accesses: %d\n", read_access + write_access);
    fprintf(file, "Read accesses: %d\n", read_access);
    fprintf(file, "Write accesses: %d\n", write_access);
    fprintf(file, "L1 Read misses: %d\n", L1_READ_MISS);
    fprintf(file, "L2 Read misses: %d\n", L2_READ_MISS);
    fprintf(file, "L1 Write misses: %d\n", L1_WRITE_MISS);
    fprintf(file, "L2 Write misses: %d\n", L2_WRITE_MISS);
    fprintf(file, "L1 Read miss rate: %.2f%%\n", 100 * (float)L1_READ_MISS / read_access);
    fprintf(file, "L2 Read miss rate: %.2f%%\n", 100 * (float)L2_READ_MISS / L1_READ_MISS);
    fprintf(file, "L1 Write miss rate: %.2f%%\n", 100 * (float)L1_WRITE_MISS / write_access);
    fprintf(file, "L2 Write miss rate: %.2f%%\n", 100 * (float)L2_WRITE_MISS / L1_WRITE_MISS);
    fprintf(file, "L1 Clean eviction: %d\n", L1_CLEAN_EVICT);
    fprintf(file, "L2 Clean eviction: %d\n", L2_CLEAN_EVICT);
    fprintf(file, "L1 dirty eviction: %d\n", L1_DIRTY_EVICT);
    fprintf(file, "L2 dirty eviction: %d\n", L2_DIRTY_EVICT);

    // 파일 닫기
    fclose(file);
	
    delete L1CACHE;
    delete L2CACHE;
}
