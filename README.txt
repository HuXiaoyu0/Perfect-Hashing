# Perfect-Hashing
data structure, homework 9
Use perfect hashing to compress or uncompress imagest.
Compress:  49x44 pixels 24 bits/pixel image(6481 bytes) -> occupancy + hash data + offset
Uncompress: vice versa.
Example command line to perform uncompression:
hw9.exe uncompress occupancy.pbm data.ppm offset.offset output.ppm
Uncompressed the data with this command:
hw9.exe compare input1.ppm input2.ppm output.pbm
The input file includes:
chair.ppm lightbulb.ppm car_original.ppm
or: 
car_hash_data.ppm + car_occupancy + car_offset.offset
To download:
http://www.cs.rpi.edu/academics/courses/fall14/csci1200/hw/09_perfect_hashing/provided_files.zip
For more information:
http://www.cs.rpi.edu/academics/courses/fall14/csci1200/hw/09_perfect_hashing/hw.pdf


 
