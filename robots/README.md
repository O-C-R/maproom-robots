# St Louis Map Room Robots

### Dependencies
- Arduino Mega
- MiniPID: https://github.com/tekdemo/MiniPID
- I2C Master Library: https://github.com/DSSCircuits/I2C-Master-Library/

### Message Protocol

#### Coordinator -> Robot

All messages prefixed with `MR` (map room)

rotate(desired_angle, measured_angle)
`MRROT+00000+00000`

move(heading_angle, magnitude)
`MRMOV+00000+00000`

calibrate(current_angle)
`MRCAL+00000`

draw(heading_angle, magnitude)
`MRDRW+00000+00000`

stop_pen_down()
`MRSTP1`

stop_pen_up()
`MRSTP0`

#### Robot -> Coordinator

heartbeat()
`RB{ID}HB` (e.g. RB01HB)
