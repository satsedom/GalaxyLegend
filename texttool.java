/*
example:
  rem delete non english language files:
  del *.cn *.fr *.ger *.ido *.it *.jp *.kr *.nl *.po *.ru *.sp *.tr *.zh

  rem parse english language file:
  java texttool item.idx item.en > item.csv
*/

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