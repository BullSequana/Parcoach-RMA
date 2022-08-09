#include "parcoachRMA.h"


using namespace std;
using namespace llvm;

namespace {

	FunctionType *handlerType = nullptr;

	FunctionType *getHandlerType(LLVMContext &C){
		if(!handlerType){
			handlerType = FunctionType::get(Type::getVoidTy(C),{Type::getInt8Ty(C)->getPointerTo(),Type::getInt64Ty(C), Type::getInt64Ty(C), Type::getInt8Ty(C)->getPointerTo()},false);
		}
		return handlerType;
	}

	FunctionCallee handlerLoad;
	FunctionCallee handlerStore;
		
	FunctionCallee &getHandlerLoad(Module *M){
		if(!handlerLoad){
			handlerLoad = M->getOrInsertFunction("LOAD",getHandlerType(M->getContext()));
		}
		return handlerLoad;
	}
 
	FunctionCallee &getHandlerStore(Module *M){
		if(!handlerStore){
			handlerStore = M->getOrInsertFunction("STORE",getHandlerType(M->getContext()));
		}
		return handlerStore;
	} 

	// To instrument LOAD/STORE 
	void CreateAndInsertFunction(Instruction &I, Value *Addr, FunctionCallee &handler, size_t size, int line, StringRef file, LLVMContext &Ctx)
	{
	
		IRBuilder<> builder(&I);
		builder.SetInsertPoint(I.getParent(),builder.GetInsertPoint());
		Value *filename = builder.CreateGlobalStringPtr(file.str());
	
		CastInst* CI =CastInst::CreatePointerCast(Addr, Type::getInt8Ty(Ctx)->getPointerTo(),"", &I);
		CastInst* CIf =CastInst::CreatePointerCast(filename, Type::getInt8Ty(Ctx)->getPointerTo(),"", &I);
		builder.CreateCall(handler, { CI , ConstantInt::get(Type::getInt64Ty(Ctx), size), ConstantInt::get(Type::getInt64Ty(Ctx), line), CIf} );
		//DEBUG INFO: print function prototype// handler.getFunctionType()->print(errs());
	}

	// To modify MPI-RMA functions
	void ReplaceCallInst(Instruction &I, CallInst &ci, int line, StringRef file, LLVMContext &Ctx, StringRef newFuncName)
	{
		IRBuilder<> builder(&ci);
		Value *filename = builder.CreateGlobalStringPtr(file.str());
	
		Function *callee = ci.getCalledFunction();
		auto newArgsType = callee->getFunctionType()->params().vec();
		newArgsType.push_back(Type::getInt8PtrTy(Ctx));
		newArgsType.push_back(Type::getInt64Ty(Ctx));
		FunctionType *handlerType = FunctionType::get(callee->getReturnType(), newArgsType, false);
		//handlerType->print(errs());	
	
		Module *M = ci.getFunction()->getParent();
		FunctionCallee newFunc = M->getOrInsertFunction (newFuncName,handlerType);
	
		SmallVector<Value *,8> Args(ci.args()); 
		CastInst* CIf = CastInst::CreatePointerCast(filename, Type::getInt8PtrTy(Ctx),"", &I);
		Args.push_back(CIf); 
		Args.push_back(ConstantInt::get(Type::getInt64Ty(Ctx), line)); 
	
	  Value *newci = builder.CreateCall(newFunc, Args );	
		ci.replaceAllUsesWith(newci);
	}
} 



ParcoachRMA::ParcoachRMA() : FunctionPass(ID) {}

void ParcoachRMA::getAnalysisUsage(AnalysisUsage &au) const
{
	au.addRequired<AAResultsWrapperPass>();	
	au.setPreservesAll();
}


