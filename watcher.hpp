#ifndef WATCHER_HPP_
#define WATCHER_HPP_

#include <string>
#include <list>
#include <set>
#include <condition_variable>
#include <map>
#include <atomic>

class FileWatcher
{
	public:
		FileWatcher(std::string id, std::string path, std::string serverid, size_t threads = 1, size_t concurrency = 0);
		~FileWatcher();
		std::string m_id;
		std::string m_path;
		std::string m_serverid;
		size_t m_threads;
		size_t m_concurrency;
		std::shared_ptr<std::list<std::string>> m_pending;
		std::shared_ptr<std::list<std::string>> m_pending_directories;
		std::shared_ptr<std::set<std::string>> m_processing;
		std::shared_ptr<std::mutex> m_mutex;
		std::shared_ptr<std::condition_variable> m_cv;
		std::shared_ptr<std::condition_variable> m_cv_directories;
		std::atomic<bool> m_stop = false;
		void start();
		void iterate();
	private:
		int m_fd = -1;
		std::map<int, std::string> m_wds;
		void watch(const std::string &directory);
		void add(const std::string &file);
};

#endif