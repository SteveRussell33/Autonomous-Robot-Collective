#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
    pluginInstance = p;

    p->addModel(modelATV);
    p->addModel(modelCLIP);
    p->addModel(modelFM);
    p->addModel(modelFOO);
    p->addModel(modelGAIN);
    p->addModel(modelMIX1);
    p->addModel(modelMIX2);
}
