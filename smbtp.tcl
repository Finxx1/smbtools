set filesizes(0) 0
set filecount [uint32]

section "Header" {
	entry "File Count" $filecount

	for {set i 0} {$i < $filecount} {incr i} {
		section $i {
			uint32 "Offset"
			set filesizes($i) [uint32]
			entry "File Size" $filesizes($i)
			uint64 "???"
			uint8  "???"
		}
	}
}

section "File Data" {
	for {set i 0} {$i < $filecount} {incr i} {
		bytes $filesizes($i) $i
	}
}