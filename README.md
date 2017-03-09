## Maproom Robot Control

#### Hosts / Ports

- Robot computer
  - IP range: 192.168.7.20-29
  - 5100: receive OSC/JSON over UDP marker positions messages from skycam (in oF)
  - 5101: receive custom UDP heartbeats from robots (in oF)
  - 5201: receive OSC/JSON over UDP messages about IR camera positon (in Node ir-passthrough)
  - 8080: Socket IO passes along messages from 5201 (in Node ir-passthrough)
- Skycam (Raspberry PI)
  - IP range: 192.168.7.50-59
  - 5300: receive OSC over UDP state control messages (in Python skycam)
- Robots
  - IP range: 192.168.7.70-79
  - 5111: receive custom UDP control messages (in Arduino robot)
