# Git 및 작업환경 세팅 정리

이 문서는 이 프로젝트를 GitHub에 올리기 위해 무엇을 했는지, 그리고 다른 작업환경에서 다시 세팅할 때 어떤 순서로 진행하면 되는지 정리한 파일입니다.

## 현재 프로젝트 상태

현재 프로젝트 폴더:

```text
C:\OpenGL Personal Project
```

연결된 GitHub 저장소:

```text
https://github.com/jhbfgh95/School-OpenGL-Project.git
```

현재 Git 브랜치:

```text
main
```

현재 로컬 저장소는 GitHub의 `origin/main`과 연결되어 있습니다.

## 이번에 한 작업 요약

이번에 한 작업은 크게 5가지입니다.

1. 현재 프로젝트 폴더를 Git 저장소로 만들었습니다.
2. Git에 올릴 파일과 올리지 않을 파일을 구분했습니다.
3. CMake/vcpkg 기반으로 다른 PC에서도 빌드할 수 있게 설정을 정리했습니다.
4. 로컬에서 initial commit을 만들었습니다.
5. GitHub 원격 저장소에 push했습니다.

## 1. Git 저장소 초기화

처음에는 이 프로젝트가 Git 저장소가 아니었습니다.

그래서 아래 명령과 같은 작업을 했습니다.

```powershell
git init
```

이 명령을 실행하면 프로젝트 폴더 안에 숨김 폴더인 `.git/`이 생깁니다.

`.git/` 폴더는 Git이 변경 이력, commit 정보, 원격 저장소 정보 등을 관리하는 공간입니다.

## 2. Git에 올릴 파일과 제외할 파일 정리

OpenGL/C++ 프로젝트는 빌드하면 실행 파일, 임시 파일, 캐시 파일이 많이 생깁니다.

예를 들면 아래 같은 것들입니다.

```text
build/
build-release/
Demo/
main.exe
CMakeCache.txt
CMakeFiles/
*.obj
*.pdb
*.dll
```

이런 파일들은 Git에 올리지 않는 것이 좋습니다.

이유는 다음과 같습니다.

- 다른 PC에서 다시 빌드하면 새로 만들어지는 파일입니다.
- 용량이 커질 수 있습니다.
- 컴퓨터마다 경로나 컴파일러 정보가 달라서 충돌이 생길 수 있습니다.
- GitHub 저장소가 지저분해집니다.

그래서 `.gitignore` 파일을 만들었습니다.

`.gitignore`는 Git에게 "이 파일들은 추적하지 마세요"라고 알려주는 파일입니다.

현재 제외되는 대표 항목은 다음과 같습니다.

```gitignore
build/
build-*/
Demo/
main.exe
*.exe
*.dll
*.obj
*.pdb
CMakeCache.txt
CMakeFiles/
```

반대로 Git에 올리는 파일은 다음과 같습니다.

```text
src/
include/
shaders/
resource/
lib/libglfw3.a
CMakeLists.txt
vcpkg.json
README.md
.gitignore
.gitattributes
```

즉, Git에는 "소스 코드 + 리소스 + 빌드 방법"을 올리고, "빌드 결과물"은 올리지 않는 구조입니다.

## 3. 줄바꿈 및 binary 파일 처리

`.gitattributes` 파일도 추가했습니다.

이 파일은 Git이 파일 종류를 더 안정적으로 다루게 해줍니다.

예를 들어 코드 파일은 텍스트로, 모델/이미지/라이브러리 파일은 binary로 취급하게 했습니다.

대표 설정은 다음과 같습니다.

```gitattributes
*.cpp text eol=lf
*.h text eol=lf
*.json text eol=lf
*.md text eol=lf

*.fbx binary
*.glb binary
*.jpg binary
*.a binary
```

이 설정을 해두면 Windows와 다른 환경을 오갈 때 줄바꿈 문제나 binary 파일 diff 문제가 줄어듭니다.

## 4. CMake 설정 정리

기존 `CMakeLists.txt`는 대략 이런 형태였습니다.

```cmake
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)
find_package(assimp CONFIG REQUIRED)
target_link_libraries(main glfw3 opengl32 gdi32 assimp::assimp)
```

동작은 하지만, 다른 환경에서 재현하기에는 조금 불안정할 수 있습니다.

그래서 다음 방향으로 정리했습니다.

