/*
 * sceneState.cpp
 *
 *  Created on: 8 Jun 2009
 *      Author: nawab
 */

#include "blackoutState.h"
#include "function.h"
#include "app.h"
#include "outputmap.h"

#include <iostream>

extern App* _app;

using namespace std;
BlackoutState::BlackoutState() {
}

BlackoutState::~BlackoutState() {
}

void BlackoutState::start() {
	//cout << "blackout\n";
	_app->outputMap()->setBlackout(true);
}

void BlackoutState::stop() {
	_app->outputMap()->setBlackout(false);
}

int BlackoutState::baseWeight() {
	return 10;
}
