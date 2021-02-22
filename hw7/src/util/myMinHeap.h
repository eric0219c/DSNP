/****************************************************************************
  FileName     [ myMinHeap.h ]
  PackageName  [ util ]
  Synopsis     [ Define MinHeap ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_MIN_HEAP_H
#define MY_MIN_HEAP_H

#include <algorithm>
#include <vector>

template <class Data>
class MinHeap
{
public:
   MinHeap(size_t s = 0) { if (s != 0) _data.reserve(s); }
   ~MinHeap() {}

   void clear() { _data.clear(); }

   // For the following member functions,
   // We don't respond for the case vector "_data" is empty!
   const Data& operator [] (size_t i) const { return _data[i]; }   
   Data& operator [] (size_t i) { return _data[i]; }

   size_t size() const { return _data.size(); }

   // TODO
   const Data& min() const { return _data[0]; }
   void insert(const Data& d) 
   {
      size_t n = _data.size() - 1;
      int t = ++n;
      _data.push_back(d);
      while(t >= 0)
      {
         int p = (t - 1) / 2;
         if(_data[t] < _data[p])
         {
            Data tmp = _data[t];
            _data[t] = _data[p];
            _data[p] = tmp;
            t = p;
         }
         else
            break;
      }
   }
   void delMin() 
   {
      if(_data.size() == 1)
      {
         _data.erase(_data.begin());
         return;
      }
      size_t n = _data.size() - 1;
      int p = 0, t = 2 * p + 1;
      _data[0] = _data[n];
      _data.pop_back();
      n--;
      while(t <= n)
      {
         if(t < n)
            if(_data[t + 1] < _data[t])
               t++;
         if(_data[t] < _data[p])
         {
            Data tmp = _data[t];
            _data[t] = _data[p];
            _data[p] = tmp;
            p = t;
            t = 2 * p + 1;
         }
         else
            break;
      }
   }

   void delData(size_t i) 
   {
      if(_data.size() == 1)
      {
         _data.erase(_data.begin());
         return;
      }
      size_t n = _data.size() - 1;
      int p = i, t = 2 * p + 1;
      _data[i] = _data[n];
      _data.pop_back();
      n--;
      while(t <= n)
      {
         if(t < n)
            if(_data[t + 1] < _data[t])
               t++;
         if(_data[t] < _data[p])
         {
            Data tmp = _data[t];
            _data[t] = _data[p];
            _data[p] = tmp;
            p = t;
            t = 2 * p + 1;
         }
         else
            break;
      }
   }
private:
   // DO NOT add or change data members
   vector<Data>   _data;
};

#endif // MY_MIN_HEAP_H
