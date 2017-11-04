#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <dirent.h>

#include <unistd.h>
#include <signal.h>

/*

test:
  ./xortool levl.tfl levl.tfl.unxor 2
  diff levl.tfl.luaq levl.tfl.unxor

  ./xortool levl.tfl.unxor levl.tfl.xor 1
  diff levl.tfl levl.tfl.xor
  
  ./xortool data1 data1.pak 3

example:
  ./xortool levl.tfl levl.luaq 2
  
  ./xortool levl.luaq levl.tfl 1
  
  ./xortool data2 data2.pak 3
  
  ./xortool data2.pak data2 4

references:
  https://tools.ietf.org/html/rfc1950

  http://www.xda-developers.com/dexpatcher-patch-android-apks-using-java/
*/

#define XOR_KEY               0x3857AL
#define MAX_PATH_LENGTH       256                //including string terminator
#define PAK_FILE_SIGNATURE    0x9D7FE2ABL
#define DEBUG_FLAG            false

typedef unsigned int uInt;
typedef unsigned long uLong;

void usage() {
  printf("Invalid parameters.\n  xortool in_path out_path operation(%s | %s | %s | %s).\n"
   ,"1=encode file"
   ,"2=decode file"
   ,"3=pack directory"
   ,"4=unpack directory"
  );
  printf("  pack directory: pack all files inside directory specified by in_path into pak file specified by out_path\n");
  printf("  unpack directory: decode all files inside pak file specified by in_path into directory specified by out_path\n");
  exit(1);
}

int fsize(FILE *fp){
  int prev = ftell(fp); //save previous position
  fseek(fp, 0L, SEEK_END);
  int sz = ftell(fp);
  fseek(fp, prev, SEEK_SET); //go back to where we were
  return sz;
}

int isFile(char *fpath) {
  struct stat sb;
  if (stat(fpath, &sb) == 0 && S_ISREG(sb.st_mode))
    return 1;
  else
    return 0;
}

int isDir(char *fpath) {
  struct stat sb;
  if (stat(fpath, &sb) == 0 && S_ISDIR(sb.st_mode))
    return 1;
  else
    return 0;
}

// Gameloft/Glitch Engine Rotating XOR Encryption/Decryption
//
// |
// V

#define rotr32(x,n) ((((unsigned)x) >> n) | ((x) << (32 - n)))
#define rotl32(x,n) (((x) << n) | ((unsigned)(x) >> (32 - n)))
#define rotr8(x,n) (((unsigned char)(x) >> n) | ((x) << (8 - n)))
#define rotl8(x,n) (((x) << n) | ((unsigned char)(x) >> (8 - n)))
#define rotr16(x,n) (((unsigned short)(x) >> n) | ((x) << (16 - n)))
#define rotl16(x,n) (((x) << n) | ((unsigned short)(x) >> (16 - n)))
#define rotr24(x,n) (((unsigned)(x) >> n) | ((x) << (24 - n)))
#define rotl24(x,n) (((x) << n) | ((unsigned)(x) >> (24 - n)))

#define CONST_A 0x19660d
#define CONST_C 0x3c6ef35f

typedef unsigned char Byte;
typedef unsigned short Word;

enum CustomPakFileEntryAttributes {
  None = 0L
 ,Encrypted = 1L
 ,Unencrypted = 4L
};

struct CustomPakFileContentsEntry {
  Byte *FileContents;
};

struct CustomPakFileNameEntry {
  char *FileName;
  char *FilePath;
  Byte *EncryptedFileName;
  uInt FileNameLength;
};

struct CustomPakFileEntry {
  uInt Offset;
  uInt Length;
  uInt FileNameOffset;
  CustomPakFileEntryAttributes Attributes;
};

struct CustomPakFileHeader {
  uInt Signature;
  uInt DataStartOffset;
  uInt FileNameTableOffset;
  uInt FilesCount;
};

int g_nRandomSeed;

void setRand(int seed)
{
	g_nRandomSeed = seed;
}

int getRand()
{
	g_nRandomSeed = CONST_A * g_nRandomSeed + CONST_C;
	return (unsigned)g_nRandomSeed >> 16;
}

int getRand(int maxValue)
{
	if (maxValue == 0) return getRand();
	else return getRand() % maxValue;
}

// Get random number between range, maxValue inclusive
int getRand(int minValue, int maxValue)
{
	return getRand(maxValue + 1 - minValue) + minValue;
}

