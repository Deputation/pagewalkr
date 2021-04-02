#pragma once

namespace scan
{
	template <typename t>
	inline t find_pattern(const char* pattern, const char* mask, void* start, size_t length)
	{
		const auto data = static_cast<const char*>(start);
		const auto pattern_length = strlen(mask);

		for (auto i = 0; i <= length - pattern_length; i++)
		{
			bool found = true;

			for (auto j = 0; j < pattern_length; j++)
			{
				if (!utils::is_valid(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(start) + i + j)))
				{
					found = false;
					break;
				}

				if (data[i + j] != pattern[j] && mask[j] != '?')
				{
					found = false;
					break;
				}
			}

			if (found)
			{
				return (t)(reinterpret_cast<uintptr_t>(start) + i);
			}
		}

		return (t)(nullptr);
	}

	bool is_in_module_range(void* address);
}