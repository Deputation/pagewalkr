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

bool utils::is_valid(void* ptr)
{
	return ptr && MmIsAddressValid(ptr);
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
		kd_break;
	}

	utils::page_scan_callback(page_base, size, physical_address);
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
		kd_break;
	}

	utils::page_scan_callback(page_base, size, physical_address);
}

void utils::pte::scan_pte(void* page_base, size_t size, PHYSICAL_ADDRESS physical_address, pte_64* entry)
{
	if (!page_base)
	{
		kd_break;
	}

	utils::page_scan_callback(page_base, size, physical_address);
}

void utils::page_scan_callback(void* page_base, size_t size, PHYSICAL_ADDRESS physical_address)
{
	// More common fn prologues and epilogues could be added to increase the detection rate. This 
	// method is far from perfect, but it can help while trying to manually analyze mapped payloads
	// in kernel space.
	// 
	// prologue -> mov     qword ptr [rsp+8],rbx

	// If the address we've been given does not pertain to the address range
	// of any kernel driver currently loaded...
	if (!scan::is_in_module_range(page_base))
	{
		auto prologue = scan::find_pattern<uint64_t>("\x48\x89\x5C\x24\x08", "xxxxx", page_base, size);

		if (prologue)
		{
			log_info("[page @%p] found a function prologue in non-module memory @%p\n", page_base, prologue)
		}
	}
}