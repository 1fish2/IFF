"64-bit IFF"
============

-- Jerry Morrison

Yes, once upon a time I did design a way (actually two, see below) to expand the file and chunk size.

[ https://groups.google.com/forum/?hl=en#!msg/comp.graphics.apps.lightwave/Zz8HEy-eDfM/fQKtAIE1Qg8J ]

It didn't go beyond a design sketch, in part because gigabyte files are already too big for a linear read/write format. At that size it's smarter to use multiple files or an editable format like Bento. Witness the hacks people used to update RIFF properties (like user ratings) without copying the entire file.

BTW the IFF standard defines a **SIGNED** 32-bit chunk-size field. It's signed for compatibility with Pascal (early Macintosh programs), and that carries over to Java. (Unsigned numbers sound appealing for array indexes until you subtract or decrement them.) Developers may well have expanded this to an UNSIGNED 32-bit chunk-size, but that's non-standard.


## Alternative Proposal 1

Since negative chunk sizes are illegal in IFF, it's technically safe to expand the range to *nearly* a full 32-bit unsigned value and specify that the last few values introduce a variable-size, extended size field. (If some developers are already already treating the chunk size field as unsigned 32-bits, this'd accept their practice as legal except for those last few values.)

Given the value ```n``` in the original chunk size field:

* ```signed n = -1``` is used as a placeholder value when writers need to back-patch the size value after writing the chunk's contents.
* ```signed n = -2``` indicates an 8 byte (64-bit) unsigned chunk size field immediately follows the original chunk size field. This supports chunk sizes in ```[0 .. 2^64 - 1]``` bytes.
* ```signed n = -3``` indicates a 16 byte (128-bit) unsigned chunk size field immediately follows the original chunk size field. This supports chunk sizes in ```[0 .. 2^128 - 1]``` bytes.
* ```signed n in [-15 .. -4]``` values are reserved.
* ```unsigned n in [0 .. 2^32 - 16]``` indicates ```n``` bytes of chunk data.

This allows more than one width to store a given chunk size value but for backwards compatibility, writers are encouraged to use narrowest chunk size field that fits. Still, a one-pass writer may have to reserve a longer field just in case.

A reader should give up on any file whose first chunk's size is too big for that reader. Check the effective chunk size value, **not** its field width.


## Alternative Proposal 2

Use new "IFF-2003" magic signatures ```'FOR2'```, ```'LIS2'```, ```'PRO2'```, and ```'CAT2'``` to indicate that those chunks and their contained chunks have unsigned 64-bit fixed-size size fields. (```'FOR1'``` is used by the BEAM file format.)

An IFF-2003 spec could also drop the pad byte that follows each odd-size IFF-85 chunk and maybe the ```'LIST'``` and ```'PROP'``` features, too.

For backwards compatibility, writers are encouraged to use IFF-85 when feasible. Of course IFF-85 readers will ignore IFF-2003 files.
