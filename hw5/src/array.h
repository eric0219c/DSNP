/****************************************************************************
  FileName     [ array.h ]
  PackageName  [ util ]
  Synopsis     [ Define dynamic array package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>
#include <algorithm>

using namespace std;

// NO need to implement class ArrayNode
//
template <class T>
class Array
{
public:
   // TODO: decide the initial value for _isSorted
   Array() : _data(0), _size(0), _capacity(0) {}
   ~Array() { delete []_data; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class Array;

   public:
      iterator(T* n= 0): _node(n) {}
      iterator(const iterator& i): _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return (*this); }
      T& operator * () { return (*_node); }
      iterator& operator ++ () { _node += 1; return (*this); }
      iterator operator ++ (int) { iterator tmp = _node; _node += 1; return (tmp); }
      iterator& operator -- () { _node -= 1; return (*this); }
      iterator operator -- (int) { iterator tmp = _node; _node -= 1; return (tmp); }

      iterator operator + (int i) const { iterator tmp = _node + i; return (tmp); }
      iterator& operator += (int i) { _node += i; return (*this); }

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
      T*    _node;
   };

   // TODO: implement these functions
   iterator begin() const { iterator tmp(_data); return tmp; }
   iterator end() const { iterator tmp(_data + _size); return tmp; }
   bool empty() const 
   { 
      if(_size == 0)
         return true;
      else
         return false; 
   }
   
   size_t size() const { return _size; }

   T& operator [] (size_t i) { return _data[i]; }
   const T& operator [] (size_t i) const { return _data[i]; }

   void push_back(const T& x) 
   { 
      if(_size == _capacity)
      {
         T* tmp = _data;

         if(_capacity == 0) {_capacity = 1;}
         else  {_capacity = _capacity * 2;}
         _data = new T[_capacity];

         for(size_t i = 0; i < _size; i++)
         {
            _data[i] = tmp[i];
         }
         delete[] tmp;
         tmp = NULL;
      }
      _data[_size++] = x;     // _data[_size] = x, _size = _size + 1;
      _isSorted = false;
   }
   //the first element will be removed and, if _size >= 2, the last element will be “copied” to the first position. 
   //However, the _data pointer itself and _capacity will NOT be changed
   void pop_front() 
   { 
      if(empty() == true) { return; }
      if(_size == 1)
         erase(_data);
      else
      {
         _data[0] = _data[_size - 1];
         _size -= 1; 
      }
      _isSorted = false;
   }
   void pop_back() 
   { 
      if(empty() == true) { return; }
      erase(_data + _size - 1);
   }

   bool erase(iterator pos) 
   {
      if(empty() == true) { return false; }
      else
      {
         *pos._node = _data[_size - 1];
      }
      _size -= 1;
      _isSorted = false;
      return true;
   }
   bool erase(const T& x) 
   {
      //cout << "here " << endl;
      iterator tmp = find(x);
      if(tmp == end())
         return false; 

      _isSorted = false;

         *tmp._node = _data[_size - 1];
         _size -= 1;
         return true;
      
   }

   iterator find(const T& x)    // o(n) if not sorted, but o(log n) if array is sorted.
   { 
      for(size_t i = 0; i < _size; i++)
      {
         if(_data[i] == x)
         {
            //cout << "I: " << i << endl;
            iterator tmp(_data + i);
            return tmp;
         }
      }
      return end(); 
   }     

   void clear() 
   { 
      _size = 0;
   }

   // [Optional TODO] Feel free to change, but DO NOT change ::sort()
   void sort() const 
   { 
      if(_isSorted == true) {return; }
      if (!empty()) ::sort(_data, _data+_size); 
      _isSorted = true;
   }


   // Nice to have, but not required in this homework...
   // void reserve(size_t n) { ... }
   // void resize(size_t n) { ... }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   T*            _data;
   size_t        _size;       // number of valid elements
   size_t        _capacity;   // max number of elements
   mutable bool  _isSorted;   // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] Helper functions; called by public member functions
   iterator BinaryFind(const T& x, size_t _halfSize)
   {
      if(x == _data[_halfSize])
      {
         iterator tmp(_data + _halfSize);
         return tmp;
      }
      else if(x > _data[_halfSize])
      {
         _halfSize = (_halfSize + _size) / 2;
         iterator tmp = BinaryFind(x, _halfSize);
         return tmp;
      }
      else
      {
         _halfSize = _halfSize / 2;
         iterator tmp = BinaryFind(x, _halfSize);
         return tmp;
      }
      if(_halfSize == 0)
         return end();
   }
};

#endif // ARRAY_H