void ParcoachRMA::instrumentMemAccesses(Function &F,LLVMContext &Ctx, DataLayout* datalayout) 
{

	Module *M = F.getParent();
	vector<Instruction*> toDelete; // CallInst to delete (all MPI-RMA are replaced by new functions) 

	for(Instruction &I : instructions(F)){

   	DebugLoc dbg = I.getDebugLoc(); // get debug infos
  	int line = 0;
  	StringRef file = "";
  	if(dbg){
       line = dbg.getLine();
       file = dbg->getFilename();
    }
  
  	// For FORTRAN codes
  	if(I.getOpcode()==Instruction::BitCast){
  		if(I.getOperand(0)->getName().startswith("mpi_")){
            count_MPI++;
  		}
  		if( I.getOperand(0)->getName() == "mpi_put_"){
  			I.getOperand(0)->setName("new_put_");
  	 		count_PUT++;
  		}else if( I.getOperand(0)->getName() == "mpi_get_"){
  			I.getOperand(0)->setName("new_get_");
  			count_GET++;
  		}else if( I.getOperand(0)->getName() == "mpi_accumulate_"){
  			I.getOperand(0)->setName("new_accumulate_");
  			count_ACC++;
  		}else if( I.getOperand(0)->getName() == "mpi_win_create_"){
  			I.getOperand(0)->setName("new_win_create_");
  	 		count_Win++;
  		}else if( I.getOperand(0)->getName() == "mpi_win_free_"){
  			I.getOperand(0)->setName("new_win_free_");
  	 		count_Free++;
  		}else if( I.getOperand(0)->getName() == "mpi_fence_"){
  			I.getOperand(0)->setName("new_fence_");
  	 		count_FENCE++;
  		}else if( I.getOperand(0)->getName() == "mpi_win_unlock_all_"){
  			I.getOperand(0)->setName("new_win_unlock_all_");
  	 		count_UNLOCKALL++;
  		}else if( I.getOperand(0)->getName() == "mpi_win_unlock_"){
  			I.getOperand(0)->setName("new_win_unlock_");
  	 		count_UNLOCK++;
  		}else if( I.getOperand(0)->getName() == "mpi_win_lock_"){
  			I.getOperand(0)->setName("new_win_lock_");
  	 		count_LOCK++;
  		}else if( I.getOperand(0)->getName() == "mpi_win_lock_all_"){
  			I.getOperand(0)->setName("new_win_lock_all_");
  	 		count_LOCKALL++;
  		}else if( I.getOperand(0)->getName() == "mpi_win_flush_"){
  			I.getOperand(0)->setName("new_win_flush_");
  	 		count_FLUSH++;
  		}else if( I.getOperand(0)->getName() == "mpi_barrier_"){
  			I.getOperand(0)->setName("new_barrier_");
  	 		count_BARRIER++;
  		}

  		if(I.getOperand(0)->getName().startswith("mpix_")){
            count_MPIX++;
  		}else if( I.getOperand(0)->getName() == "mpix_put_notify_"){
  			I.getOperand(0)->setName("new_put_notify_");
  		}else if( I.getOperand(0)->getName() == "mpix_win_create_notify_"){
  			I.getOperand(0)->setName("new_win_create_notify_");
  	 		count_Win_notify++;
  		}else if( I.getOperand(0)->getName() == "mpix_win_wait_notify_"){
  			I.getOperand(0)->setName("new_win_wait_notify_");
  	 		count_Win++;
        }
  	}
  
  	// If the instruction is a call
  	if (CallInst *ci = dyn_cast<CallInst>(&I)) 
  	{
  		if (Function *calledFunction = ci->getCalledFunction())
  		{
  			if(calledFunction->getName().startswith ("MPI_")){
  	 			count_MPI++;
  	 			if(calledFunction->getName() == "MPI_Get"){
  	 				count_GET++;
  					ReplaceCallInst(I, *ci, line, file, Ctx, "new_Get");
						toDelete.push_back(ci);
  	 			}else if(calledFunction->getName() == "MPI_Put"){
  	 				count_PUT++;
  					ReplaceCallInst(I, *ci, line, file, Ctx, "new_Put");
						toDelete.push_back(ci);
  	 			}else if(calledFunction->getName() == "MPI_Win_create"){
  	 				count_Win++;
  					calledFunction->setName("new_Win_create");
  	 			}else if(calledFunction->getName() == "MPI_Accumulate"){
  					calledFunction->setName("new_Accumulate");
  	 				count_ACC++;
  	 			}else if (calledFunction->getName() == "MPI_Win_fence"){
  	 				count_FENCE++;
  					// To uncomment with a CFG traversal
  					//vecAddr.clear();
  				}else if(calledFunction->getName() == "MPI_Win_lock"){
  					calledFunction->setName("new_Win_lock");
  					count_LOCK++;
  				}else if(calledFunction->getName() == "MPI_Win_unlock"){
  					calledFunction->setName("new_Win_unlock");
  					count_UNLOCK++;
  				}else if(calledFunction->getName() == "MPI_Win_unlock_all"){
  					calledFunction->setName("new_Win_unlock_all");
  					count_UNLOCKALL++;
  					// To uncomment with a real CFG traversal
  					//vecAddr.clear();
  				}else if(calledFunction->getName() == "MPI_Win_lock_all"){
  					calledFunction->setName("new_Win_lock_all");
  					count_LOCKALL++;
  				}else if(calledFunction->getName() == "MPI_Win_free"){
  					calledFunction->setName("new_Win_free");
  					count_Free++;
  				}else if(calledFunction->getName() == "MPI_Barrier"){
  					calledFunction->setName("new_Barrier");
  					count_BARRIER++;
  				}
  		    }
  			if(calledFunction->getName().startswith ("MPIX_")){
  	 			count_MPIX++;
  	 			if(calledFunction->getName() == "MPIX_Put_notify"){
  	 				count_PUT_NOTIFY++;
  					ReplaceCallInst(I, *ci, line, file, Ctx, "new_Put_notify");
						toDelete.push_back(ci);
                }else if(calledFunction->getName() == "MPIX_Get_notify"){
  	 				count_GET_NOTIFY++;
  					ReplaceCallInst(I, *ci, line, file, Ctx, "new_Get_notify");
						toDelete.push_back(ci);
  	 			}else if(calledFunction->getName() == "MPIX_Win_create_notify"){
  	 				count_Win_notify++;
  					calledFunction->setName("new_Win_create_notify");
  	 			}else if(calledFunction->getName() == "MPIX_Win_wait_notify"){
  	 				count_WAIT_NOTIFY++;
  					calledFunction->setName("new_Win_wait_notify");
  	 			}else if(calledFunction->getName() == "MPIX_Win_test_notify"){
  	 				count_TEST_NOTIFY++;
  					calledFunction->setName("new_Win_test_notify");
  				}
  		    }
  		}
  	// If the instruction is a LOAD or a STORE
  	} else if( StoreInst *SI = dyn_cast<StoreInst>(&I)){
  		Value* Addr = SI->getPointerOperand();
  		auto elTy = cast<PointerType>(SI->getPointerOperandType())->getElementType();
  		size_t size = datalayout->getTypeSizeInBits(elTy);
  		//DEBUG INFO: //errs() << "PARCOACH DEBUG: Found a store of " << size << " bits of type ";
  		//SI->getPointerOperandType()->print(errs());
  		//errs() << " (TypeID = " << elTy->getTypeID() <<")\n";
  		CreateAndInsertFunction(I,Addr, getHandlerStore(M), size, line, file, Ctx);
  		count_STORE++;
  	} else if(LoadInst *LI = dyn_cast<LoadInst>(&I)){
  		Value* Addr = LI->getPointerOperand();
  		auto elTy = cast<PointerType>(LI->getPointerOperandType())->getElementType();
  		size_t size = datalayout->getTypeSizeInBits(elTy);
  		//DEBUG INFO: // errs() << "PARCOACH DEBUG: Found a load of " << size << " bits of type ";
  		//LI->getPointerOperandType()->print(errs());
  		//errs() << " (TypeID = " << elTy->getTypeID() <<")\n";
  		CreateAndInsertFunction(I,Addr, getHandlerLoad(M), size, line, file, Ctx);
  		count_LOAD++;
  	}
  }
	for(Instruction *i : toDelete){
		i->eraseFromParent();
	}
}


