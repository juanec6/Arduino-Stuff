import serial
import time

arduino = serial.Serial(port="COM6", baudrate=9600)
time.sleep(2)

last_time = time.time()

while True:
    raw = arduino.readline()
    line = raw.decode().strip()

    # imprime cada 2 segundos
    if time.time() - last_time >= 2:
        print(line)
        last_time = time.time()
