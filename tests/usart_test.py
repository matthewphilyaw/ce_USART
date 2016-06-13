import serial
import argparse

def over_run():
    print "over run test started"
    port = serial.Serial(args.device, baudrate=args.baudrate, timeout=5)

    for i in range(0, 1000):
        port.write("e")

    print "blasting don. Listening to device"
    while(True):
        port.read(1)

def frame_error():
    print "frame error test started"

    for i in range(0, 50):
        print "baud: " + str(args.baudrate)
        port = serial.Serial(args.device, baudrate=args.baudrate, timeout=1)
        port.write("error")

    for i in range(0, 50):
        new_baud = args.baudrate - 100
        print "next baud: " + str(new_baud)
        port = serial.Serial(args.device, baudrate=new_baud, timeout=1)
        port.write("error")

    print "Done"

parser = argparse.ArgumentParser(description='USART tester')

parser.add_argument('-over_run', action='store_true', default=False)
parser.add_argument('-frame_error', action='store_true', default=False)
parser.add_argument('baudrate', type=int)
parser.add_argument('device', type=str)

args = parser.parse_args()

if args.over_run:
    over_run()
elif args.frame_error:
    frame_error()
else:
    parser.print_help()
