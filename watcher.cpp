#include <filesystem>
#include <syslog.h>
#include <sys/inotify.h>
#include <algorithm>
#include <poll.h>
#include <unistd.h>
#include <thread>

#include "watcher.hpp"

#define EVENT_SIZE sizeof (struct inotify_event)
#define BUFFER_LENGTH (1024 * (EVENT_SIZE + 16))

FileWatcher::FileWatcher(std::string id, std::string path, std::string serverid, size_t threads, size_t concurrency)
{
	m_id = id;
	m_path = path;
	m_serverid = serverid;
	m_threads = threads;
	m_concurrency = concurrency;

	m_pending = std::make_shared<std::list<std::string>>();
	m_processing = std::make_shared<std::set<std::string>>();
	m_mutex = std::make_shared<std::mutex>();
	m_cv = std::make_shared<std::condition_variable>();
}

void FileWatcher::start()
{
	int fd = inotify_init();
	if (fd < 0)
	{
		syslog(LOG_CRIT, "file-pickup: Could not initialize watcher for %s", m_id.c_str());
		return;
	}
	m_fd = fd;

	iterate(m_path);

	while (true)
	{
		if (m_stop) break;

		struct pollfd pfd = { m_fd, POLLIN, 0 };
		int ret = poll(&pfd, 1, 50);
		if (ret < 0)
		{
			syslog(LOG_CRIT, "file-pickup: Poll failed for %s", m_id.c_str());
			continue;
		}
		else if (ret > 0)
		{
			char buffer[BUFFER_LENGTH];
			auto length = read(m_fd, buffer, BUFFER_LENGTH);

			if (length < 0)
			{
				syslog(LOG_CRIT, "file-pickup: Read failed for %s", m_id.c_str());
				continue;
			}

			size_t i = 0;
			size_t l = length;
			while (i < l)
			{
				struct inotify_event *event = (struct inotify_event*) &buffer[i];
				if (event->len)
				{
					if ((event->mask & IN_CREATE) && (event->mask & IN_ISDIR))
					{
						m_mutex->lock();
						if (m_wds.count(event->wd) == 1)
						{
							std::string path = m_wds.at(event->wd);
							m_mutex->unlock();
							watch(path + std::string("/") + event->name);
						}
						else
							m_mutex->unlock();
					}
					if ((event->mask & IN_MOVED_TO) && (event->mask & IN_ISDIR))
					{
						m_mutex->lock();
						if (m_wds.count(event->wd) == 1)
						{
							std::string path = m_wds.at(event->wd);
							m_mutex->unlock();
							iterate(path + std::string("/") + event->name);
						}
						else
							m_mutex->unlock();
					}
					if ((event->mask & IN_MOVED_TO) && !(event->mask & IN_ISDIR))
					{
						m_mutex->lock();
						if (m_wds.count(event->wd) == 1)
						{
							std::string path = m_wds.at(event->wd);
							m_mutex->unlock();
							add(path + std::string("/") + event->name);
						}
						else
							m_mutex->unlock();
					}
				}
				else
				{
					if ((event->mask & IN_Q_OVERFLOW))
					{
						syslog(LOG_CRIT, "file-pickup: Event queue overflowed");

						m_mutex->lock();
						for (auto wd : m_wds)
							inotify_rm_watch(m_fd, wd.first);
						m_wds.clear();
						m_pending->clear();
						m_mutex->unlock();

						iterate(m_path);
					}
					if ((event->mask & IN_DELETE_SELF) || (event->mask & IN_MOVE_SELF))
					{
						m_mutex->lock();
						if (m_wds.count(event->wd) == 1)
						{
							std::string path = m_wds.at(event->wd);
							m_mutex->unlock();
							inotify_rm_watch(m_fd, event->wd);
							syslog(LOG_NOTICE, "file-pickup: Removed watcher for %s", path.c_str());
						}
						else
							m_mutex->unlock();
					}
				}
				i += EVENT_SIZE + event->len;
			}
		}
	}

	m_mutex->lock();
	for (auto wd : m_wds)
		inotify_rm_watch(m_fd, wd.first);
	m_wds.clear();
	m_mutex->unlock();

	close(m_fd);
}

void FileWatcher::add(const std::string &file)
{
	std::string ext = ".eml";
	if (ext.size() < file.size() && std::equal(ext.rbegin(), ext.rend(), file.rbegin()))
	{
		m_mutex->lock();
		m_pending->push_back(file);
		m_mutex->unlock();
		m_cv->notify_one();
	}
}

void FileWatcher::watch(const std::string &directory)
{
	int wd = inotify_add_watch(m_fd, directory.c_str(), IN_CREATE | IN_MOVED_TO | IN_DELETE_SELF | IN_MOVE_SELF);
	if (wd < 0)
		syslog(LOG_CRIT, "file-pickup: Could not start watcher for %s", directory.c_str());
	else
	{
		m_mutex->lock();
		m_wds.insert({ wd, directory });
		m_mutex->unlock();
		syslog(LOG_NOTICE, "file-pickup: Started watcher for %s", directory.c_str());
	}
}

void FileWatcher::iterate(const std::string &directory)
{
	std::thread([this, directory] () {
		pthread_setname_np(pthread_self(), std::string("p/fp/d/" + m_id).substr(0, 15).c_str());
		watch(directory.c_str());
		try {
			for (const auto & file : std::filesystem::recursive_directory_iterator(directory, std::filesystem::directory_options::skip_permission_denied))
			{
				if (file.is_directory())
					watch(file.path());
				else if (file.is_regular_file())
					add(file.path());
			}
		} catch (std::filesystem::filesystem_error &err) {
			syslog(LOG_NOTICE, "file-pickup: %s", err.what());
		}
	}).detach();
}