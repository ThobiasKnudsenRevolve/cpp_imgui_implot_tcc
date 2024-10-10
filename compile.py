import subprocess
import os
from colorama import Fore, init
import platform

init(autoreset=True)
def cmd(*args, **kwargs) -> bool:
    command_str = args[0] if isinstance(args[0], str) else ' '.join(args[0])
    print(Fore.CYAN + f"COMMAND  " + Fore.WHITE + f"{command_str}")
    try:
        result = subprocess.run(*args, **kwargs)
        if result.stdout:
            print(Fore.YELLOW + "STDOUT  " + Fore.WHITE + f"{result.stdout}")
        if result.stderr:
            print(Fore.YELLOW + "MESSAGE  " + Fore.WHITE + f"{result.stderr}")
        if result.returncode == 0:
            print(Fore.GREEN + "SUCCESS  \n")
            return True
        else:
            print(Fore.RED + "ERROR  " + Fore.WHITE + f"{result}\n")
            return False
    except FileNotFoundError:
        print(Fore.RED + f"ERRORR: FileNotFoundError occurred  " + Fore.WHITE + f"Command '{command_str}' not found\n")
        return False
    except subprocess.CalledProcessError as e:
        print(Fore.RED + "ERROR: CalledProcessError occurred   " + Fore.WHITE + f"Details: {e}\n")
        return False


cwd_win = os.getcwd()
cwd_unix = cwd_win.replace("\\", "/")

# Updated configuration to allow multiple source directories
config = {}
if platform.system() == "Windows":
    print("hello")
    config = {
        "compiler": {
            "c_compiler": "gcc",    # C compiler
            "cpp_compiler": "g++"   # C++ compiler
        },
        "cflags": {
            "common": "-Wall",        # Common flags for both C and C++ files
            "c": "",                  # Additional flags specific to C
            "cpp": "-std=c++11 "      # Additional flags specific to C++
        },
        "ldflags": "",                # Linking flags (for libraries)
        "output": {
            "object_dir": ".\\obj",   # Directory for object files
            "binary_dir": ".\\bin",   # Directory for the output executable
            "binary_name": "main"     # Name of the output binary
        },
        "src_dirs": [".\\src"],       # Allow multiple source directories
        "src_files": []               # Allow individual source files
    }

def compile(config):
    obj_dir = config["output"]["object_dir"]
    bin_dir = config["output"]["binary_dir"]

    # Create obj and bin directories if they do not exist
    if not os.path.exists(obj_dir):
        os.makedirs(obj_dir)
    if not os.path.exists(bin_dir):
        os.makedirs(bin_dir)

    object_files = []

    # Iterate over multiple source directories
    for src_dir in config["src_dirs"]:
        # Get list of C and C++ source files
        c_files = [f for f in os.listdir(src_dir) if f.endswith(".c")]
        cpp_files = [f for f in os.listdir(src_dir) if f.endswith(".cpp")]

        # Compile C files
        for c_file in c_files:
            src_file = os.path.join(src_dir, c_file)
            obj_file = os.path.join(obj_dir, os.path.splitext(c_file)[0] + ".o")
            
            # Remove the dependency check to force recompilation
            compile_cmd = (
                f"{config['compiler']['c_compiler']} "
                f"{config['cflags']['common']} "
                f"{config['cflags']['c']} "
                f"-c {src_file} -o {obj_file}"
            )
            if not cmd(compile_cmd):
                print(f"Failed to compile {src_file}")
                return False
            object_files.append(obj_file)

        # Compile C++ files
        for cpp_file in cpp_files:
            src_file = os.path.join(src_dir, cpp_file)
            obj_file = os.path.join(obj_dir, os.path.splitext(cpp_file)[0] + ".o")
            
            # Remove the dependency check to force recompilation
            compile_cmd = (
                f"{config['compiler']['cpp_compiler']} "
                f"{config['cflags']['common']} "
                f"{config['cflags']['cpp']} "
                f"-c {src_file} -o {obj_file}"
            )
            if not cmd(compile_cmd):
                print(f"Failed to compile {src_file}")
                return False
            object_files.append(obj_file)

    # Compile individual source files
    for src_file in config["src_files"]:
        file_ext = os.path.splitext(src_file)[1]
        obj_file = os.path.join(obj_dir, os.path.splitext(os.path.basename(src_file))[0] + ".o")
        
        # Determine if it's a C or C++ file and use appropriate compiler
        if file_ext == ".c":
            compile_cmd = (
                f"{config['compiler']['c_compiler']} "
                f"{config['cflags']['common']} "
                f"{config['cflags']['c']} "
                f"-c {src_file} -o {obj_file}"
            )
        elif file_ext == ".cpp":
            compile_cmd = (
                f"{config['compiler']['cpp_compiler']} "
                f"{config['cflags']['common']} "
                f"{config['cflags']['cpp']} "
                f"-c {src_file} -o {obj_file}"
            )
        else:
            print(f"Unknown file extension for {src_file}")
            return False
        
        if not cmd(compile_cmd):
            print(f"Failed to compile {src_file}")
            return False
        object_files.append(obj_file)

    # Link all object files into a final executable
    output_executable = os.path.join(bin_dir, config["output"]["binary_name"])
    link_cmd = (
        f"{config['compiler']['cpp_compiler']} "
        f"{' '.join(object_files)} "
        f"{config['ldflags']} "
        f"-o {output_executable}"
    )

    if not cmd(link_cmd):
        print(f"Failed to link object files.")
        return False

    print(f"Compilation successful. Executable created at {output_executable}")
    return True

