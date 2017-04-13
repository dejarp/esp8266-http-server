Link to source for python tool for working with elf images and flashing them via serial.
https://github.com/espressif/esptool/blob/master/esptool.py

When running make flash in eclipse, the esptool.py script outputs a bunch of backspace characters 0x08 to the console in order to erase the prior line when print percentage complete, but exclipse just doesn't handle this and there has been an outstanding bug since 2004 https://bugs.eclipse.org/bugs/show_bug.cgi?id=76936


