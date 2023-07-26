#include <syslog.h>
#include <algorithm>
#include <unistd.h>

#include "injector.hpp"

void FileInjector(std::shared_ptr<FileWatcher> watcher)
{
	while (true)
	{
		std::unique_lock<std::mutex> ul(*watcher->m_mutex);
		watcher->m_cv->wait(ul, [watcher] () {
			return watcher->m_stop || (!watcher->m_pending->empty() && (watcher->m_concurrency == 0 || watcher->m_processing->size() < watcher->m_concurrency));
		});

		if (watcher->m_stop) break;

		std::string file = watcher->m_pending->front();
		watcher->m_pending->pop_front();
		if (std::find(watcher->m_processing->begin(), watcher->m_processing->end(), file) != watcher->m_processing->end()) continue;
		watcher->m_processing->push_back(file);

		ul.unlock();

		char buf[8192];
		FILE *fp = fopen(file.c_str(), "rb");
		if (!fp)
		{
			syslog(LOG_CRIT, "file-pickup: Could not open file %s", file.c_str());
			watcher->m_mutex->lock();
			watcher->m_processing->remove(file);
			watcher->m_mutex->unlock();
			watcher->m_cv->notify_one();
			continue;
		}
		
		HalonInjectContext *hic = HalonMTA_inject_new(watcher->m_serverid.c_str());

		while (!feof(fp))
		{
			size_t r = fread(buf, 1, sizeof(buf), fp);
			if (r > 0)
				HalonMTA_inject_message_append(hic, buf, r);
		}
		fclose(fp);

		InjectCallbackPtr *icp = new InjectCallbackPtr;
		icp->file         = file;
		icp->cv           = watcher->m_cv;
		icp->processing   = watcher->m_processing;
		icp->mutex        = watcher->m_mutex;

		HalonMTA_inject_callback_set(hic, InjectCallback, icp);

		if (!HalonMTA_inject_commit(hic))
		{
			syslog(LOG_CRIT, "file-pickup: Commit failed for %s", file.c_str());
			watcher->m_mutex->lock();
			watcher->m_processing->remove(file);
			watcher->m_mutex->unlock();
			watcher->m_cv->notify_one();
			HalonMTA_inject_abort(hic);
			delete icp;
		}
	}
}

void InjectCallback(HalonInjectResultContext* hirc, void* ptr)
{
	auto icp = (InjectCallbackPtr*)ptr;

	short int code;
	HalonMTA_inject_result_getinfo(hirc, HALONMTA_RESULT_CODE, nullptr, 0, &code, nullptr);

	if (code >= 400 && code <= 499)
		syslog(LOG_CRIT, "file-pickup: Injection failed with error code %d for %s", code, icp->file.c_str());
	else
		if (unlink(icp->file.c_str()) < 0)
			syslog(LOG_CRIT, "file-pickup: Failed to remove file %s", icp->file.c_str());

	icp->mutex->lock();
	icp->processing->remove(icp->file);
	icp->mutex->unlock();
	icp->cv->notify_one();

	delete icp;
}