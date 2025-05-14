from fastapi import FastAPI, File, UploadFile
from fastapi.responses import JSONResponse
import ollama
import tempfile
import os
from datetime import datetime
import shutil

app = FastAPI()

SAVE_DIR = "./received_images"
os.makedirs(SAVE_DIR, exist_ok=True)  # Tạo thư mục nếu chưa tồn tại

@app.post("/read-number-plate/")
async def read_number_plate(file: UploadFile = File(...)):
    try:
        # Lưu file ảnh tạm thời
        with tempfile.NamedTemporaryFile(delete=False, suffix=".jpg") as tmp:
            contents = await file.read()
            tmp.write(contents)
            tmp_path = tmp.name

        # Gọi mô hình Ollama
        response = ollama.chat(
            model='llama3.2-vision',
            messages=[{
                'role': 'user',
                'content': 'Please read the Number Plate and answer only with the number plate in the exact format "XX-XX XXXX" (two letters or digits, a dash, two letters or digits, a space, four digits). No extra text or symbols.',
                'images': [tmp_path]
            }]
        ) 

        plate_text = response['message']['content'].strip().replace(" ", "").replace("\n", "")

        # Tạo timestamp cho tên file
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

        # Tên file lưu: timestamp_plate.jpg
        safe_plate = "".join(c for c in plate_text if c.isalnum())  # loại bỏ ký tự không hợp lệ
        new_filename = f"{timestamp}_{safe_plate}.jpg"
        new_filepath = os.path.join(SAVE_DIR, new_filename)

        # Sao chép file tạm vào thư mục đích
        shutil.copy(tmp_path, new_filepath)

        return JSONResponse(content={
            "number_plate": plate_text,
            "saved_path": new_filepath
        })

    except Exception as e:
        return JSONResponse(status_code=500, content={"error": str(e)})
