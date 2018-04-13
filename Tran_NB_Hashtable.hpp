// Jonathan White, Derek Spooner
// COP 4520 - Spring 2018
// NB_Hashtable.h - Defines NB_Hashtable class.
#pragma once

// NOTE FOR DEVELOPER (JON): This code is currently based off of the pseudocode
// in figure 6, which has some issues discussed in the paper. If necessary for
// the project, the code will need to be updated to include a version counter
// as seen in figure 8. This second version is designed to circumvent the ABA
// problem.

// Used throughout class to define a type that is the size of a memory word.
// the hashtable was written on a 64-bit system, so the default is a 64-bit
// addressed type.
#define WORD_SIZE_TYPE uint64_t

// Constants for <state, key>
// The order of the key and state was swapped so it would be easier to recover
// the key. A key is always assumed to have its upper two bits unset.
#define EMPTY 0x0000000000000000
#define BUSY 0x4000000000000000
#define INSERTING 0x8000000000000000
#define MEMBER 0xC000000000000000

// Constants for <scanning bit, bound>
#define SCAN_TRUE 0x8000000000000000
#define SCAN_FALSE 0x0000000000000000

// Libraries used for testing
#include <bitset>

//#include <algorithm>
//#include <atomic>
// required for use of uint64_t
//#include <cstdint>
#include <exception>
//#include <functional>
#include <iostream>
#include <string>
#include "api/api.hpp"

class NB_Hashtable
{
public:
    NB_Hashtable();

    TM_CALLABLE
    void Init();
    
    TM_CALLABLE
    bool Lookup(WORD_SIZE_TYPE k TM_ARG);
    
    TM_CALLABLE
    bool Insert(WORD_SIZE_TYPE k TM_ARG);
    
    TM_CALLABLE    
    bool Erase(WORD_SIZE_TYPE k TM_ARG);

    bool isSane();

    // TODO: Test functions?
    //void printbuckets();

private:
    TM_CALLABLE
    void InitProbeBound(WORD_SIZE_TYPE h);

    TM_CALLABLE 
    WORD_SIZE_TYPE GetProbeBound(WORD_SIZE_TYPE h TM_ARG);
    
    TM_CALLABLE 
    void ConditionallyRaiseBound(WORD_SIZE_TYPE h, WORD_SIZE_TYPE index TM_ARG);
    
    TM_CALLABLE 
    void ConditionallyLowerBound(WORD_SIZE_TYPE h, WORD_SIZE_TYPE index TM_ARG);
    
    TM_CALLABLE 
    bool DoesBucketContainCollisions(WORD_SIZE_TYPE h, WORD_SIZE_TYPE index TM_ARG);
    
    TM_CALLABLE 
    WORD_SIZE_TYPE* Bucket(WORD_SIZE_TYPE h, WORD_SIZE_TYPE index TM_ARG);

    // TM_CALLABLE
    // void InitProbeBound(WORD_SIZE_TYPE h);

    // TM_CALLABLE 
    // WORD_SIZE_TYPE GetProbeBound(WORD_SIZE_TYPE h);
    
    // TM_CALLABLE 
    // void ConditionallyRaiseBound(WORD_SIZE_TYPE h, WORD_SIZE_TYPE index);
    
    // TM_CALLABLE 
    // void ConditionallyLowerBound(WORD_SIZE_TYPE h, WORD_SIZE_TYPE index);
    
    // TM_CALLABLE 
    // bool DoesBucketContainCollisions(WORD_SIZE_TYPE h, WORD_SIZE_TYPE index);
    
    // TM_CALLABLE 
    // WORD_SIZE_TYPE* Bucket(WORD_SIZE_TYPE h, WORD_SIZE_TYPE index);

    int size;
    WORD_SIZE_TYPE* bounds;
    WORD_SIZE_TYPE* buckets;
};

NB_Hashtable::NB_Hashtable()
{
    // bounds = (WORD_SIZE_TYPE*)TM_ALLOC(sizeof(WORD_SIZE_TYPE));
    // buckets = (WORD_SIZE_TYPE*)TM_ALLOC(sizeof(WORD_SIZE_TYPE));
    size = 1024;
    bounds = new WORD_SIZE_TYPE[size];
    buckets = new WORD_SIZE_TYPE[size];
}

// Initializes buckets and bounds arrays.
TM_CALLABLE
void NB_Hashtable::Init()
{
    for (int i = 0; i < size; i++)
    {
        InitProbeBound(i);
        buckets[i] = EMPTY;
    }
}

bool NB_Hashtable::isSane(){
    for (auto i = 0; i < size; i++){
        if (!buckets[i] != std::hash<WORD_SIZE_TYPE>{}(i))
            return false;
    }
}

