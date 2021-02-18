#include "common.hpp"

size_t utils::mtrrs_max;
PPHYSICAL_MEMORY_RANGE utils::mtrrs;

void utils::get_mtrrs()
{
	mtrrs = MmGetPhysicalMemoryRanges();
	auto physical_memory_ranges_number = 0;

	for (size_t mtrrs_number = 0; (mtrrs[mtrrs_number].BaseAddress.QuadPart || mtrrs[mtrrs_number].NumberOfBytes.QuadPart);
		mtrrs_number++)
	{
		log("mtrr: %p - %p\n", mtrrs[mtrrs_number].BaseAddress.QuadPart, mtrrs[mtrrs_number].NumberOfBytes.QuadPart);

		physical_memory_ranges_number++;
	}

	mtrrs_max = physical_memory_ranges_number;
}

bool utils::is_address_in_mtrrs(PHYSICAL_ADDRESS address)
{
	for (auto i = 0; i < mtrrs_max; ++i)
	{
		if ((address.QuadPart >= mtrrs[i].BaseAddress.QuadPart)
			&& (address.QuadPart <= (mtrrs[i].BaseAddress.QuadPart + mtrrs[i].NumberOfBytes.QuadPart)))
		{
			return true;
		}
	}

	return false;
}

void utils::pdpt::large_1_gb_pdpt_callback(PHYSICAL_ADDRESS physical_address, pde_64* entry)
{
	log("[%p] 1 gb pdpt large page found!\n", entry);

	// Let's split the page in 4kb sections to scan.
	for (auto large_page_iterator = 0; large_page_iterator < 0x40000; large_page_iterator++)
	{
		utils::pdpt::scan_pdpt(entry + (large_page_iterator * PAGE_SIZE), PAGE_SIZE, physical_address, entry);
	}
}

void utils::pdpt::scan_pdpt(void* page_base, size_t size, PHYSICAL_ADDRESS physical_address, pde_64* entry)
{
	if (!page_base)
	{
		if (KD_DEBUGGER_ENABLED) __debugbreak();
	}

	log_critical("[%p] 4kb chunk of 1 gb pdpt page chunk found!\n", page_base);
}

void utils::pde::large_2_mb_pde_callback(PHYSICAL_ADDRESS physical_address, pte_64* entry)
{
	log("[%p] 2 mb pde large page found!\n", entry);

	// Let's split the page in 4kb sections to scan.
	for (auto large_page_iterator = 0; large_page_iterator < 512; large_page_iterator++)
	{
		utils::pde::scan_pde(entry + (large_page_iterator * PAGE_SIZE), PAGE_SIZE, physical_address, entry);
	}
}

void utils::pde::scan_pde(void* page_base, size_t size, PHYSICAL_ADDRESS physical_address, pte_64* entry)
{
	if (!page_base)
	{
		if (KD_DEBUGGER_ENABLED) __debugbreak();
	}

	log_critical("[%p] 4kb chunk of 2 mb pde page chunk found!\n", page_base)
}

void utils::pte::scan_pte(void* page_base, size_t size, PHYSICAL_ADDRESS physical_address, pte_64* entry)
{
	if (!page_base)
	{
		if (KD_DEBUGGER_ENABLED) __debugbreak();
	}

	log_critical("[%p] 4 kb pte backed page found!\n", page_base);
}