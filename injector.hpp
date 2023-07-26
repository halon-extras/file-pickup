#ifndef INJECTOR_HPP_
#define INJECTOR_HPP_

#include <HalonMTA.h>

#include "watcher.hpp"

struct InjectCallbackPtr{
	std::string file;
	std::shared_ptr<std::set<std::string>> processing;
	std::shared_ptr<std::mutex> mutex;
	std::shared_ptr<std::condition_variable> cv;
};

void FileInjector(std::shared_ptr<FileWatcher> watcher);
void InjectCallback(HalonInjectResultContext* hirc, void* ptr);

#endif