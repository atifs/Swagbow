/*
	Created: 4/30/2014
	Author: Cyrus
	Purpose: A circular list, i.e. a linked list except the last pointer redirects to the first object
	Notes: any object added to this list should be created with "new".
		Also, you have to use an iterator object or it will call the copy constructor on the object
		and first will be set to that new copy
*/

#ifndef __CIRCULARLIST_H
#define __CIRCULARLIST_H

template <typename T>
class CircularList {
private:
	CircularList *first;
	CircularList *next;	
public:
	T *object;

	//Cleanup must be called when you are done with the object
	CircularList(T obj) {
		next = NULL;
		//copy constructor
		object = new T(obj);	
		first = this;
	}

	T get() {
		return *object;
	}

	void append(T obj) {
		CircularList *now = this;
		CircularList **pnext = &next;
		CircularList **plast = &now;
		while( *pnext != NULL ) {			
			plast = pnext;
			pnext = &(*pnext)->next;			
		}			

		(*plast)->next = new CircularList<T>(obj);
		(*plast)->next->first = first;
	}

	CircularList *right() {	
		if( next == NULL )
			return first;
		return next;
	}

	void cleanup() {
		//We don't use a destructor, let's just delete everything manually
		delete object;

		if( next != NULL ) {
			//We allocated new structs, so we must delete them
			CircularList *n = next;
			CircularList *l = n;
			do {				
				delete n->object;
				l = n->next;
				delete n;
				n = l;
			} while( n != NULL );
		}
	}
	
};

#endif