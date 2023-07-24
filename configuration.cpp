#include "configuration.hpp"

bool ParseConfig(HalonConfig* cfg, std::list<std::shared_ptr<FileWatcher>>& watchers)
{
	auto folders = HalonMTA_config_object_get(cfg, "folders");

	if (folders)
	{
		size_t l = 0;
		HalonConfig* folder;
		while ((folder = HalonMTA_config_array_get(folders, l++)))
		{
			const char* id = HalonMTA_config_string_get(HalonMTA_config_object_get(folder, "id"), nullptr);
			const char* path = HalonMTA_config_string_get(HalonMTA_config_object_get(folder, "path"), nullptr);
			const char* serverid = HalonMTA_config_string_get(HalonMTA_config_object_get(folder, "serverid"), nullptr);

			if (!id || !path || !serverid)
				throw std::runtime_error("missing required property");

			const char* threads = HalonMTA_config_string_get(HalonMTA_config_object_get(folder, "threads"), nullptr);
			const char* concurrency = HalonMTA_config_string_get(HalonMTA_config_object_get(folder, "concurrency"), nullptr);

			auto watcher = std::make_shared<FileWatcher>(id, path, serverid, threads ? strtoul(threads, nullptr, 10) : 1, concurrency ? strtoul(concurrency, nullptr, 10) : 0);

			watchers.push_back(watcher);
		}
	}

	return true;
}