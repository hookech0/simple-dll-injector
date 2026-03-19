## DLL Injector

### What is it?

A dead simple remote process DLL injector that I built while going through MalDev Academy's course. I wanted to experiment with GUI implementation, and ImGUI in particular.

It uses `LoadLibraryW`, `VirtualAllocEx`, `WriteProcessMemory`, and `CreateRemoteThread` to inject and execute the desired DLL in a remote process.

I am slowly working on implementing ImGui to give the injector a simple GUI interface.

### Why ImGui?

For fun! I really just wanted to mess around with ImGui. I am interested in game hacking, and it is heavily used in that scene, so I thought I might as well try it out for this!

![gui in progress]()


