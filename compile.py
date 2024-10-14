import subprocess
import os
from colorama import Fore, init
import platform
import sys

cwd_win      = os.getcwd()
cwd_bash     = cwd_win.replace("\\", "/")
cwd_unix     = cwd_win.replace("\\", "/")
choco        = "\"C:\\ProgramData\\chocolatey\\bin\\choco.exe\""
make         = "\"C:\\ProgramData\\chocolatey\\bin\\make.exe\""
make_bash    = "C:/ProgramData/chocolatey/bin/make.exe"
cmake        = "\"C:\\Program Files\\CMake\\bin\\cmake.exe\""
cmake_bash   = "C/Program Files/CMake/bin/cmake.exe"
openssl      = "\"C:\\Program Files\\OpenSSL-Win64\\bin\\openssl.exe\""
mingw32_make = "\"C:\\ProgramData\\mingw64\\mingw64\\bin\\mingw32-make.exe\""
mingw32_make_bash = "C:/ProgramData/mingw64/mingw64/bin/mingw32-make.exe"
gpp          = "\"C:\\ProgramData\\mingw64\\mingw64\\bin\\g++.exe\""
gcc          = "\"C:\\ProgramData\\mingw64\\mingw64\\bin\\gcc.exe\""
gcc_bash     = "C/ProgramData/mingw64/mingw64/bin/gcc.exe"
git          = "\"C:\\Program Files\\Git\\bin\\git.exe\""
bash_bash    = "C:/Program Files/Git/bin/bash.exe"
_7zip        = "C:\\Program Files\\7-Zip\\7z.exe"
cl           = "\"C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat\""
vcpkg        = f"\"{cwd_win}\\external\\vcpkg\\vcpkg.exe\""

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
def bash(script: str) -> bool:
    return cmd(f"\"C:\\Program Files\\Git\\bin\\bash.exe\" -c \"{script}\"", shell=True, capture_output=True, text=True)

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
            "cpp": "-std=c++17 "      # Additional flags specific to C++
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
if platform.system() == "Linux":
    config = {
        "compiler": {
            "c_compiler": "gcc",    # C compiler
            "cpp_compiler": "g++"   # C++ compiler
        },
        "cflags": {
            "common": "-Wall",       # Common flags for both C and C++ files
            "c": "",                 # Additional flags specific to C
            "cpp": "-std=c++17 "     # Additional flags specific to C++
        },
        "ldflags": "",               # Linking flags (for libraries)
        "output": {
            "object_dir": "./obj",   # Directory for object files
            "binary_dir": "./bin",   # Directory for the output executable
            "binary_name": "main"    # Name of the output binary
        },
        "src_dirs": ["./src"],       # Allow multiple source directories
        "src_files": []              # Allow individual source files
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
            if not cmd(compile_cmd, shell=True):
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
            if not cmd(compile_cmd, shell=True):
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

        if not cmd(compile_cmd, shell=True):
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

    if not cmd(link_cmd, shell=True):
        print(f"Failed to link object files.")
        return False

    print(f"Compilation successful. Executable created at {output_executable}")
    return True

class program:
    def choco():
        if not cmd(f"{choco} --version", shell=True):
            cmd("powershell -Command \"Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))\"", shell=True)  
            cmd("Import-Module $env:ChocolateyInstall\\helpers\\chocolateyProfile.psm1", shell=True)
            cmd("refreshenv", shell=True)
        if not cmd(f"{choco} --version", shell=True):
            print(Fore.RED + "could not install choco")
            sys.exit()
    def make():
        if not bash(f"make --version"):
            if not cmd(f"{make} --version", shell=True):
                program.choco()
                cmd("choco install make  --installargs 'ADD_MAKE_TO_PATH=System' -y", shell=True)
                if not cmd(f"{make} --version", shell=True):
                    print(Fore.RED + "could not install make")
                    sys.exit()
            env = os.environ.copy()
            env["PATH"] = f"{make}\\..;{env['PATH']}"
            if not bash(f"make --version"):
                print(Fore.RED + "could not install make")
                sys.exit()
    def gpp():
        if not bash(f"g++ --version"):
            if not cmd(f"{gpp} --version", shell=True):
                program.choco()
                cmd("choco install mingw  --installargs 'ADD_MINGW_TO_PATH=System' -y", shell=True)
                if not cmd(f"{gpp} --version", shell=True):
                    print(Fore.RED + "could not install gpp")
                    sys.exit()
                env = os.environ.copy()
                env["PATH"] = f"{gpp}\\..;{env['PATH']}"
                if not bash(f"g++ --version"):
                    print(Fore.RED + "could not install g++")
                    sys.exit()
    def gcc():
        if not bash(f"gcc --version"):
            if not cmd(f"{gcc} --version", shell=True):
                program.choco()
                cmd("choco install mingw --installargs 'ADD_MINGW_TO_PATH=System' -y", shell=True)
                if not cmd(f"{gcc} --version", shell=True):
                    print(Fore.RED + "could not install gcc")
                    sys.exit()
            env = os.environ.copy()
            env["PATH"] = f"{gcc}\\..;{env['PATH']}"
            if not bash(f"gcc --version"):
                print(Fore.RED + "could not install gcc")
                sys.exit()
    def mingw32_make():
        if platform.system() == "Windows":
            if not bash(f"mingw32-make --version"):
                if not cmd(f"{mingw32_make} --version", shell=True):
                    program.choco()
                    cmd("choco install mingw --installargs 'ADD_MINGW_TO_PATH=System' -y", shell=True)
                    if not cmd(f"{mingw32_make} --version", shell=True):
                        print(Fore.RED + "could not install mingw32_make")
                        sys.exit()
                env = os.environ.copy()
                env["PATH"] = f"{mingw32_make}\\..;{env['PATH']}"
                if not bash(f"mingw32-make --version"):
                    print(Fore.RED + "could not install mingw32_make")
                    sys.exit()
        if platform.system() == "Linux":
            pass
        if platform.system() == "Darwin":
            pass
    def git():
        if platform.system() == "Windows":
            if not bash(f"git --version"):
                if not cmd(f"{git} --version", shell=True):
                    program.choco()
                    cmd("choco install git --force --installargs 'ADD_GIT_TO_PATH=System' -y", shell=True)
                    if not cmd(f"{git} --version", shell=True):
                        print(Fore.RED + "could not install git")
                        sys.exit()
                env = os.environ.copy()
                env["PATH"] = f"{git}\\..;{env['PATH']}"
                if not bash(f"git --version"):
                    print(Fore.RED + "could not install git")
                    sys.exit()
        if platform.system() == "Linux":
            if not cmd("git --version", shell=True):
                cmd("sudo apt-get update", shell=True)
                cmd("sudo apt-get install git", shell=True)
                cmd("source ~/.bashrc")
                if not cmd("git --version"):
                    print("could not install git")
                    sys.exit()
        if platform.system() == "Darwin":
            sys.exit()
    def cmake():
        if not bash(f"cmake --version"):
            if not cmd(f"{cmake} --version", shell=True):
                program.choco()
                cmd("choco uninstall cmake -y",  shell=True)
                cmd("choco install cmake --force --installargs 'ADD_CMAKE_TO_PATH=System' -y",  shell=True)
                if not cmd(f"{cmake} --version", shell=True):
                    print(Fore.RED + "could not install cmake")
                    sys.exit()
            env = os.environ.copy()
            env["PATH"] = f"{cmake}\\..;{env['PATH']}"
            if not bash(f"cmake --version"):
                print(Fore.RED + "could not install cmake")
                sys.exit()
    def openssl():
        if not cmd(f"{openssl} --version", shell=True):
            program.choco()
            cmd("choco uninstall openssl -y", shell=True)
            cmd("choco install openssl --force --installargs 'ADD_OPENSSL_TO_PATH=System' -y", shell=True)
            if not cmd(f"{openssl} --version", shell=True):
                print(Fore.RED + "could not install openssl")
                sys.exit()
    def _7zip():
        if not bash(f"7z"):
            if not cmd(f"\"{_7zip}\"", shell=True):
                program.choco()
                cmd("choco uninstall 7zip -y",  shell=True)
                cmd("choco install 7zip --force -y",  shell=True)
                if not cmd(f"\"{_7zip}\"", shell=True):
                    print(Fore.RED + "could not install 7zip")
                    sys.exit()
            env = os.environ.copy()
            env["PATH"] = f"{_7zip}\\..;{env['PATH']}"
            if not bash(f"7z"):
                print(Fore.RED + "could not install 7zip")
                sys.exit()
    def msvc():
        if not bash(f"cl --version"):
            program.choco()
            cmd("choco install visualstudio2022buildtools -y --package-parameters \"--add Microsoft.VisualStudio.Workload.VCTools\"", shell=True)
            cmd("\"C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvarsall.bat\" x64", shell=True)
            if not bash(f"cl --version"):
                print(Fore.RED + "could not install msvc")
                sys.exit()
    def vcpkg():
        if not cmd(f"{vcpkg} --version", shell=True):
            program.git()
            if os.path.exists(f"{cwd_win}\\external\\vcpkg"):
                bash(f"rm -rf {cwd_win}\\external\\vcpkg")
            cmd(f"mkdir {cwd_win}\\external\\vcpkg", shell=True)
            cmd(f"git clone https://github.com/microsoft/vcpkg.git {cwd_win}\\external\\vcpkg", shell=True)
            cmd(f".\\bootstrap-vcpkg.bat", cwd=f"{cwd_win}\\external\\vcpkg", shell=True)
            if not cmd(f"{vcpkg} --version", shell=True):
                print(Fore.RED + "could not install vcpkg")
                sys.exit()

class library:
    def tcc() -> bool:
        if platform.system() == "Windows":
            if not os.path.exists(f"{cwd_win}\\include") \
            or not os.path.exists(f"{cwd_win}\\lib") \
            or not os.path.exists(f"{cwd_win}\\libtcc") \
            or not os.path.exists(f"{cwd_win}\\libtcc.dll"):
                
                program.make()
                program.gpp()
                program.gcc()
                program.git()
                
                cmd(f"powershell -Command \"Remove-Item -Recurse -Force {cwd_win}\\external\\installs\\tcc\n", capture_output=True, shell=True)
                os.makedirs(f"{cwd_win}\\external\\installs\\tcc")
                cmd(f"git clone https://github.com/Tiny-C-Compiler/mirror-repository {cwd_win}\\external\\installs\\tcc", capture_output=True, text=True)
                os.makedirs(f"{cwd_win}\\external\\installs\\tcc\\build")

                configure_prefix = (os.path.abspath(os.path.join("external", "installs", "tcc", "build"))).replace("\\", "/")
                bash(f"cd {cwd_bash}/external/installs/tcc && ./configure --prefix={configure_prefix}")
                bash(f"cd {cwd_bash}/external/installs/tcc && make")
                bash(f"cd {cwd_bash}/external/installs/tcc && make install")
                bash(f"cp -ru {cwd_bash}/external/installs/tcc/build/libtcc.dll {cwd_bash}/libtcc.dll")
                bash(f"cp -ru {cwd_bash}/external/installs/tcc/build/libtcc {cwd_bash}")
                bash(f"cp -ru {cwd_bash}/external/installs/tcc/build/include {cwd_bash}")
                bash(f"cp -ru {cwd_bash}/external/installs/tcc/build/lib {cwd_bash}")

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
                
                program.git()
                    
                cmd(f"powershell -Command \"Remove-Item -Recurse -Force {cwd_win}\\external\\installs\\glfw.zip\"", shell=True)
                cmd(f"curl -L -o {cwd_win}\\external\\installs\\glfw.zip https://sourceforge.net/projects/glfw/files/glfw/3.3.10/glfw-3.3.10.bin.WIN64.zip/download", shell=True)
                cmd(f"powershell -Command \"Expand-Archive -Path {cwd_win}\\external\\installs\\glfw.zip -DestinationPath {cwd_win}\\external\\glfw\"", shell=True)
                    
                cmd(f"powershell -Command \"Remove-Item -Recurse -Force .\\external\\installs\\glew.zip\"", shell=True)
                cmd(f"curl -L -o {cwd_win}\\external\\installs\\glew.zip https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip/download", capture_output=True, text=True)
                cmd(f"powershell -Command \"Expand-Archive -Path {cwd_win}\\external\\installs\\glew.zip -DestinationPath {cwd_win}\\external\\glew\"", capture_output=True, text=True)

                if not os.path.exists(f"{cwd_win}\\external\\glew") \
                or not os.path.exists(f"{cwd_win}\\external\\glfw"):
                    print(Fore.RED + "Could not build OpenGL")
                    return False
                
                print(Fore.GREEN + "OpenGL buildt successfully in local folder\n")

            if not os.path.exists(f"{cwd_win}\\bin"):
                os.makedirs(f"{cwd_win}\\bin")
            bash(f"cp {cwd_bash}/external/glew/glew-2.1.0/bin/Release/x64/glew32.dll {cwd_bash}/bin")
            bash(f"cp {cwd_bash}/external/glfw/glfw-3.3.10.bin.WIN64/lib-mingw-w64/glfw3.dll {cwd_bash}/bin")
            config["cflags"]["common"] += f" -I{cwd_win}\\external\\glfw\\glfw-3.3.10.bin.WIN64\\include -I{cwd_win}\\external\\glew\\glew-2.1.0\\include "
            config["ldflags"] += f" -L{cwd_win}\\external\\glfw\\glfw-3.3.10.bin.WIN64\\lib-mingw-w64 -L{cwd_win}\\external\\glew\\glew-2.1.0\\lib\\Release\\x64 " 
            config["ldflags"] += " -lglew32 -lglfw3 -lopengl32 -lgdi32 " 
            return True
        if platform.system() == "Linux":

            cmd("sudo apt-get update", shell=True)
            cmd("sudo apt-get install libglfw3-dev libglew-dev", shell=True)

            if not os.path.exists(f"{cwd_win}\\bin"):
                os.makedirs(f"{cwd_win}\\bin")
            config["ldflags"] += " -lglfw -lGLEW -lGL "
        if platform.system() == "Darwin":
            sys.exit()
    def imgui():
        if platform.system() == "Windows":
            program.git()
            if not os.path.exists(f"{cwd_win}\\external\\imgui"):
                cmd(f"git clone https://github.com/adobe/imgui.git {cwd_win}\\external\\imgui",  shell=True)
            config["cflags"]["common"] += f" -I{cwd_win}\\external\\imgui -I{cwd_win}\\external\\imgui\\backends "
            config["src_dirs"].append(f"{cwd_win}\\external\\imgui")
            config["src_files"].append(f"{cwd_win}\\external\\imgui\\backends\\imgui_impl_glfw.cpp")
            config["src_files"].append(f"{cwd_win}\\external\\imgui\\backends\\imgui_impl_opengl3.cpp")
        if platform.system() == "Linux":
            program.git()
            if not os.path.exists(f"{cwd_unix}/external/imgui"):
                cmd(f"git clone https://github.com/adobe/imgui.git {cwd_unix}/external/imgui",  shell=True)
            config["cflags"]["common"] += f" -I{cwd_unix}/external/imgui -I{cwd_unix}/external/imgui/backends "
            config["src_dirs"].append(f"{cwd_unix}/external/imgui")
            config["src_files"].append(f"{cwd_unix}/external/imgui/backends/imgui_impl_glfw.cpp")
            config["src_files"].append(f"{cwd_unix}/external/imgui/backends/imgui_impl_opengl3.cpp")
        if platform.system() == "Darwin":
            sys.exit()
    def implot():
        if platform.system() == "Windows":
            program.git()
            if not os.path.exists(f"{cwd_win}\\external\\implot"):
                cmd(f"git clone https://github.com/epezent/implot.git {cwd_win}\\external\\implot", shell=True)
            config["cflags"]["common"] += f" -I{cwd_win}\\external\\implot -I{cwd_win}\\external\\implot\\backends "
            config["src_dirs"].append(f"{cwd_win}\\external\\implot")
        if platform.system() == "Linux":
            program.git()
            if not os.path.exists(f"{cwd_unix}/external/implot"):
                cmd(f"git clone https://github.com/epezent/implot.git {cwd_unix}/external/implot", shell=True)
            config["cflags"]["common"] += f" -I{cwd_unix}/external/implot -I{cwd_unix}/external/implot/backends "
            config["src_dirs"].append(f"{cwd_unix}/external/implot")
        if platform.system() == "Darwin":
            sys.exit()
    def curl() -> bool:
        if platform.system() == "Windows":
                
            if not os.path.exists(f"{cwd_win}\\include\\curl\\curl.h") \
            or not os.path.exists(f"{cwd_win}\\lib\\libcurl.lib"):    
                
                program.cmake()
                program.openssl()
                program.mingw32_make()

                cmd(f"powershell -Command \"Remove-Item -Recurse -Force {cwd_win}\\external\\installs\\curl\n", shell=True)
                os.makedirs(f"{cwd_win}\\external\\installs\\curl")
                cmd(f"git clone https://github.com/curl/curl.git {cwd_win}\\external\\installs\\curl", capture_output=True, text=True)
                os.makedirs(f"{cwd_win}\\external\\installs\\curl\\build")

                bash(f"cd {cwd_bash}/external/installs/curl/build && cmake .. -G 'MinGW Makefiles' -DCMAKE_USE_OPENSSL=ON -DOPENSSL_ROOT_DIR='C:/Program Files/OpenSSL-Win64' -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCURL_STATICLIB=ON -DBUILD_CURL_EXE=OFF -DCMAKE_C_FLAGS='-w'")
                bash(f"cd {cwd_bash}/external/installs/curl/build && mingw32-make -j16")
                
                bash(f"cp -ru {cwd_bash}/external/installs/curl/build/lib/libcurl.lib {cwd_bash}/lib/libcurl.lib")
                bash(f"cp -ru {cwd_bash}/external/installs/curl/include {cwd_bash}")
            
                if not os.path.exists(f"{cwd_win}\\include\\curl\\curl.h") \
                or not os.path.exists(f"{cwd_win}\\lib\\libcurl.lib"):    
                    print(Fore.RED + "Could not install curl")
                    sys.exit()
    def boost():
        if platform.system() == "Windows":
            if not os.path.exists(f"{cwd_win}\\external\\boost") or True:
                program.make()
                program.gcc()
                program.cmake()
                program._7zip()
                program.msvc()

                #cmd(f"curl -L https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.zip --output {cwd_win}\\external\\installs\\boost\\boost_1_81_0.zip", shell=True)
                #cmd(f"\"{_7zip}\" x {cwd_win}\\external\\installs\\boost\\boost_1_81_0.zip -o{cwd_win}\\external\\installs\\boost\\boost_1_81_0", shell=True)
                cmd(f".\\bootstrap.bat",cwd=f"{cwd_win}\\external\\installs\\boost\\boost_1_81_0", shell=True)
                #if not os.path.exists(f"{cwd_win}\\external\\boost"): 
                #    os.makedirs(f"{cwd_win}\\external\\boost")
                cmd(f".\\b2 -d1 install --with-system --with-thread address-model=64 -j4 --prefix=..\\..\\..\\boost",cwd=f"{cwd_win}\\external\\installs\\boost\\boost_1_81_0", shell=True)

                
                if not os.path.exists(f"{cwd_win}\\external\\boost"):
                    print(Fore.RED + "Could not install boost")
                    sys.exit()

        if platform.system() == "Linux":
            pass
        if platform.system() == "Darwin":
            pass
    def websocket():
        if platform.system() == "Windows":
            if not os.path.exists(f"{cwd_win}\\external\\vcpkg\\installed\\x64-windows\\lib") or True:
                program.vcpkg()
                cmd(f"{vcpkg} install websocketpp:x64-windows-static", shell=True)
                cmd(f"{vcpkg} install boost-thread:x64-windows-static", shell=True)
                if not os.path.exists(f"{cwd_win}\\external\\vcpkg\\installed\\x64-windows-static\\lib"):
                    print(Fore.RED + "Could not install websocketpp:x64-windows-static")
                    sys.exit()

            config["cflags"]["common"] += f" -I{cwd_win}\\external\\vcpkg\\installed\\x64-windows-static\\include "
            config["ldflags"] += f" -lboost_system -lboost_thread -lpthread "

        if platform.system() == "Linux":
            if not cmd("dpkg -l | grep libboost-all-dev", shell=True) \
            or not cmd("dpkg -l | grep libwebsocketpp-dev", shell=True):
                cmd("sudo apt-get update", shell=True)
                cmd("sudo apt-get install libboost-all-dev", shell=True)
                cmd("sudo apt-get install libwebsocketpp-dev", shell=True)
                if not cmd("dpkg -l | grep libboost-all-dev", shell=True) \
                or not cmd("dpkg -l | grep libwebsocketpp-dev", shell=True):
                    print("could not install websocket")
                    sys.exit()
        if platform.system() == "Darwin":
            sys.exit()
    def json():
        if platform.system() == "Windows":
            if not os.path.exists(f"{cwd_win}\\external\\json"):
                program.git()
                cmd(f"git clone https://github.com/nlohmann/json.git {cwd_win}\\external\\json", capture_output=True, text=True)
                if not os.path.exists(f"{cwd_win}\\external\\json"):
                    print(Fore.RED + "Could not install json")
                    sys.exit()
            config["cflags"]["common"] += f" -I{cwd_win}\\external\\json\\include "
        if platform.system() == "Linux":
            if not os.path.exists(f"{cwd_unix}/external/json"):
                program.git()
                cmd(f"git clone https://github.com/nlohmann/json.git {cwd_unix}/external/json", shell=True, capture_output=True, text=True)
                if not os.path.exists(f"{cwd_unix}/external/json"):
                    print(Fore.RED + "Could not install json")
                    sys.exit()
            config["cflags"]["common"] += f" -I{cwd_unix}/external/json/include "
        if platform.system() == "Darwin":
            pass


if __name__ == "__main__":
    #library.tcc()
    library.opengl()
    library.imgui()
    library.implot()
    library.json()
    library.websocket()
    cmd("g++ --version", shell=True)

    compile(config)


