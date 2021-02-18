# pagewalkr 🚶‍♂️🚶‍♀️
An x64 page table iterator written in C++ as a kernel mode windows driver. The code in this repository is released under the MIT license. 

This project usses structures provided by ia32.hpp, a project which turned the entirety of the intel manual (SDM) defined structures into a single header, by wbenny & tandasat. https://github.com/wbenny/ia32-doc

# Uses
This is just a proof of concept. It provides no real way to communicate to usermode the findings of the scan, nor does it store them in any way. You can feel free to implement your own functions and logic inside the scan functions should you decide to use this project as a physical memory scanner; or something completely new. You could even use this as a scanner to aid in analysis when trying to find rootkits or anomalies in the page tables, as it would be a near-perfect executable memory finder in the cr3 you execute it in.

# Notes
MmMapIoSpace isn't used while MmGetVirtualForPhysical is used instead because of a patch put in by microsoft that makes it no longer possible to map page tables with MmMapIoSpace.  
