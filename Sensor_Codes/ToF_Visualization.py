import serial
import numpy as np
import matplotlib.pyplot as plt

# Step 1: Configure serial port (adjust COM port!)
ser = serial.Serial('COM7', 115200)  # Replace COM7 with your actual port

# Step 2: Setup matplotlib
plt.ion()
fig, ax = plt.subplots()
heatmap = ax.imshow(np.zeros((8,8)), cmap='viridis', vmin=0, vmax=2000)
plt.colorbar(heatmap)

# Flag to control loop
running = True

# Step 3: Define key press event
def on_key(event):
    global running
    if event.key == 'x':   # press 'x' to exit
        print("Exit requested — closing heatmap.")
        running = False

fig.canvas.mpl_connect('key_press_event', on_key)

# Step 4: Main loop
while running:
    try:
        line = ser.readline().decode('utf-8').strip()
        values = line.split()

        if len(values) == 64:  # 8x8 grid
            data = np.array(values, dtype=float).reshape((8,8))

            heatmap.set_data(data)
            ax.set_title("VL53L5CX 8x8 Distance Heatmap (mm)")
            plt.draw()
            plt.pause(0.01)

    except Exception as e:
        print("Error:", e)

# Cleanup
ser.close()
plt.close(fig)
print("Program terminated cleanly.")