/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

CirMgr::~CirMgr()
{
   for(size_t i = 0; i < _gateList.size(); i++){
      if(_gateList[i] != NULL){
         delete _gateList[i];
         _gateList[i] = NULL;
      }
   }
   for(size_t i = 0; i < _FecGroups.size(); i++){
      if(_FecGroups[i] != NULL){
         delete _FecGroups[i];
         _FecGroups[i] = NULL;
      }
   }
}
enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   CirGate* const0 = new CirConstGate(0, 0);
   //cout << "before: " << _gateList.size() << endl;
   CirMgr::reset();
   //cout << "after: " << _gateList.size() << endl;
   unsigned lineNo = 1;
   ifstream inFile(fileName);
   if(!inFile.is_open())
   {
      cerr << "Failed to read in file \"" << fileName << "\"!" << endl;
      return false;
   }
   string str;
   vector<string> token;
   getline(inFile, str);
   readHeader(str, token);                      // readHeader in line 1
   _gateList.resize(_MaxValue + _outPutNum + 1);   // reserve size    
   _gateList[0] = const0;
   
   for(int i = 1; i <= _inPutNum; i++)          // readInput
   {
      lineNo++;
      getline(inFile, str);
      readInput(str, lineNo);
   }

   for(int i = 1; i <= _outPutNum; i++)         // readOutput
   {
      lineNo++;
      getline(inFile, str);
      int _id = _MaxValue + i;
      readOutput(str, lineNo, _id);
   }

   for(int i = 1; i <= _AigNum; i++)            // readAigGate
   {
      lineNo++;
      getline(inFile, str);
      readAIG(str, lineNo);
   }

   connect();
   processUnDefineGate();
   genDFSList();
   while(getline(inFile, str))
      _comment.push_back(str);
   processSymbol();
   return true;
}

void
CirMgr::readHeader(const string& firstLine, vector<string>& token)
{
   token.push_back("");
   size_t pos = myStrGetTok(firstLine, token[0]);
   string test;
   do
   {
      pos = myStrGetTok(firstLine, test, pos);
      if(test != "")
      {
         token.push_back(test);
      }
   } while (pos != string::npos);
   
   stringstream ss1; stringstream ss2; stringstream ss3; stringstream ss4; stringstream ss5;
   ss1 << token[1];  ss1 >> _MaxValue;
   ss2 << token[2];  ss2 >> _inPutNum;
   ss3 << token[3];  ss3 >> _latchNum;
   ss4 << token[4];  ss4 >> _outPutNum;
   ss5 << token[5];  ss5 >> _AigNum;

   //cout << _MaxValue << ", " << _inPut << ", " << _latch << ", " << _outPut << ", " << _Aig << endl;
}

void
CirMgr::readInput(const string& str, const int& _lineNo)
{
   stringstream ss;
   int _id;
   ss << str;  ss >> _id;
   _id = _id / 2;
   _gateList[_id] = new CirPiGate(_id, _lineNo);
   _piList.push_back(_gateList[_id]);
}

void
CirMgr::readOutput(const string& str, const int& _lineNo, const int& _id)
{
   stringstream ss;
   int _fanin;
   ss << str;  ss >> _fanin;      // str 為 fanin 的 ＩＤ
   bool inv = false;
   if(_fanin % 2 == 1)                    // 檢查是否有Inverse
      inv = true;
   _fanin = _fanin / 2;
   _gateList[_id] = new CirPoGate(_id, _lineNo);
   _poList.push_back(_gateList[_id]);
   
   _gateList[_id]->_fanin.push_back(_fanin);  
   _gateList[_id]->_invPhase.push_back(inv);          // true的話為有inverse
}

