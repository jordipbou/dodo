[UNDEFINED] INTFIELD: [IF]
\ FIXME A different/clearer implementation could be done when
\ specific memory words that deal with different sizes are
\ implemented.
: INTFIELD: ( n1 "name" -- n2 ; addr1 -- addr2 )
	1 INTS + 1- 1 INTS 1- INVERT AND \ int based align
	1 INTS +FIELD
;
[THEN]
