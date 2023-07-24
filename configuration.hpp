#ifndef CONFIGURATION_HPP_
#define CONFIGURATION_HPP_

#include <HalonMTA.h>

#include "watcher.hpp"

bool ParseConfig(HalonConfig* cfg, std::list<std::shared_ptr<FileWatcher>>& watchers);

#endif