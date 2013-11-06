"64-bit IFF"
============

-- Jerry Morrison

Yes, I did design a way (actually two, see below) to expand the file and chunk size. It didn't go further, though, so if you have a burning need for it, speak up!

[ https://groups.google.com/forum/?hl=en#!msg/comp.graphics.apps.lightwave/Zz8HEy-eDfM/fQKtAIE1Qg8J ]

The IFF standard defines a SIGNED 32-bit chunk-length. The length field is signed for Pascal compatibility (early Macintosh programs) and Pascal's reasons carried over to Java. (Unsigned numbers sound appealing for array indexes until you subtract or decrement them.)

With storage space doubling every year, a 2^31 byte maximum file size predictably becomes an issue but does it make more sense to use multiple files?


## Alternative 1

Since negative lengths are illegal in IFF, it's technically safe to specify use a few extreme values to introduce a variable-length length field. The remaining values can simply expand the range to nearly a full 32-bit unsigned value. (If some developers already are already treating the chunk size as an unsigned 32-bit value, this'd mostly match what they're doing.)

Given chunk size n:

* ```n = -1``` is used as a placeholder value when writers need to back-patch it after writing the chunk's contents.
* ```n = -2``` indicates an 8 byte (64-bit) unsigned chunk size immediately follows, supporting chunk lengths in [0 .. 2^64) bytes.
* ```n = -3``` indicates a 16 byte (128-bit) unsigned chunk size immediately follows, supporting chunk lengths in [0 .. 2^128) bytes.
* ```n in [-15 .. -4]``` values are reserved.
* ```|n| in [0 .. 2^32 - 16]``` indicates n bytes of chunk data.

Writers are encouraged to use the shorter fields for backwards compatibility. Still, a one-pass writer may have to reserve a longer field just in case.

A reader should give up on any file whose initial chunk length is too big for it. Test the chunk length value, *not* the length of the length field. Don't choke on leading zeros.


## Alternative 2

Just use new "IFF-2003" magic signatures ```'FOR2'```, ```'LIS2'```, ```'PRO2'```, and ```'CAT2'``` to indicate that those chunks and their contained chunks have unsigned 64-bit length fields. (```'FOR1'``` is used by the BEAM file format.)

IFF-2003 could drop the pad byte that follows each odd-length IFF-85 chunk and the ```'LIST'``` and ```'PROP'``` features, too.

For backwards compatibility, writers are encouraged to use IFF-85 when feasible. Of course IFF-85 readers will ignore IFF-2003 files.
