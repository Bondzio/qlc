/*
 * goboFilter.cpp
 *
 * If this is a change to the Gobo, weight it higher if it is near to the current value.
 *
 */

#include "goboFilter.h"
#include "sceneState.h"
#include "scene.h"
#include "doc.h"
#include "app.h"

#include <math.h>

#include "common/qlcfixturedef.h"
#include "common/qlccapability.h"
#include "common/qlcchannel.h"
#include "outputmap.h"

extern App* _app;

GoboFilter::GoboFilter() {
}

GoboFilter::~GoboFilter() {
}

float GoboFilter::weightModifier(State *state) {
	if (SceneState* scenestate = dynamic_cast<SceneState*>(state)) {
		float weightModifier = 1.0;

		Scene* scene = dynamic_cast<Scene*>(_app->doc()->function(scenestate->functionId()));
		QListIterator <SceneValue> it(*(scene->values()));
		while (it.hasNext() == true)
		{
			SceneValue scv(it.next());
			Fixture* fxi = _app->doc()->fixture(scv.fxi);
			QLCChannel* chan = fxi->fixtureDef()->channels()[scv.channel];

			if (chan->group() == KQLCChannelGroupGobo) {
				//this is a gobo channel change.
				//work out the distance from current value (in capabilities)
				t_value value = _app->outputMap()->value(fxi->universeAddress() + scv.channel);
				QLCCapability* oldCap = chan->searchCapability(value);
				QLCCapability* newCap = chan->searchCapability(scv.value);

				int diff = abs(chan->capabilities().indexOf(oldCap) - chan->capabilities().indexOf(newCap));

				//accumulate a weight
				switch (diff) {
				case 0:
					weightModifier *= 1;
					break;
				case 1:
					weightModifier *= 1;
					break;
				case 2:
					weightModifier *= 0.7;
					break;
				default:
					weightModifier *= 0.5;
					break;
				}
			}
		}
		return weightModifier;
	}
	else {
		return 1.0;
	}
}
