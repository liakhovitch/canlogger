import serial
import time
import csv
import struct
import os

# this port address is for the serial tx/rx pins on the GPIO header
SERIAL_PORT = 'COM7'
# be sure to set this to the same rate used on the Arduino
# test comment
SERIAL_RATE = 9600


def can_1():
    ser = serial.Serial(SERIAL_PORT, SERIAL_RATE)
    ser.write(b'read can1\n')
    # print(struct.unpack('<l', ser.read(4)))
    # length, = (struct.unpack('<i', ser.read(4)))
    #x = length.decode('utf-8')
    # print(x)
    length = (int(ser.read(4), 2))
    print(length)

    reading = ser.read(100).decode('utf-8')
    print(repr(reading))
    print("after")

    start = 0
    slice = length
    with open("can_1.csv", "a") as f:
        while slice < len(reading):
            writer = csv.writer(f, delimiter=",")
            writer.writerow([time.time(), reading[start:slice]])
            start += length
            slice += length
    os.chmod("test_data.csv", 0o777)


def can_2():
    ser = serial.Serial(SERIAL_PORT, SERIAL_RATE)
    ser.write(b'read can2\n')
    # print(struct.unpack('i', ser.read(4)))
    # length, = (struct.unpack('i', ser.read(4)))
    length = (int(ser.read(4), 2))
    print(length)

    reading = ser.read(100).decode('utf-8')
    print(repr(reading))
    print("after")

    start = 0
    slice = length
    with open("can_2.csv", "a") as f:
        while slice < len(reading):
            writer = csv.writer(f, delimiter=",")
            writer.writerow([time.time(), reading[start:slice]])
            start += length
            slice += length
    os.chmod("test_data.csv", 0o777)


def delete_can1():
    ser = serial.Serial(SERIAL_PORT, SERIAL_RATE)
    ser.write(b'delete can1\n')
    ans = ser.readline().decode('utf-8')
    if ans == 'ok\n':
        print("success, can 1 data file deleted")


def delete_can2():
    ser = serial.Serial(SERIAL_PORT, SERIAL_RATE)
    ser.write(b'delete can2\n')
    ans = ser.readline().decode('utf-8')
    if ans == 'ok\n':
        print("success, can 2 data file deleted")


def set_baud():
    ser = serial.Serial(SERIAL_PORT, SERIAL_RATE)
    print("Which can? ( 1 or 2")
    num = input()
    print("What baud rate?")
    rate = input()

    message = "baud can" + num + " " + rate + "\n"
    ser.write(message.encode('utf-8'))

    ans = ser.readline().decode('utf-8')
    if ans == 'ok\n':
        print("success")


def main():
    ans = True
    while ans:
        print("""
        1. Retrieve Can 1 Data
        2. Retrieve Can 2 Data
        3. Delete CAn 1 Data File
        4. Delete CAn 1 Data File
        5. Set Baud Rate
        6. Exit/Quit
        """)
        ans = input("What would you like to do? ")
        if ans == "1":
            can_1()
        elif ans == "2":
            can_2()
        elif ans == "3":
            delete_can1()
        elif ans == "4":
            delete_can2()
        elif ans == "5":
            set_baud()
        elif ans == "6":
            return 0
        elif ans != "":
            print("\n Not Valid Choice Try again")


if __name__ == "__main__":
    main()