void
CirMgr::readAIG(const string& str, const int& _lineNo)
{
   vector<string> token2;
   token2.push_back("");
   size_t pos = myStrGetTok(str, token2[0]);
   string test;
   do
   {
      pos = myStrGetTok(str, test, pos);
      if(test != "")
      {
         token2.push_back(test);
      }
   } while (pos != string::npos);

   stringstream ss1;    stringstream ss2;    stringstream ss3;
   int _id = 0, _fanin1 = 0, _fanin2 = 0;
   ss1 << token2[0];  ss1 >> _id;
   ss2 << token2[1];  ss2 >> _fanin1;
   ss3 << token2[2];  ss3 >> _fanin2;
   bool inv1 = false, inv2 = false;
   if(_fanin1 % 2 == 1)
      inv1 = true;
   if(_fanin2 % 2 == 1)
      inv2 = true;

   _id = _id / 2; _fanin1 = _fanin1 / 2;  _fanin2 = _fanin2 / 2;
   _gateList[_id] = new CirAigGate(_id, _lineNo);

   _gateList[_id]->_fanin.push_back(_fanin1);
   _gateList[_id]->_fanin.push_back(_fanin2);
   _gateList[_id]->_invPhase.push_back(inv1);
   _gateList[_id]->_invPhase.push_back(inv2);

}

void
CirMgr::connect()
{
   for(int i = 1; i <= _MaxValue + _outPutNum; i++)
   {//    Aig 的 case
      //cout << "I: " << i << endl;
      if(_gateList[i] != NULL)
      {
         if(_gateList[i]->_fanin.size() == 2)  
         {
            int _id1 = _gateList[i]->_fanin[0];
            int _id2 = _gateList[i]->_fanin[1];
            //cout << 1 << endl;
            if(_gateList[_id1] != NULL and _gateList[_id1]->_unDef != true)
               _gateList[i]->_faninList.push_back(_gateList[_id1]);
            if(_gateList[_id2] != NULL and _gateList[_id2]->_unDef != true)
               _gateList[i]->_faninList.push_back(_gateList[_id2]);
            //cout << 2 << endl;
            if(_gateList[_id1] != NULL and _gateList[_id1]->getTypeStr() != "UNDEF")
               _gateList[_id1]->_fanoutList.push_back(_gateList[i]);
            if(_gateList[_id2] != NULL and _gateList[_id2]->getTypeStr() != "UNDEF")
               _gateList[_id2]->_fanoutList.push_back(_gateList[i]);
            //cout << 3 << endl;
         }
      //   po 的 case
         else if(_gateList[i]->_fanin.size() == 1)
         {
            //cout << 4 << endl;
            int _id = _gateList[i]->_fanin[0];
            _gateList[i]->_faninList.push_back( _gateList[_id] );      //將ＰＯ接到他的 _fanin;
            //cout << 5 << endl;
            if(_gateList[_id] != NULL and _gateList[_id]->getTypeStr() != "UNDEF")
               _gateList[_id]->_fanoutList.push_back(_gateList[i]);
            //cout << 6 << endl;
         }
      }
   }
}

void
CirMgr::genDFSList()
{
   for(size_t i = 0; i < _gateList.size(); i++)
      if(_gateList[i] != NULL)
         _gateList[i]->setGlobalRef();

   for(int i = _MaxValue + 1; i <= _MaxValue + _outPutNum; i++)
      if(_gateList[i] != NULL)
         _gateList[i]->dfsTraversal(_dfsList);
}

void
CirMgr::processUnDefineGate()
{
   for(int i = 0; i < _gateList.size(); i++)
   {
      if(_gateList[i] != NULL){
         if(_gateList[i]->_fanin.size() != 0){
            for(int j = 0; j < _gateList[i]->_fanin.size(); j++){
               if(_gateList[_gateList[i]->_fanin[j]] == NULL){
                  CirGate* Undef = new UnDefineGate(_gateList[i]->_fanin[j], 0);
                  _gateList[i]->_faninList.push_back(Undef); 
                  Undef->_unDef = true;
                  _gateList[_gateList[i]->_fanin[j]] = Undef;
               }
            }
         }
      }

   }

}

void
CirMgr::processSymbol()
{
   string digit = "1234567890";
   string alnum = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz1234567890";
   for(size_t i = 0; i < _comment.size(); i++)
   {
      if(_comment[i][0] == 'i')
      {
         size_t start = _comment[i].find_first_of(digit);
         size_t end = _comment[i].find_first_not_of(digit, start);
         
         string _id = _comment[i].substr(start, end);

         int id = 0;
         stringstream ss;
         ss << _id; ss >> id;

         size_t start2 = _comment[i].find_first_of(alnum, end);
         size_t end2   = _comment[i].find_first_not_of(alnum, start2);
         string symbol = _comment[i].substr(start2, end2);

         _piList[id]->_symbol = symbol;
      }
      else  if(_comment[i][0] == 'o')
      {
         size_t start = _comment[i].find_first_of(digit);
         size_t end = _comment[i].find_first_not_of(digit, start);
         
         string _id = _comment[i].substr(start, end);
         
         int id = 0;
         stringstream ss;
         ss << _id; ss >> id;

         size_t start2 = _comment[i].find_first_of(alnum, end);
         size_t end2   = _comment[i].find_first_not_of(alnum, start2);
         string symbol = _comment[i].substr(start2, end2);

         _poList[id]->_symbol = symbol;
      }

   }
}

