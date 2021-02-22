/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))       // 超過 一個 option
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {    // option 不能小於 84
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
   cout << setw(15) << left << "MTReset: " 
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
   // TODO
   /* 1. lexOptions() == false
      2. If the specified “numObjects” is not a legal integer or is not a positive number.
      3. If the parameter “numObjects” is not specified or is specified multiple times.
      4. If the specified “arraySize” is not a legal integer or is not a positive number.
      5. If the parameter “arraySize” is specified multiple times.
      6. Any other syntax error with respect to the command usage.
      7. Requested memory of the object or array is greater than the block size of MemBlock (by catching “bad_alloc()”). */
   // Use try-catch to catch the bad_alloc exception
   int num = 0;
   int arrayNum = 0;
   
   vector<string> options; string test;
   int location = 0; 
  
//-------------------------------------------------------------------------------------------------------------------//
   bool newArray = false;
   if (!CmdExec::lexOptions(option, options))     //已把 option 塞入 options 這個動態陣列
      return CMD_EXEC_ERROR;
   
   for (size_t i = 0; i < options.size(); ++i) 
   {
      if (myStrNCmp("-Array", options[i], 2) == 0) 
      { 
         location = i;                  // 紀錄 -A 位置
         newArray = true;
      }
   }
   
//-------------------------------------------------------------------------------------------------------------------//
   if(newArray == true)                                  // 若 new array = true, 代表要有三段option.
   {
      if(options.size() < 3)
         return CmdExec::errorOption(CMD_OPT_MISSING, "");

      else if(options.size() > 3)
         return CmdExec::errorOption(CMD_OPT_EXTRA, options[options.size() - 1]);
         
      if(myStr2Int(options[location + 1], arrayNum) == false or arrayNum <= 0)           // 如果 -A 下一個 不是數字
      {
         cerr << "Error: Illegal option!! (" << options[location + 1] << ")" <<endl;
         return CMD_EXEC_ERROR;
      } 
      
      if(location == 1)                              // 如果 -A 在 numObjects後面 , numObjects 會在 options[0]
      {
         if(myStr2Int(options[0], num) == false or num <= 0)
         {
            cerr << "Error: Illegal option!! (" << options[0] << ")" <<endl;
            return CMD_EXEC_ERROR;
         } 
      }
      
      else if(location == 0 and newArray == true)    // 如果 -A 在 numObjects前面 , numObjects 會在 options[2]
      {
         if(myStr2Int(options[2], num) == false or num <= 0) 
         {
            cerr << "Error: Illegal option!! (" << options[2] << ")" <<endl;
            return CMD_EXEC_ERROR;
         } 
      }
   try {mtest.newArrs(num, arrayNum);} catch (bad_alloc){return CMD_EXEC_ERROR;} 
   
   return CMD_EXEC_DONE;
   }
//-------------------------------------------------------------------------------------------------------------------//
   if(newArray == false)                                        // 當不是要新增 array 時, command應該是 mtn 200 之類的
   {
      bool single = CmdExec::lexSingleOption(option, test, false);

      if (single == false)     // 如果不是 single option
         return CMD_EXEC_ERROR;

      else if(single == true)             // only one mandatory option
      {
         if(myStr2Int(test, num) == false or num <= 0)   //If “numObjects” is not a legal integer or is not a positive number.
         {
            cerr << "Error: Illegal option!! (" << test << ")" <<endl;
            return CMD_EXEC_ERROR;
         }
         else
         {
            try {mtest.newObjs(num);} catch (bad_alloc){return CMD_EXEC_ERROR;} 
            return CMD_EXEC_DONE;
         }
      }
   }
   

//----------------------------------------------//
}

