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

extern App* _app;

BlackoutState::BlackoutState() {
}

BlackoutState::~BlackoutState() {
}

void BlackoutState::start() {
	_app->outputMap()->setBlackout(true);
}

void BlackoutState::stop() {
	_app->outputMap()->setBlackout(false);
}

