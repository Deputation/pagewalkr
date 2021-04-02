#include "common.hpp"

void thread_start(void* context)
{
	log_info("Thread started.\n");

	// Get the MTRRs, (physical memory ranges) and print them.
	utils::get_mtrrs();

	// Get the kernel's page directory
	cr3 kernel_cr3;
	kernel_cr3.flags = __readcr3();

	log_info("ntoskrnl.exe page directory: %p\n", kernel_cr3.address_of_page_directory);

	// Get the pml4 table entry (starting from rs4 insider builds, you can no longer MmMapIoSpace page tables by DESIGN.)
	// Here we shift by PAGE_SHIFT (12) since what's contained in cr3 is actually a page frame number rather than a fixed 
	// physical address. Shifting by 12 bits left equals to multiplying by 4096 (0x1000) which equals to multiplying by
	// PAGE_SIZE. With a page frame number, we're treating physical memory as an array composed by members (pages) each 4kb
	// (4096 or 0x1000) bytes big to work with. Simply multiplying by 0x1000 would work as well. Here we're simply using the
	// page frame number in the cr3 register to find the base of the PML4 page table in the 4-level paging structure.
	// To scan other processes in the system as well, you should grab their cr3 and walk their paging tables in a similar way.
	PHYSICAL_ADDRESS phys_buffer;
	phys_buffer.QuadPart = kernel_cr3.address_of_page_directory << PAGE_SHIFT;

	pml4e_64* pml4 = reinterpret_cast<pml4e_64*>(MmGetVirtualForPhysical(phys_buffer));

	if (!MmIsAddressValid(pml4) || !pml4)
	{
		log_info("invalid pml4 found...\n");

		goto RETURN;
	}

	// Let's start looping through the pml4s
	for (auto pml4_index = 0; pml4_index < 512; pml4_index++)
	{
		// Transform the page frame number to a physical address for the next paging structure in hierarchy...
		phys_buffer.QuadPart = pml4[pml4_index].page_frame_number << PAGE_SHIFT;

		if (!pml4[pml4_index].present)
		{
			// PML4 not present
			continue;
		}

		log("pml4 present @%p\n", phys_buffer.QuadPart);

		// Now we will get the pdpt entries this pml4 maps.
		pdpte_64* pdpt = reinterpret_cast<pdpte_64*>(MmGetVirtualForPhysical(phys_buffer));

		if (!utils::is_valid(pdpt))
		{
			// Invalid pdpt table ptr
			continue;
		}

		for (auto pdpt_index = 0; pdpt_index < 512; pdpt_index++)
		{
			// Transform the page frame number to a physical address for the next paging structure in hierarchy...
			phys_buffer.QuadPart = pdpt[pdpt_index].page_frame_number << PAGE_SHIFT;

			if (!pdpt[pdpt_index].present)
			{
				// No pdpt present here... move on.
				continue;
			}

			if (pdpt[pdpt_index].large_page)
			{
				pde_64* pde = reinterpret_cast<pde_64*>(MmGetVirtualForPhysical(phys_buffer));

				if (!utils::is_valid(pde))
				{
					// Table ptr invalid.
					continue;
				}

				utils::pdpt::large_1_gb_pdpt_callback(phys_buffer, pde);

				// Page dealt with, move on.
				continue;
			}

			// Keep on enumerating the pdpt table.
			pde_64* pde = reinterpret_cast<pde_64*>(MmGetVirtualForPhysical(phys_buffer));

			if (!utils::is_valid(pde))
			{
				// Table ptr invalid.
				continue;
			}

			for (auto pde_index = 0; pde_index < 512; pde_index++)
			{
				// Calculate address of the next paging structure in the hierarchy.
				phys_buffer.QuadPart = pde[pde_index].page_frame_number << PAGE_SHIFT;
			
				if (!pde[pde_index].present)
				{
					// No pde here...
					continue;
				}

				if (pde[pde_index].large_page)
				{
					pte_64* pte = reinterpret_cast<pte_64*>(MmGetVirtualForPhysical(phys_buffer));

					if (!utils::is_valid(pte))
					{
						// Invalid table ptr.
						continue;
					}

					utils::pde::large_2_mb_pde_callback(phys_buffer, pte);

					// Large table handled, let's move on.
					continue;
				}

				pte_64* pte = reinterpret_cast<pte_64*>(MmGetVirtualForPhysical(phys_buffer));

				if (!utils::is_valid(pte))
				{
					// Invalid table ptr.
					continue;
				}

				for (auto pte_index = 0; pte_index < 512; pte_index++)
				{
					phys_buffer.QuadPart = pte[pte_index].page_frame_number << PAGE_SHIFT;

					if (!pte[pte_index].present)
					{
						// no pte here...
						continue;
					}

					pte_64* individual_pte = reinterpret_cast<pte_64*>(MmGetVirtualForPhysical(phys_buffer));

					if (utils::is_address_in_mtrrs(phys_buffer) && utils::is_valid(individual_pte))
					{
						auto four_kb_page = MmGetVirtualForPhysical(phys_buffer);

						utils::pte::scan_pte(four_kb_page, PAGE_SIZE, phys_buffer, individual_pte);
					}
				}
			}
		}
	}

RETURN:
	log_info("Thread returning...\n");

	return;
}

void DriverUnload(PDRIVER_OBJECT driver_object)
{
	log_info("freeing memory...");

	// Undocumented memory allocation.
	ExFreePool(utils::mtrrs);

	return;
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path)
{
	UNREFERENCED_PARAMETER(registry_path);

	driver_object->DriverUnload = DriverUnload;

	HANDLE thread_handle;

	return PsCreateSystemThread(&thread_handle, THREAD_ALL_ACCESS, 0, 0, 0, thread_start, 0);
}