- CMake 최소 버전을 `3.21`로 올렸습니다.
- C/C++ 프로젝트임을 명시했습니다.
- C++17을 명확히 설정했습니다.
- `find_package(OpenGL REQUIRED)`를 사용하게 했습니다.
- `find_package(assimp CONFIG REQUIRED)`로 Assimp를 vcpkg에서 찾게 했습니다.
- `find_package(glfw3 CONFIG QUIET)`로 vcpkg의 GLFW를 우선 사용하게 했습니다.
- vcpkg GLFW가 없으면 기존 `lib/libglfw3.a`를 fallback으로 쓰게 했습니다.

즉, 현재 구조는 다음과 같습니다.

```text
vcpkg에 glfw3가 있으면:
  vcpkg glfw3 사용

vcpkg에 glfw3가 없으면:
  lib/libglfw3.a 사용
```

Assimp는 반드시 vcpkg 같은 CMake 패키지 경로에서 찾아야 합니다.

## 5. vcpkg.json 추가

`vcpkg.json` 파일을 추가했습니다.

현재 내용은 다음과 같습니다.

```json
{
  "name": "opengl-study",
  "version-string": "0.1.0",
  "dependencies": [
    "assimp",
    "glfw3"
  ]
}
```

이 파일은 "이 프로젝트는 assimp와 glfw3가 필요합니다"라고 vcpkg에 알려주는 파일입니다.

다른 PC에서 CMake를 실행할 때 vcpkg toolchain을 붙이면, vcpkg가 이 파일을 보고 필요한 패키지를 준비합니다.

## 6. 빌드 검증

현재 PC의 기존 빌드 설정을 확인해보니 다음 조합을 쓰고 있었습니다.

```text
Generator: MinGW Makefiles
Compiler: C:/msys64/ucrt64/bin/g++.exe
vcpkg: C:/vcpkg
triplet: x64-mingw-static
```

그래서 아래 명령으로 새 빌드 폴더에서 configure와 build를 검증했습니다.

```powershell
cmake -B build-gitcheck -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
cmake --build build-gitcheck
```

최종적으로 빌드가 성공했습니다.

```text
[100%] Built target main
```

`build-gitcheck/`는 검증용 빌드 폴더이며, `.gitignore`에 의해 Git에는 올라가지 않습니다.

## 7. commit 생성

Git에 올릴 파일들을 추가했습니다.

```powershell
git add .
```

그 다음 initial commit을 만들었습니다.

```powershell
git commit -m "Initial project setup"
```

생성된 commit:

```text
bb46d34 Initial project setup
```

commit은 그 시점의 프로젝트 상태를 저장한 스냅샷이라고 보시면 됩니다.

## 8. 브랜치 이름 main으로 변경

처음 Git 저장소를 만들면 기본 브랜치 이름이 `master`일 수 있습니다.

요즘 GitHub에서는 보통 `main`을 기본 브랜치로 많이 씁니다.

그래서 브랜치 이름을 `main`으로 변경했습니다.

```powershell
git branch -M main
```

## 9. GitHub 원격 저장소 연결

사용자께서 만든 GitHub 저장소 URL:

```text
https://github.com/jhbfgh95/School-OpenGL-Project.git
```

이 URL을 `origin`이라는 이름으로 연결했습니다.

```powershell
git remote add origin https://github.com/jhbfgh95/School-OpenGL-Project.git
```

여기서 `origin`은 보통 "내 프로젝트의 기본 원격 저장소"라는 뜻으로 쓰는 이름입니다.

연결 상태는 아래 명령으로 확인할 수 있습니다.

```powershell
git remote -v
```

## 10. GitHub로 push

로컬 commit을 GitHub에 올렸습니다.

```powershell
git push -u origin main
```

이 명령은 로컬 `main` 브랜치를 GitHub의 `main` 브랜치로 업로드합니다.

`-u` 옵션은 앞으로 `git push`만 입력해도 `origin/main`으로 push되도록 연결해주는 옵션입니다.

현재 상태는 다음과 같습니다.

```text
local main -> origin/main
```

## 앞으로 작업할 때 기본 흐름

평소 작업 흐름은 보통 이렇습니다.

```powershell
git status
git add .
git commit -m "작업 내용 설명"
git push
```

각 명령의 의미는 다음과 같습니다.

```powershell
git status
```

현재 어떤 파일이 수정되었는지 확인합니다.

```powershell
git add .
```

현재 변경된 파일들을 다음 commit에 포함할 준비를 합니다.

```powershell
git commit -m "작업 내용 설명"
```

변경사항을 하나의 기록으로 저장합니다.

```powershell
git push
```

로컬 commit을 GitHub에 업로드합니다.

## 다른 PC에서 처음 받을 때

다른 작업환경에서는 먼저 GitHub 저장소를 clone합니다.

```powershell
git clone https://github.com/jhbfgh95/School-OpenGL-Project.git
cd School-OpenGL-Project
```

