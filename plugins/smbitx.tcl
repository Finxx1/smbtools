little_endian

set l [len]

for {set i 0} {$i < $l / 20} {incr i} {
	section $i {
		float x1
		float y1
		float x2
		float y2
		float rotation
	}
}