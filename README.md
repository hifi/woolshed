Woolshed
========
a naive attempt to emulate Classic MacOS runtime

*NOTE*: This is a hacky experiment and is by no means pretty.

Caveats
-------
 - This is an ugly hack that hasn't been completely refactored
 - 64MB of RAM is allocated for the process and there's no real memory management
 - There's very little sanity checks anywhere so it will cause buffer overflows
 - It only runs on Little Endian systems but it should be easy to fix it
 - It only builds on 64-bit Linux so far, I guess

Quick Start
-----------
Depends on Qt5.

```
$ make
$ ./woolshed run examples/hello
```

You can see quite a bit of information about the PEF image with:
```
$ ./woolshed dump examples/hello
```

Hacking
-------
Most of the runtime initialization code is in src/run.c and in src/pef.c except
the CPU emulation.

System libraries are in lib/ and all library function calls take the CPU state
as their sole argument.

Acks
----
Resources used to write this hack:

 - [SheepShaver][1] BeOS PPC CPU emulation code
 - [Mac OS Runtime Architectures (book)][2] for PEF reference
 - GNU binutils for PowerPC to disassemble code with objdump
 - Macintosh Programmer's Workshop to compile examples
 - Original [Apple Developer Connection][3] API reference

[1]: https://sheepshaver.cebix.net/
[2]: https://web.archive.org/web/20020202081513/http://developer.apple.com:80/techpubs/mac/runtimehtml/RTArch-2.html
[3]: http://mirror.informatimago.com/next/developer.apple.com/index.html
