/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>

using namespace std;

template <class T> class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
   BSTreeNode(const T& d, BSTreeNode<T>* l = 0, BSTreeNode<T>* r = 0):
      _data(d), _left(l), _right(r){}
   
   friend class BSTree<T>;
   friend class BSTree<T>::iterator;
   friend class AdtTest;
   T                _data;
   BSTreeNode<T>*   _left;
   BSTreeNode<T>*   _right;
   BSTreeNode<T>*   _parent;
};


template <class T>
class BSTree
{
public:
   BSTree() {
      _dummyNode = new BSTreeNode<T>(T());
      _dummyNode->_left =  _dummyNode->_right = _dummyNode->_parent = NULL;
      _root = _dummyNode;
      _size = 0;
   }
   ~BSTree(){ clear(); delete _root; }


   class iterator 
   { 
      friend class BSTree;
      friend class AdtTest;
   public:
      iterator(BSTreeNode<T>* n = 0): _node(n) {}
      iterator(const iterator& i): _node(i._node) {}
      ~iterator() {}

      const T& operator * () const { return _node->_data; }
      T& operator * () { return _node->_data; }              // *li 拿到不是 _node 而是裡面的 _data.
      
      iterator& operator ++ () // ++N，把自己的 Node = Node -> next， return *(this)
      { 
         if(_node->_right != NULL) 
         {
            _node = _node->_right;
            while(_node->_left != NULL)
            {
               _node = _node->_left;
            }
            return(*this);
         }   
         else
         {
            bool find = false;
            do
            {
               //cout << "heeeeeeeeee" << endl;
               if(_node->_parent->_left == _node)
               { find = true; _node = _node->_parent; break; }

               if(_node->_parent == NULL)
               { find = true; break; }

               _node = _node->_parent;
            }while(find == false);

            return(*this);
         }
      }  
      iterator operator ++ (int) { iterator tmp = (iterator)_node; ++_node; return tmp; }    // N++，把自己先備份起來.
      iterator& operator -- () 
      {
         iterator tmp = (iterator)_node;
         iterator tmp2 = (iterator)_node;
         while(tmp._node->_parent != NULL)
         {
            tmp._node = tmp._node->_parent;
         }
         if(FindMin(tmp._node) == this->_node)
         {
            return (*this);
         }
         this->_node = tmp2._node;

         if(_node->_left != NULL)
         {
            _node = _node->_left;
            while(_node->_right != NULL)
            {
               _node = _node->_right;
            }
            return *(this);
         }   
         else
         {
            bool find = false;
            do
            {
               if(_node->_parent->_right == _node)
               {
                  find = true; _node = _node->_parent; break; 
               }

               if(_node->_parent == NULL)
               { find = true; break; }

               _node = _node->_parent;
            }while(find == false);

            return *(this);
         }
         
      }
      iterator operator -- (int) 
      { 
         iterator tmp = (iterator)(*this);
         --(*this); 
         return tmp; 
      }

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
      BSTreeNode<T>* _node;
      
      BSTreeNode<T>* FindMax (BSTreeNode<T>* _base) const
      {
         if(_base->_right == NULL)
            return _base->_parent;
         else
            return FindMax(_base->_right);
      }
      BSTreeNode<T>* FindMin (BSTreeNode<T>* _base) const
      {
         if(_base->_left == NULL)
            return _base;
         else
            return FindMin(_base->_left);
      }
   };

   iterator begin() const    // For BSTree, it is the leftmost element. Return end() if the ADT is empty.
   { 
      //cout << "BEGINNNN" << endl;
      if(empty()) { return end(); }
      return (iterator)(FindMin(_root));
   }
   iterator end() const { return iterator(_dummyNode); }
   bool empty() const {if(_size == 0) return true; return false;}

   size_t size() const  { return _size; }

   void insert(const T& x)
   {
      BSTreeNode<T>* insert_node = new BSTreeNode<T>(x);
      BSTreeNode<T>* z = _root ;    // 哨兵
      BSTreeNode<T>* y = 0;         // 新手爸媽
      if(_size == 0)
      {
         _root = insert_node;
         _root->_right = _dummyNode;
         _size += 1;
         return;
      }
      while(z != NULL and z != _dummyNode)
      {
         //cout << "HHH" << endl;
         y = z;
         if(insert_node->_data < z->_data)
            z = z->_left;
         else if(insert_node->_data > z->_data)
            z = z->_right;
         else if(insert_node->_data == z->_data)
            z = z->_left;
         else           // x -> _data = insert_node -> _data ， _data 已存在
            break;
      }

      insert_node->_parent = y;

      if(insert_node->_data > y->_data)
         y->_right = insert_node;
      else
         y->_left = insert_node;
/*
      if(insert_node == FindMax(_root))
         insert_node->_right = _dummyNode;       // set the dummyNode in the maxNode.*/
      BSTreeNode<T>* tmp2 = FindMax(_root);
      tmp2->_right = _dummyNode;
      _dummyNode->_parent = tmp2;
      _size += 1;
   }