float getRand(float minValue, float maxValue)
{
	return (maxValue - minValue) * 0.000015259f * getRand();
}
	
int random(int maxValue)
{
	if (maxValue <= 0) return 0;
	g_nRandomSeed = CONST_A * g_nRandomSeed + CONST_C;
	return (unsigned)(g_nRandomSeed >> 16) % maxValue;
}

int random(int minValue, int maxValue)
{
	if (maxValue - minValue <= 0) return 0;
	return random(maxValue - minValue) + minValue;
}

// Gameloft/Glitch Engine Rotating XOR Decryption
void DECODE_XOR32(Byte* source, unsigned int length, Byte* dest, int key)
{
	unsigned offset = 0;
	setRand(key);
	
	if (length > 3)
	{
		do
		{
			// Load into unsigned int byte-by-byte for portability
			unsigned procLong =
          (          *(source + offset    )      )
				| ((unsigned)*(source + offset + 1) << 8 )
				| ((unsigned)*(source + offset + 2) << 16)
				| ((unsigned)*(source + offset + 3) << 24);
			int rotDir = getRand(2);
			int rotAmount = getRand(32); // Separated out: compiles into ror and rol vs shift and OR
			if (rotDir)
				procLong = rotr32(procLong, rotAmount);
			else
				procLong = rotl32(procLong, rotAmount);
			// Decrypt MSB order
			*(dest + offset + 3) = (Byte)(getRand(0x100) ^ (procLong >> 24));
			*(dest + offset + 2) = (Byte)(getRand(0x100) ^ (procLong >> 16));
			*(dest + offset + 1) = (Byte)(getRand(0x100) ^ (procLong >> 8 ));
			*(dest + offset    ) = (Byte)(getRand(0x100) ^ (procLong      ));
			offset += 4;
		}
		while (length > offset + 3);
	}
	int rotDir = getRand(2); // 0 == left, 1 == right
	switch (length - offset)
	{
		case 1:
			{
				Byte val = *(source + offset);
				int rotAmount = getRand(8);
				if (rotDir)
					val = rotr8(val, rotAmount);
				else
					val = rotl8(val, rotAmount);
				*(dest + offset) = (Byte)getRand(0x100) ^ val;
			}
			break;
		case 2:
			{
				Word val =
            (      *(source + offset    )     )
					| ((Word)*(source + offset + 1) << 8);
				int rotAmount = getRand(16);
				if (rotDir)
					val = rotr16(val, rotAmount);
				else
					val = rotl16(val, rotAmount);
				*(dest + offset + 1) = (Byte)(getRand(0x100) ^ (val >> 8));
				*(dest + offset    ) = (Byte)(getRand(0x100) ^ (val     ));
			}
			break;
		case 3:
			{
				unsigned val =
            (          *(source + offset    )      )
					| ((unsigned)*(source + offset + 1) << 8 )
					| ((unsigned)*(source + offset + 2) << 16);
				int rotAmount = getRand(24);
				if (rotDir)
					val = rotr24(val, rotAmount);
				else
					val = rotl24(val, rotAmount);
				*(dest + offset + 2) = (Byte)(getRand(0x100) ^ (val >> 16));
				*(dest + offset + 1) = (Byte)(getRand(0x100) ^ (val >> 8 ));
				*(dest + offset    ) = (Byte)(getRand(0x100) ^ (val      ));
			}
			break;
		case 0:
			if (length == offset) break;
		default:
			printf("something went wrong %d !\n", length - offset);
	}
}