void
MTNewCmd::usage(ostream& os) const
{  
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
   cout << setw(15) << left << "MTNew: " 
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
//The “indices” (index) (See command “MTDelete”) of the objects/arrays are their positions in the vectors. 
//They are used to find the objects/arrays for the “delete/delete[]” related commands. 
//When an object/array is deleted, the corresponding position in the list is set to ‘0’ (i.e. NULL). 
//If another “delete/delete[]” command is trying to delete the object/array with the same index, 
//there will be no action and no error message.

/* 1. lexOptions() == false
   2. If both “-Index” and “-Random” are specified.
   3. If none of “-Index” or “-Random” is specified.
   4. If the parameter “-Index” is specified multiple times.
   5. If the specified “objId” (for “-Index”) is not a legal integer, is smaller than 0, 
      or is greater than or equal to the size of “_objList” (if no “-Array”) or “_arrList” (if with “-Array”). 
      Print out error message like:
      Size of object list (10) is <= 12!! or Size of array list (10) is <= 12!!

// Assume size of object/array list is 10, and 12 is the specified parameter
   6. If the parameter “-Random” is specified multiple times.
   7. If the specified “numRandId” is not a legal integer, or is not a positive number.
   8. If the parameter “-Random (size_t numRandId)” is specified but the “_objList” (if no “-Array”) or “_arrList” (if with “-Array”) is empty.
      In such case, print out error message:  Size of object list is 0!! or Size of array list is 0!!
   9. Any other syntax error with respect to the command usage.*/
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
   int objId = 0;
   vector<string> options; string test;
   int location = 0; int locationArray = 0;
   bool index = false, random = false, array = false;
//-------------------------------------------------------------------------------------------------------------------//
   if (!CmdExec::lexOptions(option, options))     //已把 option 塞入 options 這個動態陣列
      return CMD_EXEC_ERROR;

   for (size_t i = 0; i < options.size(); ++i) 
   {
      if (myStrNCmp("-Index", options[i], 2) == 0) 
      { 
         location = i;                  // 紀錄 -A 位置
         index = true;
      }
      else if(myStrNCmp("-Random", options[i], 2) == 0) 
      { 
         location = i;                  // 紀錄 -A 位置
         random = true;
      }
      if(myStrNCmp("-Array", options[i], 2) == 0) 
      { 
         locationArray = i;                  // 紀錄 -A 位置
         array = true;
      }
   }
//-------------------------------------------------------------------------------------------------------------------//
   if(index == true and random == true)                     // 2. If both “-Index” and “-Random” are specified.
   {
      cerr << "Error: “-Index” and “-Random” cannot be specified at the same time!! " << endl;
      return CMD_EXEC_ERROR;
   }
   else if(index == false and random == false)              // 3. If none of “-Index” or “-Random” is specified.
   {
      cerr << "Error: “-Index” or “-Random” should be specified at least one!! " << endl;
      return CMD_EXEC_ERROR;
   }
//-------------------------------------------------------------------------------------------------------------------//
   if(array == true)                                     // 若有 -A 長度應為 3
   {
         if(options.size() < 3)                                // 若有 -A 長度卻小於 3
            return CmdExec::errorOption(CMD_OPT_MISSING, "");       
         else if(options.size() > 3)                           // 若有 -A 長度卻大於 3
            return CmdExec::errorOption(CMD_OPT_EXTRA, options[options.size()-1]);
      //----------------------------------------------------//  
         if(locationArray == 0)                                // 如果 -A 位置在最前面
         {
            if(myStr2Int(options[2], objId) == false or objId < 0)   //If “objId” is not a legal integer or is not a positive number.
            {
               cerr << "Error: Illegal option!! (" << options[2] << ")" <<endl;
               return CMD_EXEC_ERROR;
            }
         }
         else if(locationArray == 2)
         {
            if(myStr2Int(options[1], objId) == false or objId < 0)   //If “objId” is not a legal integer or is not a positive number.
            {
               cerr << "Error: Illegal option!! (" << options[1] << ")" <<endl;
               return CMD_EXEC_ERROR;
            }
         }
      
//--------------------------------- index or random ---------------------------------------//
      if(index == true)
      { 
         if(objId >= mtest.getArrListSize())
         {
            cerr << "Error: Size of array list (" << mtest.getArrListSize() << ") is <= " << objId << "!!" << endl;
            return CMD_EXEC_ERROR;
         }
         try {mtest.deleteArr(objId);} catch (bad_alloc){return CMD_EXEC_ERROR;}
      }
      else if(random == true)
      {
         if(mtest.getArrListSize() == 0)
         {
            cerr << "Size of array list is 0!!" << endl;
            return CMD_EXEC_ERROR;
         }
         for(int i = 1; i <= objId; i++)
         {
            int randomNum = rnGen(mtest.getArrListSize() );
            try {mtest.deleteArr(randomNum);} catch (bad_alloc){return CMD_EXEC_ERROR;}
         }
      }
   }
   
//-------------------------------------------------------------------------------------------------------------------//
   else if(array == false)                // object case
   {
      if(options.size() < 2)
         return CmdExec::errorOption(CMD_OPT_MISSING, "");
      else if(options.size() > 2)
         return CmdExec::errorOption(CMD_OPT_EXTRA, options[options.size()-1]);
      if(myStr2Int(options[1], objId) == false or objId < 0)
      {
         cerr << "Error: Illegal option!! (" << options[1] << ")" <<endl;
         return CMD_EXEC_ERROR;
      }
      
//--------------------------------- index or random ---------------------------------------//
      if(index == true)
      {
         if(objId >= mtest.getObjListSize())
         {
            cerr << "Error: Size of object list (" << mtest.getObjListSize() << ") is <= " << objId << "!!" << endl;
            return CMD_EXEC_ERROR;
         }
         try {mtest.deleteObj(objId);} catch (bad_alloc){return CMD_EXEC_ERROR;}
      }
      else if(random == true)
      {
         if(mtest.getObjListSize() == 0)
         {
            cerr << "Size of object list is 0!!" << endl;
            return CMD_EXEC_ERROR;
         }
         for(int i = 1; i <= objId; i++)
         {
            int randomNum = rnGen(mtest.getObjListSize() );
            try {mtest.deleteObj(randomNum);} catch (bad_alloc){return CMD_EXEC_ERROR;}
         }
      }
      /*
      If the specified “objId” (for “-Index”) is not a legal integer, is smaller than 0, 
      or is greater than or equal to the size of “_objList” (if no “-Array”) or “_arrList” (if with “-Array”). 
      Print out error message like:
      Size of object list (10) is <= 12!! or Size of array list (10) is <= 12!!
      
      8. If the parameter “-Random (size_t numRandId)” is specified but the “_objList” (if no “-Array”) 
      or “_arrList” (if with “-Array”) is empty.
      In such case, print out error message:  Size of object list is 0!! or Size of array list is 0!!*/
   }

   return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{  
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
   cout << setw(15) << left << "MTDelete: " 
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
   cout << setw(15) << left << "MTPrint: " 
        << "(memory test) print memory manager info" << endl;
}


