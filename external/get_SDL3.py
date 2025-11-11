import os
import sys
import shutil
import zipfile
import urllib.request

VERSION = "3.2.26"

def get_compiler_from_args():
    if len(sys.argv) > 1:
        arg = sys.argv[1].strip().lower()
        if arg == "mingw":
            return "mingw"
        elif arg in ("msvc", "vc"):
            return "vc"
        else:
            print("Invalid argument. Use 'mingw' or 'msvc'")
            sys.exit(1)
    return None

COMPILER = get_compiler_from_args()

if not COMPILER:
    while True:
        print("Select library type to install:")
        print("1 - MinGW")
        print("2 - MSVC")
        choice = input("Your choice (1/2): ").strip()
        if choice == "1":
            COMPILER = "mingw"
            break
        elif choice == "2":
            COMPILER = "vc"
            break
        else:
            print("Invalid choice. Try again.\n")

if COMPILER == "mingw":
    ZIP_NAME = f"SDL3-devel-{VERSION}-mingw.zip"
    FINAL_DIR = os.path.join(".", "SDL3_MinGW")
else:
    ZIP_NAME = f"SDL3-devel-{VERSION}-VC.zip"
    FINAL_DIR = os.path.join(".", "SDL3_MSVC")

URL = f"https://www.libsdl.org/release/{ZIP_NAME}"
ZIP_PATH = os.path.join(".", ZIP_NAME)
EXTRACT_TEMP = os.path.join(".", f"SDL3-{VERSION}")

print(f"Selected option: {COMPILER.upper()}")
print(f"SDL3 will be installed into: {FINAL_DIR}\n")

if os.path.exists(FINAL_DIR):
    print(f"SDL3 already exists in {FINAL_DIR}")
    sys.exit(0)

if not os.path.exists(ZIP_PATH):
    print(f"Downloading {URL}...")
    try:
        urllib.request.urlretrieve(URL, ZIP_PATH)
    except Exception as ex:
        print(f"Download error: {ex}")
        sys.exit(1)
else:
    print(f"Archive already exists: {ZIP_PATH}")

print("Extracting archive...")
try:
    with zipfile.ZipFile(ZIP_PATH, 'r') as zip_ref:
        zip_ref.extractall(".")
except Exception as ex:
    print(f"Extraction error: {ex}")
    sys.exit(1)

if os.path.exists(EXTRACT_TEMP):
    print(f"Renaming {EXTRACT_TEMP} to {FINAL_DIR}...")
    try:
        shutil.move(EXTRACT_TEMP, FINAL_DIR)
    except Exception as ex:
        print(f"Rename error: {ex}")
        sys.exit(1)
else:
    print(f"Error: folder {EXTRACT_TEMP} not found after extraction.")
    sys.exit(1)

try:
    os.remove(ZIP_PATH)
    print(f"Removed archive {ZIP_PATH}")
except Exception as ex:
    print(f"Archive removal error: {ex}")

print(f"SDL3 ({COMPILER.upper()}) successfully installed into {FINAL_DIR}")
