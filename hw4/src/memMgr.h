/****************************************************************************
  FileName     [ memMgr.h ]
  PackageName  [ cmd ]
  Synopsis     [ Define Memory Manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MEM_MGR_H
#define MEM_MGR_H

#include <cassert>
#include <iostream>
#include <iomanip>
#include <stdlib.h>

using namespace std;

// Turn this on for debugging
// #define MEM_DEBUG

//--------------------------------------------------------------------------
// Define MACROs
//--------------------------------------------------------------------------
#define MEM_MGR_INIT(T) \
MemMgr<T>* const T::_memMgr = new MemMgr<T>

#define USE_MEM_MGR(T)                                                      \
public:                                                                     \
   void* operator new(size_t t) { return (void*)(_memMgr->alloc(t)); }      \
   void* operator new[](size_t t) { return (void*)(_memMgr->allocArr(t)); } \
   void  operator delete(void* p) { _memMgr->free((T*)p); }                 \
   void  operator delete[](void* p) { _memMgr->freeArr((T*)p); }            \
   static void memReset(size_t b = 0) { _memMgr->reset(b); }                \
   static void memPrint() { _memMgr->print(); }                             \
private:                                                                    \
   static MemMgr<T>* const _memMgr

// You should use the following two MACROs whenever possible to 
// make your code 64/32-bit platform independent.
// DO NOT use 4 or 8 for sizeof(size_t) in your code
//
#define SIZE_T      sizeof(size_t)         // 電腦預設為 8
#define SIZE_T_1    (sizeof(size_t) - 1)   // promote 的時候會用到 size_t - 1

// TODO: Define them by SIZE_T and/or SIZE_T_1 MACROs.
//
// To promote 't' to the nearest multiple of SIZE_T; 
// e.g. Let SIZE_T = 8;  toSizeT(7) = 8, toSizeT(12) = 16
#define toSizeT(t)     (t % 8 == 0)? t : 8 * (t / 8) + 8 // TODO
//
// To demote 't' to the nearest multiple of SIZE_T
// e.g. Let SIZE_T = 8;  downtoSizeT(9) = 8, downtoSizeT(100) = 96
#define downtoSizeT(t)  (t % 8 == 0)? t : 8 * ((t / 8) - 1) + 8  // TODO

// R_SIZE is the size of the recycle list
#define R_SIZE 256

//--------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------
template <class T> class MemMgr;


//--------------------------------------------------------------------------
// Class Definitions
//--------------------------------------------------------------------------
// T is the class that use this memory manager
//
// Make it a private class;
// Only friend to MemMgr;
//
template <class T>
class MemBlock
{
   friend class MemMgr<T>;

   // Constructor/Destructor
   MemBlock(MemBlock<T>* n, size_t b) : _nextBlock(n) {
      _begin = _ptr = new char[b]; _end = _begin + b; }
   ~MemBlock() { delete [] _begin; }

   // Member functions
   void reset() { _ptr = _begin; }
   // 1. Get (at least) 't' bytes memory from current block
   //    Promote 't' to a multiple of SIZE_T
   // 2. Update "_ptr" accordingly
   // 3. The return memory address is stored in "ret"
   // 4. Return false if not enough memory
   bool getMem(size_t t, T*& ret) {
      // TODO
      size_t promoteT = toSizeT(t);
      ret = (T*)_ptr;                        // 3 return memory address is stored in ret
      if(promoteT <= getRemainSize())
      {
         _ptr = _ptr + promoteT;
         return true;
      }
      else if(promoteT > getRemainSize())  // 4. Return false if not enough memory
      {
         return false;
      }           

   } 
   size_t getRemainSize() const { return size_t(_end - _ptr); }
      
   MemBlock<T>* getNextBlock() const { return _nextBlock; }

   // Data members
   char*             _begin;
   char*             _ptr;
   char*             _end;
   MemBlock<T>*      _nextBlock;
};

// Make it a private class;
// Only friend to MemMgr;
//
template <class T>
class MemRecycleList
{
   friend class MemMgr<T>;

   // Constructor/Destructor
   MemRecycleList(size_t a = 0) : _arrSize(a), _first(0), _nextList(0) {} 
   ~MemRecycleList() { reset(); }

   // Member functions
   // ----------------
   size_t getArrSize() const { return _arrSize; }
   MemRecycleList<T>* getNextList() const { return _nextList; }
   void setNextList(MemRecycleList<T>* l) { _nextList = l; }
   // pop out the first element in the recycle list
   // 要從recycle拿到東西時，就從recycle list 的 first pop 一個出來
   
   T* popFront() {
      // TODO 
      if(_first == NULL)       //檢查 recycle list 是不是空的，如果是空的 pop 會 segmentation fault.
         return NULL;
      T* originalFirst = _first;
      size_t* tmp = (size_t*)_first;    // tmp 指向 first 位置
      if(*tmp != 0)
         _first = (T*)(*tmp);              //把 first ubdate 到 first 的下一個。
      else if(*tmp == 0)                // 如果原本list沒有 nextList, _first要變ＮＵＬＬ
         _first = NULL;                  
      return originalFirst;         
   }
   // push the element 'p' to the beginning of the recycle list
   // delete 某個 memory block 的時候把這個東西丟到 front
   void  pushFront(T* p) {
      // TODO
     // _nextList = _first;
      T* originalFirst = _first;
      _first = p;                          // _first 指向新的 p
      size_t originalAdd = (size_t)originalFirst;       // 把本來 first 的位址轉成 size_t 型態
      size_t* tmp = (size_t*)_first;       // tmp 指向 first 位置
      *tmp = originalAdd;                  // 在新的 first 的 前 8 個 BYTE 放 下一個串接的 address.
   }
   // Release the memory occupied by the recycle list(s)
   // DO NOT release the memory occupied by MemMgr/MemBlock
   // 整個東西要把記憶體還給系統時要reset。 把 recycle List 清乾淨，但不要隨便delete. 否則會 double delete.
   void reset() {
      // TODO
      MemRecycleList<T>* next = _nextList;
      while(next != NULL)
      {
         delete next;
         next = next -> _nextList;
      }
      next = NULL;
      _nextList = NULL;
      
      //delete _first;
      _first = NULL;
   }

   // Helper functions
   // ----------------
   // count the number of elements in the recycle list
   size_t numElm() const {
      // TODO
      size_t count = 0;
      T* tmp = _first;                      // tmp 指向 first 位置
      while(tmp != NULL)
      {
         count++;
         size_t* tmp2 = (size_t*)tmp;       // tmp2 指向 tmp
         tmp = (T*)(*tmp2);                 // 把 first ubdate 到 first 的下一個。
      }
      return count;
   }

   // Data members
   size_t              _arrSize;   // the array size of the recycled data
   T*                  _first;     // the first recycled data
   MemRecycleList<T>*  _nextList;  // next MemRecycleList
                                   //      with _arrSize + x*R_SIZE
};

template <class T>
class MemMgr
{
   #define S sizeof(T)

public:
   MemMgr(size_t b = 65536) : _blockSize(b) {
      assert(b % SIZE_T == 0);
      _activeBlock = new MemBlock<T>(0, _blockSize);  // 第一個參數是指  next Block是誰
      for (int i = 0; i < R_SIZE; ++i)
         _recycleList[i]._arrSize = i;
   }
   ~MemMgr() { reset(); delete _activeBlock; }

   // 1. Remove the memory of all but the firstly allocated MemBlocks
   //    That is, the last MemBlock searchd from _activeBlock.
   //    reset its _ptr = _begin (by calling MemBlock::reset())
   // 2. reset _recycleList[]
   // 3. 'b' is the new _blockSize; "b = 0" means _blockSize does not change
   //    if (b != _blockSize) reallocate the memory for the first MemBlock
   //    MemBlock 串在一起，清掉時把後面全部清掉，第一個MemBlock檢查 _blockSize 有沒有一樣
   //    如果一樣，不用清掉，只要把 _ptr reset就好，不一樣的話，清掉再去跟系統重新要一塊新的 _blockSize大小的 MemBlock.
   // 4. Update the _activeBlock pointer
   void reset(size_t b = 0) {
      assert(b % SIZE_T == 0);
      #ifdef MEM_DEBUG
      cout << "Resetting memMgr...(" << b << ")" << endl;
      #endif // MEM_DEBUG
      // TODO
      
      while(true)
      {
         MemBlock<T>* tmp = _activeBlock;
         if(tmp -> _nextBlock != NULL)
            delete tmp;
         else                           // 找到最一開始的 _activeBlock 了。
            break;
         _activeBlock = _activeBlock -> _nextBlock;
      }
      //cout << "false2 " << endl;
      for(size_t i = 0; i < R_SIZE; i++)
      {
         MemRecycleList<T>* clean = &(_recycleList[i]);
         clean -> reset();
      }
      //cout << "false3 " << endl;
      if(b == 0 or b == _blockSize)
         _activeBlock -> reset();       // 把 ptr reset.
      else if(b != _blockSize)
      {
         delete _activeBlock;
         _activeBlock = NULL;
         _activeBlock = new MemBlock<T>(0, b);
         _blockSize = toSizeT(b);                //  reset blockSize
      }
      //cout << "false5 " << endl;
   }
   // Called by new
   T* alloc(size_t t) {
      assert(t == S);
      #ifdef MEM_DEBUG
      cout << "Calling alloc...(" << t << ")" << endl;
      #endif // MEM_DEBUG
      return getMem(t);
   }
   // Called by new[]
   T* allocArr(size_t t) {
      #ifdef MEM_DEBUG
      cout << "Calling allocArr...(" << t << ")" << endl;
      #endif // MEM_DEBUG
      // Note: no need to record the size of the array == > system will do
      return getMem(t);
   }
   // Called by delete
   void  free(T* p) {
      #ifdef MEM_DEBUG
      cout << "Calling free...(" << p << ")" << endl;
      #endif // MEM_DEBUG
      getMemRecycleList(0)->pushFront(p);
   }
   // Called by delete[]
   void  freeArr(T* p) {
      #ifdef MEM_DEBUG
      cout << "Calling freeArr...(" << p << ")" << endl;
      #endif // MEM_DEBUG
      // TODO
      // Get the array size 'n' stored by system,
      // which is also the _recycleList index
      size_t* tmp = (size_t*)p;        // tmp 指向 p
      size_t n = (size_t)*tmp;        

      #ifdef MEM_DEBUG
      cout << ">> Array size = " << n << endl;
      cout << "Recycling " << p << " to _recycleList[" << n << "]" << endl;
      #endif // MEM_DEBUG
      // add to recycle list...
      getMemRecycleList(n)->pushFront(p);
   }
   void print() const {
      cout << "=========================================" << endl
           << "=              Memory Manager           =" << endl
           << "=========================================" << endl
           << "* Block size            : " << _blockSize << " Bytes" << endl
           << "* Number of blocks      : " << getNumBlocks() << endl
           << "* Free mem in last block: " << _activeBlock->getRemainSize()
           << endl
           << "* Recycle list          : " << endl;
      int i = 0, count = 0;
      while (i < R_SIZE) {
         const MemRecycleList<T>* ll = &(_recycleList[i]);
         while (ll != 0) {
            size_t s = ll->numElm();
            if (s) {
               cout << "[" << setw(3) << right << ll->_arrSize << "] = "
                    << setw(10) << left << s;
               if (++count % 4 == 0) cout << endl;
            }
            ll = ll->_nextList;
         }
         ++i;
      }
      cout << endl;
   }
   
private:
   size_t                     _blockSize;
   MemBlock<T>*               _activeBlock;
   MemRecycleList<T>          _recycleList[R_SIZE];

   // Private member functions
   //
   // t: #Bytes; MUST be a multiple of SIZE_T
   // return the size of the array with respect to memory size t
   // [Note] t must >= S
   // [NOTE] Use this function in (at least) getMem() to get the size of array
   //        and call getMemRecycleList() later to get the index for
   //        the _recycleList[]
   size_t getArraySize(size_t t) const {
      assert(t % SIZE_T == 0);
      assert(t >= S);
      // TODO
      size_t arraySize = ( (t - sizeof(size_t)) / sizeof(T) );
      return arraySize;
   }
   // Go through _recycleList[m], its _nextList, and _nexList->_nextList, etc,
   //    to find a recycle list whose "_arrSize" == "n"
   // If not found, create a new MemRecycleList with _arrSize = n
   //    and add to the last MemRecycleList
   // So, should never return NULL
   // [Note]: This function will be called by MemMgr->getMem() to get the
   //         recycle list. Therefore, the recycle list is first created
   //         by the MTNew command, not MTDelete.

   // push the element 'p' to the beginning of the recycle list
   // delete 某個 memory block 的時候把這個東西丟到 front
   // getMemRecycleList(0)->pushFront(p); void  pushFront(T* p)
   MemRecycleList<T>* getMemRecycleList(size_t n) {
      //cout << "n: " << n << endl;
      size_t m = n % R_SIZE;
      // TODO
     MemRecycleList<T>* tmp = &(_recycleList[m]);
     do
     {
         if(tmp -> _arrSize == n)
            return tmp;
         else if(tmp -> _nextList == NULL)
            break;
         else if(tmp -> _nextList != NULL)
            tmp = tmp -> _nextList;    
     }while(tmp != NULL);
     MemRecycleList<T>* newList = new MemRecycleList<T>(n);   // n 初始化 _arrSize
     tmp -> _nextList = newList;
     return newList;

   }
   // t is the #Bytes requested from new or new[]
   // Note: Make sure the returned memory is a multiple of SIZE_T
   T* getMem(size_t t) {
      T* ret = 0;
      
      #ifdef MEM_DEBUG
      cout << "Calling MemMgr::getMem...(" << t << ")" << endl;
      #endif // MEM_DEBUG
      // 1. Make sure to promote t to a multiple of SIZE_T
      t = toSizeT(t);
      
      // 2. Check if the requested memory is greater than the block size.
      //    If so, throw a "bad_alloc()" exception.
      //    Print this message for exception
      //    cerr << "Requested memory (" << t << ") is greater than block size"
      //         << "(" << _blockSize << "). " << "Exception raised...\n";
      // TODO
      if(t > _blockSize)
      {
         cerr << "Requested memory (" << t << ") is greater than block size"
               << "(" << _blockSize << "). " << "Exception raised...\n";
         throw bad_alloc();   //throw a "bad_alloc()" exception.  MTNEW in memCmd.cpp will catch
      }
      // 3. Check the _recycleList first...
      //    Print this message for memTest.debug
      //    #ifdef MEM_DEBUG
      //    cout << "Recycled from _recycleList[" << n << "]..." << ret << endl;
      //    #endif // MEM_DEBUG
      //    => 'n' is the size of array
      //    => "ret" is the return address
      //cout << "beforeT: " << t << endl;
      size_t n = getArraySize(t);
      // TODO
      bool testRecycle = false;
      if(getMemRecycleList(n) -> _first != NULL)
      {
         
         if(getMemRecycleList(n) -> _arrSize == n)
         {
            ret = getMemRecycleList(n) -> _first;
            getMemRecycleList(n) -> popFront();
            testRecycle = true;
         }
         return ret;
      }
      // If no match from recycle list...
      // 4. Get the memory from _activeBlock  
      bool getMemBlock = _activeBlock -> getMem(t, ret);

      if( t <= _activeBlock->getRemainSize() and testRecycle == false)
      {
         if(getMemBlock == true)
         {
            return ret;
         }
      }
      
      // 5. If not enough, recycle the remained memory and print out ---
      //    Note: recycle to the as biggest array index as possible
      //    Note: rn is the array size
      //    Print this message for memTest.debug
      //    #ifdef MEM_DEBUG
      //    cout << "Recycling " << ret << " to _recycleList[" << rn << "]\n";
      //    #endif // MEM_DEBUG
      //    ==> allocate a new memory block, and print out ---
      //    #ifdef MEM_DEBUG
      //    cout << "New MemBlock... " << _activeBlock << endl;
      //    #endif // MEM_DEBUG
      // TODO
      //   先檢查剩下的 active block 夠不夠放， 假設剩 100 BYTE， 卻要塞 168 BYTE進去的話，要先把 100 -> 88 recycle，
      //    再跟系統要一個新的  active block， 放 168 BYTE 進去。
      else if(t > _activeBlock->getRemainSize() and testRecycle == false)    // 剩 88 BYTE， 卻要塞 168 BYTE進去的話，要先把 88 recycle
      {
         if(getMemBlock == false)
         {
            size_t remain = _activeBlock -> getRemainSize();
            if(remain >= sizeof(T))
            {
               remain = getArraySize(remain);                           // remain 變成 arraySize, EX: 100 - 8 / 88 = 1, 100頂多再放一個 88 size 的 array,
               getMemRecycleList(remain)->pushFront(ret);               // 從原本 _ptr 的位址 recycle
            }
               _activeBlock = new MemBlock<T>(_activeBlock, _blockSize); // 第一個參數為 _nextBlock, 將原本的_activeBlock 推到 nextBlock。
               _activeBlock -> getMem(t, ret);
         }
      }

      // 6. At the end, print out the acquired memory address
      #ifdef MEM_DEBUG
      cout << "Memory acquired... " << ret << endl;
      #endif // MEM_DEBUG
      return ret;
   }
   // Get the currently allocated number of MemBlock's
   size_t getNumBlocks() const {
      // TODO
      size_t count = 1;
      MemBlock<T>* tmp = _activeBlock;
      tmp = _activeBlock -> getNextBlock();
      while(tmp != NULL)
      {
         count ++;
         tmp = tmp -> getNextBlock();
      }
      return count;
   }

};

#endif // MEM_MGR_H
