"64-bit IFF"
============

-- Jerry Morrison

Yes, I did design a way (actually two, see below) to expand the file and chunk size. It didn't go further, though, so if you have a burning need for it, speak up!

[ https://groups.google.com/forum/?hl=en#!msg/comp.graphics.apps.lightwave/Zz8HEy-eDfM/fQKtAIE1Qg8J ]

The IFF standard defines a **SIGNED** 32-bit chunk-size field. It's signed for compatibility with Pascal (early Macintosh programs), and that carries over to Java. (Unsigned numbers sound appealing for array indexes until you subtract or decrement them.)

With storage space doubling every year, a 2^31 byte maximum data size predictably becomes an issue but does it make more sense to use multiple files?


## Alternative Proposal 1

Since negative chunk sizes are illegal in IFF, it's technically safe to specify that the last few values introduce a variable-size size field. The remaining values can expand the range to *nearly* a full 32-bit unsigned value. (If some developers already are already treating the chunk size field as unsigned 32-bits, this'd mostly match what they're doing.)

Given the value ```n``` in the original chunk size field:

* ```signed n = -1``` is used as a placeholder value when writers need to back-patch the size value after writing the chunk's contents.
* ```signed n = -2``` indicates an 8 byte (64-bit) unsigned chunk size field immediately follows the original chunk size field. This supports chunk sizes in ```[0 .. 2^64 - 1]``` bytes.
* ```signed n = -3``` indicates a 16 byte (128-bit) unsigned chunk size field immediately follows the original chunk size field. This supports chunk sizes in ```[0 .. 2^128 - 1]``` bytes.
* ```signed n in [-15 .. -4]``` values are reserved.
* ```unsigned n in [0 .. 2^32 - 16]``` indicates ```n``` bytes of chunk data.

Writers are encouraged to use shorter chunk size fields for backwards compatibility. Still, a one-pass writer may have to reserve a longer field just in case.

A reader should give up on any file whose first chunk's size is too big for the reader. Test the resulting chunk size value, **not** the size of the size field.


## Alternative Proposal 2

Use new "IFF-2003" magic signatures ```'FOR2'```, ```'LIS2'```, ```'PRO2'```, and ```'CAT2'``` to indicate that those chunks and their contained chunks have unsigned 64-bit fixed-size size fields. (```'FOR1'``` is used by the BEAM file format.)

A IFF-2003 spec could also drop the pad byte that follows each odd-size IFF-85 chunk and the ```'LIST'``` and ```'PROP'``` features, too.

For backwards compatibility, writers are encouraged to use IFF-85 when feasible. Of course IFF-85 readers will ignore IFF-2003 files.
