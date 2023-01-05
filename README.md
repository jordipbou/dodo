 --			                                                                      
({o ~~/)

Based on ideas from Forth, APL/K, Lisp and Joy.

# Data structures

There are 3 basic data structures:

* BYTE -- 8 bits
* CELL -- 16, 32 or 64 bits (depends on target architecture)
* PAIR -- 2 cells
	* NEXT	-- PAIR * (last 2 bits are used to store typing information)
	* VALUE -- CELL

# Typing

Although DODO grows from an untyped Forth based VM, typing is required to 
allow working with lists that may contain other lists or scalars.

As every PAIR pointer will be aligned to a 32 bit address (minimum for a 16
bit CELL architecture, where a PAIR will be 32 bit wide) that leaves 2 bits
free to store typing information.

Those 2 bits will have different meanings depending on where are used, but
they are used to, at least, indicate the following information:

- Free pair	(this will always be and will be the only one using 00)
- Number (scalar)
- eXecution Token
- List
- Primitive word (defined in C)
- Colon definition
- Immediate word






# Memory model

Memory is divided in three regions: header, contiguous and managed.

## Header

Stores VM info:

* err -- CELL error code

## Node based memory region

Region is divided in blocks of two cells. Initially, they are all connected
as a linked list of free nodes. As they are being used (on the stack or
anywhere) there are taken from that list. Used nodes can be returned back
to the list.

## Contiguous region

As in Forth. 

# Words


