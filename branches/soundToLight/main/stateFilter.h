/*
 * stateFilter.h
 */

#include "state.h"

#ifndef STATEFILTER_H_
#define STATEFILTER_H_

class StateFilter {
public:
	StateFilter();
	~StateFilter();

	virtual float weightModifier(State *state) = 0;
};

#endif /* STATEFILTER_H_ */
