#pragma once

namespace utils
{
	extern size_t mtrrs_max;
	extern PPHYSICAL_MEMORY_RANGE mtrrs;

	void get_mtrrs();
	bool is_address_in_mtrrs(PHYSICAL_ADDRESS address);

	namespace pdpt
	{
		void large_1_gb_pdpt_callback(PHYSICAL_ADDRESS physical_address, pde_64* entry);
		void scan_pdpt(void* page_base, size_t size, PHYSICAL_ADDRESS physical_address, pde_64* entry);
	}

	namespace pde
	{
		void large_2_mb_pde_callback(PHYSICAL_ADDRESS physical_address, pte_64* entry);
		void scan_pde(void* page_base, size_t size, PHYSICAL_ADDRESS physical_address, pte_64* entry);
	}

	namespace pte
	{
		void scan_pte(void* page_base, size_t size, PHYSICAL_ADDRESS physical_address, pte_64* entry);
	}
}