// Gameloft/Glitch Engine Rotating XOR Encryption
void ENCODE_XOR32(Byte* source, unsigned int length, Byte* dest, int key)
{
	unsigned offset = 0;
	setRand(key);
	
	if (length > 3)
	{
		do
		{
			int rotDir = getRand(2);
			int rotAmount = getRand(32);
			unsigned procLong =
          ((getRand(0x100) ^ (unsigned)*(source + offset + 3)) << 24)
				| ((getRand(0x100) ^ (unsigned)*(source + offset + 2)) << 16)
				| ((getRand(0x100) ^ (unsigned)*(source + offset + 1)) << 8 )
				| ((getRand(0x100) ^ (unsigned)*(source + offset    ))      );
			if (rotDir)
				procLong = rotl32(procLong, rotAmount);
			else
				procLong = rotr32(procLong, rotAmount);
			*(dest + offset    ) = (Byte)(procLong      );
			*(dest + offset + 1) = (Byte)(procLong >> 8 );
			*(dest + offset + 2) = (Byte)(procLong >> 16);
			*(dest + offset + 3) = (Byte)(procLong >> 24);
			offset += 4;
		}
		while (length > offset + 3);
	}
	int rotDir = getRand(2); // 0 == left, 1 == right
	switch (length - offset)
	{
		case 1:
			{
				int rotAmount = getRand(8);
				Byte val = (getRand(0x100) ^ *(source + offset));
				if (rotDir)
					val = rotl8(val, rotAmount);
				else
					val = rotr8(val, rotAmount);
				*(dest + offset) = (Byte)val;
			}
			break;
		case 2:
			{
				int rotAmount = getRand(16);
				Word val =
            ((getRand(0x100) ^ (Word)*(source + offset + 1)) << 8)
					| ((getRand(0x100) ^ (Word)*(source + offset    ))     );
				if (rotDir)
					val = rotl16(val, rotAmount);
				else
					val = rotr16(val, rotAmount);
				*(dest + offset    ) = (Byte)(val     );
				*(dest + offset + 1) = (Byte)(val >> 8);
			}
			break;
		case 3:
			{
				int rotAmount = getRand(24);
				unsigned val =
            ((getRand(0x100) ^ (unsigned)*(source + offset + 2)) << 16)
					| ((getRand(0x100) ^ (unsigned)*(source + offset + 1)) << 8 )
					| ((getRand(0x100) ^ (unsigned)*(source + offset    ))      );
				if (rotDir)
					val = rotl24(val, rotAmount);
				else
					val = rotr24(val, rotAmount);
				*(dest + offset    ) = (Byte)(val      );
				*(dest + offset + 1) = (Byte)(val >> 8 );
				*(dest + offset + 2) = (Byte)(val >> 16);
			}
			break;
	}
}

// ^
// |
//
// Gameloft/Glitch Engine Rotating XOR Encryption/Decryption

void encodeFile(char *in_path, char *out_path) {
  //open input file
  FILE *in_file = fopen(in_path, "rb");
  if (in_file == NULL) {
    printf("Failed to open input file '%s'\n", in_path);
    exit(1);
  }

  //check input size
  uLong in_len = fsize(in_file);
  if (in_len == 0) {
    printf("Input file is empty\n");
    exit(1);
  }

  //allocate input buffer
  Byte *in_data = (Byte*)calloc((uInt)in_len, 1);
  if (in_data == NULL) {
    printf("Out of memory\n");
    exit(1);
  }

  //read input file into input buffer
  fread(in_data, in_len, 1, in_file);

  //close input file
  fclose(in_file);

  //allocate output buffer
  Byte *out_data = (Byte*)calloc((uInt)in_len, 1);
  if (out_data == NULL) {
    printf("Out of memory\n");
    exit(1);
  }

  //perform operation
  printf("Encrypting file %s to %s\n", in_path, out_path);
  ENCODE_XOR32(in_data, in_len, out_data, XOR_KEY);

  //open output file
  FILE *out_file = fopen(out_path, "wb");
  if (out_file == NULL) {
    printf("Failed to open output file '%s'\n", out_path);
    exit(1);
  }

  //write output buffer into output file
  fwrite(out_data, 1, in_len, out_file);
  
  //close output file
  fclose(out_file);

  //free memory
  free(in_data);
  free(out_data);
}

void decodeFile(char *in_path, char *out_path) {
  //open input file
  FILE *in_file = fopen(in_path, "rb");
  if (in_file == NULL) {
    printf("Failed to open input file '%s'\n", in_path);
    exit(1);
  }

  //check input size
  uLong in_len = fsize(in_file);
  if (in_len == 0) {
    printf("Input file is empty\n");
    exit(1);
  }

  //allocate input buffer
  Byte *in_data = (Byte*)calloc((uInt)in_len, 1);
  if (in_data == NULL) {
    printf("Out of memory\n");
    exit(1);
  }

  //read input file into input buffer
  fread(in_data, in_len, 1, in_file);

  //close input file
  fclose(in_file);

  //allocate output buffer
  Byte *out_data = (Byte*)calloc((uInt)in_len, 1);
  if (out_data == NULL) {
    printf("Out of memory\n");
    exit(1);
  }

  //perform operation
  printf("Decrypting file %s to %s\n", in_path, out_path);
  DECODE_XOR32(in_data, in_len, out_data, XOR_KEY);

  //open output file
  FILE *out_file = fopen(out_path, "wb");
  if (out_file == NULL) {
    printf("Failed to open output file '%s'\n", out_path);
    exit(1);
  }

  //write output buffer into output file
  fwrite(out_data, 1, in_len, out_file);
  
  //close output file
  fclose(out_file);

  //release memory
  free(in_data);
  free(out_data);
}

