# pinmonitor
Pin status monitoring service/daemon for Raspberry pi.

- A list of pins to be monitored has to be supplied as a .conf file. All the designated pins would be set as input and would be monitored for status changes.
- All state changes would be sent to the respective script that is specified in the .conf file along with the read value as a parameter. For E.g. sample.py 1
