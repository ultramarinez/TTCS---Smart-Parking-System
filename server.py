import serial
import base64
import time
import os
import re

# Cấu hình Serial
ser = serial.Serial('COM25', 115200)  
buffer = ""

if not os.path.exists('received_images'):
    os.makedirs('received_images')
    
def send_command(slot_id, status):
    command = f"{slot_id},{status}\n"  
    ser.write(command.encode())
    print(f"Đã gửi lệnh: {command.strip()}")

send_command("A1", "full")
send_command("B3", "full")

while True:
    if ser.in_waiting > 0:
        data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
        buffer += data

    # Xử lý buffer
    while '\n' in buffer:
        line, buffer = buffer.split('\n', 1)
        line = line.strip()
        
        if line.startswith("[CAM") and ']' in line:
            cam_id_end = line.find(']')
            cam_id = line[4:cam_id_end]
            image_data = line[cam_id_end + 1:]
            
            try:
                img_bytes = base64.b64decode(image_data)
                timestamp = int(time.time())
                filename = f'received_images/CAM{cam_id}_{timestamp}.jpg'
                with open(filename, 'wb') as f:
                    f.write(img_bytes)
                print(f"Đã lưu ảnh: {filename}")
            except Exception as e:
                print(f"Lỗi xử lý ảnh: {str(e)}")