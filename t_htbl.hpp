#include <iostream>
#include <string>
#include <bitset>
#include <exception>
#include <memory>
#include "api/api.hpp"

#define EMPTY 0
#define BUSY 1
#define INSERTING 3
#define MEMBER 4

#define SCAN_FALSE 0
#define SCAN_TRUE 1

using std::make_pair;
using std::pair;

typedef pair<int, int> WST;

class NB_Hashtable
{
public:
    NB_Hashtable();

    TM_CALLABLE
    void Init();
    
    TM_CALLABLE
    bool Lookup(int k TM_ARG);
    
    TM_CALLABLE
    bool Insert(int k TM_ARG);
    
    TM_CALLABLE    
    bool Erase(int k TM_ARG);

    bool isSane();

private:
    TM_CALLABLE
    int quad_pos(int h, int index);

    TM_CALLABLE
    void InitProbeBound(WST h);

    TM_CALLABLE 
    WST GetProbeBound(WST h TM_ARG);
    
    TM_CALLABLE 
    void ConditionallyRaiseBound(WST h, int index TM_ARG);
    
    TM_CALLABLE 
    void ConditionallyLowerBound(WST h, int index TM_ARG);
    
    TM_CALLABLE 
    bool DoesBucketContainCollisions(WST h, int index TM_ARG);
    
    TM_CALLABLE 
    WST Bucket(WST h, int index TM_ARG);

    int size;
    WST* bounds;
    WST* buckets;
};

// Fixed size: ancticipated size based on half the number of transactions to occur.
NB_Hashtable::NB_Hashtable(){
    size = 262144;
    bounds = new WST[size];
    buckets = new WST[size];
}


TM_CALLABLE
void NB_Hashtable::Init(){
    for (auto i = 0; i < size; i++){
        InitProbeBound(make_pair(i, EMPTY));
    }
}

// Validation function that is required for the use of the bmharness.cpp main
bool NB_Hashtable::isSane(){
    for (auto i = 0; i < size; i++){
        if (!buckets[i].first != (i%size))
            return false;
    }
    return true;
}

TM_CALLABLE
bool NB_Hashtable::Insert(int k TM_ARG)
{
    int max, i = 0, pos;
    
    WST h = make_pair((k % size), EMPTY);
    WST k_ins, k_bus, k_mem, k_emp;
  
    while(true)
    {
        pos = quad_pos(h.first, i);

        if (buckets[pos].second == EMPTY){
            buckets[pos].second = BUSY;
            break;
        }

        i++;
        if (i >= size)
      	{
            // Maybe this should still throw an error not entirely sure of the effects of this.
            try
            {
                throw "Table full";
            }
            catch (std::string s)
            {
                std::cout << s << std::endl;
            }
        }
    }

    // attempt to insert a unique copy of k
    do
    {
        pos = quad_pos(h.first, i);

        // set state bit of <key, state> to inserting (10)
        k_ins = make_pair(k, INSERTING);
        k_bus = make_pair(k, BUSY);
        k_mem = make_pair(k, MEMBER);
        k_emp = make_pair(k, EMPTY);
        buckets[pos] =  move(k_ins);

        // std::cout << "H " << h << " I " << i << std::endl;
        ConditionallyRaiseBound(h, i TM_PARAM);

        // Scan through probe sequence
        max = GetProbeBound(h TM_PARAM).first;

        for (auto j = 0; j < max; j++)
        {
            if (j != i)
            {
                pos = quad_pos(h.first, j);

                // Stall concurrent inserts
		        if (buckets[pos] == k_ins)
                {
                    buckets[pos] = move(k_bus);
                }

                // Abort if k is already a member
		        if (buckets[pos] == k_mem)
                {
                    pos = quad_pos(h.first, i);
                    buckets[pos] = move(k_bus);
                    ConditionallyLowerBound(h, i TM_PARAM);
                    buckets[pos] = move(k_emp);

                    return false;
                }
            }
        }

        pos = quad_pos(h.first, i);
        if (buckets[pos] == k_ins)
            buckets[pos] = move(k_mem);

        // attempt to set bit of <key, state> to member (11)
    }
    while(buckets[pos] != k_mem);

    return true;
}

