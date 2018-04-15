
#include "Tran_NB_Hashtable.hpp"
#include <bitset>
#include <api/api.hpp>
#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <pthread.h>
#include <api/api.hpp>
#include <common/platform.hpp>
#include <common/locks.hpp>

#include <stm/config.h>
#if defined(STM_CPU_SPARC)
#include <sys/types.h>
#endif

/**
 *  Step 1:
 *    Include the configuration code for the harness, and the API code.
 */
#include <iostream>
#include <api/api.hpp>
#include "bmconfig.hpp"

/**
 *  We provide the option to build the entire benchmark in a single
 *  source. The bmconfig.hpp include defines all of the important functions
 *  that are implemented in this file, and bmharness.cpp defines the
 *  execution infrastructure.
 */
#ifdef SINGLE_SOURCE_BUILD
#include "bmharness.cpp"
#endif

NB_Hashtable* NB_SET;

void bench_init(){
	NB_SET = new NB_Hashtable();
	NB_SET->Init();

	TM_BEGIN_FAST_INITIALIZATION();
	for (uint64_t w = 0; w < CFG.elements; w += 2)
		NB_SET->Insert(w TM_PARAM);
	TM_END_FAST_INITIALIZATION();
	std::cout << "We're on our way." << std::endl;
}

void bench_test(uintptr_t, u_int32_t* seed)
{
	uint64_t val = rand_r(seed) %CFG.elements;
	uint64_t act = rand_r(seed) % 100;
	if (act < CFG.lookpct){
		TM_BEGIN(atomic){
			NB_SET->Lookup(val TM_PARAM);
		} TM_END;
	}
	else if( act < CFG.inspct){
		TM_BEGIN(atomic){
			NB_SET->Insert(val TM_PARAM);
		}TM_END;
	}
	else {
		TM_BEGIN (atomic){
			NB_SET->Erase(val TM_PARAM);
		}TM_END;
	}
}

bool bench_verify() { return NB_SET->isSane(); }

void bench_reparse()
{
    if (CFG.bmname == "") CFG.bmname = "List";
}
/*

int main (){

	TM_SYS_INIT();
	TM_THREAD_INIT();

	NB_Hashtable* h = new NB_Hashtable();
	h->Init();

	TM_SYS_SHUTDOWN();

	return 0;
}

*/