void printBytes(Byte *data, uInt size) {
  for (int i = 0; i < size; i++) {
    printf("%02X", (Byte)data[i]);
  }
  printf("\n");
}

void printChars(Byte *data, uInt size) {
  for (int i = 0; i < size; i++) {
    printf("%c ", (Byte)data[i]);
  }
  printf("\n");
}

void printBytesAndChars(Byte *data, uInt size) {
  printBytes(data, size);
  printChars(data, size);
}

void sighandler(int signum) {
  printf("Process %d got signal %d\n", getpid(), signum);
  signal(signum, SIG_DFL);
  kill(getpid(), signum);
}

//+---------------+--------------------------------+-------+-------------+--------------------------------+
//| type          | name                           | type  | default     | description                    |
//+---------------+--------------------------------+-------+-------------+--------------------------------+
//| header        | Unknown Number                 | uInt  | 0x9D7FE2ABL |                                |
//|               | Data Start Offset              | uInt  |             |                                |
//|               | FileName Table Offset          | uInt  |             |                                |
//|               | Files Count                    | uInt  |             |                                |
//+---------------+--------------------------------+-------+-------------+--------------------------------+
//| file entries  | File Start Offset              | uInt  |             |                                |
//| 1..N          | File Length                    | uInt  |             |                                |
//|               | FileName Offset                | uInt  |             |                                |
//|               | File Attributes                | uInt  | 0x4L        | 1=Encrypted, >1=Unencrypted    |
//+---------------+--------------------------------+-------+-------------+--------------------------------+
//| file names    | FileName Data                  | char* |             | null terminated                |
//| 1..N          |                                |       |             |                                |
//+---------------+--------------------------------+-------+-------------+--------------------------------+
//| file contents | File Data                      | Byte* |             |                                |
//| 1..N          |                                |       |             |                                |
//+---------------+--------------------------------+-------+-------------+--------------------------------+