그 다음 vcpkg가 없다면 설치합니다.

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
```

그리고 프로젝트를 빌드합니다.

```powershell
cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
cmake --build build
```

실행합니다.

```powershell
.\build\main.exe
```

## 다른 PC에서 작업 후 올릴 때

다른 PC에서 코드를 수정한 뒤에는 다음 순서로 올리면 됩니다.

```powershell
git status
git add .
git commit -m "작업 내용 설명"
git push
```

## 다른 PC에서 최신 코드 가져올 때

GitHub에 올라간 최신 코드를 현재 PC로 가져오려면 다음 명령을 씁니다.

```powershell
git pull
```

작업 시작 전에 `git pull`을 먼저 하는 습관을 들이면 충돌 가능성이 줄어듭니다.

추천 흐름은 다음과 같습니다.

```powershell
git pull
```

작업합니다.

```powershell
git status
git add .
git commit -m "작업 내용 설명"
git push
```

## CMake 빌드 폴더 주의점

CMake는 빌드 폴더에 generator 정보를 저장합니다.

예를 들어 한 번 `Visual Studio 17 2022`로 configure한 폴더에 나중에 `MinGW Makefiles`로 다시 configure하면 오류가 납니다.

그래서 generator를 바꿀 때는 기존 빌드 폴더를 지우거나, 새 빌드 폴더를 써야 합니다.

예시:

```powershell
cmake -B build-mingw -S . -G "MinGW Makefiles" ...
```

```powershell
cmake -B build-msvc -S . -G "Visual Studio 17 2022" ...
```

서로 다른 컴파일러나 generator는 빌드 폴더를 분리하는 것이 좋습니다.

## PATH에 대해

이 프로젝트에서 중요한 경로는 다음과 같습니다.

```text
C:\vcpkg
C:\msys64\ucrt64\bin
```

현재 MinGW 빌드를 쓰려면 `mingw32-make`, `g++` 등이 실행 가능해야 합니다.

`C:\msys64\ucrt64\bin`이 PATH에 들어있으면 편합니다.

확인은 PowerShell에서 이렇게 할 수 있습니다.

```powershell
g++ --version
mingw32-make --version
cmake --version
```

각 명령이 정상적으로 버전을 출력하면 기본 도구 경로는 잡힌 것입니다.

## Assimp는 어디서 오는가

Assimp는 프로젝트 폴더 안에 직접 들어있는 라이브러리가 아닙니다.

현재는 vcpkg를 통해 가져오는 구조입니다.

즉, 다른 PC에서 Assimp를 직접 다운로드해서 PATH에 넣기보다, 아래 CMake 명령에서 vcpkg toolchain을 붙이는 방식으로 해결합니다.

```powershell
-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

그리고 `vcpkg.json`에 있는 dependencies를 보고 vcpkg가 `assimp`를 준비합니다.

## GLFW는 어디서 오는가

GLFW는 두 가지 경로가 있습니다.

1. vcpkg의 `glfw3`
2. 프로젝트 안의 `lib/libglfw3.a`

CMake는 vcpkg의 `glfw3`를 먼저 찾습니다.

못 찾으면 프로젝트 안의 `lib/libglfw3.a`를 사용합니다.

## 자주 쓰는 Git 명령어

현재 상태 확인:

```powershell
git status
```

변경사항 추가:

```powershell
git add .
```

commit 만들기:

```powershell
git commit -m "메시지"
```

GitHub로 올리기:

```powershell
git push
```

GitHub에서 최신 코드 받기:

```powershell
git pull
```

commit 기록 보기:

```powershell
git log --oneline
```

원격 저장소 확인:

```powershell
git remote -v
```

## 작업할 때 추천 습관

작업 시작 전:

```powershell
git pull
```

작업 중간 확인:

```powershell
git status
```

작업이 어느 정도 끝났을 때:

```powershell
git add .
git commit -m "무엇을 했는지 짧게 설명"
git push
```

## 지금 기억하면 되는 핵심

Git에는 빌드 결과물이 아니라, 소스 코드와 빌드 방법을 올립니다.

이 프로젝트에서 빌드 방법은 다음 파일들이 담당합니다.

```text
CMakeLists.txt
vcpkg.json
README.md
```

GitHub에는 이미 현재 프로젝트가 올라가 있습니다.

다른 PC에서는 다음 순서만 기억하면 됩니다.

```powershell
git clone https://github.com/jhbfgh95/School-OpenGL-Project.git
cd School-OpenGL-Project
cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
cmake --build build
```

작업 후에는 다음 순서입니다.

```powershell
git status
git add .
git commit -m "작업 내용"
git push
```