// Determines whether k is a member of the set.
TM_CALLABLE
bool NB_Hashtable::Lookup(WORD_SIZE_TYPE k TM_ARG)
{
    // hash something
    WORD_SIZE_TYPE max, temp, h;	
    //TM_WRITE(h, hash h from k);
    TM_WRITE(max, GetProbeBound(h TM_PARAM));
    TM_WRITE(temp, (k | MEMBER));

    h = std::hash<WORD_SIZE_TYPE> {}(k);
    //WORD_SIZE_TYPE max = GetProbeBound(h);
    //WORD_SIZE_TYPE temp = (k | MEMBER);

    // i is WORD_SIZE_TYPE so it can be passed to Bucket().
    for (WORD_SIZE_TYPE i = 0; i <= max; i++)
    {
        // std::cout << (Bucket(h, i) & ~MEMBER) << std::endl;
        // if (TM_READ(Bucket(h, i)) == TM_READ(temp))
        if (*Bucket(h, i TM_PARAM) == temp)
        {
            return true;
        }
    }

    return false;
}

// Insert k into the set if it is not a member
TM_CALLABLE
bool NB_Hashtable::Insert(WORD_SIZE_TYPE k TM_ARG)
{
    // WORD_SIZE_TYPE i, temp, h;	
    // TM_WRITE(h, hash h from k);
    // TM_WRITE(i, 0);
    // TM_WRITE(temp, (k | MEMBER));
    WORD_SIZE_TYPE h = std::hash<WORD_SIZE_TYPE> {}(k);
    WORD_SIZE_TYPE i = 0, temp;

    // Attempt to change bucket entry from state empty (00) to busy (01).
    temp = EMPTY;

    while(*Bucket (h, i TM_PARAM) == BUSY)
    //while (!std::atomic_compare_exchange_weak(Bucket(h, i), &temp, BUSY))
    {
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
        // set state bit of <key, state> to inserting (10)
	// TM_WRITE(temp, (k | INSERTING)
        temp = (k | INSERTING);
        
	    TM_WRITE(*Bucket(h,i TM_PARAM), temp);
	//Bucket(h, i) = temp;
        ConditionallyRaiseBound(h, i TM_PARAM);

        // Scan through probe sequence
        WORD_SIZE_TYPE max = GetProbeBound(h TM_PARAM);
        for (WORD_SIZE_TYPE j = 0; j < max; j++)
        {
            if (j != i)
            {
                // Stall concurrent inserts
		        if (*Bucket(h, j TM_PARAM) == temp)
                //if (Bucket(h, j) == temp)
                {
                    if (*Bucket(h,i TM_PARAM)==temp)
		                TM_WRITE(*Bucket(h, j TM_PARAM), (WORD_SIZE_TYPE)BUSY);
                    //std::atomic_compare_exchange_strong(Bucket(h, j),
                      //                                  &temp,
                        //                                BUSY);
                }
                // Abort if k is already a member
		        if (*Bucket(h,j TM_PARAM) == (k|MEMBER))
                //if (Bucket(h, j) == (k | MEMBER))
                {
		            TM_WRITE(*Bucket(h, i TM_PARAM), (WORD_SIZE_TYPE)BUSY);
                    ConditionallyLowerBound(h, i TM_PARAM);
        		    TM_WRITE(*Bucket(h, i TM_PARAM), (WORD_SIZE_TYPE)EMPTY);
                    //Bucket(h, i) = BUSY;
                    //ConditionallyLowerBound(h, i);
                    //Bucket(h, i) = EMPTY;
                    return false;
                }
            }
        }
        // attempt to set bit of <key, state> to member (11)
    }
    // while (!std::atomic_compare_exchange_weak(Bucket(h,i),
    //         &temp,
    //         (k | MEMBER)));
    while(*Bucket (h, i TM_PARAM) == (k | MEMBER));

    // std::bitset<64> tempBits((k | MEMBER));
    // std::cout << tempBits.to_string() << std::endl;

    return true;
}

// Remove k from the set if it is a member
TM_CALLABLE
bool NB_Hashtable::Erase(WORD_SIZE_TYPE k TM_ARG)
{
    WORD_SIZE_TYPE max;
    WORD_SIZE_TYPE h = std::hash<WORD_SIZE_TYPE> {}(k);

    TM_WRITE(max, GetProbeBound(h TM_PARAM));
    // scan probe sequence
    WORD_SIZE_TYPE temp = (k | MEMBER);

    for (WORD_SIZE_TYPE i = 0; i <= max; i++)
    {
        // remove a copy of <k, member>
        // May have to modify this to specify that k's status must be member
        // if that is not a pre-condition for Erase().
        if (*Bucket(h, i TM_PARAM) == /*k*/ temp)
        {
            // Set status bit to busy (01)
            //if (std::atomic_compare_exchange_strong(Bucket(h, i), &temp, BUSY))
	        if (*Bucket (h, i TM_PARAM) == (k | MEMBER));
            {
		
                ConditionallyLowerBound(h, i TM_PARAM);
		        TM_WRITE(*Bucket(h, i TM_PARAM), (WORD_SIZE_TYPE)EMPTY);
                //Bucket(h, i) = EMPTY;
                return true;
            }
        }
    }

    return false;
}

