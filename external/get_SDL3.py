import os
import sys
import shutil
import zipfile
import urllib.request

VERSION = "3.2.26"

while True:
    print("Выберите тип библиотеки для установки:")
    print("1 - MinGW")
    print("2 - MSVC")
    choice = input("Ваш выбор (1/2): ").strip()

    if choice == "1":
        COMPILER = "mingw"
        ZIP_NAME = f"SDL3-devel-{VERSION}-mingw.zip"
        FINAL_DIR = os.path.join(".", "SDL3_MinGW")
        break
    elif choice == "2":
        COMPILER = "VC"
        ZIP_NAME = f"SDL3-devel-{VERSION}-VC.zip"
        FINAL_DIR = os.path.join(".", "SDL3_MSVC")
        break
    else:
        print("Неверный выбор. Попробуйте снова.\n")

URL = f"https://www.libsdl.org/release/{ZIP_NAME}"
ZIP_PATH = os.path.join(".", ZIP_NAME)
EXTRACT_TEMP = os.path.join(".", f"SDL3-{VERSION}")

print(f"Выбран вариант: {COMPILER.upper()}")
print(f"SDL3 будет установлена в: {FINAL_DIR}\n")

if os.path.exists(FINAL_DIR):
    print(f"SDL3 уже существует в {FINAL_DIR}")
    sys.exit(0)

if not os.path.exists(ZIP_PATH):
    print(f"Скачиваем {URL}...")
    try:
        urllib.request.urlretrieve(URL, ZIP_PATH)
    except Exception as ex:
        print(f"Ошибка скачивания: {ex}")
        sys.exit(1)
else:
    print(f"Архив уже скачан: {ZIP_PATH}")

print("Распаковываем архив...")
try:
    with zipfile.ZipFile(ZIP_PATH, 'r') as zip_ref:
        zip_ref.extractall(".")
except Exception as ex:
    print(f"Ошибка распаковки: {ex}")
    sys.exit(1)

if os.path.exists(EXTRACT_TEMP):
    print(f"Переименовываем {EXTRACT_TEMP} в {FINAL_DIR}...")
    try:
        shutil.move(EXTRACT_TEMP, FINAL_DIR)
    except Exception as ex:
        print(f"Ошибка переименования: {ex}")
        sys.exit(1)
else:
    print(f"Ошибка: папка {EXTRACT_TEMP} не найдена после распаковки")
    sys.exit(1)

try:
    os.remove(ZIP_PATH)
    print(f"Удалён архив {ZIP_PATH}")
except Exception as ex:
    print(f"Ошибка удаления архива: {ex}")

print(f"SDL3 ({COMPILER.upper()}) успешно установлена в {FINAL_DIR}")
