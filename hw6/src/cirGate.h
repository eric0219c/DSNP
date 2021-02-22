/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{

public:
   friend class CirMgr;
   CirGate(int _id, int _No): _gateId(_id), _lineNo(_No)
   {
     _globalRef = 0;
     _ref = 0;
     _levelTest = 0;
     _invMySelf = false;
     _repeat = false;
     _unDef = false;
     _symbol = "";
   };
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const;
   unsigned getLineNo() const { return _lineNo; }

   // Printing functions
   virtual void printGate() const{};
   void reportGate() const;
   void reportFanin(int level) ;
   void reportFanout(int level) ;
   
   bool isGlobalRef(){ return (_ref == _globalRef); }
   void setToGlobalRef(){ _ref = _globalRef; }
   void setGlobalRef() { _globalRef++; }
   void dfsTraversal(GateList& _dfsList);
   void recurInLevel(GateList& _faninLevelList, int level);
   void recurOutLevel(GateList& _fanoutLevelList, int level);
   void setInInvSelf(CirGate*);
   void setOutInvSelf(CirGate*);
   void printInLevel(CirGate*, int level) const;
   
protected:
   int              _gateId;
   unsigned         _lineNo;
   IdList           _fanin;
   IdList           _fanout;
   vector<bool>     _invPhase;
   GateList         _faninList;
   GateList         _fanoutList;
   unsigned         _globalRef;
   unsigned         _ref; 
   string           _symbol;
   bool             _unDef;
   GateList         _faninLevelList;
   GateList         _fanoutLevelList;
   bool             _invMySelf;
   unsigned         _levelTest;
   bool             _repeat;

};

class CirPiGate : public CirGate
{
public:
  friend class CirMgr;
  friend class CirGate;
  CirPiGate(int _id, int _No) : CirGate(_id, _No){};
  ~CirPiGate(){};
  void printGate() const{};

protected:
   
};

class CirPoGate : public CirGate
{
public:
  friend class CirMgr;
  friend class CirGate;
  CirPoGate(int _id, int _No) : CirGate(_id, _No){};
  ~CirPoGate(){};
  void printGate() const{};

protected:
   
};

class CirAigGate : public CirGate
{
public:
  friend class CirMgr;
  friend class CirGate;
  CirAigGate(int _id, int _No) : CirGate(_id, _No){};
  ~CirAigGate(){};
  void printGate() const{};

protected:

};

class CirConstGate : public CirGate
{
public:
  friend class CirMgr;
  friend class CirGate;
  CirConstGate(int _id, int _No) : CirGate(_id = 0, _No = 0){};
  ~CirConstGate(){};
  void printGate() const{};

};


class UnDefineGate : public CirGate
{
public:
  friend class CirMgr;
  friend class CirGate;
  UnDefineGate(int _id, int _No) : CirGate(_id, _No = 0){};
  ~UnDefineGate(){};
  void printGate() const{};

};

#endif // CIR_GATE_H
