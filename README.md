 --			                                                                      
({o ~~/)

# The DODO Programming Language

DODO is a interpreted, stack-based, concatenative, homoiconic language with
manual memory management.

Inspired mainly by Forth/Joy/Factor, APL/K and Lisp/Scheme (not in that order),
its intended to be incorporated as a scripting engine in other programs.

The design requirements are:

* Very easy to embed
* Very easy to port
* No garbage collection
* Not slow, but performance can be resolved with dedicated C functions.







# Ideas

## Om like debugging

Extending Om programming language and making each word aware of the stacks state
and source code after the word, a debugging session can be thought as:

: double dup + ;
: square dup * ;
: f double square ;

debug 5 f

<0>				||	5 f
<1> 5			|| 	f
<1> 5			|| 	[ double square ]
<1> 5			|| 	[ [ dup + ] square ]
<2> 5 5		|| 	[ [ + ] square ]
<1> 10		|| 	[ square ]
<1> 10		|| 	[ [ dup * ] ]
<2> 10 10	|| 	[ [ * ] ]
<1> 100		||

What happens its very visual and understandable.

As it seems, its already done	

# The DODO programming language

DODO is a programming language that trusts the programmer.
That's its best virtue and its greatest flaw.

Inspired mainly by Forth/Joy/Factor, APL/K and Lisp/Scheme (not in that order).

The main difference between DODO and other Forth systems is that the memory
block is made up of pairs of cells and organized as a doubly linked list of
free pairs (all memory).

Contiguous array based memory can be reserved from the end of that list and
free pairs can be taken from the beginning of that list.

By using the linked list for stacks and temporary regions one block can be
reused by a lot of VMs. The intention is create a Erlang like virtual machine.

## Memory

                +---------------------------+ <-- SIZE
                |       Managed region      | <-- FREE HEAD (somewhere here)
                +---------------------------+ <-- LOWEST ASSIGNED PAIR
                |       Unused region       |
                +---------------------------+ <-- FREE TAIL
            +-> |      Reserved region      |
Data space  +   +---------------------------+ <-- HERE
            +-> |     Contiguous region     |
                +---------------------------+ <-- BLOCK ADDRESS + HEADER SIZE
                |       Block header        |
                +---------------------------+ <-- BLOCK ADDRESS

## Forth primitives

LIT<v>
ALLOT

## Lisp primitives

CONS

## Concepts I like from other programming languages

- Forth interactivity
- Forth double stacks
- Forth low level access
- APL/K human readable terse syntax
- Staple Forth human readable terse syntax
- Forth factorization
- Lisp/Scheme easyness to create abstractions
- Lisp/Scheme macros
- PicoLisp uniform memory layout
- newLisp ORO memory management
- Go channels for synchronization
- ReactiveX function composition
- Om abstractions over code
- Factor/RetroForth quotations
- Reforth inner word
- Javascript ES6 module system
- APL/K operations over arrays/list
- Forth tethered connections
- Smalltalk/Factor image based runtime

# Bytecode

		!						store												( a x -- ) 
		@						fetch												( a -- x )

		/						>r													( x -- ) ( R: -- x )
		\						r>													( -- x ) ( R: x -- )

		^						jump												( a -- )
		`						recurse											( -- ) ( R: -- a )
		#						call												( a -- ) ( R: -- a )
		;						ret													( -- ) ( R: a -- )
		#;					tail call (jump)						( a -- ) ( -- )
		R^					relative address jump				( a -- )
		R#					relative address call				( a -- ) ( R: -- a (absolute) )
		R#;					relative address tail call  ( a -- ) ( -- )

		_						drop												( x -- )
		d						dup													( x -- x x )
		s						swap												( x y -- y x )
		o						over												( x y -- x y x )
		r						rot													( x y z -- z x y )
		s_					nip	[ swap drop ]						( x y -- y )
		so					tuck [ swap over ]					( x y -- y x y )

		__					2drop [ drop drop ]					( x y -- )
		oo					2dup [ over over ]					( x y -- x y x y )
		r/r\				2swap	[ rot >r rot r> ]			( u v x y -- x y u v )
								2over												
								2rot

		n						literal n (0-9)							( -- n )
		0n-					negative literal n (-1-9)		( -- n )

		Bn					byte literal								( -- n )
		Wnn					word literal								( -- n )
		Dnnnn				dword literal								( -- n )
		Qnnnnnnnn		qword literal								( -- n )

		Bn^					address literals (with jump/call)
		Bn#
		Wnn^
		Wnn#
		Dnnnn^
		Dnnnn#
		Qnnnnnnnn^
		Qnnnnnnnn#

		+						add													( x y -- x+y )
		-						sub													( x y -- x-y )
		*						mult												( x y -- x*y )
		/						div													( x y -- x/y )
		%						modulo											( x y -- x mod y )

		>						gt													( x y -- x>y )
		=						eq													( x y -- x=y )
		<						lt													( x y -- x<y )
		>=					gt or eq										( x y -- x>=y )
		<=					lt or eq										( x y -- x<=y )

		&						and													( x y -- x&y )
		|						or													( x y -- x|y )
		~						not													( x -- ~x )

		?..(..)			if else then								( x -- )
		~?..(..)		if not else then						( x -- )
		[.....]			loop
								?indexed loops

		,						key													( -- x )
		.						emit												( x -- )


