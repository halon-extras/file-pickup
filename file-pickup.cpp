#include <thread>

#include "configuration.hpp"
#include "injector.hpp"

std::list<std::shared_ptr<FileWatcher>> watchers;
std::list<std::thread> workers;

HALON_EXPORT
int Halon_version()
{
	return HALONMTA_PLUGIN_VERSION;
}

HALON_EXPORT
bool Halon_init(HalonInitContext* hic)
{
	HalonConfig* cfg = nullptr;
	HalonMTA_init_getinfo(hic, HALONMTA_INIT_CONFIG, nullptr, 0, &cfg, nullptr);
	if (!cfg)
		return false;
	
	return ParseConfig(cfg, watchers);
}

HALON_EXPORT
void Halon_ready(HalonReadyContext* hrc)
{
	for (auto &watcher : watchers)
	{
		workers.push_back(std::thread([watcher] () {
			pthread_setname_np(pthread_self(), std::string("p/fp/w/" + watcher->m_id).substr(0, 15).c_str());
			watcher->start();
		}));

		for (size_t i = 0; i < watcher->m_threads; ++i)
		{
			workers.push_back(std::thread([watcher] () {
				pthread_setname_np(pthread_self(), std::string("p/fp/i/" + watcher->m_id).substr(0, 15).c_str());
				FileInjector(watcher);
			}));
		}
	}
}

HALON_EXPORT
void Halon_early_cleanup()
{
	for (auto &watcher : watchers)
	{
		watcher->m_stop = true;
		watcher->m_cv->notify_all();
	}

	for (auto &worker : workers)
		worker.join();
}
