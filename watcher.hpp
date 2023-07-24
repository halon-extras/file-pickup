#ifndef WATCHER_HPP_
#define WATCHER_HPP_

#include <string>
#include <list>
#include <condition_variable>
#include <map>
#include <atomic>

class FileWatcher
{
	public:
		FileWatcher(std::string id, std::string path, std::string serverid, size_t threads = 1, size_t concurrency = 0);
		std::string m_id;
		std::string m_path;
		std::string m_serverid;
		size_t m_threads;
		size_t m_concurrency;
		std::shared_ptr<std::list<std::string>> m_pending;
		std::shared_ptr<std::list<std::string>> m_processing;
		std::shared_ptr<std::mutex> m_mutex;
		std::shared_ptr<std::condition_variable> m_cv;
		std::atomic<bool> m_stop = false;
		void start();
	private:
		int m_fd = 0;
		std::map<int, std::string> m_wds;
		void watch(std::string directory);
		void iterate(std::string directory);
		void add(std::string file);
};

#endif