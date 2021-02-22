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
   string report ="= " + this->getTypeStr() + "(" + id + "), line " + lineNo;

   if(this->_unDef == true)
   {
      cout << "==================================================" << endl;
      cout << setw(49) << left << report << "=" << endl;
      cout << "==================================================" << endl;
   }
   else
   {
      cout << "==================================================" << endl;
      cout << setw(49) << left << report << "=" << endl;
      cout << "==================================================" << endl;
   }
   
}


void
CirGate::reportFanin(int level) 
{
   assert (level >= 0);
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
   for(size_t i = 0; i < this->_fanin.size(); i++)
   {
      if(!this->_faninList[i]->isGlobalRef())
      {
         this->_faninList[i]->setToGlobalRef();
         this->_faninList[i]->dfsTraversal(_dfsList);
      }
   }
   if(this->_unDef == false)
      _dfsList.push_back(this);
   
}

void
CirGate::recurInLevel(GateList& _faninLevelList, int level)
{
   printInLevel(this, level);
   _faninLevelList.push_back(this);
   
   if(this->_levelTest == level or this->_repeat == true)
      return;

   this->_repeat = true;

   if(this->_fanin.size() != 0)
   {
      for(size_t i = 0; i < this->_fanin.size(); i++)
      {
         setInInvSelf(this);
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
         this->_fanoutList[i]->_levelTest = this->_levelTest + 1;
         this->_fanoutList[i]->recurOutLevel(_fanoutLevelList, level);
      }
   }
}

string
CirGate::getTypeStr() const 
{
   if(this->_unDef == true)
      return "UNDEF";
   if(this->_fanin.size() == 0 and this->_gateId == 0)
     return "CONST";
   if(this->_fanin.size() == 0 and this->_gateId != 0)
      return "PI";
   else if(this->_fanin.size() == 1)
     return "PO";
   else if(this->_fanin.size() == 2)
     return "AIG";
}

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

void
CirGate::setOutInvSelf(CirGate* test)
{
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
            {
               test->_fanoutList[i]->_invMySelf = false;
            }
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
   if(test->_repeat == true and test->_levelTest != level and test->_fanin.size() != 0)
      cout << " (*)";
   cout << endl;
}