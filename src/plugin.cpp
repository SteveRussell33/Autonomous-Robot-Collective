#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
    pluginInstance = p;

    p->addModel(modelFM);
    p->addModel(modelFOLD);
    p->addModel(modelGAIN);
    p->addModel(modelDRV);
}
