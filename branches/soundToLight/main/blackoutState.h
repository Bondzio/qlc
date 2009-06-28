/*
 * blackoutState.h
 *
 *  Created on: 8 Jun 2009
 *      Author: nawab
 */

#ifndef BLACKOUTSTATE_H_
#define BLACKOUTSTATE_H_

#include "state.h"

class BlackoutState: public State {
public:
	BlackoutState();
	virtual ~BlackoutState();

	virtual void start();
	virtual void stop();

	virtual int baseWeight();
};

#endif /* BLACKOUTSTATE_H_ */
