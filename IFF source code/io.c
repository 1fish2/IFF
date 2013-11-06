/* @(#) io.c 96/04/19 1.5 */

#include <kernel/types.h>
#include <kernel/mem.h>
#include <file/fileio.h>
#include <misc/iff.h>
#include <string.h>
#include "io.h"


/*****************************************************************************/


static Err FileOpen(RawFile **file, const char *key, bool writeMode)
{
FileOpenModes mode;

    if (writeMode)
        mode = FILEOPEN_WRITE_NEW;
    else
        mode = FILEOPEN_READ;

    return OpenRawFile(file, key, mode);
}


/*****************************************************************************/


static Err FileClose(RawFile *file)
{
    return CloseRawFile(file);
}


/*****************************************************************************/


static int32 FileRead(RawFile *file, void *buffer, uint32 numBytes)
{
    return ReadRawFile(file, buffer, numBytes);
}


/*****************************************************************************/


static int32 FileWrite(RawFile *file, const void *buffer, uint32 numBytes)
{
    return WriteRawFile(file, buffer, numBytes);
}


/*****************************************************************************/


static int32 FileSeek(RawFile *file, int32 position)
{
    return SeekRawFile(file, position, FILESEEK_CURRENT);
}


/*****************************************************************************/


const IFFIOFuncs fileFuncs =
                       {(IFFOpenFunc)  FileOpen,
                        (IFFCloseFunc) FileClose,
                        (IFFReadFunc)  FileRead,
                        (IFFWriteFunc) FileWrite,
                        (IFFSeekFunc)  FileSeek};
