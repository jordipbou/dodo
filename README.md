 --			                                                                      
({o ~~/)

Based on ideas from Forth, APL/K, Lisp and Joy/Factor.

The requirements for Dodo are:

* C VM easily embedded on other applications.
* Interpreted for great interactivity, but must allow adding compiled functions.
* VM can not be fast, but must not add too much overhead to compiled functions.

# Data structures

There are 3 basic data structures:

	· BYTE -- 8 bits
	· CELL -- 16, 32 or 64 bits (depends on target architecture)
	· PAIR -- 2 cells
		· NEXT	-- PAIR * (last 2 bits are used to store typing information)
		· VALUE -- CELL
						+- PAIR * (last 2 bits can be used to store subtyping information)

# Types

Although DODO grows from an untyped Forth based VM, typing is required to 
allow working with lists that may contain other lists or scalars.

As every PAIR pointer will be aligned to a 32 bit address (minimum for a 16
bit CELL architecture, where a PAIR will be 32 bit wide) that leaves 2 bits
free to store typing information.

The 4 types defined are:

- Atom
- List (allows subtypes by using the 2 bits of the value/reference)
- Primitive word (defined in C)
- Colon definition





# Memory model

Memory is divided in four regions: header, contiguous, transient and managed.

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

	+-------+-------+			 +-------+-------+
	|   | W +   ---------->|   | L + NFA@ -------> name counted byte array
	+---|---+-------+      +---|---+-------+
	                           v
												 +-------+-------+
												 |   | L |  XT@ -------> list representing interpretation semantics
												 +---|---+-------+
												     v?
												 +-------+-------+
												 |   0   | NDCS@ ------> list representing compilation semantics
												 +-------+-------+



