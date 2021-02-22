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

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;
class  SimValueKey;

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr();

   friend bool compare();
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
   void reset();
   void connect();
   void genDFSList();
   void processUnDefineGate();
   void processSymbol();
   void printSymbol(CirGate*) const;

   // Member functions about circuit optimization
   void sweep();
   void optimize();
   void deleteConnection(CirGate*&);
   void doubleConnection(CirGate*&, CirGate*&);
   void ConnectToZero(CirGate*&, CirGate*&);
   void cutAnotherFanout(CirGate*&, CirGate*&);
   void setNewInv(CirGate*&, CirGate*&);
   void setNewInv2(CirGate*&, CirGate*&, bool*&);
   void printOpt(CirGate*&, CirGate*&) const;

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }
   void gateSimulation();
   void FecInitialize();
   void HashSimulation();
   void CollectValidFecGrp(HashSet<SimValueKey, IdList>&, vector<IdList*>&);
   void printTest();
   void resetSimValue();
   void sortFecGrps();

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();
   void genProofModel(SatSolver& s);
   void reportResult(const SatSolver& solver, bool result) const;
   void fraigMerge(CirGate*&, CirGate*&, bool&);
   void findMinForFec(int* findMinInFec);
   void setDfsNum();
   void eraseFEC();
   void doProve(SatSolver&, int*&);
   void SatSimulation(int*& , int&, vector<size_t*>&, int);
   void resetMinInFec(int*&);
   void cleanFEC();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*&) const;
   void printFecGate(const CirGate*) const;
   void printGateSimV(const CirGate*) const;
   void GateDfsWrite(CirGate*&, GateList&, GateList&, GateList&) const;


private:
   ofstream           *_simLog;

   void readHeader(const string&, vector<string>&);
   void readInput(const string&, const int&);
   void readOutput(const string&, const int&, const int&);
   void readAIG(const string&, const int&);

   GateList             _piList;
   GateList             _poList;
   GateList             _gateList;
   int                  _MaxValue;
   int                  _inPutNum;
   int                  _latchNum;
   int                  _outPutNum;
   int                  _AigNum;
   GateList             _dfsList;
   vector<IdList*>      _FecGroups;
   vector<string>       _comment;
   
   
};

#endif // CIR_MGR_H
