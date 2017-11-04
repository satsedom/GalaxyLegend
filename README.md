Galaxy Legend is built for Android OS and is coded at multiple layers, each level utilizing a different language, but all serve a specific purpose.

Under the hood it uses native code libraries built in C. These handle compression, encryption, OS / Java / LUA / Flash interactions, etc.

Java is used to interact with OS, Google, etc.

Game logic is coded in LUA and Flash.

LUA compiled modules are stored as TFL files with contents Cyphered/Encrypted and in some cases Compressed.

Flash compiled modules are stored as TFS files with contents Compressed.

Text strings are split into index and language specific files. 

Current version is 1.9.0.

In this release, all LUA modules and parsed Text strings are available as examples of decompiled code.

Note: some of the LUA modules are loaded at runtime, they are not currently included in this example.

________________________________________________________________________________

Following are some steps in getting this work done yourself. I will add more steps as I get time to describe them.

Simple things first.

Download and extract apk:
```
Create temp directory, say c:\temp\gl\gl190\.
Download latest apk from https://www.apk4fun.com to c:\temp\gl\gl190\gl190.apk.
Download and install 7zip, WinZip, or WinRar.
Extract contents of apk file into c:\temp\gl\gl190\.
Extract contents of c:\temp\gl\gl190\assets\tap4fun.zip into c:\temp\gl\gl190\assets\.
This will create a file structure similar as the path on an Android device: tap4fun\galaxylegend\AppOriginalData\.
```

Compile texttool program:
```
Download and install Java JDK.
javac texttool.java
```

Parse Text index and language files into readable CSV format using texttool:
```
Execute for each text file:
java texttool c:\temp\gl\gl190\assets\tap4fun\galaxylegend\AppOriginalData\data2\text\item.idx c:\temp\gl\gl190\assets\tap4fun\galaxylegend\AppOriginalData\data2\text\item.en > item.csv
```

Difficult things next.

Notes:
```
PAK files are archives of files using Gameloft/Glitch Engine Rotating XOR Encryption/Decryption.
TFL files are LUA binaries using Gameloft/Glitch Engine Rotating XOR Encryption/Decryption.
```

Install Unix Shell on Windows:
```
Download and install Cygwin. This makes it easy to create and execute scripts.
Choose to install gcc. We need 32bit C compiler specifically.
Choose to install make.
More details to come...
```

Compile xortool program:
```
More details to come...
```

Extract PAK archives using xortool:
```
More details to come...
```

Decrypt TFL files into LUAQ binary executable format using xortool:
```
More details to come...
```

Modify LUA decompiler OP codes from defaults to T4F rearranged codes:
```
More details to come...
```

Decompile LUAQ executables into readable LUA programs:
```
More details to come...
```

More details to come...
