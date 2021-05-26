import serial 
import time 

while (True):
    ser = serial.Serial(port='/dev/cu.usbmodem14301', baudrate=9600, timeout=1)
    num = input("Enter a number (0 - 255): ")
    strNum = str.encode(num)

    print("Sending...", strNum)
    ser.write(strNum)
    time.sleep(0.05)

    echoLine = ser.readline()
    print(echoLine)
    print()