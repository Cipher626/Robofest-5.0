import serial
import numpy as np
import matplotlib.pyplot as plt

# Configure serial port (adjust COM port!)
ser = serial.Serial('COM7', 115200)  # Replace COM7 with your actual port

plt.ion()
fig, ax = plt.subplots()
heatmap = ax.imshow(np.zeros((8,8)), cmap='viridis', vmin=0, vmax=2000)
plt.colorbar(heatmap)

running = True

def on_key(event):
    global running
    if event.key == 'x':
        print("Exit requested — closing heatmap.")
        running = False

fig.canvas.mpl_connect('key_press_event', on_key)

while running:
    try:
        line = ser.readline().decode('utf-8').strip()
        values = line.split()

        if len(values) == 64:
            data = np.array(values, dtype=float).reshape((8,8))

            # Ignore zeros (outliers filtered in Arduino)
            data[data == 0] = np.nan

            heatmap.set_data(data)
            ax.set_title("VL53L5CX 8x8 Distance Heatmap (mm)")
            plt.draw()
            plt.pause(0.01)

    except Exception as e:
        print("Error:", e)

ser.close()
plt.close(fig)
print("Program terminated cleanly.")