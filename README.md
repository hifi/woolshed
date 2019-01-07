peftool
=======
a poor attempt to emulate Classic MacOS applications

*NOTE*: This is a hacky experiment and is by no means pretty.

Caveats
-------
 - This is an ugly hack that hasn't been refactored, it can barely run "Hello World"
 - 64MB of RAM is allocated for the process and there's no real memory management
 - There's very little sanity checks anywhere so it will cause buffer overflows
 - It only runs on Little Endian systems (because byte swapping is incomplete)
 - It only builds on 64-bit Linux so far, I guess

Quick Start
-----------
```
$ make
$ ./peftool run examples/hello
```

You can see quite a bit of information about the PEF image with:
```
$ ./peftool dump examples/hello
```

Hacking
-------
Most of the runtime initialization code is in src/run.c and in src/pef.c except
the CPU emulation.

System libraries are in lib/ and all library function calls take the CPU state
as their sole argument. This may need to be refactored into something more clean
before going too deep.

Acks
----
Resources used to write this hack:

 - [SheepShaver][1] BeOS PPC CPU emulation code
 - [Mac OS Runtime Architectures (book)][2] for PEF reference
 - GNU binutils for PowerPC to disassemble code with objdump
 - Macintosh Programmer's Workshop to compile examples/hello

[1]: https://sheepshaver.cebix.net/
[2]: https://web.archive.org/web/20020202081513/http://developer.apple.com:80/techpubs/mac/runtimehtml/RTArch-2.html
