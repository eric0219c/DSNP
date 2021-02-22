/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <algorithm>
#include <vector>
#include <ctime>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"
//#include "myHashSet.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
    HashSet<HashKey, GateList> _hashSet(_gateList.size() / 2);

    for(size_t i = 0; i < _dfsList.size(); i++){
      //cout << "I: " << i << endl;
      if(_dfsList[i]->isAig())
      {
        size_t a = 2 * (_dfsList[i]->_faninList[0]->_gateId);
        if(_dfsList[i]->_invPhase[0]) { a++; }
        size_t b = 2 * (_dfsList[i]->_faninList[1]->_gateId);
        if(_dfsList[i]->_invPhase[1]) { b++; }

        HashKey hashKey(a, b, _dfsList[i]->_gateId);
        hashKey.swap();
        size_t mergeGate;
        if(_hashSet.check(hashKey, mergeGate))
        {//Strashing: 1216 merging 3386...
          cout << "Strashing: " << _gateList[mergeGate]->_gateId << " merging " << _dfsList[i]->_gateId << "..." << endl;
          
          _gateList[mergeGate]->merge(_dfsList[i]);
          //delete _dfsList[i];
          _gateList[_dfsList[i]->_gateId] = NULL;
          _dfsList[i] = NULL;
          _AigNum--;
        }
        else
          _hashSet.insert(hashKey);
      }
    }
    
    _dfsList.clear();
    genDFSList();
}
//-------------------------------------------------------------------------------------------------------------------------//

void
CirMgr::fraig()
{
   SatSolver solver;
   solver.initialize();  
   setDfsNum();   
   //
   int* findMinInFec;
   while(_FecGroups.size()){
      findMinInFec =  new int[_FecGroups.size()];
      genProofModel(solver);
      doProve(solver, findMinInFec);
   }

}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

void
CirMgr::genProofModel(SatSolver& s)
{
   // Allocate and record variables; No Var ID for POs
   Var v = s.newVar();
   _gateList[0]->setVar(v);
   for (size_t i = 0, n = _dfsList.size(); i < n; ++i) {
      Var v = s.newVar();
      if(_dfsList[i] and _dfsList[i]->_gateId != 0)
         _dfsList[i]->setVar(v);
   }

   // Hard code the model construction here...
   // [2] AIG 4 1 2 ==> [2] = [0] & [1]
   s.addAigCNF(_gateList[0]->getVar(), _gateList[0]->getVar(), true, _gateList[0]->getVar(), false);
   for(int i = 0; i < _dfsList.size(); i++){
      if(_dfsList[i]->isAig())
      {
      s.addAigCNF(_dfsList[i]->getVar(), 
                _dfsList[i]->_faninList[0]->getVar(), _dfsList[i]->_invPhase[0],
                _dfsList[i]->_faninList[1]->getVar(), _dfsList[i]->_invPhase[1]);
      }
   }
}


void
CirMgr::fraigMerge(CirGate*& replace, CirGate*& beReplace, bool& inverse)
{
   for(int i = 0; i < beReplace->_fanoutList.size(); i++)
      replace->_fanoutList.push_back(beReplace->_fanoutList[i]);
   
   for(int i = 0; i < beReplace->_fanoutList.size(); i++){
      for(int j = 0; j < beReplace->_fanoutList[i]->_faninList.size(); j++){
         if(beReplace->_fanoutList[i]->_faninList[j] == beReplace){
            beReplace->_fanoutList[i]->_faninList[j] = replace;     

            if(inverse){
               if(beReplace->_fanoutList[i]->_invPhase[j])
                  beReplace->_fanoutList[i]->_invPhase[j] = 0;
               else
                  beReplace->_fanoutList[i]->_invPhase[j] = 1;
            }
         }
      }
   }

   for(int i = 0; i < beReplace->_faninList.size(); i++){
      for(int j = 0; j < beReplace->_faninList[i]->_fanoutList.size(); j++){
         if(beReplace->_faninList[i]->_fanoutList[j] == beReplace)
            beReplace->_faninList[i]->_fanoutList.erase(beReplace->_faninList[i]->_fanoutList.begin() + j);
      }
   }

   delete beReplace;
   beReplace = NULL;
}

