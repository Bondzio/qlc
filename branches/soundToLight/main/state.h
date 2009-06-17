/*
 * state.h
 *
 *  Created on: 8 Jun 2009
 *      Author: nawab
 */

#ifndef STATE_H_
#define STATE_H_

class State {
public:
	State();
	virtual ~State();

	virtual void start() = 0;
	virtual void stop() = 0;
};

#endif /* STATE_H_ */
