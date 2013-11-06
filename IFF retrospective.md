
IFF Retrospective
=================

Jerry Morrison
2013-11-05

* In the original IFF doc, I should've listed Geoff Brown of Uhuru Sound Software in the credits and referenced his use of chunked data and 4-character-codes in Deluxe Music Construction Set in "Previous Work." He provided a lot of input and experience to IFF's design even though he wasn't part of the "Standards Committee." My screw-up. Sorry, Geoff!

* Keep it simple! The LIST/PROP feature and the chunk pad byte were useful but not worth the total cost when you consider explaining, learning, implementing the standard, and bug potential. The docs would've been better as a concise spec with a separate doc to explain its rationale and utility. The ILBM format interleaves raster bit-planes to enable incrementally reading and displaying a file. That was great (esp. at floppy disk speeds) but the example implementation was overcomplicated.

* When you push a standard, you get a lot of pushback. Developers on 8-bit microcomputers were unhappy about 8 bytes of overhead per chunk. Developers on little-endian CPUs were unhappy having to do byte-swapping for everyday I/O even though it's simpler and faster than conditional byte-swapping. So when Greg Riker -- then at Microsoft -- asked for input on storing resource files, I recommended IFF but folded too soon on pushing Microsoft to adopt big-endian chunk lengths. Hence RIFF, the "Resource IFF" aka "Reverse IFF." Was the rift avoidable?

* A sample hex dump is a good way to introduce a file format: https://groups.google.com/forum/?hl=en#!msg/comp.graphics/Jl56NJsUySE/7q7J4itrjPYJ

* IFF needed a broader way to distribute the documentation and source code, long before the web. More PR, too.

* AIFF is an application of the IFF standard although Apple didn't say that explicitly.

* PNG was a further evolution of these design ideas. Its use of upper and lower case characters in chunk IDs is clever.