// Main function of the pass
bool ParcoachRMA::runOnFunction(Function &F) 
{
		DataLayout* datalayout = new DataLayout(F.getParent());
		auto CyanErr = []() { return WithColor(errs(), raw_ostream::Colors::CYAN); };
		LLVMContext &Ctx = F.getContext();

		instrumentMemAccesses(F, Ctx, datalayout);
        delete datalayout;

		CyanErr() << "PARCOACH STATISTICS (function " << F.getName() << "): " << count_MPI << " MPI functions including "
                                                      << count_GET << " MPI_Get, "
												      << count_PUT << " MPI_Put, "
													  << count_ACC << " MPI_Accumulate, "
													  << count_FENCE << " MPI_Win_fence, "
													  << count_LOCK << " MPI_Lock, "
													  << count_LOCKALL << " MPI_Lockall "
													  << count_UNLOCK << " MPI_Unlock, "
													  << count_UNLOCKALL << " MPI_Unlockall, "
													  << count_Free << " MPI_Win_free, "
													  << count_BARRIER << " MPI_Barrier, "
													  << count_Win << " MPI_Win_create \n";
		CyanErr() << "PARCOACH STATISTICS (function " << F.getName() << "): " << count_LOAD << " LOAD and " << count_STORE << " STORE\n"; 
		CyanErr() << "PARCOACH STATISTICS (function " << F.getName() << "): " << count_MPIX << " MPIX functions including "
												      << count_PUT_NOTIFY << " MPIX_Put_notify, "
												      << count_GET_NOTIFY << " MPIX_Get_notify, "
												      << count_WAIT_NOTIFY << " MPIX_Win_wait_notify, "
												      << count_TEST_NOTIFY << " MPIX_Win_test_notify, "
													  << count_Win_notify << " MPIX_Win_create_notify \n";
	//DEBUG INFO: dump the module// F.getParent()->print(errs(),nullptr); 

	return true;
}




