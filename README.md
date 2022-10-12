 --													
({o ~~/)

# The DODO virtual machine and programming language

## Memory layout

+------+------+------------------------+
| Cell | Item | Content                |
+------+------+------------------------+
|  0   |  0   | Block size             |
|  1   |      | First free / Last free |
|  2   |  1   | Last free item         |
|  3   |      |                        |
| ....



## Where it comes from?

## Ideas

- Simpler first
- The programmer knows what he/she wants
- The programmer must be able to protect him/herself




# Ideas from other programming languages

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


