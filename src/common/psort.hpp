/*
Copyright (C) 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* Priority sort template function
 * 
 * Author: Mihalis Georgoulopoulos 2005
 */

#include <cstring>
#include <algorithm>

#ifndef _PSORT_HEADER_
#define _PSORT_HEADER_

#define SORT_LOHI	false
#define SORT_HILO	true

template <class T> int less(const T* a, const T* b)
{
	return (*a < *b);
}

template <class T> int greater(const T* a, const T* b)
{
	return !(*a < *b);
}

template <class T, class P> void sort(T *elements, P *priorities, unsigned int n, bool hilo)
{
	int (*criterion)(const P*, const P*);

	if (hilo)
		criterion = greater;
	else
		criterion = less;

	P **pointers = new P*[n];
	for (unsigned int i=0; i<n; i++)
		pointers[i] = priorities + i;

	// sort priority pointers
	std::sort(pointers, pointers + n, criterion);
	
	// collect sorted items
	T *sorted_el = new T[n];
	P *sorted_pr = new P[n];
	for (unsigned int i=0; i<n; i++)
	{
		sorted_el[i] = elements[pointers[i] - priorities];
		sorted_pr[i] = *pointers[i];
	}

	memcpy(elements, sorted_el, n * sizeof(T));
	memcpy(priorities, sorted_pr, n * sizeof(P));
	
	// cleanup
	delete [] pointers;
	delete [] sorted_el;
	delete [] sorted_pr;
}

#endif // ndef _PSORT_HEADER_
