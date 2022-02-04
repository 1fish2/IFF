"64-bit IFF"
============

-- Jerry Morrison

Yes, once upon a time I did design ways to expand the file and chunk size to 64 bits and larger.

It didn't go beyond a design sketch, in part because gigabyte files are already big for a sequential read/write format, at least on 2003 era hardware. It may be handier to use multiple files or an editable format like Bento. Witness the hacks people used to update RIFF properties (like user ratings) without copying the entire file.

Note that the IFF standard defines a **SIGNED** 32-bit chunk-size field. It's signed for compatibility with Pascal (early Macintosh programs) and Java follows suit. (Unsigned numbers have potential issues when you subtract them.) Some developers have expanded the usage to an UNSIGNED 32-bit chunk-size, but that's not to the original specification.


## Design Proposal 1 [Not adopted]

Since negative chunk sizes are technically illegal in IFF, it's theoretically safe to expand the range to *nearly* a full 32-bit unsigned value and specify that the last few values indicate a variable-size, extended size field. For developers who are already treating the chunk size as unsigned 32-bits, this would make their practice legal except for those last few values.

Given the value ```n``` in the original chunk size field:

* ```signed n = -1``` is kept as a temporary placeholder value when writers plan to back-patch the size after writing the chunk's contents.
* ```signed n = -2``` indicates an 8 byte (64-bit) unsigned chunk size field immediately following the original chunk size field. This supports chunk sizes in ```[0 .. 2^64 - 1]``` bytes.
* ```signed n = -3``` indicates a 16 byte (128-bit) unsigned chunk size field immediately following the original chunk size field. This supports chunk sizes in ```[0 .. 2^128 - 1]``` bytes.
* ```signed n in [-15 .. -4]``` values are reserved.
* ```unsigned n in [0 .. 2^32 - 16]``` indicates ```n``` bytes of chunk data.

This allows more than one field width to store a given chunk size value but for backwards compatibility, writers would be encouraged to use narrowest chunk size field that fits. A writer that back-patches the size might still have to reserve a longer field just in case.

A reader should give up on any file whose first chunk's size is too big for that reader. Check the effective chunk size value, **not** its field width.


## Design Proposal 2 [Not adopted]

Use new "IFF-2003" magic signatures ```'FOR2'```, ```'LIS2'```, ```'PRO2'```, and ```'CAT2'``` to indicate that those chunks and their contained chunks have unsigned 64-bit fixed-size size fields. (```'FOR1'``` is used by the BEAM file format.)

An IFF-2003 spec could also drop the pad byte that follows each odd-size IFF-85 chunk and maybe the ```'LIST'``` and ```'PROP'``` features.

For backwards compatibility, writers would be encouraged to use IFF-85 when feasible. IFF-85 readers will ignore IFF-2003 files.


## Philips DSDIFF

Philips created a 64-bit IFF derivative, the [Direct Stream Digital Interchange File Format: DSDIFF](https://www.sonicstudio.com/pdf/dsd/DSDIFF_1.5_Spec.pdf). It's identified via the `FRM8` tag and it keeps the 16-bit chunk padding.

Also see their document [Recommended usage of DSDIFF version 1.5](https://www.sonicstudio.com/pdf/dsd/DSDIFF_1.5_RecommendedUsage.pdf).


## Notes

* ["2^16-1 limit in modeler"](https://groups.google.com/forum/?hl=en#!msg/comp.graphics.apps.lightwave/Zz8HEy-eDfM/fQKtAIE1Qg8J) thread on comp.graphics.apps.lightwave.
