import subprocess
import os
import platform
import time
from urllib import request
import filecmp
import threading
import requests
import datetime 
import socket
import sys

IS_WINDOWS = sys.platform.startswith('win')

# utilities 
def run(cmd, echo=True, shell=True, printOutput = True) -> str:
    """ Runs a command, returns the output """    

    if echo:
        print(cmd)
    output = subprocess.check_output(cmd, shell=shell).decode("utf-8") 
    if printOutput:
      print(output)
    return output

def run_powershell(cmd: str, printOutput = True) -> str:
    cmd2=f"PowerShell -NoProfile -ExecutionPolicy Bypass -Command \"& {cmd}\""
    return run(cmd2, True,printOutput)

def create_and_enter(path: str):
    if not os.path.exists(path):
        os.mkdir(path)
    os.chdir(path)
    return os.getcwd()


def is_listening(addr, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    result = sock.connect_ex((addr,port))
    if result == 0:
        return True
    return False

def kill_by_name(name: str):
    try:
        if(IS_WINDOWS):
            run(f"taskkill /IM {name} /F", False, True, False)
        else:
            run(f"killall -9 {name}", False, True, False)
    except:
      pass
    
def kill_by_port(port: int, verbose = False):
    try:
        if(IS_WINDOWS):
            run_powershell(f"Get-Process -Id (Get-NetTCPConnection -LocalPort \"{port}\").OwningProcess | Stop-Process", verbose, True, verbose)
        else:
            run(f"fuser -k -TERM -n tcp {port}", verbose, True, verbose)
    except:
      pass
        

# tests

# serve a file, download it, wait for the server to die
def test_single_download(cmd: str) -> bool:    
    daemon = subprocess.Popen(f"{cmd}  -p 1234 -s served_file.bin served_file.bin", shell = True, stdin = subprocess.PIPE)    
    def driver():
        daemon.communicate(b"0\n") # select the loopback interface

    t = threading.Thread(target=driver)
    t.start()

    time.sleep(1)
    remote_url = 'http://127.0.0.1:1234/served_file.bin'
    request.urlretrieve(remote_url, "downloaded_file.bin")
    good = filecmp.cmp("served_file.bin", "downloaded_file.bin")
    t.join()
    return good


# serve a file, download it twice, kill the server
def test_multiple_download(cmd: str) -> bool:    

    daemon = subprocess.Popen(f"{cmd} -k -p 1234 -s served_file.bin served_file.bin", shell = True, stdin = subprocess.PIPE)    
    def driver():        
        daemon.communicate(b"0\n") # select the loopback interface

    t = threading.Thread(target=driver)
    t.start()

    time.sleep(1)
    remote_url = 'http://127.0.0.1:1234/served_file.bin'
    request.urlretrieve(remote_url, "downloaded_file.bin")
    good = filecmp.cmp("served_file.bin", "downloaded_file.bin")
    time.sleep(1)
    request.urlretrieve(remote_url, "downloaded_file2.bin")
    good = filecmp.cmp("served_file.bin", "downloaded_file2.bin")
    good = good and filecmp.cmp("served_file.bin", "downloaded_file2.bin")
    daemon.kill()

    t.join()
    return good

def test_single_upload(cmd: str) -> bool:    
    daemon = subprocess.Popen(f"{cmd} -v -r . -p 1234 -s entry_point", shell = True, stdin = subprocess.PIPE)    
    error = False
    def driver():
        try:
          daemon.communicate(b"0\n", 10) # select the loopback interface
        except Exception as e: 
          print(e)
          error=e

        

    t = threading.Thread(target=driver)
    t.start()

    with open('to_upload_file.bin', 'wb') as fout:
        fout.write(os.urandom(1024 * 1024)) 

    time.sleep(2)
    remote_url = 'http://127.0.0.1:1234/upload/entry_point'
    files = {'files': ('uploaded_file.bin', open('to_upload_file.bin', 'rb'))} #, 'application/octet-stream', {'Expires': '0'}
    values = {'submit': 'submit'}
    print("Trying to upload... ")
    response = requests.post(remote_url, files=files, data = values)
    print(response.content)    
    print("Something was uploaded, let's check if it's right ")
    good = filecmp.cmp("to_upload_file.bin", "uploaded_file.bin")
    t.join()
    if error:
        raise error
    return good


# main

BASE_FOLDER = "../output"
PROCESS_NAME = "QrFiletransfer"
QRFILETRANSFER = PROCESS_NAME

if platform.system() == "Windows":
    QRFILETRANSFER += ".exe"


QRFILETRANSFER = os.path.abspath(os.path.join(BASE_FOLDER, QRFILETRANSFER))


output = run(f"{QRFILETRANSFER} -h",False)

assert "qr-filentransfer-cpp" in output

print("QrFileTransfer is in the right position, we can start testing")

create_and_enter("temp")
with open('served_file.bin', 'wb') as fout:
    fout.write(os.urandom(1024 * 1024)) 


tests=[test_single_download, test_multiple_download, test_single_upload]

try:
    os.remove("serverlog.log")
except:
    pass

with open('aggregated_log.log', 'w') as fout:
    for t in tests:
        kill_by_port(1234)
        fout.write(f"{t.__name__}\t{datetime.datetime.now()}\n")
        print("Calling", t.__name__)

        result = False
        try:
          result = t(QRFILETRANSFER)
        except Exception as e: 
          print(e)
          fout.write(f"{str(e)}\n")
          kill_by_port(1234, True)

        
        print("\t->", result)
        
        if(os.path.exists("serverlog.log")):
            with open('serverlog.log', 'r') as fin:
                fout.write(fin.read())

        fout.write(f"\t->{result}\n----------------\n")