void 
CirMgr::reportResult(const SatSolver& solver, bool result) const
{
   solver.printStats();
   cout << (result? "SAT" : "UNSAT") << endl;
   if (result) {
      for (size_t i = 0, n = _gateList.size(); i < n; ++i)
         cout << solver.getValue(_gateList[i]->getVar()) << endl;
   }
}
void
CirMgr::doProve(SatSolver& solver, int*& findMinInFec)
{
   findMinForFec(findMinInFec); 
   bool result;
   Var newV = solver.newVar();
   int countSat = 0;
   bool UnSat = false;
   vector<size_t*> SatSimValue;
   SatSimValue.push_back( new size_t[_piList.size()] );
   int idx = 0;
   for(int i = 0; i < _piList.size(); i++)
      SatSimValue[idx][i] = 0;
   //int idx = 0;

   for(int i = 0; i < _FecGroups.size(); i++){
      int min = findMinInFec[i];
      unsigned num1 = (*_FecGroups[i])[min] / 2;

      for(int j = 0; j < _FecGroups[i]->size(); j++){
         if(j != min){
            newV = solver.newVar();
            unsigned num2 = (*_FecGroups[i])[j] / 2;
            if(_gateList[num1] and _gateList[num2]){
               bool inv1 = (*_FecGroups[i])[min] % 2 == 0? false : true;
               bool inv2 = (*_FecGroups[i])[j] % 2 == 0? false : true;
               bool reverse = inv1 == inv2? false : true;
               solver.addXorCNF(newV, _gateList[num1]->getVar(), inv1, _gateList[num2]->getVar(), inv2);
               solver.assumeRelease();  // Clear assumptions
               solver.assumeProperty(newV, true);  // k = 1

               result = solver.assumpSolve();           
               // if false, UNSAT -> equivalent -> merge;
               if(!result){
                  UnSat = true;
                  fraigMerge(_gateList[num1], _gateList[num2], reverse);
                  (*_FecGroups[i])[j] = -1;              // to express this need to be delete in the future;
                  cout << "Fraig: " << num1 << " merging ";
                  if(reverse) cout << "!";
                  cout << num2 << "..." << endl;
                  _AigNum--;
               }
            }
            
               // if true, SAT -> not equivalent !
               if(result){     //solver.getValue(gates[i]->getVar()) << endl;
                  countSat++;
                  for(int i = 0; i < _piList.size(); i++){
                     SatSimValue[idx][i] <<= 1;
                     SatSimValue[idx][i] += solver.getValue(_piList[i]->getVar());
                  }
                  if(countSat % 64 == 0){
                     SatSimValue.push_back( new size_t[_piList.size()] );
                     idx++;
                     for(int i = 0; i < _piList.size(); i++)
                        SatSimValue[idx][i] = 0;
                  }
               }
            
            }
         }
      }
   _dfsList.clear();
   genDFSList();
   cleanFEC();
   eraseFEC();             // to pack FEC
   if(UnSat)
      cout << "Updating by UNSAT... Total #FEC Group = " << _FecGroups.size() << endl;
   if(countSat != 0){
      for(int i = 0; i < SatSimValue.size(); i++)
         SatSimulation(findMinInFec, countSat, SatSimValue, i);
   }
   _dfsList.clear();
   genDFSList();
   delete[] findMinInFec;
   findMinInFec = NULL;
}
void
CirMgr::findMinForFec(int* findMinInFec)
{
   int minJ = 0;
   for(int i = 0; i < _FecGroups.size(); i++){
      int min = (*_FecGroups[i])[0];
      minJ = 0;
      for(int j = 1; j < _FecGroups[i]->size(); j++){
         unsigned int num = (*_FecGroups[i])[j];
         if(_gateList[num / 2]->_dfsNum < _gateList[min / 2]->_dfsNum){
            min = num;
            minJ = j;
         }
      }
      findMinInFec[i] = minJ;
   }
}

void 
CirMgr::setDfsNum()
{
   _gateList[0]->_dfsNum = -1;
   for(int i = 0; i < _dfsList.size(); i++)
      _dfsList[i]->_dfsNum = i;
}

void
CirMgr::eraseFEC()
{
   for(int i = 0; i < _FecGroups.size(); i++)
      for(int j = _FecGroups[i]->size() - 1; j >= 0; j--)
         if( (*_FecGroups[i])[j] == -1 or _gateList[(*_FecGroups[i])[j] / 2]->_inDfs == false)
            (*_FecGroups[i]).erase( (*_FecGroups[i]).begin() + j);

   for(int i = _FecGroups.size() - 1; i >= 0; i--)
      if((*_FecGroups[i]).size() == 1 or (*_FecGroups[i]).size() == 0)
         _FecGroups.erase(_FecGroups.begin() + i);
}

void
CirMgr::SatSimulation(int*& findMinInFec, int& countSat, vector<size_t*>& SatSimValue, int idx)
{
   // to generate new FEC GRPS.
   eraseFEC();  
   _dfsList.clear();
   genDFSList();

   for(int i = 0; i < _piList.size(); i++)
      _piList[i]->_simValue = SatSimValue[idx][i];
   gateSimulation();
   HashSimulation();
   cout << "Updating by SAT... Total #FEC Group = " << _FecGroups.size() << endl;
   //Updating by SAT... Total #FEC Group = 0
}

void
CirMgr::resetMinInFec(int*& findMinInFec)
{
   findMinInFec = new int[_FecGroups.size()];
   findMinForFec(findMinInFec);
   for(int i = 0; i < _FecGroups.size(); i++){
      int ID = (*_FecGroups[i])[findMinInFec[i]];
      _gateList[ID / 2]->_dfsNum = 1000000;
   }
   //findMinForFec(findMinInFec);
}

void 
CirMgr::cleanFEC()
{
   int* sortDfsList = new int[_dfsList.size()];
  for(size_t i = 0; i < _dfsList.size(); i++)
    sortDfsList[i] = _dfsList[i]->_gateId;

  sort(sortDfsList, sortDfsList + _dfsList.size());
  
  int test = 0;
  bool* exist = new bool[_MaxValue + _outPutNum + 1];
  for(int i = 0; i <= _MaxValue + _outPutNum; i++)
    exist[i] = false;
  
  for(size_t i = 0; i < _gateList.size(); i++){
    if(_gateList[i] != NULL and _gateList[i]->_gateId == sortDfsList[test]){
      test++;
      exist[i] = true;
    }
  }

  for(size_t i = 0; i <= _MaxValue + _outPutNum; i++)
    if(exist[i] == false)
      if(_gateList[i] != NULL)
        _gateList[i]->_inDfs = false;

   delete[] sortDfsList;
   sortDfsList = nullptr;
   delete[] exist;
   exist = nullptr;
  

}