#include <nvs_flash.h>
#include "param_store.hpp"

using namespace menu;

ParamStore::ParamStore(uint8_t maxProjects, uint8_t maxVoices) : maxProjects(maxProjects), maxVoices(maxVoices)
{
    nvs_flash_init();
}
