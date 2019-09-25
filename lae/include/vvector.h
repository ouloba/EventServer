#ifndef REDEFINE_VECTOR_H_
#define REDEFINE_VECTOR_H_

#include "ICGui.h"

#include<memory>
template<typename  T>
class DLL_CLASS vvector
{
public:
	vvector(){
		//_head = _end = _tail = NULL;
		_head = (T*)::operator new(1*sizeof(T));
		_end = _head;
		_tail = _head+1;
		_grown = 0;
	}

	vvector(int num){
		_head = (T*)::operator new(num*sizeof(T));
		_end = _head;
		_tail = _head+num;
		_grown = 0;
	}

	~vvector(){
		if(_head) ::operator delete(_head);
	}
	

	void swap(vvector<T>& v){
		T*  __head = _head;
		T*  __tail = _tail;
		T*  __end  = _end;
		int __grown   = _grown;
		_head = v._head;
		_tail = v._tail;
		_end  = v._end;
		_grown=v._grown;
		v._head = __head;
		v._tail = __tail;
		v._end  = __end;
		v._grown=__grown;
	}

	T& operator[](int pos){
		return  *(_head+pos);
	}

	const T& operator[](int pos) const {
		return  *(_head+pos);
	}
	
	const T&   back( )const{ return *_end; }
	T& back(){ return *_end;}

	int size() const { return _end-_head;}
	int capcity()const {return (_tail-_end);}
	void clear(){
		_end = _head;
	}

	bool empty()const {return ((_end-_head)==0 );}

	T* begin() { return _head;}
	T* end(){ return _end;}

	const T& front()const{
		return *_head;
	}

	T& front(){
		return *_head;
	}

	void push_back(const T& t){
		push_back(const_cast<T&>(t));
	}

	void erase(T* pos){
		memmove(pos, pos+1, sizeof(T)*(_end-pos));
		_end--;
	}

	void push_back(T& t){
		if(capcity()==0){
			int l = (_tail-_head);
			int nl = (_grown <= 0) ? (2 * (l + 1)) : (_grown + l);
			T* _new_head = (T*)::operator new(nl*sizeof(T));
			memcpy(_new_head,_head,sizeof(T)*l);
			::operator delete(_head);
			_head = _new_head;
			_end = _head+l;
			_tail = _head+nl;
		}

		memcpy(_end, &t, sizeof(T));
		_end++;
	}

	void grown(int _n){
		_grown = _n;
	}
	
private:
	T* _head;
	T* _end;
	T* _tail;	
	int _grown;
};


#endif