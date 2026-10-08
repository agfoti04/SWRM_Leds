# Uses PySerial
import serial

# Serial information
serial_port = 'dev/USB0'
baud_rate = 115200
write_to_file_path = "encoder_data.txt"

# Write to an output file
output_file = open(write_to_file_path, "w+")
ser = serial.Serial(serial_port, baud_rate)
while True:
    line = ser.readline()
    line = line.decode("utf-8")
    output_file.write(line)
