/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
  
  int* sortDfsList = new int[_dfsList.size()];
  for(size_t i = 0; i < _dfsList.size(); i++)
    sortDfsList[i] = _dfsList[i]->_gateId;

  sort(sortDfsList, sortDfsList + _dfsList.size());
  
  int test = 0;
  bool* exist = new bool[_MaxValue + _outPutNum + 1];
  for(int i = 0; i <= _MaxValue + _outPutNum; i++)
    exist[i] = false;
  
  for(size_t i = 0; i < _gateList.size(); i++)
  {
    if(_gateList[i] != NULL and _gateList[i]->_gateId == sortDfsList[test])
    {
      test++;
      exist[i] = true;
    }
  }

  for(size_t i = 0; i <= _MaxValue + _outPutNum; i++)
  {
    if(exist[i] == false)
    { //Sweeping: AIG(5) removed...
      if(_gateList[i] != NULL)
      {
        if(_gateList[i]->_faninList.size())
        {
          cout << "Sweeping: " << _gateList[i]->getTypeStr() << "(" << _gateList[i]->_gateId << ") removed..." << endl;
          deleteConnection(_gateList[i]);
          delete _gateList[i];
          _gateList[i] = NULL;
          _AigNum--;
        }
        /*
        else if(_gateList[i]->_unDef)
        {
          cout << "Sweeping: " << _gateList[i]->getTypeStr() << "(" << _gateList[i]->_gateId << ") removed..." << endl;
          delete _gateList[i];
          _gateList[i] = NULL;
        }
        */
      }
    }
  }
  delete[] sortDfsList;
  sortDfsList = nullptr;
  delete[] exist;
  exist = nullptr;
  
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...