def tcc() -> bool:
    if platform.system() == "Windows":
        if not os.path.exists(f"{cwd_win}\\include") \
        or not os.path.exists(f"{cwd_win}\\lib") \
        or not os.path.exists(f"{cwd_win}\\libtcc") \
        or not os.path.exists(f"{cwd_win}\\libtcc.dll"):
            
            if not cmd("git --version", cwd=cwd_win, shell=True):
                if not cmd("winget --version", cwd=cwd_win, shell=True):
                    print(Fore.RED + "You need to manually install winget or Git.")
                    return False
                if not cmd("winget install --id Git.Git -e --source winget", cwd=cwd_win, shell=True):
                    print(Fore.RED + "Could not install Git via winget.")
                    return False
            if not cmd("make --version", cwd=cwd_win, shell=True) \
            or not cmd("g++ --version", cwd=cwd_win, shell=True):
                if not cmd("choco --version", cwd=cwd_win, shell=True):
                    cmd("powershell -Command \"Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))\"", cwd=cwd_win, shell=True)
                cmd("powershell -Command \"choco install make mingw -y\"", cwd=cwd_win, shell=True)

            cmd(f"powershell -Command \"Remove-Item -Recurse -Force .\\external\\installs\\tcc\"", cwd=cwd_win, shell=True)
            bash_path = "C:\\Program Files\\Git\\bin\\bash.exe"
            os.makedirs(f"{cwd_win}\\external\\installs\\tcc")
            cmd(f"git clone https://github.com/Tiny-C-Compiler/mirror-repository .\\external\\installs\\tcc", cwd=cwd_win, capture_output=True, text=True)
            os.makedirs(f"{cwd_win}\\external\\installs\\tcc\\build")
            configure_prefix = (os.path.abspath(os.path.join("external", "installs", "tcc", "build"))).replace("\\", "/")
            cmd([bash_path, '-c', f"cd {cwd_unix}/external/installs/tcc && ./configure --prefix={configure_prefix}"], capture_output=True, text=True)
            cmd([bash_path, '-c', f"cd {cwd_unix}/external/installs/tcc && make"], capture_output=True, text=True)
            cmd([bash_path, '-c', f"cd {cwd_unix}/external/installs/tcc && make install"], capture_output=True, text=True)
            cmd([bash_path, '-c', f"cp -ru {cwd_unix}/external/installs/tcc/build/libtcc.dll {cwd_unix}/libtcc.dll"], capture_output=True, text=True)
            cmd([bash_path, '-c', f"cp -ru {cwd_unix}/external/installs/tcc/build/libtcc {cwd_unix}"], capture_output=True, text=True)
            cmd([bash_path, '-c', f"cp -ru {cwd_unix}/external/installs/tcc/build/include {cwd_unix}"], capture_output=True, text=True)
            cmd([bash_path, '-c', f"cp -ru {cwd_unix}/external/installs/tcc/build/lib {cwd_unix}"], capture_output=True, text=True)

            if not os.path.exists(f"{cwd_win}\\include") \
            or not os.path.exists(f"{cwd_win}\\lib") \
            or not os.path.exists(f"{cwd_win}\\libtcc") \
            or not os.path.exists(f"{cwd_win}\\libtcc.dll"):
                print(Fore.RED + "Could not install TinyCC.")
                return False
            print(Fore.GREEN + "TinyCC built successfully in the local folder.\n")
        config["cflags"]["common"] += " -I libtcc "
        config["ldflags"] += " libtcc.dll " 
        return True
    if platform.system() == "Linux":
        pass
    if platform.system() == "Darwin":
        pass

