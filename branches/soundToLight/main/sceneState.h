/*
 * sceneState.h
 *
 *  Created on: 8 Jun 2009
 *      Author: nawab
 */

#ifndef SCENESTATE_H_
#define SCENESTATE_H_

#include "state.h"
#include "scene.h"

class SceneState: public State {
public:
	SceneState(t_function_id sceneId);
	virtual ~SceneState();

	t_function_id functionId() { return m_sceneId;}

	virtual void start();
	virtual void stop();

	virtual float baseWeight();

	t_function_id m_sceneId;
};

#endif /* SCENESTATE_H_ */
