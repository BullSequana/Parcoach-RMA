#ifndef PARCOACHRMA_H
#define PARCOACHRMA_H

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/WithColor.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstIterator.h"

using namespace std;
using namespace llvm;


namespace {
  class ParcoachRMA : public FunctionPass {

	public:
    static char ID;
    ParcoachRMA();
		virtual ~ParcoachRMA() {};
		virtual void getAnalysisUsage(AnalysisUsage &au) const;
    virtual bool runOnFunction(Function &F);


	private:
		static int count_MPI, count_GET, count_PUT, count_ACC, count_FLUSH, count_FENCE, count_Win, count_Free, count_LOCK, count_UNLOCK, count_UNLOCKALL, count_LOCKALL, count_LOAD, count_STORE, count_BARRIER;
		static int count_MPIX, count_PUT_NOTIFY, count_GET_NOTIFY, count_WAIT_NOTIFY, count_TEST_NOTIFY, count_Win_notify;
		// Keep the address of all MPI functions in the function
		vector<Value *> vecAddr = vector<Value *>();

		void instrumentMemAccesses(Function &F,LLVMContext &Ctx, DataLayout* datalayout);

 };
	
}

#endif

