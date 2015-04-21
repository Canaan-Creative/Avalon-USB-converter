Avalon-USB-Converter
====================
Avalon-USB-Converter is an USB2IIC & USB2UART converter based on LPC USB Serial I/O Library.

It converts all API calls into USB messages which are transferred to LPC11u14,which in turn communicates with serial devices using I2C and UART interfaces. 

It uses USB-CDC class as transport mechanism.

Directory structure
===================
* `firmware`: Avalon-USB-Converter firmware
* `loopbackdemo`: CDC loopback testing

IDE Support
=============
* LPCXpresso 6 or above (Free Edition)


Discussion
==========
* IRC: #avalon @freenode.net
* Mailing list: http://lists.canaan-creative.com/

License
=======
This is free and unencumbered public domain software. For more information,
see http://unlicense.org/ or the accompanying UNLICENSE file.

Files under verilog/superkdf9/ have their own license (Lattice Semi-
conductor Corporation Open Source License Agreement).

Some files may have their own license disclaim by the author.
