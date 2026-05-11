# OpenGL Study

OpenGL, GLFW, GLAD, GLM, stb_image, Assimp를 사용하는 CMake 기반 개인 학습 프로젝트입니다.

## 새 작업환경 세팅

필요한 프로그램:

- Git
- CMake 3.21 이상
- C++ 컴파일러
  - 현재 프로젝트는 `lib/libglfw3.a`가 있어 MinGW 계열에서 바로 빌드할 수 있습니다.
  - Visual Studio/MSVC를 쓸 경우 vcpkg의 `glfw3` 패키지를 쓰는 쪽을 권장합니다.
- vcpkg

vcpkg가 없다면 한 번만 설치합니다.

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
```

현재 PC와 같은 MinGW/vcpkg static 조합으로 빌드합니다.

```powershell
git clone <repo-url>
cd "OpenGL Personal Project"

cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
cmake --build build
```

실행:

```powershell
.\build\main.exe
```

Visual Studio/MSVC generator를 쓰려면 `-G "MinGW Makefiles"`와 `-DVCPKG_TARGET_TRIPLET=x64-mingw-static`을 빼고, vcpkg 기본 triplet인 `x64-windows`로 다시 configure하면 됩니다. 단, 기존 `build/` 폴더는 generator가 섞이지 않도록 지우거나 다른 빌드 폴더를 사용해야 합니다.

## Git에 올리는 기준

Git에 포함할 파일:

- `src/`
- `include/`
- `shaders/`
- `resource/`
- `lib/libglfw3.a`
- `CMakeLists.txt`
- `vcpkg.json`
- `.gitignore`
- `README.md`

Git에 포함하지 않을 파일:

- `build/`
- `build-release/`
- `Demo/`
- `main.exe`
- `*.dll`
- `*.obj`
- `*.pdb`
- `CMakeCache.txt`

## 원격 저장소 연결

로컬 Git 초기화:

```powershell
git init
git add .
git commit -m "Initial commit"
```

GitHub/GitLab 등에서 빈 원격 저장소를 만든 뒤 연결합니다.

```powershell
git branch -M main
git remote add origin <repo-url>
git push -u origin main
```
