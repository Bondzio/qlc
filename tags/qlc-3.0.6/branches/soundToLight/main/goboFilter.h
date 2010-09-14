/*
 * goboFilter.h
 */

#include "stateFilter.h"
#include "state.h"

#ifndef GOBOFILTER_H_
#define GOBOFITER_H_

class GoboFilter : public StateFilter {
public:
	GoboFilter();
	~GoboFilter();

	float weightModifier(State *state);
};

#endif /* GOBOFILTER_H_ */
