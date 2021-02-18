# pagewalkr 🚶‍♀️🚶‍♂️🐕🐩
An x64 page table iterator written in C++ as a kernel mode x64 Windows driver. The code in this repository is released under the MIT license. The project has only been tested on Windows 10 20h2.

This project usses structures provided by ia32.hpp, a project which turned the entirety of the intel manual (SDM) defined structures into a single header, by wbenny & tandasat. https://github.com/wbenny/ia32-doc

## Uses
This is just a proof of concept. It provides no real way to communicate to usermode the findings of the scan, nor does it store them in any way. You can feel free to implement your own functions and logic inside the scan functions should you decide to use this project as a physical memory scanner; or something completely new. You could even use this as a scanner to aid in analysis when trying to find rootkits or anomalies in the page tables, as it would be a near-perfect executable memory scanner in the cr3 you execute it in. You can switch the cr3 and run the algorithm nearly as-is to get the same effect (scanning the entire physical memory of a process).

## Notes
MmMapIoSpace isn't used while MmGetVirtualForPhysical is used instead because of a patch put in by microsoft that makes it no longer possible to map page tables with MmMapIoSpace as an exploit mitigation. You can't map page tables anymore by DESIGN, you can't therefore just patch away microsoft's "patch" to the function, as far as my understanding goes. However, it's not hard to replicate the inner workings of MmMapIoSpace. Replacing a page's PTE and using ``__invlpg`` on the page address you previously mapped this way should be more than enough to "emulate" its (old) inner workings. Of course, to be able to run this in your system, you'll need to have test signing enabled, otherwise you won't be able to run the driver without digitally signing it.

## Example
A small example is contained in the repository which simply runs the find_pattern function (a function well known to reverse engineers and beginner hackers alike) that is meant to find a byte pattern in a given range of addresses described by a start address and a size. The scan will start at the "start" address and end at "start + size". The example scans for a common function prologue:

``48 89 5C 24 08    mov qword ptr [rsp+8], rbx``

once the pattern is found, it will print this to the kernel debugger, and you'll be able to verify for yourself that the pattern is, indeed, there with ``uf <address>``.

## Getting started
- Install VS2019
- Install the WDK (Windows driver kit) + the WDK extension for VS2019
- Build as-is, release and debug mode in x64 should both be compiling out of the box.
- Open a CMD prompt as admin, run "``sc create pagewalkr type= kernel binPath= "C:\path\to\the\file\you\just\built.sys"``".
- Open a CMD prompt as admin, run "``sc start pagewalkr``, and look at the kernel debugger to see the results.
