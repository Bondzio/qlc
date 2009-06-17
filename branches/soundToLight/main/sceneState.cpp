/*
 * sceneState.cpp
 *
 *  Created on: 8 Jun 2009
 *      Author: nawab
 */

#include "sceneState.h"
#include "function.h"
#include "app.h"
#include "doc.h"

extern App* _app;

SceneState::SceneState(t_function_id sceneId) {
	m_sceneId = sceneId;
}

SceneState::~SceneState() {
}

void SceneState::start() {
	Function* function = _app->doc()->function(m_sceneId);
	Q_ASSERT(function != NULL);

	function->start(_app->masterTimer());
}


void SceneState::stop() {
	Function* function = _app->doc()->function(m_sceneId);
	Q_ASSERT(function != NULL);

	function->stop(_app->masterTimer());
}