   // For BSTree, its leftmost node (i.e. begin()) will be removed.
   void pop_front()    
   {
      if(empty()) { return; }

      erase(begin());
      //delete tmp;
   }
    // remove the last (rightmost for BSTree) element in the ADT. 
    // No action will be taken and no error will be issue if the ADT is empty.
   void pop_back()  
   {
      if(empty()) { return; }

      BSTreeNode<T>* tmp = FindMax(_root);
      if(tmp == _root)
      {
         _root = tmp->_left;
         BSTreeNode<T>* newMax = FindMax(_root);
         newMax->_right = _dummyNode;
         _dummyNode->_parent = newMax;
         tmp->_right = NULL;
         delete tmp;
         _size -= 1;
      }
      else if(tmp->_left == NULL)
      {
         tmp->_parent->_right = _dummyNode;
         _dummyNode->_parent = tmp->_parent;
         tmp->_right = NULL;
         delete tmp;
         _size -= 1;
      }
      else if(tmp->_left != NULL)
      {
         (tmp->_parent)->_right = tmp->_left;
         (tmp->_left)->_parent = tmp->_parent;
         
         BSTreeNode<T>* tmp2 = tmp->_left;
         while(tmp2->_right != NULL)
         {
            tmp2 = tmp2->_right;
         }
         tmp2->_right = _dummyNode;
         _dummyNode->_parent = tmp2;
         tmp->_left = NULL; tmp->_right = NULL;
         delete tmp;
         _size -= 1;
      }
      
   }

   bool erase(iterator pos)         // 未考慮 dummy Node
   {
      if(empty()) {return false;}
      BSTreeNode<T>* tmp = pos._node;
      if(tmp == _root)
      {
         //cout << "root" << endl;
         BSTreeNode<T>* successor;
         if(tmp->_right == NULL and tmp->_left != NULL)
         {
            successor = tmp->_left;
            successor->_parent = _root;
            Replace(tmp, successor);
            delete tmp;
         }   
         else if(tmp->_left == NULL and tmp->_right != NULL)
         {
            successor = tmp->_right;
            successor->_parent = _root;
            Replace(tmp, successor);
            delete tmp;
         }   
      }
      else if(tmp->_right == _dummyNode)
      {
         //cout << "dummy" << endl;
         if(tmp->_left != NULL)
         {
            tmp->_parent->_right = tmp->_left;
            tmp->_left->_parent = tmp->_parent;
         }
         else
         {
            tmp->_parent->_right = NULL;
            tmp->_parent = NULL;
         }
         delete tmp;
         
         BSTreeNode<T>* tmp222 = FindMax(_root);
         tmp222->_right = _dummyNode;
         _dummyNode->_parent = tmp222;
      }
      // Leaf Case , trivial
      else if(tmp->_left == NULL and tmp->_right == NULL)    
      {
         //cout << "here1" << endl;
         cutParent(tmp);
         delete tmp;
      }

      //Two Children Case
      if(tmp->_left != NULL and tmp->_right != NULL and tmp->_right != _dummyNode)  
      {
         //cout << "here2" << endl;
         BSTreeNode<T>* successor = Successor(tmp);
         ReplaceSuccessor(tmp, successor);
         delete tmp;
      }

      // One Child Case , trivial
      else if((tmp->_left != NULL or tmp->_right != NULL) and tmp->_right != _dummyNode)                         
      {
         bool dummy = false;
         if(tmp->_right == _dummyNode)
         {
            dummy = true;
         }
         if(tmp->_left != NULL)
         {
            if(tmp->_parent->_left == tmp)
            {
               tmp->_parent->_left = tmp->_left;
               tmp->_left->_parent = tmp->_parent;
            }
            else if(tmp->_parent->_right == tmp)
            {
               tmp->_parent->_right = tmp->_left;
               tmp->_left->_parent = tmp->_parent;
            }
            delete tmp;
         }
         else if(tmp->_right != NULL and tmp->_right != _dummyNode)
         {
            if(tmp->_parent->_left == tmp)
            {
               tmp->_parent->_left = tmp->_right;
               tmp->_right->_parent = tmp->_parent;
            }
            else if(tmp->_parent->_right == tmp)
            {
               tmp->_parent->_right = tmp->_right;
               tmp->_right->_parent = tmp->_parent;
            }
            delete tmp;
         }
         if(dummy == true)
         {
            BSTreeNode<T>* newMax = FindMax(_root);
            newMax->_right = _dummyNode;
            _dummyNode->_parent= newMax;
         }
      }
      _size -= 1;
      //cout << "size:  " << _size << endl;
      return true;
   }