//filenames are encrypted
//file contents are already encrypted
//file contents do not need to be encrypted inside the pak file
void packDir(char *dir_path, char *file_path) {
  printf("Encrypting directory %s to file %s\n", dir_path, file_path);

  //determine path lengths
  uInt dir_path_len = strlen(dir_path);
  char sep = '/';
  uInt sep_len = 1;

  //open input directory
  DIR *dir;
  struct dirent *dir_ent;
  if ((dir = opendir(dir_path)) == NULL) {
    printf("Failed to open input directory '%s'\n", dir_path);
    exit(1);
  }

  //define file header
  CustomPakFileHeader hdr;
  hdr.FilesCount = 0;

  //define file names array
  CustomPakFileNameEntry *fnams = NULL;
  
  //enumerate input directory
  while ((dir_ent = readdir(dir)) != NULL) {
    //determine path lengths
    char *fname = dir_ent->d_name;
    uInt fname_len = strlen(fname) + 1;
    uInt fpath_len = dir_path_len + sep_len + fname_len;

    //allocate memory to hold full file path
    char *fpath = (char*)calloc(fpath_len, 1);
    if (fpath == NULL) {
      printf("Out of memory\n");
      exit(1);
    }

    //compose full file fpath
    sprintf(fpath, "%s%c%s", dir_path, sep, fname);

    //filter-in all entries but regular files
    if (isFile(fpath)) {
      //allocate memory to hold encrypted file name
      Byte *enc_fname = (Byte*)calloc(fname_len, 1);
      if (enc_fname == NULL) {
        printf("Out of memory\n");
        exit(1);
      }

      //encrypt file name
      ENCODE_XOR32((Byte*)fname, fname_len - 1, enc_fname, XOR_KEY);

      uLong fnam_sz = sizeof(CustomPakFileNameEntry);
  
      //resize/reallocate memory to hold file names array
      if (fnams == NULL) {
        fnams = (CustomPakFileNameEntry*)calloc(++hdr.FilesCount * fnam_sz, 1);
        if (fnams == NULL) {
          printf("Out of memory\n");
          exit(1);
        }
      } else {
        CustomPakFileNameEntry *fnams_tmp = (CustomPakFileNameEntry*)realloc(fnams, ++hdr.FilesCount * fnam_sz);
        if (fnams_tmp == NULL) {
          printf("Out of memory\n");
          exit(1);
        }
        fnams = fnams_tmp;
      }

      //allocate memory for array elements
      fnams[hdr.FilesCount - 1].FileName = (char*)calloc(fname_len, 1);
      if (fnams[hdr.FilesCount - 1].FileName == NULL) {
        printf("Out of memory\n");
        exit(1);
      }
      fnams[hdr.FilesCount - 1].EncryptedFileName = (Byte*)calloc(fname_len, 1);
      if (fnams[hdr.FilesCount - 1].EncryptedFileName == NULL) {
        printf("Out of memory\n");
        exit(1);
      }
      fnams[hdr.FilesCount - 1].FilePath = (char*)calloc(fpath_len, 1);
      if (fnams[hdr.FilesCount - 1].FilePath == NULL) {
        printf("Out of memory\n");
        exit(1);
      }

      //populate array elements
      sprintf((char*)fnams[hdr.FilesCount - 1].FileName, fname);
      sprintf((char*)fnams[hdr.FilesCount - 1].FilePath, fpath);
      memcpy(fnams[hdr.FilesCount - 1].EncryptedFileName, enc_fname, fname_len);
      fnams[hdr.FilesCount - 1].FileNameLength = fname_len;

      //release memory
      free(enc_fname);
    }

    //release memory
    free(fpath);
  }
 
  //close input directory
  closedir(dir);

  //define work variables
  uLong hdr_sz = sizeof(CustomPakFileHeader);
  uLong fent_sz = sizeof(CustomPakFileEntry);
  uLong fcon_sz = sizeof(CustomPakFileContentsEntry);

  //define header and entries structures
  CustomPakFileEntry *fents = (CustomPakFileEntry*)calloc(hdr.FilesCount * fent_sz, 1);
  if (fents == NULL) {
    printf("Out of memory\n");
    exit(1);
  }

  CustomPakFileContentsEntry *fcons = (CustomPakFileContentsEntry*)calloc(hdr.FilesCount * fcon_sz, 1);
  if (fcons == NULL) {
    printf("Out of memory\n");
    exit(1);
  }

  uLong tot_fnams_len = 0;
  uLong tot_fcons_len = 0;

  //process files
  for (int i=0; i < hdr.FilesCount; i++) {
    //open file
    FILE *file = fopen(fnams[i].FilePath, "rb");
    if (file == NULL) {
      printf("Failed to open input file '%s'\n", fnams[i].FilePath);
      exit(1);
    }

    //get file contents size
    uLong fcon_len = fsize(file);

    //allocate memory to hold file contents
    fcons[i].FileContents = (Byte*)calloc(fcon_len, 1);
    if (fcons[i].FileContents == NULL) {
      printf("Out of memory\n");
      exit(1);
    }
    
    //read file contents into memory
    fread(fcons[i].FileContents, fcon_len, 1, file);

    //close file
    fclose(file);

    //maintain entry
    fents[i].Attributes = Unencrypted;
    fents[i].FileNameOffset = tot_fnams_len;
    fents[i].Length = fcon_len;

    //maintain work variables
    tot_fnams_len += fnams[i].FileNameLength;
  }

  //populate header
  hdr.FileNameTableOffset = hdr_sz + (fent_sz * hdr.FilesCount);
  hdr.DataStartOffset = hdr.FileNameTableOffset + tot_fnams_len;
  hdr.Signature = PAK_FILE_SIGNATURE;

  //maintain file entries array - determine file contents offsets
  for (int i=0; i < hdr.FilesCount; i++) {
    fents[i].Offset = hdr.DataStartOffset + tot_fcons_len;
    //maintain work variables
    tot_fcons_len += fents[i].Length;
  }

  //open output file
  FILE *out_file = fopen(file_path, "wb");
  if (out_file == NULL) {
    printf("Failed to open output file '%s'\n", file_path);
    exit(1);
  }

  //write output buffer into output file
  fwrite(&hdr, 1, hdr_sz, out_file);
  for (int i=0; i < hdr.FilesCount; i++) {
    fwrite(&fents[i], 1, fent_sz, out_file);
  }
  for (int i=0; i < hdr.FilesCount; i++) {
    fwrite(fnams[i].EncryptedFileName, 1, fnams[i].FileNameLength, out_file);
  }
  for (int i=0; i < hdr.FilesCount; i++) {
    fwrite(fcons[i].FileContents, 1, fents[i].Length, out_file);
  }

  //close output file
  fclose(out_file);

  //free memory
  free(fnams);
}

