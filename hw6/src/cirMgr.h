/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#include "cirDef.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr(){}
   ~CirMgr() {}

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const 
   {
      if(gid > _MaxValue + _outPutNum)
         return 0; 
      else
         return _gateList[gid];
   }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void writeAag(ostream&) const;
   void connect();
   void genDFSList();
   void processUnDefineGate();
   int  countNum() const;
   void processSymbol();
   void printSymbol(CirGate*) const;
private:

   void readHeader(const string&, vector<string>&);
   void readInput(const string&, const int&);
   void readOutput(const string&, const int&, const int&);
   void readAIG(const string&, const int&);

   GateList       _piList;
   GateList       _poList;
   GateList       _gateList;
   int            _MaxValue;
   int            _inPutNum;
   int            _latchNum;
   int            _outPutNum;
   int            _AigNum;
   GateList       _dfsList;
   vector<string> _comment;


};

#endif // CIR_MGR_H