char ParcoachRMA::ID = 0;
int ParcoachRMA::count_GET = 0;
int ParcoachRMA::count_PUT = 0;
int ParcoachRMA::count_ACC = 0;
int ParcoachRMA::count_Win = 0;
int ParcoachRMA::count_Free = 0;
int ParcoachRMA::count_FENCE = 0;
int ParcoachRMA::count_FLUSH = 0;
int ParcoachRMA::count_LOCK = 0;
int ParcoachRMA::count_LOCKALL = 0;
int ParcoachRMA::count_UNLOCK = 0;
int ParcoachRMA::count_UNLOCKALL = 0;
int ParcoachRMA::count_BARRIER = 0;
int ParcoachRMA::count_MPI = 0;
int ParcoachRMA::count_MPIX = 0;
int ParcoachRMA::count_LOAD = 0;
int ParcoachRMA::count_STORE = 0;
int ParcoachRMA::count_PUT_NOTIFY = 0;
int ParcoachRMA::count_GET_NOTIFY = 0;
int ParcoachRMA::count_WAIT_NOTIFY = 0;
int ParcoachRMA::count_TEST_NOTIFY = 0;
int ParcoachRMA::count_Win_notify = 0;


static RegisterPass<ParcoachRMA> X("parcoach", "Parcoach RMA Pass", true, false);

/* FIXME: This way of registering the pass provokes segfaults with
 * LLVM-9.  Commented for now, if this can be fixed to allow reuse the
 * clang -Xclang -load -Xclang command, it would be great. */
//static RegisterStandardPasses Y(
//    PassManagerBuilder::EP_EarlyAsPossible,
//    [](const PassManagerBuilder &Builder,
//       legacy::PassManagerBase &PM) { PM.add(new ParcoachRMA()); });


