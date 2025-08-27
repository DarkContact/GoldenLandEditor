import os
import urllib.request
import zipfile
import shutil

VERSION = "3.2.20"
ZIP_NAME = f"SDL3-devel-{VERSION}-mingw.zip"
URL = f"https://www.libsdl.org/release/{ZIP_NAME}"
ZIP_PATH = os.path.join(".", ZIP_NAME)
EXTRACT_TEMP = os.path.join(".", f"SDL3-{VERSION}")
FINAL_DIR = os.path.join(".", "SDL3")

if os.path.exists(FINAL_DIR):
    print(f"SDL3 уже существует в {FINAL_DIR}")
    exit(0)

if not os.path.exists(ZIP_PATH):
    print(f"Скачиваем {URL}...")
    try:
        urllib.request.urlretrieve(URL, ZIP_PATH)
    except Exception as e:
        print(f"Ошибка скачивания: {e}")
        exit(1)
else:
    print(f"Архив уже скачан: {ZIP_PATH}")

print(f"Распаковываем архив...")
try:
    with zipfile.ZipFile(ZIP_PATH, 'r') as zip_ref:
        zip_ref.extractall(".")
except Exception as e:
    print(f"Ошибка распаковки: {e}")
    exit(1)

if os.path.exists(EXTRACT_TEMP):
    print(f"Переименовываем {EXTRACT_TEMP} в {FINAL_DIR}...")
    try:
        shutil.move(EXTRACT_TEMP, FINAL_DIR)
    except Exception as e:
        print(f"Ошибка переименования: {e}")
        exit(1)
else:
    print(f"Ошибка: папка {EXTRACT_TEMP} не найдена после распаковки")
    exit(1)

# Удаляем архив
try:
    os.remove(ZIP_PATH)
    print(f"Удалён архив {ZIP_PATH}")
except Exception as e:
    print(f"Ошибка удаления архива: {e}")

print(f"SDL3 успешно установлен в {FINAL_DIR}")
