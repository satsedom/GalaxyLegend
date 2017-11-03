Galaxy Legend is built for Android OS and is coded at multiple layers, each level utilizing a different language, but all serve a specific purpose.

Under the hood it uses native code libraries built in C. These handle compression, encryption, OS / Java / LUA / Flash interactions, etc.

Java is used to interact with OS, Google, etc.

Game logic is coded in LUA and Flash.

LUA compiled modules are stored as TFL files with contents Cyphered/Encrypted and in some cases Compressed.

Flash compiled modules are stored as TFS files with contents Compressed.

Text strings are split into index and language specific files. 

Current version is 1.9.0.

In this release, all LUA modules and parsed Text strings are available as examples of decompiled code.

Note: some of the LUA modules are loaded at runtime, they are not currnetly included in this example.
