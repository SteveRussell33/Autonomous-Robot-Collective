#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
    pluginInstance = p;

    p->addModel(modelATV);
    p->addModel(modelFM);
    p->addModel(modelMIX2);
}