//filenames are encrypted
//file contents are already encrypted
//file contents do not need to be encrypted inside the pak file
void unpackDir(char *file_path, char *dir_path) {
  printf("Decrypting file %s to directory %s\n", file_path, dir_path);

  //determine path lengths
  uInt dir_path_len = strlen(dir_path);
  char sep = '/';
  uInt sep_len = 1;

  //open input file
  FILE *in_file = fopen(file_path, "rb");
  if (in_file == NULL) {
    printf("Failed to open input file '%s'\n", file_path);
    exit(1);
  }

  //check input file size
  uLong in_len = fsize(in_file);
  if (in_len == 0) {
    printf("Input file is empty\n");
    exit(1);
  }

  //allocate input file buffer
  Byte *in_bytes = (Byte*)calloc(in_len, 1);
  if (in_bytes == NULL) {
    printf("Out of memory while trying to allocate %d for input file\n", in_len);
    exit(1);
  }

  //read input file into input file buffer
  fread(in_bytes, in_len, 1, in_file);

  //close input file
  fclose(in_file);

  //define work variables
  uInt uint_sz = sizeof(uInt);

  uInt in_offset = 0;

  uLong hdr_sz = sizeof(CustomPakFileHeader);
  uLong fent_sz = sizeof(CustomPakFileEntry);
  uLong fnam_sz = sizeof(CustomPakFileNameEntry);
  uLong fcon_sz = sizeof(CustomPakFileContentsEntry);
  
  //define and populate file header
  CustomPakFileHeader fhdr;
  fhdr.Signature           = (uInt&)in_bytes[in_offset]; in_offset+=uint_sz;
  fhdr.DataStartOffset     = (uInt&)in_bytes[in_offset]; in_offset+=uint_sz;
  fhdr.FileNameTableOffset = (uInt&)in_bytes[in_offset]; in_offset+=uint_sz;
  fhdr.FilesCount          = (uInt&)in_bytes[in_offset]; in_offset+=uint_sz;

  //define and allocate arrays
  CustomPakFileEntry *fents = (CustomPakFileEntry*)calloc(fhdr.FilesCount * fent_sz, 1);
  if (fents == NULL) {
    printf("Out of memory while trying to allocate %d for CustomPakFileEntry\n", fhdr.FilesCount * fent_sz);
    exit(1);
  }

  CustomPakFileNameEntry *fnams = (CustomPakFileNameEntry*)calloc(fhdr.FilesCount * fnam_sz, 1);
  if (fnams == NULL) {
    printf("Out of memory while trying to allocate %d for CustomPakFileNameEntry\n", fhdr.FilesCount * fnam_sz);
    exit(1);
  }

  CustomPakFileContentsEntry *fcons = (CustomPakFileContentsEntry*)calloc(fhdr.FilesCount * fcon_sz, 1);
  if (fcons == NULL) {
    printf("Out of memory while trying to allocate %d for CustomPakFileContentsEntry\n", fhdr.FilesCount * fcon_sz);
    exit(1);
  }

  //populate file entries
  for (int i=0; i < fhdr.FilesCount; i++) {
    memcpy(&fents[i], &in_bytes[in_offset+(i*fent_sz)], fent_sz);
  }

  //populate file names
  for (int i=0; i < fhdr.FilesCount; i++) {
    //determine filename length
    if (i < (fhdr.FilesCount - 1)) {
      fnams[i].FileNameLength = fents[i+1].FileNameOffset - fents[i].FileNameOffset;
    } else {
      fnams[i].FileNameLength = fhdr.DataStartOffset - fhdr.FileNameTableOffset - fents[i].FileNameOffset;
    }

    //determine full file path length
    uInt fpath_len = dir_path_len + sep_len + fnams[i].FileNameLength;

    //allocate memory for array elements
    fnams[i].EncryptedFileName = (Byte*)calloc(fnams[i].FileNameLength, 1);
    if (fnams[i].EncryptedFileName == NULL) {
      printf("Out of memory while trying to allocate %d for EncryptedFileName\n", fnams[i].FileNameLength);
      exit(1);
    }
    fnams[i].FileName = (char*)calloc(fnams[i].FileNameLength, 1);
    if (fnams[i].FileName == NULL) {
      printf("Out of memory while trying to allocate %d for FileName\n", fnams[i].FileNameLength);
      exit(1);
    }
    fnams[i].FilePath = (char*)calloc(fpath_len, 1);
    if (fnams[i].FilePath == NULL) {
      printf("Out of memory while trying to allocate %d for FilePath\n", fpath_len);
      exit(1);
    }

    //read encrypted file name
    memcpy(fnams[i].EncryptedFileName, (Byte*)&in_bytes[fhdr.FileNameTableOffset + fents[i].FileNameOffset], fnams[i].FileNameLength);

    //decrypt file name
    DECODE_XOR32(fnams[i].EncryptedFileName, fnams[i].FileNameLength - 1, (Byte*)fnams[i].FileName, XOR_KEY);

    //compose full file fpath
    sprintf(fnams[i].FilePath, "%s%c%s", dir_path, sep, fnams[i].FileName);
  }

  //populate file contents
  for (int i=0; i < fhdr.FilesCount; i++) {
    //allocate memory to hold file contents
    fcons[i].FileContents = (Byte*)calloc(fents[i].Length, 1);
    if (fcons[i].FileContents == NULL) {
      printf("Out of memory while trying to allocate %d for FileContents\n", fents[i].Length);
      exit(1);
    }
    //read file contents
    memcpy(fcons[i].FileContents, (Byte*)&in_bytes[fents[i].Offset], fents[i].Length);
  }

  //create/open directory
  if (mkdir(dir_path, 0755) == -1) {
    printf("Failed to create output directory or directory already exists '%s'\n", dir_path);
    exit(1);
  }

  //create files
  for (int i=0; i < fhdr.FilesCount; i++) {
    //open output file
    FILE *out_file = fopen(fnams[i].FilePath, "wb");
    if (out_file == NULL) {
      printf("Failed to open output file '%s'\n", fnams[i].FilePath);
      exit(1);
    }
    //write file contents into output file
    fwrite(fcons[i].FileContents, 1, fents[i].Length, out_file);
    //close output file
    fclose(out_file);
  }
  
  //free memory
  for (int i=0; i < fhdr.FilesCount; i++) {
    free(fcons[i].FileContents);
    free(fnams[i].FilePath);
    free(fnams[i].FileName);
    free(fnams[i].EncryptedFileName);
  }
  free(fcons);
  free(fnams);
  free(fents);
  free(in_bytes);
}

