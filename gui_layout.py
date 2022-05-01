from client_software import *
from tkinter import *

root = Tk()


def can1Click():
    can_1()
    myLabel = Label(root, text="Success!")
    myLabel.pack()

def can2Click():
    can_1()
    myLabel = Label(root, text="Success!")
    myLabel.pack()

def del_can_1():
    delete_can1()
    myLabel = Label(root, text="Success!")
    myLabel.pack()

def del_can_2():
    delete_can2()
    myLabel = Label(root, text="Success!")
    myLabel.pack()


def myClick():
    myLabel = Label(root, text="Success!")
    myLabel.pack()


can_1_button = Button(root, text="Retrieve Can1", command=can1Click, fg="blue")
can_1_button.pack()
can_2_button = Button(root, text="Retrieve Can2", command=myClick, fg="blue")
can_2_button.pack()
delete_1_button = Button(root, text="Delete Can1 Datafile", command=myClick, fg="blue")
delete_1_button.pack()

root.mainloop()
