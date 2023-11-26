ascii 4 "Signature"
set symlen [uint16]
entry "Symbol Listing Length" $symlen
uint32 "Symbol Count"
hex 2 "Signature"
uint32 "Frames?"
section "Symbols" {
	set i 0
	while {[pos] < $symlen + 0x10} {
		cstr "ascii" $i
		incr i
	}
}