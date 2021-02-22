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
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
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
     _symbol = "";
     _simValue = 0;
     _unDef = 0;
     _var = 0; // may need to change
     _dfsNum = -1;
     _inDfs = false;
   };
   virtual ~CirGate() {}

   // Basic access methods
   virtual string getTypeStr() const {return  "";};
   unsigned getLineNo() const { return _lineNo; }
   virtual bool isAig() const = 0;
   int  getId() const {return _gateId;}
   Var  getVar() const{return _var;}
  // Printing functions
   virtual void printGate() const{};
   void reportGate() const;
   void reportFanin(int level) ;
   void reportFanout(int level) ;
   // Functions
   bool isGlobalRef(){ return (_ref == _globalRef); }
   void setToGlobalRef(){ _ref = _globalRef; }
   void setGlobalRef() { _globalRef++; }
   void dfsTraversal(GateList& _dfsList);
   void recurInLevel(GateList& _faninLevelList, int level);
   void recurOutLevel(GateList& _fanoutLevelList, int level);
   void setInInvSelf(CirGate*);
   void setOutInvSelf(CirGate*);
   void printInLevel(CirGate*, int level) const;
   void setVar(const Var& v){_var = v;}
   int  getDfsNum(){return _dfsNum;}
   void merge(CirGate*&);
   
protected:
   int              _gateId;
   unsigned         _lineNo;
   IdList           _fanin;
   vector<bool>     _invPhase;
   GateList         _faninList;
   GateList         _fanoutList;
   unsigned         _globalRef;
   unsigned         _ref; 
   string           _symbol;
   bool             _invMySelf;
   unsigned         _levelTest;
   bool             _repeat;
   bool             _unDef;
   size_t           _simValue;
   Var              _var;
   int              _dfsNum;
   bool             _inDfs;
};

class CirPiGate : public CirGate
{
public:
  friend class CirMgr;
  friend class CirGate;
  CirPiGate(int _id, int _No) : CirGate(_id, _No){};
  ~CirPiGate(){};
  virtual string getTypeStr() const { return "PI"; }
  virtual bool isAig() const { return false; }
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
  virtual string getTypeStr() const { return "PO"; }
  virtual bool isAig() const { return false; }
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
  virtual string getTypeStr() const { return "AIG"; }
  virtual bool isAig() const { return true; }
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
  virtual string getTypeStr() const { return "CONST"; }
  virtual bool isAig() const { return false; }
  void printGate() const{};

};


class UnDefineGate : public CirGate
{
public:
  friend class CirMgr;
  friend class CirGate;
  UnDefineGate(int _id, int _No) : CirGate(_id, _No = 0){};
  ~UnDefineGate(){};
  virtual string getTypeStr() const { return "UNDEF"; }
  virtual bool isAig() const { return false; }
  void printGate() const{};

};

#endif // CIR_GATE_H
