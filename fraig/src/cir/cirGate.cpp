/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   stringstream ss1, ss2; string lineNo; string id;
   ss1 << this->getLineNo();  ss1 >> lineNo;
   ss2 << this->_gateId;      ss2 >> id;
   string symbol = "";
   if(this->_symbol != "")
      symbol = "\"" + this->_symbol + "\"";
   string report ="= " + this->getTypeStr() + "(" + id + ")" + symbol + ", line " + lineNo;

   cout << "================================================================================" << endl;
   cout << report << endl;                         //setw(49) << left << report << "=" << endl;
   cout << "= FECS: ";  cirMgr->printFecGate(this);
   cout << "= Value: " ; cirMgr->printGateSimV(this);
   cout << "================================================================================" << endl;
   
   /*
   = PO(25)”23GAT$PO”, line 9 
   = FECs:
   = Value: 00110010_11110101_00110001_10001010_00100100_11101110_10010100_11111001
   */
}


void
CirGate::reportFanin(int level) 
{
   assert (level >= 0);
   GateList _faninLevelList;
   this->recurInLevel(_faninLevelList, level);
   for(size_t i = 0; i < _faninLevelList.size(); i++)
   {
      _faninLevelList[i]->_invMySelf = false;         // reset
      _faninLevelList[i]->_levelTest = 0;
      _faninLevelList[i]->_repeat = false;
   }
   _faninLevelList.clear();
}

void
CirGate::reportFanout(int level)
{
   assert (level >= 0);
   GateList _fanoutLevelList;
   this->recurOutLevel(_fanoutLevelList, level);
   //cout << "SIZE: " << _fanoutLevelList.size() << endl;
   for(size_t i = 0; i < _fanoutLevelList.size(); i++)
   {
      _fanoutLevelList[i]->_invMySelf = false;
      _fanoutLevelList[i]->_levelTest = 0;
      _fanoutLevelList[i]->_repeat = false;
   }
   _fanoutLevelList.clear();
}

void
CirGate::dfsTraversal(GateList& _dfsList)
{
   for(size_t i = 0; i < this->_faninList.size(); i++)
   {
      if(!this->_faninList[i]->isGlobalRef())
      {
         this->_faninList[i]->setToGlobalRef();
         this->_faninList[i]->dfsTraversal(_dfsList);
      }
   }
   if(this->getTypeStr() != "UNDEF"){
      _dfsList.push_back(this);
      this->_inDfs = true;
   }
   
}

void
CirGate::recurInLevel(GateList& _faninLevelList, int level)
{
   printInLevel(this, level);
   _faninLevelList.push_back(this);
   
   if(this->_levelTest == level or this->_repeat == true)
      return;

   this->_repeat = true;

   if(this->_faninList.size() != 0)
   {
      for(size_t i = 0; i < this->_faninList.size(); i++)
      {
         if(this->_invPhase[i])
            this->_faninList[i]->_invMySelf = true;
         else
            this->_faninList[i]->_invMySelf = false;
         this->_faninList[i]->_levelTest = this->_levelTest + 1;
         this->_faninList[i]->recurInLevel(_faninLevelList, level);
      }
   }
}

void
CirGate::recurOutLevel(GateList& _fanoutLevelList, int level)
{
   setOutInvSelf(this);
   printInLevel(this, level);
   _fanoutLevelList.push_back(this);
   
   if(this->_levelTest == level or this->_repeat == true)
      return;

   this->_repeat = true;
   if(this->_fanoutList.size() != 0)
   {
      for(size_t i = 0; i < this->_fanoutList.size(); i++)
      {
         setOutInvSelf(this);
         this->_fanoutList[i]->_levelTest = this->_levelTest + 1;
         this->_fanoutList[i]->recurOutLevel(_fanoutLevelList, level);
      }
   }
}
/*
string
CirGate::getTypeStr() const 
{
   if(this->_unDef == true)
      return "UNDEF";
   if(this->_faninList.size() == 0 and this->_gateId == 0)
     return "CONST";
   if(this->_faninList.size() == 0 and this->_gateId != 0)
      return "PI";
   else if(this->_faninList.size() == 1)
     return "PO";
   else if(this->_faninList.size() == 2)
     return "AIG";
}
/*
void
CirGate::setInInvSelf(CirGate* test)
{
   if(test->_faninList.size() == 0)
      return;
   if(test->_faninList.size() == 1)
   {
      if(test->_invPhase[0] == true)
         test->_faninList[0]->_invMySelf = true;
      else
         test->_faninList[0]->_invMySelf = false;
   }
   else if(test->_faninList.size() == 2)
   {
      if(test->_invPhase[0] == true)
         test->_faninList[0]->_invMySelf = true;
      else
         test->_faninList[0]->_invMySelf = false;

      if(test->_invPhase[1] == true)
         test->_faninList[1]->_invMySelf = true;
      else 
         test->_faninList[1]->_invMySelf = false;
   }
}
*/
void
CirGate::setOutInvSelf(CirGate* test)
{
  // cout << "             ID: " << test->_gateId << endl;
   if(test->_fanoutList.size() == 0)
      return;
   for(size_t i = 0; i < test->_fanoutList.size(); i++)
   {
      if(test->_fanoutList[i]->_faninList.size() != 0)
      {
         for(size_t j = 0; j < test->_fanoutList[i]->_faninList.size(); j++)
         {
            if(test->_fanoutList[i]->_faninList[j] == test and test->_fanoutList[i]->_invPhase[j] == true)
            {
               test->_fanoutList[i]->_invMySelf = true;
               break;
            }
            else
               test->_fanoutList[i]->_invMySelf = false;
         }
      }
   }
}
void
CirGate::printInLevel(CirGate* test, int level) const
{
   for(size_t i = 1; i <= test->_levelTest; i++)
      cout << "  ";
   if(test->_invMySelf == true)
      cout << "!";
   cout << test->getTypeStr() << " " << test->_gateId;
   if(test->_repeat == true and test->_levelTest != level and test->_faninList.size() != 0)
      cout << " (*)";
   cout << endl;
}

void
CirGate::merge(CirGate*& beMerge)
{
   for(size_t i = 0; i < beMerge->_fanoutList.size(); i++)
   {
      _fanoutList.push_back(beMerge->_fanoutList[i]);
      for(size_t j = 0; j < beMerge->_fanoutList[i]->_faninList.size(); j++)
      {
         if(beMerge->_fanoutList[i]->_faninList[j] == beMerge)
            beMerge->_fanoutList[i]->_faninList[j] = this;
      }
      beMerge->_fanoutList[i] = NULL;
   }
   for(size_t i = 0; i < beMerge->_faninList.size(); i++)
      for(int j = (beMerge->_faninList[i]->_fanoutList.size() - 1) ; j >= 0; j--)
         if(beMerge->_faninList[i]->_fanoutList[j] == beMerge)
            beMerge->_faninList[i]->_fanoutList.erase(beMerge->_faninList[i]->_fanoutList.begin() + j);

   
   delete beMerge;
   //beMerge = NULL;
}