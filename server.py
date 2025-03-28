import serial
import base64
import time
import os
import re

# Cấu hình cổng Serial
ser = serial.Serial('COM10', 115200)  # Thay đổi cổng COM tương ứng
buffer = ""

# Tạo thư mục lưu ảnh
if not os.path.exists('received_images'):
    os.makedirs('received_images')

while True:
    if ser.in_waiting > 0:
        data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
        buffer += data

    # Xử lý từng dòng trong buffer
    while '\n' in buffer:
        line, buffer = buffer.split('\n', 1)
        line = line.strip()
        
        # Kiểm tra định dạng [CAMx]base64_data
        if line.startswith("[CAM") and ']' in line:
            # Tách ID CAM và dữ liệu ảnh
            cam_id_end = line.find(']')
            cam_id = line[4:cam_id_end]
            image_data = line[cam_id_end + 1:]
            
            try:
                # Giải mã base64 và lưu ảnh
                img_bytes = base64.b64decode(image_data)
                timestamp = int(time.time())
                filename = f'received_images/CAM{cam_id}_{timestamp}.jpg'
                with open(filename, 'wb') as f:
                    f.write(img_bytes)
                print(f"Đã lưu ảnh: {filename}")
            except Exception as e:
                print(f"Lỗi xử lý ảnh: {str(e)}")