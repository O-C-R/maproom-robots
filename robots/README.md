# St Louis Map Room Robots

#### Message Protocol

All messages prefixed with `MR` (map room)

rotate(desired_angle, measured_angle)
`MRROT+00000+00000`

move(heading_angle, magnitude)
`MRMOV+00000+00000`

calibrate(current_angle)
`MRCAL+00000`

draw(heading_angle, magnitude)
`MRDRW+00000+00000`

stop()
`MRSTP`


