/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"


using namespace std;
#define gateList_simulation_test 0
// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
class SimValueKey
{
public:
   SimValueKey(size_t a, int b, IdList* f = 0) : _simValue(a), _id(b){}
   ~SimValueKey(){}

   size_t operator () () const { return _simValue; }   // as hash function
   bool operator == (const SimValueKey& k) const { return(_simValue == k._simValue); }
   bool operator != (const SimValueKey& k) const { return(_simValue == ~(k._simValue)); }
   size_t getSimValue() const { return _simValue; }
   IdList*  getFecGrp() const{ return _fecGrp; }
   void  setFecGrp(IdList*& d) { _fecGrp = d; }
   int       getId() const  { return _id;}
private:
   size_t    _simValue;
   int         _id;
   IdList*     _fecGrp;
};
/**************************************/
/*   Static varaibles and functions   */
/**************************************/
void swap(IdList*& a, IdList*& b){
    IdList* temp = a;
    a = b;
    b = temp;
}

int Partition(vector<IdList*>& arr, int front, int end){
    int pivot = (*arr[end])[0];
    int i = front -1;
    for (int j = front; j < end; j++) {
        if ((*arr[j])[0] < pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    i++;
    swap(arr[i], arr[end]);
    return i;
}
void QuickSort(vector<IdList*>& arr, int front, int end){
    if (front < end) {
        int pivot = Partition(arr, front, end);
        QuickSort(arr, front, pivot - 1);
        QuickSort(arr, pivot + 1, end);
    }
}
void printSSS(unsigned int simV)
{
  int a[33] = {0};
  int idx = 32;
  for(int i = 1; i <= 4; i++){
    for(int j = 1; j <= 8; j++){
      a[idx] = simV % 2;
      idx--;
      simV >>= 1;
    }
  }
  for(int i = 1; i <= 32; i++){
    cout << a[i];
    if(i % 8 == 0)
      cout << "__";
  }
}
/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
  vector<size_t>  _simulation;
  _simulation.resize(_inPutNum);
  srand(time(NULL));
  if(_FecGroups.size() == 0)
    FecInitialize();          // initialize for FEC Groups.
  int count = 0, fail = 0;

  for(int j = 0; j < _inPutNum; j++)
    _piList[j]->_simValue = 0;

  while(true){

    for(int k = 0; k < _inPutNum; k++)
        _simulation[k] = rand();
    
    for(int i = 1; i <= 64; i++){
      for(int j = 0; j < _inPutNum; j++){
        _piList[j]->_simValue <<= 1;
        _piList[j]->_simValue += _simulation[j] % 2;
        _simulation[j] = (_simulation[j] >> 1);
      }  
    }
    int originalFecGrps = _FecGroups.size();
    gateSimulation();
    HashSimulation();
    count += 64;

    if(_FecGroups.size() == originalFecGrps){
      fail++;
      if(fail >= _inPutNum * 1.5){
        cout << count << " patterns simulated." << endl; 
        break;
      }
    } 
  }
  sortFecGrps();
}

void
CirMgr::fileSim(ifstream& patternFile)
{
  vector<size_t>  _simulation;
  _simulation.resize(_inPutNum);
  string str[64] = {""};
  if(_FecGroups.size() == 0)
    FecInitialize(); 
  for(int j = 0; j < _inPutNum; j++)
    _piList[j]->_simValue = 0;
  int count = 0;
  
  int patternNum = 0;
  bool reset = false;
  while(!patternFile.eof()){
  
      getline(patternFile, str[count], '\n');
      if(str[count] != "" and str[count] != " " and str[count] != "\n"){
        if(reset){
          resetSimValue();
          reset = false;
        }
        int fileSize = 0;
        for(int i = 0; i < str[count].size(); i++)
          if(isdigit(str[count][i]))
            fileSize++;
        if(fileSize != _inPutNum){
          cerr << "Error: Pattern(" << str[count] << ") length(" << str[count].size() << 
          ") does not match the number of inputs(" << _inPutNum << ") in a circuit!!" << endl;
          break;
        }
        for(int j = 0; j < _inPutNum; j++){
          if(str[count][j] != '0' and str[count][j] != '1' and str[count][j] != ' '){
            cerr << "Error: Pattern(" << str[count] << ") contains a non-0/1 character(\'" << str[count][j] << "\')." << endl;
            break;
          }
        }
        int idx = 0;
        for(int j = 0; j < str[count].size(); j++){
          if(!isdigit(str[count][j])) continue;
          size_t tmp = str[count][j] - '0';
          tmp <<= count;
          _piList[idx]->_simValue += tmp;
          idx++;
        }
        count++;
        if(count % 64 == 0)
        {
          gateSimulation();
          HashSimulation();
          patternNum += count;
          reset = true;
          count = 0;
        }

      }
    
    if(patternFile.eof() and count % 64 != 0)
    {
      gateSimulation();
      HashSimulation();
      patternNum += count;
      count = 0;
    }
  }
  
  cout << patternNum << " patterns simulated." << endl;
  sortFecGrps();
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void
CirMgr::gateSimulation()
{
  for(size_t i = 0; i < _dfsList.size(); i++){
    if(_dfsList[i]->getTypeStr() == "AIG"){

      if(!_dfsList[i]->_invPhase[0] and !_dfsList[i]->_invPhase[1])     // 都沒有invert的時候
        _dfsList[i]->_simValue = (_dfsList[i]->_faninList[0]->_simValue) & (_dfsList[i]->_faninList[1]->_simValue);

      else if(_dfsList[i]->_invPhase[0] and !_dfsList[i]->_invPhase[1]) // input0 has invert
        _dfsList[i]->_simValue = (~(_dfsList[i]->_faninList[0]->_simValue)) & (_dfsList[i]->_faninList[1]->_simValue);

      else if(!_dfsList[i]->_invPhase[0] and _dfsList[i]->_invPhase[1]) // input1 has invert
        _dfsList[i]->_simValue = (_dfsList[i]->_faninList[0]->_simValue) & (~(_dfsList[i]->_faninList[1]->_simValue));
      
      else                                                              // input0 and input1 has inverse
        _dfsList[i]->_simValue = (~(_dfsList[i]->_faninList[0]->_simValue)) & (~(_dfsList[i]->_faninList[1]->_simValue));
    } 
    // 281 2183 456 520
    /*
    if(_dfsList[i]->_gateId == 281 or _dfsList[i]->_gateId == 1616 or _dfsList[i]->_gateId == 4208 or _dfsList[i]->_gateId == 6061)
    {
      cout << "ID: " << _dfsList[i]->_gateId << ", simV: " ; printSSS(_dfsList[i]->_simValue); cout << endl;
      cout << "Fanin1: " << _dfsList[i]->_faninList[0]->_gateId << ", simV: " ; printSSS(_dfsList[i]->_faninList[0]->_simValue); cout << endl;
      cout << "Fanin2: " << _dfsList[i]->_faninList[1]->_gateId << ", simV: " ; printSSS(_dfsList[i]->_faninList[1]->_simValue); cout << endl;
      cout << endl;
    }
    */
  }
}

void 
CirMgr::FecInitialize()
{
  int count = 0;
  for(size_t i = 0; i < _dfsList.size(); i++)
    if(_dfsList[i]->isAig())
      count++;
  IdList* FecGroup = new IdList;
  (*FecGroup).resize(count + 1);
  _gateList[0]->_simValue = 0;
  (*FecGroup)[0] = _gateList[0]->_gateId;
  
  int idx = 1;
  for(size_t i = 0; i < _dfsList.size(); i++){
    if(_dfsList[i]->getTypeStr() == "AIG"){
      (*FecGroup)[idx] = 2 * (_dfsList[i]->_gateId);
      idx++;
    }
  }
  for(int i = 0; i < _FecGroups.size(); i++)
    delete _FecGroups[i];
  _FecGroups.clear();
  _FecGroups.push_back(FecGroup);
}

void
CirMgr::HashSimulation()
{

  vector<IdList*>   newReplaceFECGrps; 
  for(unsigned i = 0; i < _FecGroups.size(); i++){

    HashSet<SimValueKey, IdList>  newFecGrps(_FecGroups[i]->size());      // count = AIG num in dfsList.

    for(unsigned j = 0; j < (_FecGroups[i]->size()); j++){
      unsigned FecId;
      if((*_FecGroups[i])[j] % 2 == 0) FecId =  (*_FecGroups[i])[j] ;
      else                             FecId =  (*_FecGroups[i])[j] - 1;

      SimValueKey simValueKey(_gateList[((*_FecGroups[i])[j] / 2)]->_simValue, FecId);
      IdList* grp;

      if(newFecGrps.checkGrp(simValueKey, grp)){
        (*grp).push_back(simValueKey.getId());
      }
      if(newFecGrps.invCheckGrp(simValueKey, grp)){
        int id = simValueKey.getId();
        (*grp).push_back(id + 1);
      }
      else{
        newFecGrps.createNewGrp(simValueKey);
        newFecGrps.add(simValueKey);
      }
    }
    CollectValidFecGrp(newFecGrps, newReplaceFECGrps);
  }
  for(int i = 0; i < _FecGroups.size(); i++)
    delete _FecGroups[i];
  _FecGroups.clear();
  _FecGroups.swap(newReplaceFECGrps);
}

void
CirMgr::CollectValidFecGrp(HashSet<SimValueKey, IdList>& newFecGrps, vector<IdList*>& fecGrps)
{
  HashSet<SimValueKey, IdList>::iterator it = newFecGrps.begin();
  for(; it != newFecGrps.end(); ++it){
    SimValueKey* test = it.getObjAdr();
    IdList* tmp = test->getFecGrp();
    if(tmp->size() != 1){
      fecGrps.push_back(tmp);
    }
    else{
      delete tmp;
      tmp = NULL;
    }
  }
}

void
CirMgr::resetSimValue()
{
  for(int i = 0; i < _piList.size(); i++)
    _piList[i]->_simValue = 0;
  for(int i = 0; i < _dfsList.size(); i++)
    if(_dfsList[i]->isAig())
      _dfsList[i]->_simValue = 0;
}

void
CirMgr::sortFecGrps()
{
  for(int i = 0; i < _FecGroups.size(); i++)
  sort((*_FecGroups[i]).begin(), (*_FecGroups[i]).end());
  QuickSort(_FecGroups, 0, _FecGroups.size() - 1);
}