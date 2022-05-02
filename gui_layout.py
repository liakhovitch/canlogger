from client_software import *
from tkinter import *

root = Tk()


def can1Click():
    can_1()
    myLabel = Label(root, text="Success!",font=('Comic Sans MS', 12, 'bold italic'))
    myLabel.pack()

def can2Click():
    can_2()
    myLabel = Label(root, text="Success!",font=('Comic Sans MS', 12, 'bold italic'))
    myLabel.pack()

def del_can_1():
    delete_can1()
    myLabel = Label(root, text="can1 ~should~ be deleted",fg="#FF00FF", font=('Comic Sans MS', 12, 'bold italic'))
    myLabel.pack()

def del_can_2():
    delete_can2()
    myLabel = Label(root, text="can2 ~should~ be deleted",font=('Comic Sans MS', 12, 'bold italic'))
    myLabel.pack()


def baud_click():
    myLabel = Label(root, text="which CAN which would you like to select?",font=('Comic Sans MS', 12, 'bold italic'))
    myLabel.pack()
    pick_can()


def pick_can():
    pick_can1 = Button(text="Can1", command=pick_rate1, fg="blue",font=('Comic Sans MS', 12, 'bold italic'))
    pick_can1.pack()
    pick_can2 = Button(text="Can2", command=pick_rate2, fg="blue",font=('Comic Sans MS', 12, 'bold italic'))
    pick_can2.pack()


def pick_rate1():
    myLabel = Label(root, text="Select Baud Rate",font=('Comic Sans MS', 12, 'bold italic'))
    myLabel.pack()

    pick_can1b250 = Button(text="250k", command=pick1_baud250, fg="blue",font=('Comic Sans MS', 12, 'bold italic'))
    pick_can1b250.pack()

    pick_can1b500 = Button(text="500k", command=pick1_baud500, fg="blue",font=('Comic Sans MS', 12, 'bold italic'))
    pick_can1b500.pack()

    pick_can1b1000 = Button(text="1000k", command=pick1_baud1000, fg="blue",font=('Comic Sans MS', 12, 'bold italic'))
    pick_can1b1000.pack()


def pick_rate2():
    myLabel = Label(root, text="Select Baud Rate", font=('Comic Sans MS', 12, 'bold italic'))
    myLabel.pack()

    pick_can2b250 = Button(text="250k", command=pick2_baud250, fg="blue", font=('Comic Sans MS', 12, 'bold italic'))
    pick_can2b250.pack()

    pick_can2b500 = Button(text="500k", command=pick2_baud500, fg="blue", font=('Comic Sans MS', 12, 'bold italic'))
    pick_can2b500.pack()

    pick_can2b1000 = Button(text="1000k", command=pick2_baud1000, fg="blue", font=('Comic Sans MS', 12, 'bold italic'))
    pick_can2b1000.pack()


def pick2_baud250():
    ser = serial.Serial(SERIAL_PORT, SERIAL_RATE)
    message = "baud can2" + " " + "250k" + "\n"
    ser.write(message.encode('ascii'))

    ans = ser.readline().decode('ascii')
    if ans == 'ok\n':
        print("success, set can 2 to 250k!!")

def pick1_baud250():
    ser = serial.Serial(SERIAL_PORT, SERIAL_RATE)
    message = "baud can1" + " " + "250k" + "\n"
    ser.write(message.encode('ascii'))

    ans = ser.readline().decode('ascii')
    if ans == 'ok\n':
        print("success, set can 1 to 250k!!")

def pick1_baud500():
    ser = serial.Serial(SERIAL_PORT, SERIAL_RATE)
    message = "baud can1" + " " + "500k" + "\n"
    ser.write(message.encode('ascii'))

    ans = ser.readline().decode('ascii')
    if ans == 'ok\n':
        print("success, set can 1 to 500k!!")

def pick2_baud500():
    ser = serial.Serial(SERIAL_PORT, SERIAL_RATE)
    message = "baud can2" + " " + "500k" + "\n"
    ser.write(message.encode('ascii'))

    ans = ser.readline().decode('ascii')
    if ans == 'ok\n':
        print("success, set can 2 to 500k!!")


def pick1_baud1000():
    ser = serial.Serial(SERIAL_PORT, SERIAL_RATE)
    message = "baud can1" + " " + "1000k" + "\n"
    ser.write(message.encode('ascii'))

    ans = ser.readline().decode('ascii')
    if ans == 'ok\n':
        print("success, set can 1 to 500k!!")


def pick2_baud1000():
    ser = serial.Serial(SERIAL_PORT, SERIAL_RATE)
    message = "baud can2" + " " + "1000k" + "\n"
    ser.write(message.encode('ascii'))

    ans = ser.readline().decode('ascii')
    if ans == 'ok\n':
        print("success,set can 2 to 1000k!!")

def myClick():
    myLabel = Label(root, text="Success!")
    myLabel.pack()


can_1_button = Button(root, text="Retrieve Can1", command=can1Click, fg="red", font=('Comic Sans MS', 12, 'bold italic'))
can_1_button.pack()
can_2_button = Button(root, text="Retrieve Can2", command=can2Click, fg="orange",font=('Comic Sans MS', 12, 'bold italic'))
can_2_button.pack()
delete_1_button = Button(root, text="Delete Can1", command=del_can_1, fg="#FAED27", font=('Comic Sans MS', 12, 'bold italic'))
delete_1_button.pack()
delete_2_button = Button(root, text="Delete Can2", command=del_can_2, fg="green",font=('Comic Sans MS', 12, 'bold italic'))
delete_2_button.pack()
baud_button = Button(root, text="Set Baud Rate", command=baud_click, fg="blue",font=('Comic Sans MS', 12, 'bold italic'))
baud_button.pack()


root.mainloop()

