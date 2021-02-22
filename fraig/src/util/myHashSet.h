/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//

template <class Data>
class HashSet
{
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;

   public:
      iterator(vector<Data>* b, size_t r, size_t n, size_t c = 0) : _buckets(b), _row(r), _numBuckets(n), _column(c){}
      ~iterator(){};
      
      const Data& operator * () const { return _buckets[_row][_column]; }
      iterator& operator ++ () 
      {     
         if(_column == _buckets[_row].size() - 1){
            for(size_t i = _row + 1; i < _numBuckets; i++){
               if(_buckets[i].size()){
                  _row = i;
                  _column = 0;
                  return (*this);
               }
            }
         }
         else if(_column != _buckets[_row].size() - 1)
         {
            _column++;
            return (*this);
         }
         
         _row = _numBuckets;
         _column = 0;
         return (*this);
      }
      iterator& operator ++(int)
      {
         iterator tmp = *this;
         ++(*this);
         return tmp;
      }

      iterator& operator -- ()
      {
         if(_column != 0)
         {
            _column--;
            return (*this);
         }
         else
         {
           for(size_t i = _row - 1; i >= 0; i--)
           {
               if(_buckets[i].size())
               {
                  _row = i;
                  _column = _buckets[i].size() - 1;
                  return (*this);
               }
           }
         }

      }

      void operator = (const iterator& i)
      {
         _buckets[_row][_column] = i._buckets[_row][_column];
         _row = i._row;
         _column = i._column;
      }

      bool operator == (const iterator& i) const 
      {
         if(_row == i._row and _column == i._column)
            return true;
         return false;
      }
      bool operator != (const iterator& i) const 
      { 
         if(_row == i._row and _column == i._column)
            return false;
         return true;
      }

   private:
      vector<Data>*     _buckets;
      size_t            _row;
      size_t            _numBuckets;
      size_t            _column;
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const 
   { 
      for(size_t i = 0; i < _numBuckets; i++)
         if(_buckets[i].size())
            return iterator(_buckets, i, _numBuckets);
      
      return iterator(_buckets, _numBuckets, _numBuckets);
   }
   // Pass the end
   iterator end() const         
   { 
      return iterator(_buckets, _numBuckets, _numBuckets, 0);
   }
   // return true if no valid data
   bool empty() const 
   { 
      for(size_t i = 0; i < _numBuckets; i++)
         if(_buckets[i].size() != 0)
            return false;
      return true; 
   }
   // number of valid data
   size_t size() const 
   { 
      size_t s = 0;
      for(size_t i = 0; i < _numBuckets; i++)
         if(_buckets[i].size() != 0)
            s += _buckets[i].size();
      return s; 
   }       // go through every bucket, and count every bucket.size()

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const 
   { 
      size_t num = bucketNum(d);
      for(size_t i = 0; i < _buckets[num].size(); i++)
      {
         if(_buckets[num][i] == d)
            return true;
      }
      return false; 
   }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const 
   { 
      size_t num = bucketNum(d);
      for(size_t i = 0; i < _buckets[num].size(); i++)
      {
         if(_buckets[num][i] == d)
         {
            d = _buckets[num][i];
            return true;
         }
      }
      return false; 
   }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;               // 如果原本有，ＲＥＰＬＡＣＥ掉，ＲＥＴＵＲＮ ＴＲＵＥ
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d) 
   { 
      size_t num = bucketNum(d);
      for(size_t i = 0; i < _buckets[num].size(); i++)
      {
         if(_buckets[num][i] == d)
         {
            _buckets[num][i] = d;
            return true;
         }
      }
      size_t n = _buckets[num].size();
      _buckets[num].resize(++n);
      _buckets[num][n - 1] = d;
      return false; 
   }

   // return true if inserted successfully (i.e. d is not in the hash)  // 原本沒有
   // return false is d is already in the hash ==> will not insert      // 原本有了
   bool insert(const Data& d) 
   { 
      size_t num = bucketNum(d);
      for(size_t i = 0; i < _buckets[num].size(); i++)
         if(_buckets[num][i] == d)
            return false;
      _buckets[num].push_back(d);
      return true; 
   }

   // return true if removed successfully (i.e. d is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const Data& d) 
   { 
      size_t num = bucketNum(d);
      for(size_t i = 0; i < _buckets[num].size(); i++)
      {
         if(_buckets[num][i] == d)
         {
            _buckets[num].erase(_buckets[num].begin() + i);
            return true;
         }
      }
      return false; 
   }

private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;      // _buckets = new vector<Data>[b]

   size_t bucketNum(const Data& d) const {
      return (d() % _numBuckets); }
};


class HashKey
{

public:             // as hash function
   HashKey(size_t a, size_t b) : _in0(a), _in1(b){}
   ~HashKey(){}
   size_t operator () () const 
   {
      size_t k = 0, n = ((_in0 + _in1) <= 5)? (_in0 + _in1) : 5;
      for (size_t i = 0; i < n; ++i)
         k ^= ((_in0 + _in1) << (i*6));
      return k;

   }
   bool operator == (const HashKey& k) const 
   {
      return(_in0 == k._in0 and _in1 == k._in1);   
   }
   void swap()
   {
      if(_in0 > _in1)
      {
         size_t tmp = _in0;
         _in0 = _in1;
         _in1 = tmp;
      }
   }
private:
   size_t   _in0;
   size_t   _in1;
};

#endif // MY_HASH_SET_H