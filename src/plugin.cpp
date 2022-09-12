#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
    pluginInstance = p;

    p->addModel(modelATV);
    p->addModel(modelCLIP);
    p->addModel(modelDRV);
    p->addModel(modelFM);
    p->addModel(modelFOLD);
    p->addModel(modelGAIN);
}
