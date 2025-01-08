import tkinter as tk
from tkinter import ttk
from tkcalendar import Calendar
import serial
import json

# Function to connect to COM port
def connect_com_port():
    global ser
    com_port = com_port_var.get()
    baud_rate = baud_rate_var.get()

    try:
        ser = serial.Serial(com_port, baud_rate, timeout=1)
        connect_button.config(bg="green")
        status_label.config(text=f"Connected to {com_port} at {baud_rate} baud", fg="green")
    except Exception as e:
        connect_button.config(bg="red")
        status_label.config(text=f"Error: {e}", fg="red")

# Function to send parameters in JSON format
def send_parameters():
    if not ser.is_open:
        status_label.config(text="Please connect to the COM port first.", fg="red")
        return

    # Collect input data
    time = f"{hour_entry.get()}:{minute_entry.get()}"
    day = day_entry.get()
    month = month_entry.get()
    year = year_entry.get()

    # Create JSON object
    data = {
        "time": time,
        "day": day,
        "month": month,
        "year": year
    }

    json_data = json.dumps(data)
    
    # Send data over the serial port
    ser.write(json_data.encode('utf-8'))

    status_label.config(text=f"Data Sent: {json_data}", fg="blue")

# Function to handle calendar date selection
def update_date_fields(event):
    selected_date = cal.get_date()
    date_parts = selected_date.split('-')
    year_entry.delete(0, tk.END)
    year_entry.insert(0, date_parts[0])
    month_entry.delete(0, tk.END)
    month_entry.insert(0, date_parts[1])
    day_entry.delete(0, tk.END)
    day_entry.insert(0, date_parts[2])

# Create main window
root = tk.Tk()
root.title("Serial Communication App")

# COM Port selection
com_port_label = tk.Label(root, text="Select COM Port:")
com_port_label.grid(row=0, column=0, padx=10, pady=10)
com_ports = [f"COM{i}" for i in range(1, 10)]  # COM1 to COM9 (adjust according to your system)
com_port_var = tk.StringVar()
com_port_dropdown = ttk.Combobox(root, textvariable=com_port_var, values=com_ports)
com_port_dropdown.grid(row=0, column=1, padx=10, pady=10)

# Baud rate selection
baud_rate_label = tk.Label(root, text="Select Baud Rate:")
baud_rate_label.grid(row=1, column=0, padx=10, pady=10)
baud_rates = ["9600", "115200", "19200", "38400", "57600"]  # Example baud rates
baud_rate_var = tk.StringVar()
baud_rate_dropdown = ttk.Combobox(root, textvariable=baud_rate_var, values=baud_rates)
baud_rate_dropdown.grid(row=1, column=1, padx=10, pady=10)

# Time input (Hour and Minute)
time_label = tk.Label(root, text="Enter Time (HH:MM):")
time_label.grid(row=2, column=0, padx=10, pady=10)

hour_label = tk.Label(root, text="HH:")
hour_label.grid(row=2, column=1, padx=10, pady=10)
hour_entry = tk.Entry(root, width=5)
hour_entry.grid(row=2, column=2, padx=10, pady=10)

minute_label = tk.Label(root, text="MM:")
minute_label.grid(row=2, column=3, padx=10, pady=10)
minute_entry = tk.Entry(root, width=5)
minute_entry.grid(row=2, column=4, padx=10, pady=10)

# Date input (Day, Month, Year)
date_label = tk.Label(root, text="Enter Date (DD/MM/YYYY):")
date_label.grid(row=3, column=0, padx=10, pady=10)

day_label = tk.Label(root, text="Day:")
day_label.grid(row=3, column=1, padx=10, pady=10)
day_entry = tk.Entry(root, width=5)
day_entry.grid(row=3, column=2, padx=10, pady=10)

month_label = tk.Label(root, text="Month:")
month_label.grid(row=3, column=3, padx=10, pady=10)
month_entry = tk.Entry(root, width=5)
month_entry.grid(row=3, column=4, padx=10, pady=10)

year_label = tk.Label(root, text="Year:")
year_label.grid(row=3, column=5, padx=10, pady=10)
year_entry = tk.Entry(root, width=5)
year_entry.grid(row=3, column=6, padx=10, pady=10)

# Calendar to select a date
cal_label = tk.Label(root, text="Select Date:")
cal_label.grid(row=4, column=0, padx=10, pady=10)
cal = Calendar(root, selectmode="day", date_pattern="yyyy-mm-dd")
cal.grid(row=4, column=1, columnspan=2, padx=10, pady=10)
cal.bind("<<CalendarSelected>>", update_date_fields)

# Connect Button
connect_button = tk.Button(root, text="Connect", command=connect_com_port, bg="red")
connect_button.grid(row=5, column=0, columnspan=2, pady=10)

# Send Parameters Button
send_button = tk.Button(root, text="Send Parameters", command=send_parameters)
send_button.grid(row=6, column=0, columnspan=2, pady=10)

# Status label
status_label = tk.Label(root, text="Not connected", fg="red")
status_label.grid(row=7, column=0, columnspan=2, pady=10)

# Start the Tkinter event loop
root.mainloop()