int main(int argc, char *argv[]) {
  signal(SIGSEGV, sighandler);

  //check parameters
  //argv[0] contains basename
  if (argc != 4) {
    usage();
  }

  //read parameters
  char *in_path = argv[1];
  char *out_path = argv[2];
  char *operation = argv[3];

  //check operation
  int op_encFile = (strcmp(operation, "1") == 0 ? 1 : 0);
  int op_decFile = (strcmp(operation, "2") == 0 ? 1 : 0);
  int op_packDir = (strcmp(operation, "3") == 0 ? 1 : 0);
  int op_unpackDir = (strcmp(operation, "4") == 0 ? 1 : 0);
  if (!op_encFile && !op_decFile && !op_packDir && !op_unpackDir) {
    usage();
  }

  //validate parameters
  if (   (op_encFile && (!isFile(in_path) || isDir(out_path)))
      || (op_decFile && (!isFile(in_path) || isDir(out_path)))
      || (op_packDir  && (!isDir(in_path)  || isDir(out_path)))
      || (op_unpackDir  && (!isFile(in_path) || isFile(out_path)))
  ) {
    usage();
  }
  
  //perform operations
  if        (op_encFile) {
    encodeFile(in_path, out_path);
  } else if (op_decFile) {
    decodeFile(in_path, out_path);
  } else if (op_packDir)  {
    packDir(in_path, out_path);
  } else if (op_unpackDir)  {
    unpackDir(in_path, out_path);
  }

  return 0;
}