void
CirMgr::optimize()
{
  for(size_t i = 0; i < _dfsList.size(); i++){
    if(_dfsList[i]->_faninList.size() == 2)
    {
      //      Const0 case
      if((_dfsList[i]->_faninList[0]->_gateId == 0 and _dfsList[i]->_invPhase[0] == false) or
         (_dfsList[i]->_faninList[1]->_gateId == 0 and _dfsList[i]->_invPhase[1] == true) )
      {
        printOpt(_dfsList[i]->_faninList[0], _dfsList[i]);

        for(size_t j = 0; j < _dfsList[i]->_fanoutList.size(); j++)
          if(_dfsList[i]->_invPhase[0])   // if fanin[0] = inverse
            setNewInv(_dfsList[i]->_fanoutList[j], _dfsList[i]);

        doubleConnection(_dfsList[i]->_faninList[0], _dfsList[i]);
        cutAnotherFanout(_dfsList[i]->_faninList[1], _dfsList[i]);

        delete _dfsList[i];
        _gateList[_dfsList[i]->_gateId] = NULL;
        _dfsList[i] = NULL;
        _AigNum--;
      }

      else if((_dfsList[i]->_faninList[1]->_gateId == 0 and _dfsList[i]->_invPhase[1] == false)  or
              (_dfsList[i]->_faninList[0]->_gateId == 0 and _dfsList[i]->_invPhase[0] == true) )
      {
        printOpt(_dfsList[i]->_faninList[1], _dfsList[i]);

        for(size_t j = 0; j < _dfsList[i]->_fanoutList.size(); j++)
          if(_dfsList[i]->_invPhase[1])   // if fanin[0] = inverse
            setNewInv(_dfsList[i]->_fanoutList[j], _dfsList[i]);
        
        doubleConnection(_dfsList[i]->_faninList[1], _dfsList[i]);
        cutAnotherFanout(_dfsList[i]->_faninList[0], _dfsList[i]);

        delete _dfsList[i];
        _gateList[_dfsList[i]->_gateId] = NULL;
        _dfsList[i] = NULL;
        _AigNum--;
      }
      //    identical Fanin Case
      else if(_dfsList[i]->_faninList[0] == _dfsList[i]->_faninList[1] and _dfsList[i]->_invPhase[0] == _dfsList[i]->_invPhase[1])
      {
        //cout << "Case3: ";
        printOpt(_dfsList[i]->_faninList[0], _dfsList[i]);

        bool* setAlready = new bool[_gateList.size()];
        for(size_t i = 0; i < _gateList.size(); i++)
          setAlready[i] = false;
        //cout << "test1" << endl;
        for(size_t j = 0; j < _dfsList[i]->_fanoutList.size(); j++){
          if(_dfsList[i]->_invPhase[0]){           // if both inverse
              setNewInv2(_dfsList[i]->_fanoutList[j], _dfsList[i], setAlready);
              setAlready[_dfsList[i]->_fanoutList[j]->_gateId] = true;
          }
        }

        doubleConnection(_dfsList[i]->_faninList[0], _dfsList[i]);
        delete _dfsList[i];
        _gateList[_dfsList[i]->_gateId] = NULL;
        _dfsList[i] = NULL;
        _AigNum--;
        delete[] setAlready;
        setAlready = NULL;
      }
      //    Inverted fanins
      else if(_dfsList[i]->_faninList[0] == _dfsList[i]->_faninList[1] and _dfsList[i]->_invPhase[0] != _dfsList[i]->_invPhase[1])
      {
        //cout << "Case4: ";
        printOpt(_gateList[0], _dfsList[i]);
        
        ConnectToZero(_gateList[0], _dfsList[i]);
        cutAnotherFanout(_dfsList[i]->_faninList[0], _dfsList[i]);
        cutAnotherFanout(_dfsList[i]->_faninList[1], _dfsList[i]);

        delete _dfsList[i];
        _gateList[_dfsList[i]->_gateId] = NULL;
        _dfsList[i] = NULL;
        _AigNum--;
      }
    }
  }


  _dfsList.clear();
  genDFSList();
  
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

void
CirMgr::deleteConnection(CirGate*& base)
{
    if(base->_faninList[0] != NULL and !base->_faninList[0]->_unDef)
      for(size_t i = 0; i < base->_faninList[0]->_fanoutList.size(); i++)
        if(base->_faninList[0]->_fanoutList[i] == base)
          base->_faninList[0]->_fanoutList.erase(base->_faninList[0]->_fanoutList.begin() + i);

    if(base->_faninList[1] != NULL and !base->_faninList[1]->_unDef)
      for(size_t i = 0; i < base->_faninList[1]->_fanoutList.size(); i++)
        if(base->_faninList[1]->_fanoutList[i] == base)
          base->_faninList[1]->_fanoutList.erase(base->_faninList[1]->_fanoutList.begin() + i);
}

void
CirMgr::doubleConnection(CirGate*& target, CirGate*& original)
{
  for(size_t i = 0; i < original->_fanoutList.size(); i++)
    for(size_t j = 0; j < original->_fanoutList[i]->_faninList.size(); j++)
      if(original->_fanoutList[i]->_faninList[j] == original)
        original->_fanoutList[i]->_faninList[j] = target;
  
  size_t a = (target->_fanoutList.size() - 1);
  
  for(int i = a; i >= 0; i--)
    if(target->_fanoutList[i] == original)
      target->_fanoutList.erase(target->_fanoutList.begin() + i);

  for(size_t i = 0; i < original->_fanoutList.size(); i++)
    target->_fanoutList.push_back(original->_fanoutList[i]);

}

void
CirMgr::ConnectToZero(CirGate*& target, CirGate*& original)
{
  for(size_t i = 0; i < original->_fanoutList.size(); i++)
    for(size_t j = 0; j < original->_fanoutList[i]->_faninList.size(); j++)
      if(original->_fanoutList[i]->_faninList[j] == original)
        original->_fanoutList[i]->_faninList[j] = target;
  for(size_t i = 0; i < original->_fanoutList.size(); i++)
    target->_fanoutList.push_back(original->_fanoutList[i]);
}
void
CirMgr::cutAnotherFanout(CirGate*& base, CirGate*& original)
{
  for(size_t k = 0; k < base->_fanoutList.size(); k++)
    if(base->_fanoutList[k] == original)
      base->_fanoutList.erase(base->_fanoutList.begin() + k);
}

void
CirMgr::setNewInv(CirGate*& base, CirGate*& original)
{
  for(size_t i = 0; i < base->_faninList.size(); i++)
  {
    if(base->_faninList[i] == original)
    {
      if(base->_invPhase[i] == 0)
        base->_invPhase[i] = 1;
      else if(base->_invPhase[i] == 1)
        base->_invPhase[i] = 0;
      return;
    }
  }
}

void
CirMgr::printOpt(CirGate*& Replace, CirGate*& beReplace) const
{
  cout << "Simplifying: " << Replace->_gateId << " merging "; //<< beReplace->_gateId << "..." << endl;
  if(beReplace->_invPhase[0] and beReplace->_invPhase[1])
    cout << "!";
  cout << beReplace->_gateId << "..." << endl;
}

void
CirMgr::setNewInv2(CirGate*& base, CirGate*& original, bool*& setAlready)
{
  for(size_t i = 0; i < base->_faninList.size(); i++)
  {
    if(base->_faninList[i] == original)
    {
      if(base->_invPhase[i] == 0 and setAlready[base->_gateId] == false)
        base->_invPhase[i] = 1;
      else if(base->_invPhase[i] == 1 and setAlready[base->_gateId] == false)
        base->_invPhase[i] = 0;
    }
  }
}