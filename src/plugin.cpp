#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
    pluginInstance = p;

    p->addModel(modelATV);
    p->addModel(modelCLIP);
    p->addModel(modelFM);
    p->addModel(modelGAIN);
    p->addModel(modelTRACK4);
}
