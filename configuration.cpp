#include "configuration.hpp"

bool ParseConfig(HalonConfig* cfg, std::list<std::shared_ptr<FileWatcher>>& watchers)
{
	auto directories = HalonMTA_config_object_get(cfg, "directories");

	if (directories)
	{
		size_t l = 0;
		HalonConfig* directory;
		while ((directory = HalonMTA_config_array_get(directories, l++)))
		{
			const char* id = HalonMTA_config_string_get(HalonMTA_config_object_get(directory, "id"), nullptr);
			const char* path = HalonMTA_config_string_get(HalonMTA_config_object_get(directory, "path"), nullptr);
			const char* serverid = HalonMTA_config_string_get(HalonMTA_config_object_get(directory, "serverid"), nullptr);

			if (!id || !path || !serverid)
				throw std::runtime_error("missing required property");

			const char* threads = HalonMTA_config_string_get(HalonMTA_config_object_get(directory, "threads"), nullptr);
			const char* concurrency = HalonMTA_config_string_get(HalonMTA_config_object_get(directory, "concurrency"), nullptr);

			auto watcher = std::make_shared<FileWatcher>(id, path, serverid, threads ? strtoul(threads, nullptr, 10) : 1, concurrency ? strtoul(concurrency, nullptr, 10) : 0);

			watchers.push_back(watcher);
		}
	}

	return true;
}