void
CirMgr::reset()
{
   vector<CirGate*>().swap(_gateList);
   vector<CirGate*>().swap(_piList);
   vector<CirGate*>().swap(_poList);
   vector<CirGate*>().swap(_dfsList);
   vector<string>().swap(_comment);
}
/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout << "Circuit Statistics" << endl;
   cout << "==================" << endl;
   cout << "  PI   " << setw(9) << right << _inPutNum  << endl;
   cout << "  PO   " << setw(9) << right << _outPutNum << endl;
   cout << "  AIG  " << setw(9) << right << _AigNum    << endl;
   cout << "------------------" << endl;
   cout << "  Total" << setw(9) << right << _inPutNum + _outPutNum + _AigNum << endl;
}

void
CirMgr::printNetlist() const
{
/*
   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _dfsList[i]->printGate();
   }
*/
   cout << endl;
   //int Num = countNum();
   for(size_t i = 0; i < _dfsList.size(); i++)
   {
      if(_dfsList[i] == NULL)
         break;
      cout << "[" << i << "] ";
      if(_dfsList[i]->_faninList.size() == 0 and _dfsList[i]->_gateId == 0)                // const 0 case
         cout << "CONST0";
      else if(_dfsList[i]->_faninList.size() == 0 and _dfsList[i]->_gateId != 0)           // PI case
         cout << "PI  " << _dfsList[i]->_gateId;
      else  if(_dfsList[i]->_faninList.size() == 1)
      {
         cout << "PO  " << _dfsList[i]->_gateId << " ";
         if(_dfsList[i]->_faninList[0]->getTypeStr() == "UNDEF")
            cout << "*";
         if(_dfsList[i]->_invPhase[0] == true)
            cout << "!" << _dfsList[i]->_faninList[0]->_gateId;
         else
            cout << _dfsList[i]->_faninList[0]->_gateId;
      }
      else if(_dfsList[i]->isAig())
      {
         cout << "AIG " << _dfsList[i]->_gateId << " " ;
         if(_dfsList[i]->_faninList[0]->getTypeStr() == "UNDEF")
            cout << "*";
         if(_dfsList[i]->_invPhase[0] == true)
            cout << "!" << _dfsList[i]->_faninList[0]->_gateId << " ";
         else
            cout << _dfsList[i]->_faninList[0]->_gateId << " ";

         if(_dfsList[i]->_faninList[1]->getTypeStr() == "UNDEF")
            cout << "*";
         if(_dfsList[i]->_invPhase[1] == true)
            cout << "!" << _dfsList[i]->_faninList[1]->_gateId;
         else
            cout << _dfsList[i]->_faninList[1]->_gateId;
      }
      printSymbol(_dfsList[i]);
      cout << endl;
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(int i = 0; i < _inPutNum; i++)
      cout << " " << _piList[i]->_gateId;
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(int i = 0; i < _outPutNum; i++)
      cout << " " << _poList[i]->_gateId;
   cout << endl;
}

void
CirMgr::printFloatGates() const
{

   GateList FloatingFanin, NotUsed;
   bool* test = new bool[_MaxValue + _outPutNum + 1];
   for(size_t i = 0; i <= _MaxValue + _outPutNum; i++)
      test[i] = false;

   for(size_t i = 0; i < _gateList.size(); i++){
      if(_gateList[i] != NULL){
         for(size_t j = 0; j < _gateList[i]->_faninList.size(); j++)
            if(_gateList[i]->_faninList[j]->getTypeStr() == "UNDEF" and test[i] == false)
               {FloatingFanin.push_back(_gateList[i]); test[i] = true;}}}
     
   for(size_t i = 0; i < _gateList.size(); i++)
      if(_gateList[i] != NULL and _gateList[i]->_fanoutList.size() == 0 and _gateList[i]->_gateId != 0 
            and _gateList[i]->getTypeStr() != "UNDEF" and _gateList[i]->_gateId <=_MaxValue)
         NotUsed.push_back(_gateList[i]);

   if(NotUsed.size())
   {
      cout << "Gates defined but not used  : ";
      for(size_t i = 0; i < NotUsed.size(); i++){
         if(i != NotUsed.size() - 1)
            cout << NotUsed[i]->_gateId << " ";
         else
            cout << NotUsed[i]->_gateId;
      }
      cout << endl;
   }

   if(FloatingFanin.size())
   {
      cout << "Gates with floating fanin(s): ";
      for(size_t i = 0; i < FloatingFanin.size(); i++){
         if(i != FloatingFanin.size() - 1)
            cout << FloatingFanin[i]->_gateId << " ";
         else
            cout << FloatingFanin[i]->_gateId;
      }
      cout << endl;
   }

   delete[] test;
   test = NULL;
}

void
CirMgr::printFECPairs() const
{   
   for(int i = 0; i < _FecGroups.size(); i++){
      cout << "[" << i << "] ";
      for(int j = 0; j < _FecGroups[i]->size(); j++){
         if( (*_FecGroups[i])[0] % 2 == 1){
            if( (*_FecGroups[i])[j] % 2 == 1 )
               cout << ( ((*_FecGroups[i])[j] - 1) / 2) ;
            else
               cout << "!" << ( ((*_FecGroups[i])[j]) / 2) ;
            if(j != _FecGroups[i]->size() - 1)
               cout << " ";
         }
         else{
            if( (*_FecGroups[i])[j] % 2 == 0 )
               cout << ( (*_FecGroups[i])[j] / 2) ;
            else
               cout << "!" << (((*_FecGroups[i])[j] - 1 ) / 2) ;
            if(j != _FecGroups[i]->size() - 1)
               cout << " ";
         }
      }
      cout << endl;
   }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   int countAig = 0;
   for(size_t i = 0; i < _dfsList.size(); i++)
   {
      if(_dfsList[i] == NULL)
         break;
      if(_dfsList[i]->getTypeStr() == "AIG")
         countAig++;
   }

   outfile << "aag " << _MaxValue << " " << _inPutNum << " 0 " << _outPutNum << " " << countAig << endl;
   for(size_t i = 0; i < _piList.size(); i++)
      outfile << (_piList[i]->_gateId) * 2 << endl;
   for(size_t i = 0; i < _poList.size(); i++)
   {
      if(_poList[i]->_invPhase[0] == true)
         outfile << (_poList[i]->_faninList[0]->_gateId) * 2 + 1 << endl;
      else
         outfile << (_poList[i]->_faninList[0]->_gateId) * 2 << endl;
   }
   
   for(size_t i = 0; i < _dfsList.size(); i++)
   {
      if(_dfsList[i] == NULL)
         break;
      if(_dfsList[i]->getTypeStr() == "AIG")
      {
         outfile << (_dfsList[i]->_gateId) * 2 << " ";
         if(_dfsList[i]->_invPhase[0] == true)
            outfile << (_dfsList[i]->_faninList[0]->_gateId) * 2 + 1 << " " ;
         else
            outfile << (_dfsList[i]->_faninList[0]->_gateId) * 2  << " ";

         if(_dfsList[i]->_invPhase[1] == true)
            outfile << (_dfsList[i]->_faninList[1]->_gateId) * 2 + 1 ;
         else
            outfile << (_dfsList[i]->_faninList[1]->_gateId) * 2 ;
         outfile << endl;
      }
   }
   for(size_t i = 0; i < _comment.size(); i++)
   {
      if(_comment[i][0] == 'i')
         outfile << _comment[i] << endl;
      if(_comment[i][0] == 'o')
         outfile << _comment[i] << endl;
      if(_comment[i][0] == 'c')
         outfile << 'c' << endl;
   }
   outfile << "AAG output by Lian Kuo" << endl;
}

void
CirMgr::writeGate(ostream& outfile, CirGate*& g) const
{
   for(size_t i = 0; i < _gateList.size(); i++)
      if(_gateList[i] != NULL)
         _gateList[i]->setGlobalRef();
   GateList tmpPI, tmpPO, tmpAIG;
   GateDfsWrite(g, tmpPI, tmpPO, tmpAIG);

   outfile << "aag " << g->_gateId << " " << tmpPI.size() << " 0 " << 1 << " " << tmpAIG.size() << endl;

   // for PI
   for(int i = 0; i < tmpPI.size(); i++)
      outfile << (tmpPI[i]->_gateId) * 2 << endl;

   // for PO
   outfile << (g->_gateId) * 2 << endl;

   for(size_t i = 0; i < tmpAIG.size(); i++)
   {
      outfile << (tmpAIG[i]->_gateId) * 2 << " ";
      if(tmpAIG[i]->_invPhase[0] == true)
         outfile << (tmpAIG[i]->_faninList[0]->_gateId) * 2 + 1 << " " ;
      else
         outfile << (tmpAIG[i]->_faninList[0]->_gateId) * 2  << " ";

      if(tmpAIG[i]->_invPhase[1] == true)
         outfile << (tmpAIG[i]->_faninList[1]->_gateId) * 2 + 1 ;
      else
         outfile << (tmpAIG[i]->_faninList[1]->_gateId) * 2 ;
      outfile << endl;
   }
      //Write gate (1093) by Chung-Yang (Ric) Huang
   outfile << "o0 " << g->_gateId << endl;
   outfile << "c" << endl;
   outfile << "Write gate (" << g->_gateId << ") by Lian Kuo" << endl;
}

void
CirMgr::printSymbol(CirGate* gate) const
{
   if(gate->_symbol != "")
      cout << " (" << gate->_symbol << ")" ;
}

void
CirMgr::printFecGate(const CirGate* gate) const
{

   int findI = -1; bool inv = false;
   for(int i = 0; i < _FecGroups.size(); i++){
      for(int j = 0; j < _FecGroups[i]->size(); j++){
         if(((*_FecGroups[i])[j] / 2) == gate->_gateId){
            findI = i;
            if((*_FecGroups[i])[j] % 2 == 1)
               inv = true;
         }
      }
   }
   if(findI != -1){
      for(int i = 0; i < _FecGroups[findI]->size(); i++){
         if(((*_FecGroups[findI])[i] / 2) != gate->_gateId){
            if(inv){
               if( (*_FecGroups[findI])[i] % 2 == 1 )
                  cout << ( ((*_FecGroups[findI])[i] - 1) / 2) ;
               else
                  cout << "!" << ( ((*_FecGroups[findI])[i]) / 2) ;
               if(i != _FecGroups[findI]->size() - 1)
                     cout << " ";
            }
            else{
               if( (*_FecGroups[findI])[i] % 2 == 0 )
                  cout << ( (*_FecGroups[findI])[i] / 2) ;
               else
                  cout << "!" << (((*_FecGroups[findI])[i] - 1 ) / 2) ;
                  if(i != _FecGroups[findI]->size() - 1)
                     cout << " ";
            }
         }
      }
   }
    cout << endl;
      
}

void
CirMgr::printGateSimV(const CirGate* gate) const
{
   unsigned int SimV[64] = {0};
   size_t simValue = gate->_simValue;
   for(int i = 63; i >= 0; i--)
   {
      SimV[i] = simValue % 2;
      simValue >>= 1;
   }
   for(int i = 0; i <= 7; i++){
      for(int j = 0; j < 8; j++){
         cout << SimV[(8 * i) + j];
      }
      if(i != 7)
         cout << "_";
   }
   cout << endl;
}

void
CirMgr::GateDfsWrite(CirGate*& gate, GateList& tmpPI, GateList& tmpPO, GateList& tmpAIG) const
{
   for(size_t i = 0; i < gate->_faninList.size(); i++)
   {
      if(!gate->_faninList[i]->isGlobalRef())
      {
         gate->_faninList[i]->setToGlobalRef();
         GateDfsWrite(gate->_faninList[i], tmpPI, tmpPO, tmpAIG);
      }
   }
   if(gate->getTypeStr() == "PI")
      tmpPI.push_back(gate);
   else if(gate->getTypeStr() == "PO")
      tmpPO.push_back(gate);
   else if(gate->getTypeStr() == "AIG")
      tmpAIG.push_back(gate);
}