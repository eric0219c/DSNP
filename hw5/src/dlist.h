/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>

template <class T> class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
   friend class DList<T>;
   friend class DList<T>::iterator;

   DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0):
      _data(d), _prev(p), _next(n) {}

   // [NOTE] DO NOT ADD or REMOVE any data member
   T              _data;
   DListNode<T>*  _prev;       // dummy Node 剛 create的時候， prev 跟 next 都指到自己
   DListNode<T>*  _next;
};


template <class T>
class DList
{
public:
   // TODO: =decide the initial value for _isSorted
   DList() {
      _head = new DListNode<T>(T());
      _head->_prev = _head->_next = _head; // _head is a dummy node
   }
   ~DList() { clear(); delete _head; }

   // DO NOT add any more data member or function for class iterator
   class iterator                                       // 把 ListNode 包起來 / listNode 要 friend iterator, 讓 iterator用
   {
      friend class DList;

   public:
      iterator(DListNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return _node->_data; }
      T& operator * () { return _node->_data; }              // *li 拿到不是 _node 而是裡面的 _data.
      iterator& operator ++ () { _node = _node -> _next; return *(this); }  // ++N，把自己的 Node = Node -> next， return *(this)
      iterator operator ++ (int) {iterator tmp = *this; _node = _node -> _next; return tmp; }    // N++，把自己先備份起來，把自己的 Node = Node -> next， return 備份，較沒效率。
      iterator& operator -- () { _node = _node -> _prev; return *(this); }
      iterator operator -- (int) {iterator tmp = *this; _node = _node -> _prev; return tmp; }

      iterator& operator = (const iterator& i) { _node = i._node; return *(this); }

      bool operator != (const iterator& i) const 
      {
         if(_node != i._node)
            return true;
         else
            return false;
      }
      bool operator == (const iterator& i) const 
      {
         if(_node == i._node)
            return true;
         else
            return false;
      }

   private:
      DListNode<T>* _node;
   };

   // TODO: implement these functions
   iterator begin() const { return (iterator)(_head -> _next); }
   iterator end() const { return (iterator)(_head); }        // end   檢查 head -> next 是不是指到 head,
   bool empty() const 
   { 
      if(_head -> _next == _head)
         return true;
      else
         return false;  
   }      // empty 檢查 head -> next 是不是指到 head, 而不是 NULL
   size_t size() const 
   {  
      size_t count = 0;
      if(empty() == true)
         return count;

      DListNode<T>* tmp = _head;
      do
      {
         tmp = tmp -> _next;
         count++;
      }while(tmp -> _next != _head);

      return count; 
   }

   void push_back(const T& x) 
   { 
      DListNode<T>* store = new DListNode<T>(x);
      DListNode<T>* tmp = _head;

      if(empty() == false)
      {
         (_head -> _prev) -> _next = store;
         store -> _next = _head;
         store -> _prev = _head -> _prev;
         _head -> _prev = store;
      }
      else         // if empty
      { 
         store -> _next = _head;
         store -> _prev = _head;
         _head -> _prev = store;
         _head -> _next = store;
      }
      _isSorted = false;
   }

   void pop_front() 
   { 
      if(empty() == true) {return;}

      DListNode<T>* tmp = _head -> _next;   // tmp = first one
      _head -> _next = tmp -> _next;        // first one = second one
      (tmp -> _next) -> _prev = _head;
      delete tmp;
   }                      
   void pop_back() 
   { 
      if(empty() == true){return; }
      
      DListNode<T>* tmp = _head -> _prev;
      (tmp -> _prev) -> _next = _head;
      _head -> _prev = tmp -> _prev;
      delete tmp;
      
   }

   // return false if nothing to erase
   bool erase(iterator pos)    // no need to check whether pos == end()).
   { 
      //cout << "HERE" << endl;
      if(empty() == true)
         return false; 

      DListNode<T>* tmp = pos._node;

      if(tmp == _head -> _next)
         pop_front();
      else if(tmp ->_next == _head)
         pop_back();
      else
      {
         (tmp -> _prev) -> _next = tmp -> _next;
         (tmp -> _next) -> _prev = tmp -> _prev;
         delete tmp;
      }
      //delete tmp;
      return true;
   }
   bool erase(const T& x) 
   { 
      iterator li = find(x);
      if(li == end())            // cannot find x
         return false; 
      else
         erase(li);
      return true;
   }

   iterator find(const T& x) 
   { 
      iterator li;
      for(li = begin(); li != end(); li++)
      {
         if(li._node -> _data == x)
            return li;
      }

      return end();     // cannot find x
   }

   void clear() 
   { 
      DListNode<T>* tmp = _head -> _next;
      DListNode<T>* tmp2;
      if(tmp == _head)    // empty
         return;
      else
      {
         do
         {
            tmp2 = tmp;
            delete tmp2;
            tmp = tmp -> _next;
         } while(tmp != _head);
         _head -> _prev = _head;
         _head -> _next = _head;
      }
      

   }  // delete all nodes except for the dummy node

   void sort() const          // BUBBLE SORT o(n2)
   { 
      if(_isSorted == true) { return; }
      iterator li, li2, test = --end();
      for(li = begin(); li != end(); li++)
      {
        for(li2 = begin(); li2 != test; li2++)
        {
            if(li2._node -> _data > li2._node -> _next -> _data)
            {
               //cout << "TTT" << endl;
               T tmp = li2._node -> _data;
               li2._node -> _data = li2._node -> _next -> _data;
               li2._node -> _next -> _data = tmp;
            }
        }
        test = --test;         // n n-1 n-2 ....
      }
      _isSorted = true;
   }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   DListNode<T>*  _head;     // = dummy node if list is empty
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] helper functions; called by public member functions
};

#endif // DLIST_H
