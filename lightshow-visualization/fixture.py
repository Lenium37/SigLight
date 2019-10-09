from PIL import Image, ImageTk
from tkinter import Label
import threading


class Fixture:
    def __init__(self, canvas, color, column, value_changes):
        self.canvas = canvas
        self.color = color
        self.lamp = ImageTk.PhotoImage(Image.new("RGBA", (100, 100), color=self.color))
        self.label = Label(image=self.lamp)
        self.label.image = self.lamp
        self.column = column
        self.label.grid(row=0, column=self.column)

        self.color_toggle = True
        self.counter = 1
        self.value_changes = value_changes
        self.value_change_counter = 0

    def callback(self):
        self.canvas.after(1, self.update_color)

    def update_color(self):
        self.color = self.value_changes[self.value_change_counter]
        self.lamp = ImageTk.PhotoImage(Image.new("RGBA", (100, 100), color=self.color))
        self.label.config(image=self.lamp)
        self.label.image = self.lamp
        self.label.grid(row=0, column=self.column)

        self.value_change_counter = self.value_change_counter + 1
        if self.value_change_counter < len(self.value_changes):
            timer = threading.Timer(0.016, self.callback)
            timer.daemon = True
            timer.start()