   bool erase(const T& x)
   {
      
      iterator li = find(x);
      if(li == end())            // cannot find x
      {
         //cout << "end" << endl;
         return false; 
      }
      else
         erase(li);
      return true;
   }

   iterator find(const T& x)
   {
      iterator li;
      for(li = begin(); li != end(); ++li)
      {
         //cout << "here" << endl;
         if(li._node -> _data == x)
         {
            //cout << li._node->_data << ", " << x << endl;
            return li;
         }   
      }
      
      return end();     // cannot find x
   }

   void clear()
   {
      size_t test = _size;
      do
      {
         pop_front();
         test--;
      } while (test != 0);
      
   }

   void sort(){ return; }
   void print() const{ return; }


   private:
      BSTreeNode<T>*    _root;
      BSTreeNode<T>*    _dummyNode;
      size_t            _size;

      BSTreeNode<T>* FindMin (BSTreeNode<T>* _base) const
      {
         if(_base->_left == NULL)
            return _base;
         else
            return FindMin(_base->_left);
      }
      
      BSTreeNode<T>* FindMax (BSTreeNode<T>* _base) const
      {
         if(_base->_right == NULL or _base->_right == _dummyNode)
            return _base;
         else
            return FindMax(_base->_right);
      }

      BSTreeNode<T>* Successor(BSTreeNode<T>* _base)
      {
         if(_base->_right == NULL)
            return NULL;
         else
            return FindMin(_base->_right);
      }

      BSTreeNode<T>* SecondMin(BSTreeNode<T>* _base)
      {
         if(_base->_left == NULL)
            return _base;
         else
            return FindMax(_base->_left);  
      }

      BSTreeNode<T>* Insert(const T& x, BSTreeNode<T>* _base)
      {
         if(_size == 0)
            return new BSTreeNode<T>(x, NULL, NULL, NULL); 
         else if(x == _base->_data)
            return _base;
         else if(x < _base->_data)
            _base->_left = Insert(x, _base->_left);
         else
            _base->_right = Insert(x, _base->_right);
         return _base;
      }
      
      void cutParent(BSTreeNode<T>* child)
      {
         if(((child->_parent)->_left) == child)
            (child->_parent)->_left = NULL;

         else
            (child->_parent)->_right = NULL;
      }

      void Replace(BSTreeNode<T>* beReplace, BSTreeNode<T>* replace)
      {
         if(replace->_parent == _root)
         {
            //cout << "here" << endl;
            _root = replace;
            beReplace->_left = NULL;
            beReplace->_right = NULL;
            if(beReplace->_right == _dummyNode)
            {
               BSTreeNode<T>* tmp22 = FindMax(_root);
               tmp22->_right = _dummyNode;
               _dummyNode->_parent = tmp22;
            }
            return;
         }

         if(beReplace->_right == _dummyNode)
         {
            BSTreeNode<T>* tmp22 = FindMax(_root);
            tmp22->_right = _dummyNode;
            _dummyNode->_parent = tmp22;
         }
//------------------------設定 beReplace 的子孫們新的家長為 replace---------------------------------------//
         if(beReplace->_left != NULL)
            beReplace->_left->_parent = replace;
         if(beReplace->_right != NULL)
            beReplace->_right->_parent = replace;
//------------------------設定 Replace 的 左右---------------------------------------//
         if(beReplace->_left != replace)
            replace->_left = beReplace->_left;
         else
            replace->_left = NULL;

         if(beReplace->_right != replace)
            replace->_right = beReplace->_right;
         else
            replace->_right = NULL;
//---------------------------------------------------------------------------------//
         beReplace->_left = NULL;
         beReplace->_right = NULL;

//------------------------設定 Replace 的 Parent------------------------------------//
         replace->_parent = beReplace->_parent;
         if(beReplace->_parent->_left == beReplace)
            beReplace->_parent->_left = replace;
         else
            beReplace->_parent->_right = replace;
//---------------------------------------------------------------------------------//
         beReplace->_parent = NULL;
      }

      void ReplaceSuccessor(BSTreeNode<T>* beReplace, BSTreeNode<T>* successor)
      {
         if(beReplace == _root)
            _root = successor;

         if(successor->_right != NULL)
         {
            successor->_parent->_left = successor->_right;
            successor->_right->_parent = successor->_parent;
         }
         beReplace->_left->_parent = successor;
         beReplace->_right->_parent = successor;
         successor->_left = beReplace->_left;
         successor->_right = beReplace->_right;
         beReplace->_left = NULL;
         beReplace->_right = NULL;
         
      }

      //algorithm postOrder(TreeNode t)
      //Input: a tree node (can be considered to be a tree)
      //Output: None.
      void postOrder(BSTreeNode<T>* base)
      {
         if(base->_left != NULL)
            postOrder(base->_left);
         if(base->_right != NULL)
            postOrder(base->_right);
         
      }
      

};

#endif // BST_H
