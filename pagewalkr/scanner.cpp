#include "common.hpp"

bool scan::is_in_module_range(void* address)
{
	using rtl_pc_to_file_header_t = PVOID(NTAPI*)(PVOID in_value, PVOID* out_base);
	static rtl_pc_to_file_header_t pc_to_file_header = nullptr;

	if (!pc_to_file_header)
	{
		log_info("locating RtlPcToFileHeader!\n");

		UNICODE_STRING procedure_name = {};
		RtlInitUnicodeString(&procedure_name, L"RtlPcToFileHeader");

		pc_to_file_header = 
			reinterpret_cast<rtl_pc_to_file_header_t>(MmGetSystemRoutineAddress(&procedure_name));

		log_info("RtlPcToFileHeader was found @%p\n", pc_to_file_header);
	}

	if (pc_to_file_header)
	{
		auto output_value = reinterpret_cast<PVOID>(0ull);

		if (!pc_to_file_header(address, &output_value))
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		log_info("failure trying to locate RtlPcToFileHeader\n");
	}

	return false;
}