// Private member functions

// Sets the bound entry bound and scanning bit to <false, 0>
TM_CALLABLE
void NB_Hashtable::InitProbeBound(WORD_SIZE_TYPE h)
{
    bounds[h % size] = SCAN_FALSE;
}

// Returns the maximum offset of any collision in a probe sequence as well as
// that bound's scanning bit <bound, scanning>
TM_CALLABLE
WORD_SIZE_TYPE NB_Hashtable::GetProbeBound(WORD_SIZE_TYPE h TM_ARG)
{
    return TM_READ(bounds[h % size]);
}

// NOTE FOR DEVELOPER (JON): Index has to be WORD_SIZE_TYPE (uint64_t) instead
// of an int because it refers at a memory location AT index in an array.
// Ensure maximum >= index
TM_CALLABLE
void NB_Hashtable::ConditionallyRaiseBound(WORD_SIZE_TYPE h, WORD_SIZE_TYPE index TM_ARG)
{
    WORD_SIZE_TYPE old_bound, new_bound;

    do
    {
        TM_WRITE(bounds[h % size], old_bound);
        new_bound = std::max(old_bound, index);
    }
    while (TM_READ(bounds[h%size])== old_bound);
    //while (!std::atomic_compare_exchange_weak(&(bounds[h % size]), &old_bound, new_bound));
}

// Allow maximum < index
TM_CALLABLE
void NB_Hashtable::ConditionallyLowerBound(WORD_SIZE_TYPE h, WORD_SIZE_TYPE index TM_ARG)
{
    WORD_SIZE_TYPE bound, expectedFalse, expectedTrue;

    TM_WRITE(bound,bounds[h % size]);
    // If scanning bit is set, unset it
    if ((bound & SCAN_TRUE) == (SCAN_TRUE))
    {
        if (TM_READ(bounds[h % size])== bound)
            TM_WRITE(bounds[h % size], (bound & ~SCAN_TRUE));
        //std::atomic_compare_exchange_weak(&bounds[h % size], &bound, (bound & ~SCAN_TRUE));
    }

    expectedFalse = index & ~SCAN_TRUE;
    if (index > 0)
    {
        while (true)
        //while (std::atomic_compare_exchange_weak(&bounds[h % size], &expectedFalse, (index | SCAN_TRUE)))
        {
            if (TM_READ(bounds[h%size]) == expectedFalse){
                TM_WRITE(bounds[h%size], (index | SCAN_TRUE));
                break;
            }

            WORD_SIZE_TYPE i = index - 1;
            while ((i > 0) && (!DoesBucketContainCollisions(h, i TM_PARAM)))
            {
                i--;
            }
            expectedTrue = index | SCAN_TRUE;

            if(TM_READ(bounds[h%size])== expectedTrue);
            {
                TM_WRITE(bounds[h%size], (i & ~SCAN_TRUE));
            }
            //std::atomic_compare_exchange_strong(&bounds[h % size], &expectedTrue, (i & ~SCAN_TRUE));
        }
    }
}

// Return bucket entry at hash value plus offset (using quadratic probing)
TM_CALLABLE
WORD_SIZE_TYPE* NB_Hashtable::Bucket(WORD_SIZE_TYPE h, WORD_SIZE_TYPE index TM_ARG)
{
    return &(buckets[(h + index * (index + 1) / 2) % size]);
}

// In the paper, - refers to the empty state in <key, state>. There are 4
// states: empty, busy, inserting, and member. The paper was written in 2005
// and more than on a 32-bit system. This means that there are 2 unused bits
// at the end of every address---just enough to identify these 4 states.
// Tentatively, here the codes I am using for my state codes:
// empty = 00
// busy = 01
// inserting = 10
// member = 11
TM_CALLABLE
bool NB_Hashtable::DoesBucketContainCollisions(WORD_SIZE_TYPE h, WORD_SIZE_TYPE index TM_ARG)
{
    // <state, key>
    WORD_SIZE_TYPE k;
    TM_WRITE(k, *Bucket(h, index TM_PARAM));
    // Recover key from <state, key>
    WORD_SIZE_TYPE key = k & ~MEMBER;
    return ((k != EMPTY) && (std::hash<WORD_SIZE_TYPE> {}(key) == h));
}