TM_CALLABLE
bool NB_Hashtable::Erase(int k TM_ARG)
{
    int max, pos;
    
    WST h = make_pair((k % size), EMPTY);
    max = (GetProbeBound(h TM_PARAM)).first;

    WST k_mem = make_pair(k, MEMBER);
    WST k_emp = make_pair(k, EMPTY);

    for (auto i = 0; i <= max; i++)
    {
        // remove a copy of <k, member>
        // May have to modify this to specify that k's status must be member
        // if that is not a pre-condition for Erase().
        pos = quad_pos(h.first, i);
        
        if (Bucket(h, i TM_PARAM).second == /*k*/ MEMBER)
        {

            // Set status bit to busy (01)
	        if (buckets[pos] == k_mem);
            {
		
                ConditionallyLowerBound(h, i TM_PARAM);
                buckets[pos] = move(k_emp);
                return true;
            }
        }
    }

    return false;
}

// Determines whether k is a member of the set.
TM_CALLABLE
bool NB_Hashtable::Lookup(int k TM_ARG)
{
    // hash something
    int max;    
    WST h = make_pair((k % size), EMPTY);
    max = (GetProbeBound(h TM_PARAM)).first;

    for (auto  i = 0; i <= max; i++)
    {
        if ((Bucket(h, i TM_PARAM)).second == MEMBER)
        {
            return true;
        }
    }

    return false;
}



TM_CALLABLE
void NB_Hashtable::InitProbeBound(WST h)
{
    bounds[h.first % size] = make_pair(0,SCAN_FALSE);
}

// Returns the maximum offset of any collision in a probe sequence as well as
// that bound's scanning bit <bound, scanning>
TM_CALLABLE
WST NB_Hashtable::GetProbeBound(WST h TM_ARG)
{
    return bounds[h.first % size];
}

// NOTE FOR DEVELOPER (JON): Index has to be WST (uint64_t) instead
// of an int because it refers at a memory location AT index in an array.
// Ensure maximum >= index
TM_CALLABLE
void NB_Hashtable::ConditionallyRaiseBound(WST h, int index TM_ARG)
{
    WST old_bound, new_bound;

    while(true){

        old_bound = bounds[h.first % size];
        
        new_bound.first = std::max(old_bound.first, index);
        
        if (bounds[h.first % size] == old_bound)
        {
            // Swap in the new bounds. pair operation.. not atomic..
            swap(bounds[h.first % size],new_bound);
            break;
        }
    }
}

// Allow maximum < index
TM_CALLABLE
void NB_Hashtable::ConditionallyLowerBound(WST h, int index TM_ARG)
{
    WST bound, expectedFalse, expectedTrue;
    bound = bounds [h.first % size];

    // If scanning bit is set, unset it
    if (bound.second == SCAN_TRUE)
    {
        if(bounds[ h.first % size ] == bound)
            bounds[h.first % size] = make_pair(bound.first, SCAN_FALSE);
    }

    expectedFalse = make_pair(index,SCAN_FALSE);
    if (index > 0)
    {
        while (true)
        {
            if ((bounds[ h.first % size ]) == expectedFalse){
                bounds[ h.first % size ] = make_pair(index, SCAN_TRUE);
                break;
            }

            int i = index - 1;
            while ((i > 0) && (!DoesBucketContainCollisions(h, i TM_PARAM)))
            {
                i--;
            }
            expectedTrue = make_pair(index, SCAN_TRUE);

            if(bounds[ h.first % size ] == expectedTrue);
            {
                bounds[ h.first % size ] = make_pair(i, SCAN_FALSE);
            }
        }
    }
}


int NB_Hashtable::quad_pos (int h, int index)
{
    return (h + index * (index + 1) / 2) % size;
}

// Return bucket entry at hash value plus offset (using quadratic probing)
TM_CALLABLE
WST NB_Hashtable::Bucket(WST h, int index TM_ARG)
{
    return buckets[(h.first + index * (index + 1) / 2) % size];
}

TM_CALLABLE
bool NB_Hashtable::DoesBucketContainCollisions(WST h, int index TM_ARG)
{
    // <state, key>
    WST k;
    k = Bucket(h, index TM_PARAM);
    // Recover key from <state, key>
    int key = k.second;
    return ((k.second != EMPTY) && ((key % size) == h.first));
}