def opengl() -> bool:
    if platform.system() == "Windows":
        if not os.path.exists(f"{cwd_win}\\external\\glew") \
        or not os.path.exists(f"{cwd_win}\\external\\glfw"):
            
            if not cmd("git --version", cwd=cwd_win, shell=True):
                if not cmd("winget --version", cwd=cwd_win, shell=True):
                    print(Fore.RED + "You need to manually install winget or Git.", cwd=cwd_win)
                    return False
                if not cmd("winget install --id Git.Git -e --source winget", cwd=cwd_win, shell=True):
                    print(Fore.RED + "Could not install Git via winget.")
                    return False
                
            cmd("powershell -Command \"Remove-Item -Recurse -Force .\\external\\installs\\glfw.zip\"", cwd=cwd_win, shell=True)
            cmd("curl -L -o .\\external\\installs\\glfw.zip https://sourceforge.net/projects/glfw/files/glfw/3.3.10/glfw-3.3.10.bin.WIN64.zip/download", cwd=cwd_win, shell=True)
            cmd("powershell -Command \"Expand-Archive -Path glfw.zip -DestinationPath ..\\glfw\"", cwd=f"{cwd_win}\\external\\installs", shell=True)
                
            cmd("powershell -Command \"Remove-Item -Recurse -Force .\\external\\installs\\glew.zip\"", cwd=cwd_win, shell=True)
            cmd("curl -L -o .\\external\\installs\\glew.zip https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip/download", cwd=cwd_win, capture_output=True, text=True)
            cmd("powershell -Command \"Expand-Archive -Path glew.zip -DestinationPath ..\\glew\"", cwd=f"{cwd_win}\\external\\installs", capture_output=True, text=True)

            if not os.path.exists(f"{cwd_win}\\external\\glew") \
            or not os.path.exists(f"{cwd_win}\\external\\glfw"):
                print(Fore.RED + "Could not build OpenGL")
                return False
            
            print(Fore.GREEN + "OpenGL buildt successfully in local folder\n")

        bash_path = "C:\\Program Files\\Git\\bin\\bash.exe"
        if not os.path.exists(".\\bin"):
            os.makedirs(".\\bin")
        cmd([bash_path, '-c', f"cp {cwd_unix}/external/glew/glew-2.1.0/bin/Release/x64/glew32.dll {cwd_unix}/bin"], capture_output=True, text=True)
        cmd([bash_path, '-c', f"cp {cwd_unix}/external/glfw/glfw-3.3.10.bin.WIN64/lib-mingw-w64/glfw3.dll {cwd_unix}/bin"], capture_output=True, text=True)
        config["cflags"]["common"] += " -I external\\glfw\\glfw-3.3.10.bin.WIN64\\include -I external\\glew\\glew-2.1.0\\include "
        config["ldflags"] += "-L external\\glfw\\glfw-3.3.10.bin.WIN64\\lib-mingw-w64 -L external\\glew\\glew-2.1.0\\lib\\Release\\x64 -lglew32 -lglfw3 -lopengl32 -lgdi32 " 
        return True
    if platform.system() == "Linux":
        pass
    if platform.system() == "Darwin":
        pass

# https://github.com/adobe/imgui.git
def imgui() -> bool:
    if platform.system() == "Windows":
        cwd_win = os.getcwd()
        if not os.path.exists(f"{cwd_win}\\external\\imgui"):
            cmd("git clone https://github.com/adobe/imgui.git .\\external\\imgui", cwd = cwd_win, shell=True)
        config["cflags"]["common"] += "-I external\\imgui -I external\\imgui\\backends "
        config["src_dirs"].append(".\\external\\imgui")
        config["src_files"].append(".\\external\\imgui\\backends\\imgui_impl_glfw.cpp")
        config["src_files"].append(".\\external\\imgui\\backends\\imgui_impl_opengl3.cpp")
    if platform.system() == "Linux":
        pass
    if platform.system() == "Darwin":
        pass

# https://github.com/epezent/implot.git
def implot() -> bool:
    if platform.system() == "Windows":
        if not os.path.exists(f"{cwd_win}\\external\\implot"):
            cmd("git clone https://github.com/epezent/implot.git .\\external\\implot", cwd = cwd_win, shell=True)
        config["cflags"]["common"] += "-I external\\implot -I external\\implot\\backends "
        config["src_dirs"].append(".\\external\\implot")
    if platform.system() == "Linux":
        pass
    if platform.system() == "Darwin":
        pass


if __name__ == "__main__":
    tcc()
    opengl()
    imgui()
    implot()
    compile(config)

    
