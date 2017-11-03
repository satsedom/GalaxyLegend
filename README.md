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

Following are some steps in getting this work done yourself. I will add more steps as I get time to describe them.

Download and extract:
```
Create temp directory, say c:\temp\gl\gl190\.
Download latest apk from https://www.apk4fun.com to c:\temp\gl\gl190\gl190.apk.
Download and install 7zip, WinZip, or WinRar.
Extract contents of apk file into c:\temp\gl\gl190\.
Extract contents of c:\temp\gl\gl190\assets\tap4fun.zip into c:\temp\gl\gl190\assets\.
This will create a file structure similar as the path on an Android device: tap4fun\galaxylegend\AppOriginalData\.
```

Java program parse GL index and text files (texttool.java):
```java
import java.io.*;
import java.lang.*;
import java.util.*;
import java.util.zip.*;
import java.nio.*;
import java.nio.file.*;
import java.nio.charset.*;

public class texttool
{
  static class TextEntry {
    public short index;
    public String name;
    public String description;
  }

	public static void main(String[] args) throws Exception
	{
    if (args.length != 2) {
      System.err.println("Usage: texttool idxFile txtFile");
      System.exit(1);
    }
    ByteBuffer twoBytes = ByteBuffer.allocate(2);
    twoBytes.order(ByteOrder.LITTLE_ENDIAN);
    String idxFile = args[0];
    String txtFile = args[1];
    byte[] idxBytes = Files.readAllBytes(Paths.get(idxFile));
    int idxPos = 0;
    byte[] txtBytes = Files.readAllBytes(Paths.get(txtFile));
    int txtPos = 0;
    twoBytes.clear();
    twoBytes.put(idxBytes[idxPos++]);
    twoBytes.put(idxBytes[idxPos++]);
    short idxCount = twoBytes.getShort(0);
    twoBytes.clear();
    twoBytes.put(txtBytes[txtPos++]);
    twoBytes.put(txtBytes[txtPos++]);
    short txtCount = twoBytes.getShort(0);
    if (idxCount != txtCount) {
      System.err.println("Number of entries in two files don't match");
      System.exit(1);
    }
    HashMap<Integer, TextEntry> entries = new HashMap<Integer, TextEntry>();
    for (int i=0; i<idxCount; i++) {
      twoBytes.clear();
      twoBytes.put(idxBytes[idxPos++]);
      twoBytes.put(idxBytes[idxPos++]);
      short idxStrLen = twoBytes.getShort(0);
      String idxStr = new String(idxBytes, idxPos, idxStrLen);
      idxPos += idxStrLen;
      twoBytes.clear();
      twoBytes.put(txtBytes[txtPos++]);
      twoBytes.put(txtBytes[txtPos++]);
      short txtStrLen = twoBytes.getShort(0);
      String txtStr = new String(txtBytes, txtPos, txtStrLen);
      txtPos += txtStrLen;
      TextEntry entry = new TextEntry();
      entry.index = (short)(i + 1);
      entry.name = idxStr;
      entry.description = txtStr;
      entries.put(i+1, entry);
      System.out.println("\"" + entry.name + "\",\"" + entry.description + "\"");
    }
	}
}
```

Compile and run:
```
Download and install Java JDK.
javac texttool.java
java texttool c:\temp\gl\gl190\assets\tap4fun\galaxylegend\AppOriginalData\data2\text\item.idx c:\temp\gl\gl190\assets\tap4fun\galaxylegend\AppOriginalData\data2\text\item.en > item.csv
```

More to come...
