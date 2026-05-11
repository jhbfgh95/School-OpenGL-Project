# CMake, Assimp, DLL 공부 노트

이 문서는 OpenGL 프로젝트를 세팅하면서 자주 헷갈리는 `CMake`, `Assimp`, `DLL`, `정적 라이브러리`, `vcpkg`, `PATH` 개념을 공부하기 위해 정리한 문서입니다.

현재 프로젝트 기준으로 설명합니다.

## 큰 그림

이 프로젝트는 대략 이런 구조로 빌드됩니다.

```text
소스 코드
  src/main.cpp
  src/glad.c

헤더 파일
  include/

리소스
  shaders/
  resource/

빌드 설정
  CMakeLists.txt
  vcpkg.json

외부 라이브러리
  Assimp
  GLFW
  OpenGL
  GLAD
  GLM
  stb_image
```

여기서 역할을 나누면 다음과 같습니다.

```text
CMake
  프로젝트를 어떻게 빌드할지 정리하는 도구

Assimp
  FBX, OBJ, GLTF, GLB 같은 3D 모델 파일을 읽어주는 라이브러리

DLL
  실행 파일이 실행 중에 불러오는 동적 라이브러리 파일

vcpkg
  Assimp, GLFW 같은 C/C++ 라이브러리를 설치하고 CMake에 연결해주는 패키지 관리자
```

## 1. CMake란 무엇인가

CMake는 컴파일러가 아닙니다.

CMake는 "빌드 파일을 만들어주는 도구"입니다.

예를 들어 Windows에서는 여러 빌드 방식이 있습니다.

```text
Visual Studio
MinGW Makefiles
Ninja
NMake Makefiles
```

CMake는 `CMakeLists.txt`를 읽고, 선택한 generator에 맞는 빌드 파일을 만들어줍니다.

예를 들어 현재 프로젝트에서는 이런 명령을 씁니다.

```powershell
cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
```

이 명령의 의미는 다음과 같습니다.

```text
cmake
  CMake 실행

-B build
  build 폴더에 빌드 파일을 만들기

-S .
  현재 폴더를 소스 폴더로 사용하기

-G "MinGW Makefiles"
  MinGW용 Makefile을 만들기

-DCMAKE_TOOLCHAIN_FILE=...
  vcpkg와 연결하기

-DVCPKG_TARGET_TRIPLET=x64-mingw-static
  64비트 MinGW static 라이브러리 조합 사용하기
```

그 다음 실제 컴파일은 아래 명령이 합니다.

```powershell
cmake --build build
```

즉, 보통 CMake 빌드는 두 단계입니다.

```text
1단계 configure/generate
  cmake -B build -S .

2단계 build
  cmake --build build
```

## 2. CMakeLists.txt란 무엇인가

`CMakeLists.txt`는 CMake가 읽는 프로젝트 설정 파일입니다.

현재 프로젝트의 핵심 내용은 다음과 같습니다.

```cmake
cmake_minimum_required(VERSION 3.21)
project(OpenGL_Study LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

의미:

```text
cmake_minimum_required
  최소 CMake 버전 지정

project
  프로젝트 이름과 사용하는 언어 지정

CMAKE_CXX_STANDARD
  C++17 사용
```

소스 파일은 아래처럼 모읍니다.

```cmake
file(GLOB SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)
```

의미:

```text
src 폴더 안의 .cpp 파일과 .c 파일을 자동으로 SOURCES 목록에 넣기
```

실행 파일은 아래처럼 만듭니다.

```cmake
add_executable(main ${SOURCES})
```

의미:

```text
main.exe라는 실행 파일을 SOURCES로부터 만들기
```

헤더 경로는 아래처럼 추가합니다.

```cmake
target_include_directories(main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
```

의미:

```text
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Model/model.h"
```

같은 include를 찾을 때 `include/` 폴더를 보게 하는 설정입니다.

## 3. find_package란 무엇인가

현재 프로젝트에는 이런 코드가 있습니다.

```cmake
find_package(OpenGL REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG QUIET)
```

`find_package`는 외부 라이브러리를 찾는 CMake 명령입니다.

예를 들어:

```cmake
find_package(assimp CONFIG REQUIRED)
```

이 명령은 CMake에게 이렇게 말하는 것입니다.

```text
Assimp 라이브러리를 찾아주세요.
못 찾으면 configure를 실패시켜주세요.
```

`CONFIG`는 보통 vcpkg 같은 패키지 관리자가 설치한 CMake 설정 파일을 찾겠다는 뜻입니다.

Assimp가 제대로 설치되어 있으면 CMake는 이런 target을 사용할 수 있게 됩니다.

```cmake
assimp::assimp
```

이 target을 링크하면 include 경로, 라이브러리 경로, 필요한 추가 의존성 일부를 CMake가 같이 처리합니다.

## 4. target_link_libraries란 무엇인가

현재 프로젝트에는 이런 코드가 있습니다.

```cmake
target_link_libraries(main PRIVATE
    OpenGL::GL
    gdi32
    assimp::assimp
)
```

링크는 "내 실행 파일과 외부 라이브러리를 연결하는 과정"입니다.

컴파일은 `.cpp`를 `.obj`로 바꾸는 단계입니다.

링크는 여러 `.obj`와 라이브러리를 합쳐서 최종 `.exe`를 만드는 단계입니다.

예를 들어 코드에서 Assimp를 씁니다.

```cpp
#include <assimp/Importer.hpp>

Assimp::Importer importer;
```

이 코드는 컴파일할 때 헤더가 필요하고, 링크할 때 Assimp 라이브러리 본체가 필요합니다.

헤더만 있으면 컴파일은 될 수 있지만, 링크 단계에서 실패할 수 있습니다.

그래서 CMake에서 아래처럼 연결해야 합니다.

```cmake
target_link_libraries(main PRIVATE assimp::assimp)
```

## 5. Assimp란 무엇인가

Assimp는 Open Asset Import Library의 줄임말입니다.

3D 모델 파일을 읽기 위한 라이브러리입니다.

직접 FBX, OBJ, GLTF, GLB 파일 포맷을 전부 파싱하는 것은 매우 어렵습니다.

Assimp는 그 복잡한 일을 대신 해줍니다.

이 프로젝트에서는 `include/Model/model.h`에서 Assimp를 사용합니다.

대표 include:

```cpp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
```

모델 로드는 대략 이런 식입니다.

```cpp
Assimp::Importer importer;

const aiScene* scene = importer.ReadFile(
    path,
    aiProcess_Triangulate |
    aiProcess_FlipUVs |
    aiProcess_GenSmoothNormals |
    aiProcess_CalcTangentSpace |
    aiProcess_JoinIdenticalVertices
);
```

의미:

```text
Importer
  모델 파일을 읽는 객체

ReadFile
  파일 경로를 받아서 aiScene으로 변환

aiScene
  Assimp가 읽어낸 전체 모델 데이터
```

## 6. Assimp의 aiScene 구조

Assimp는 모델 파일을 읽으면 `aiScene`이라는 구조로 변환합니다.

대략 이런 구조입니다.

```text
aiScene
  mRootNode
    aiNode
      mesh index
      child nodes

  mMeshes
    aiMesh
      vertices
      normals
      texture coordinates
      faces

  mMaterials
    aiMaterial
      texture paths
      diffuse color
      specular data
```

현재 프로젝트의 흐름은 다음과 같습니다.

```text
Model::loadModel
  Assimp로 파일 읽기
  scene 획득
  root node부터 순회 시작

Model::processNode
  node가 가진 mesh index 확인
  각 mesh를 processMesh로 변환
  child node도 재귀적으로 처리

Model::processMesh
  aiMesh의 vertex, normal, uv, index 추출
  material에서 texture 정보 추출
  Mesh 객체 생성
```

즉, Assimp는 파일을 읽어주고, 프로젝트 코드는 그 데이터를 OpenGL이 그릴 수 있는 `Mesh` 형태로 바꿉니다.

## 7. Assimp 후처리 옵션

현재 사용하는 옵션:

```cpp
aiProcess_Triangulate |
aiProcess_FlipUVs |
aiProcess_GenSmoothNormals |
aiProcess_CalcTangentSpace |
aiProcess_JoinIdenticalVertices
```

각 의미는 다음과 같습니다.

```text
aiProcess_Triangulate
  모든 면을 삼각형으로 변환합니다.
  OpenGL에서는 보통 삼각형 단위로 그리기 때문에 중요합니다.

aiProcess_FlipUVs
  UV 좌표의 Y 방향을 뒤집습니다.
  이미지 좌표계와 OpenGL 텍스처 좌표계 차이를 맞출 때 사용합니다.

aiProcess_GenSmoothNormals
  normal이 없으면 부드러운 normal을 생성합니다.

aiProcess_CalcTangentSpace
  tangent/bitangent를 계산합니다.
  나중에 normal map 같은 것을 쓸 때 필요합니다.

aiProcess_JoinIdenticalVertices
  같은 vertex를 합칩니다.
  메모리 사용량을 줄이고 index 기반 렌더링에 유리합니다.
```

## 8. DLL이란 무엇인가

DLL은 Dynamic Link Library의 줄임말입니다.

Windows에서 실행 중에 불러오는 라이브러리 파일입니다.

예:

```text
assimp-vc143-mt.dll
glfw3.dll
libstdc++-6.dll
libgcc_s_seh-1.dll
libwinpthread-1.dll
```

실행 파일인 `main.exe`가 실행될 때 필요한 DLL을 찾지 못하면 이런 오류가 날 수 있습니다.

```text
The code execution cannot proceed because xxx.dll was not found.
```

한국어 Windows에서는 대략 이런 식으로 나옵니다.

```text
xxx.dll이 없어 코드 실행을 진행할 수 없습니다.
```

## 9. 정적 라이브러리와 동적 라이브러리

C/C++ 라이브러리는 크게 두 종류로 생각하면 됩니다.

```text
정적 라이브러리
  .lib, .a

동적 라이브러리
  .dll
```

Windows/MSVC 기준:

```text
.lib
  정적 라이브러리일 수도 있고, DLL import library일 수도 있습니다.

.dll
  실행 시 필요한 동적 라이브러리입니다.
```

MinGW 기준:

```text
.a
  정적 라이브러리 또는 import library입니다.

.dll
  실행 시 필요한 동적 라이브러리입니다.
```

정적 링크와 동적 링크의 차이:

```text
정적 링크
  라이브러리 코드가 exe 안에 포함됩니다.
  실행 시 별도 DLL이 덜 필요합니다.
  exe 용량이 커질 수 있습니다.

동적 링크
  exe가 DLL을 실행 중에 불러옵니다.
  exe 용량은 줄 수 있지만 DLL 배포가 필요합니다.
```

## 10. 현재 프로젝트의 x64-mingw-static

현재 프로젝트는 vcpkg triplet으로 이 값을 사용합니다.

```text
x64-mingw-static
```

의미:

```text
x64
  64비트

mingw
  MinGW 컴파일러 계열

static
  가능한 정적 라이브러리로 빌드
```

그래서 Assimp와 GLFW를 되도록 정적으로 링크합니다.

이 방식의 장점:

```text
DLL 복사 문제가 줄어듭니다.
실행 파일 배포가 단순해질 수 있습니다.
MinGW 환경에서 기존 프로젝트와 잘 맞습니다.
```

단점:

```text
빌드 시간이 길어질 수 있습니다.
실행 파일 크기가 커질 수 있습니다.
모든 라이브러리가 완전히 static으로만 처리되지는 않을 수 있습니다.
```

## 11. PATH란 무엇인가

PATH는 Windows가 실행 파일을 찾는 경로 목록입니다.

예를 들어 PowerShell에서 이렇게 입력합니다.

```powershell
cmake --version
```

Windows는 현재 폴더와 PATH에 등록된 폴더들을 뒤져서 `cmake.exe`를 찾습니다.

MinGW를 쓰려면 보통 아래 경로가 PATH에 있어야 편합니다.

```text
C:\msys64\ucrt64\bin
```

vcpkg는 꼭 PATH에 없어도 됩니다.

현재 빌드 명령에서 이렇게 직접 경로를 넘기기 때문입니다.

```powershell
-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

## 12. DLL을 찾는 순서

Windows에서 exe가 DLL을 찾을 때 대표적으로 보는 위치는 다음과 같습니다.

```text
1. exe가 있는 폴더
2. 시스템 폴더
3. Windows 폴더
4. PATH에 등록된 폴더들
```

그래서 DLL 문제를 해결하는 흔한 방법은 두 가지입니다.

```text
방법 1
  필요한 DLL을 exe 옆에 복사합니다.

방법 2
  DLL이 있는 폴더를 PATH에 추가합니다.
```

프로젝트 배포 관점에서는 보통 방법 1이 더 명확합니다.

개발 PC 세팅 관점에서는 방법 2도 자주 씁니다.

하지만 현재 프로젝트는 `x64-mingw-static`을 사용하므로 DLL 문제를 최대한 줄이는 방향입니다.

## 13. 왜 vcpkg를 쓰는가

Assimp를 직접 설치하려면 보통 이런 것들을 직접 맞춰야 합니다.

```text
include 경로
library 경로
DLL 경로
Debug/Release 구분
x86/x64 구분
MSVC/MinGW 구분
static/dynamic 구분
```

이 중 하나만 틀려도 빌드가 실패하거나 실행이 실패할 수 있습니다.

vcpkg를 쓰면 이 과정을 많이 줄일 수 있습니다.

프로젝트는 `vcpkg.json`에 필요한 라이브러리를 적습니다.

```json
{
  "dependencies": [
    "assimp",
    "glfw3"
  ]
}
```

그리고 CMake 실행 시 vcpkg toolchain을 넘깁니다.

```powershell
-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

그러면 CMake의 `find_package(assimp CONFIG REQUIRED)`가 vcpkg 쪽에서 Assimp를 찾을 수 있습니다.

## 14. vcpkg triplet이란 무엇인가

triplet은 vcpkg가 어떤 방식으로 라이브러리를 빌드할지 정하는 이름입니다.

예:

```text
x64-windows
x64-windows-static
x64-mingw-dynamic
x64-mingw-static
```

대략 의미:

```text
x64-windows
  64비트 Windows, 보통 MSVC, 동적 링크 중심

x64-windows-static
  64비트 Windows, 보통 MSVC, 정적 링크 중심

x64-mingw-dynamic
  64비트 MinGW, 동적 링크 중심

x64-mingw-static
  64비트 MinGW, 정적 링크 중심
```

현재 프로젝트는 기존 빌드 캐시 기준으로 `x64-mingw-static`을 쓰고 있었습니다.

그래서 README와 문서에도 이 방식을 기준으로 적어두었습니다.

## 15. Generator란 무엇인가

CMake generator는 CMake가 어떤 빌드 시스템 파일을 만들지 정하는 옵션입니다.

예:

```text
MinGW Makefiles
Visual Studio 17 2022
Ninja
NMake Makefiles
```

현재 프로젝트 명령:

```powershell
-G "MinGW Makefiles"
```

이 옵션은 CMake에게 MinGW용 Makefile을 만들라고 지시합니다.

주의할 점:

한 번 `build/` 폴더를 어떤 generator로 만들면, 같은 폴더에서 다른 generator로 바꾸면 안 됩니다.

예를 들어:

```text
build/ 폴더를 Visual Studio generator로 생성
나중에 같은 build/ 폴더를 MinGW Makefiles로 다시 생성
```

이렇게 하면 generator 충돌 오류가 납니다.

해결 방법:

```text
1. build 폴더 삭제 후 다시 configure
2. build-mingw, build-msvc처럼 폴더를 나눠 사용
```

추천:

```powershell
cmake -B build-mingw -S . -G "MinGW Makefiles" ...
cmake -B build-msvc -S . -G "Visual Studio 17 2022" ...
```

## 16. include 경로와 library 경로

C++에서 외부 라이브러리를 쓰려면 보통 두 가지가 필요합니다.

```text
1. include 경로
2. library 링크
```

include 경로는 컴파일 단계에서 필요합니다.

예:

```cpp
#include <assimp/Importer.hpp>
```

컴파일러가 이 파일을 찾으려면 Assimp include 경로를 알아야 합니다.

library 링크는 링크 단계에서 필요합니다.

예:

```cpp
Assimp::Importer importer;
```

이 객체의 실제 구현 코드는 Assimp 라이브러리에 있습니다.

링커가 그 구현을 찾으려면 Assimp 라이브러리와 연결되어야 합니다.

CMake target을 잘 쓰면 이 둘을 한 번에 처리할 수 있습니다.

```cmake
target_link_libraries(main PRIVATE assimp::assimp)
```

## 17. GLAD, GLFW, GLM, stb_image의 역할

이 프로젝트에는 Assimp 외에도 여러 라이브러리가 있습니다.

```text
GLFW
  창 생성, OpenGL context 생성, 키보드/마우스 입력 처리

GLAD
  OpenGL 함수 포인터 로딩

GLM
  벡터, 행렬, 카메라, 변환 계산용 수학 라이브러리

stb_image
  이미지 파일을 읽어서 텍스처 데이터로 변환

Assimp
  3D 모델 파일을 읽어서 mesh/material/texture 정보로 변환
```

흐름으로 보면:

```text
GLFW
  창을 만들고 OpenGL을 사용할 준비를 함

GLAD
  OpenGL 함수를 실제로 호출 가능하게 로드함

Assimp
  모델 파일을 읽음

stb_image
  텍스처 이미지 파일을 읽음

GLM
  모델, 뷰, 프로젝션 행렬 계산

OpenGL
  최종적으로 화면에 그림
```

## 18. 현재 프로젝트에서 모델 로딩 흐름

현재 `src/main.cpp`에서는 모델 경로를 이렇게 잡습니다.

```cpp
std::string modelPath = resolvePath("resource/Models/CleGLTF.glb");
Model testModel(modelPath);
```

`Model` 생성자에서 Assimp가 파일을 읽습니다.

```cpp
Model(const std::string& path)
{
    minBounds = glm::vec3(std::numeric_limits<float>::max());
    maxBounds = glm::vec3(std::numeric_limits<float>::lowest());
    loadModel(path);
}
```

`loadModel` 내부에서:

```cpp
Assimp::Importer importer;
const aiScene* scene = importer.ReadFile(path, options);
```

그 다음:

```text
scene->mRootNode부터 순회
mesh 데이터 추출
texture 데이터 추출
OpenGL VAO/VBO/EBO로 넘길 수 있는 Mesh 객체 생성
```

## 19. 문제 상황별 원인

### CMake가 assimp를 못 찾는 경우

오류 예:

```text
Could not find a package configuration file provided by "assimp"
```

가능한 원인:

```text
vcpkg toolchain을 안 붙였습니다.
vcpkg에 assimp가 설치되지 않았습니다.
triplet이 기존 설정과 다릅니다.
```

해결:

```powershell
cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
```

### generator 충돌이 나는 경우

오류 예:

```text
Does not match the generator used previously
```

가능한 원인:

```text
같은 build 폴더에서 Visual Studio와 MinGW를 번갈아 사용했습니다.
```

해결:

```text
build 폴더를 지우거나 새 build 폴더를 씁니다.
```

예:

```powershell
cmake -B build-mingw -S . -G "MinGW Makefiles" ...
```

### DLL이 없다고 나오는 경우

오류 예:

```text
xxx.dll was not found
```

가능한 원인:

```text
동적 링크 라이브러리를 사용했는데 DLL이 exe 옆에 없습니다.
PATH에 DLL 폴더가 없습니다.
```

해결:

```text
DLL을 exe 옆에 복사합니다.
또는 DLL 폴더를 PATH에 추가합니다.
또는 static triplet을 사용합니다.
```

### 헤더 파일을 못 찾는 경우

오류 예:

```text
fatal error: assimp/Importer.hpp: No such file or directory
```

가능한 원인:

```text
include 경로가 안 잡혔습니다.
find_package 또는 target_include_directories 설정이 잘못되었습니다.
```

Assimp의 경우 보통 vcpkg toolchain 누락일 가능성이 큽니다.

## 20. 공부 순서 추천

처음부터 모든 것을 완벽하게 이해하려고 하면 어렵습니다.

추천 순서는 다음과 같습니다.

```text
1. Git
  clone, status, add, commit, push, pull

2. CMake 기본
  -B, -S, --build, CMakeLists.txt

3. C++ 빌드 과정
  compile, link, include path, library path

4. vcpkg
  vcpkg.json, toolchain, triplet

5. DLL과 static link
  .dll, .lib, .a, PATH

6. Assimp
  aiScene, aiNode, aiMesh, aiMaterial
```

이 프로젝트를 기준으로 공부하면 가장 먼저 이해할 명령은 이것입니다.

```powershell
cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
```

이 명령을 풀어서 설명할 수 있으면 CMake와 vcpkg의 큰 흐름은 잡힌 것입니다.

## 21. 핵심 요약

```text
CMake
  빌드 파일을 만들어주는 도구입니다.

CMakeLists.txt
  어떤 소스를 어떤 라이브러리와 연결해서 exe로 만들지 적는 파일입니다.

Assimp
  3D 모델 파일을 읽어주는 라이브러리입니다.

DLL
  exe가 실행 중에 불러오는 동적 라이브러리입니다.

정적 라이브러리
  빌드할 때 exe에 포함되는 방식의 라이브러리입니다.

vcpkg
  외부 C/C++ 라이브러리를 설치하고 CMake와 연결해주는 도구입니다.

PATH
  Windows가 실행 파일이나 DLL을 찾을 때 참고하는 경로 목록입니다.
```

현재 프로젝트는 `vcpkg + CMake + MinGW + x64-mingw-static` 조합을 기준으로 세팅되어 있습니다.

## 22. 프로그램 빌드란 무엇인가

프로그램 빌드는 소스 코드를 실행 가능한 파일로 바꾸는 전체 과정입니다.

현재 프로젝트에서는 최종 결과물이 보통 이것입니다.

```text
main.exe
```

C++ 프로그램이 `main.exe`가 되기까지는 크게 이런 단계를 거칩니다.

```text
1. 전처리
2. 컴파일
3. 어셈블
4. 링크
5. 실행
```

간단히 말하면:

```text
전처리
  #include, #define 같은 전처리 지시문을 처리합니다.

컴파일
  .cpp 파일을 기계어에 가까운 중간 파일로 바꿉니다.

어셈블
  컴파일 결과를 object 파일로 만듭니다.

링크
  여러 object 파일과 라이브러리를 합쳐 exe를 만듭니다.

실행
  만들어진 exe를 실행합니다.
```

실제로는 컴파일러가 여러 단계를 묶어서 처리하기 때문에, 사용자는 보통 `cmake --build build`만 실행합니다.

## 23. 현재 프로젝트의 빌드 흐름

현재 프로젝트의 빌드는 이런 흐름입니다.

```text
src/main.cpp
src/glad.c
include/
shaders/
resource/
vcpkg packages
  assimp
  glfw3

        |
        v

CMake configure

        |
        v

Makefile 생성

        |
        v

컴파일
  main.cpp -> main.cpp.obj
  glad.c   -> glad.c.obj

        |
        v

링크
  main.cpp.obj
  glad.c.obj
  OpenGL
  GLFW
  Assimp
  gdi32

        |
        v

main.exe
```

현재 CMake 빌드 명령은 다음과 같습니다.

```powershell
cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
cmake --build build
```

첫 번째 명령은 빌드 준비입니다.

```powershell
cmake -B build -S .
```

이 단계에서는 아직 프로그램을 완성하지 않습니다.

대신 CMake가 다음 정보를 확인합니다.

```text
컴파일러가 있는지
C++17을 쓸 수 있는지
Assimp를 찾을 수 있는지
GLFW를 찾을 수 있는지
OpenGL을 찾을 수 있는지
어떤 소스 파일을 빌드할지
어떤 라이브러리를 링크할지
```

두 번째 명령은 실제 빌드입니다.

```powershell
cmake --build build
```

이 단계에서 실제로 `.cpp`, `.c` 파일이 컴파일되고, 마지막에 `main.exe`가 만들어집니다.

## 24. configure와 build의 차이

CMake에서 가장 헷갈리는 부분이 configure와 build의 차이입니다.

```text
configure/generate
  빌드 방법을 준비하는 단계

build
  실제로 컴파일하고 exe를 만드는 단계
```

예를 들어:

```powershell
cmake -B build -S .
```

이 명령은 `build/` 폴더 안에 여러 파일을 만듭니다.

대표적으로:

```text
build/Makefile
build/CMakeCache.txt
build/CMakeFiles/
```

이 파일들은 "어떻게 빌드할지" 저장한 파일입니다.

그 다음:

```powershell
cmake --build build
```

이 명령이 `build/Makefile` 등을 사용해서 실제 빌드를 수행합니다.

## 25. CMakeCache.txt란 무엇인가

`CMakeCache.txt`는 CMake configure 결과를 저장하는 파일입니다.

예를 들어 이런 정보가 들어갑니다.

```text
사용한 generator
사용한 컴파일러
vcpkg toolchain 경로
Assimp 위치
GLFW 위치
빌드 옵션
```

그래서 한 번 `build/` 폴더를 만들면 CMake가 이전 설정을 기억합니다.

이것이 편할 때도 있지만, 문제를 만들 때도 있습니다.

예를 들어 처음에는 Visual Studio로 configure하고, 나중에 같은 `build/` 폴더에 MinGW로 configure하면 충돌이 납니다.

```text
Does not match the generator used previously
```

이럴 때는 새 빌드 폴더를 쓰는 것이 좋습니다.

```powershell
cmake -B build-mingw -S . -G "MinGW Makefiles" ...
```

또는 기존 `build/` 폴더를 삭제하고 다시 configure합니다.

## 26. Debug 빌드와 Release 빌드

프로그램은 보통 Debug 또는 Release 모드로 빌드할 수 있습니다.

```text
Debug
  디버깅에 유리합니다.
  최적화가 적습니다.
  실행 파일이 크고 느릴 수 있습니다.

Release
  실행 성능에 유리합니다.
  최적화가 들어갑니다.
  디버깅 정보가 적을 수 있습니다.
```

MinGW Makefiles 같은 single-config generator에서는 configure 때 빌드 타입을 지정합니다.

Debug:

```powershell
cmake -B build-debug -S . -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
cmake --build build-debug
```

Release:

```powershell
cmake -B build-release -S . -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
cmake --build build-release
```

Visual Studio generator 같은 multi-config generator에서는 build 단계에서 config를 지정하는 경우가 많습니다.

```powershell
cmake --build build --config Release
```

현재 프로젝트는 MinGW Makefiles 기준으로 설명하고 있으므로, `-DCMAKE_BUILD_TYPE=Release`처럼 configure 단계에서 지정하는 방식이 이해하기 쉽습니다.

## 27. 배치파일이란 무엇인가

배치파일은 Windows 명령어를 순서대로 실행하는 스크립트 파일입니다.

확장자는 보통 `.bat` 또는 `.cmd`입니다.

예:

```text
run.bat
build.bat
setup.bat
```

배치파일 안에는 PowerShell이 아니라 Windows CMD 명령어를 적습니다.

예를 들어:

```bat
@echo off
cd /d "%~dp0"
main.exe
```

이 배치파일은 다음 일을 합니다.

```text
@echo off
  실행되는 명령어 자체를 화면에 계속 출력하지 않게 합니다.

cd /d "%~dp0"
  배치파일이 있는 폴더로 이동합니다.

main.exe
  같은 폴더에 있는 main.exe를 실행합니다.
```

## 28. %~dp0이란 무엇인가

배치파일에서 `%~dp0`은 자주 보이는 특수 표현입니다.

의미는 다음과 같습니다.

```text
현재 실행 중인 배치파일이 있는 폴더 경로
```

예를 들어 `run.bat`이 아래 위치에 있다고 가정합니다.

```text
C:\OpenGL Personal Project\Demo\run.bat
```

그러면 `%~dp0`은 대략 아래 경로가 됩니다.

```text
C:\OpenGL Personal Project\Demo\
```

그래서:

```bat
cd /d "%~dp0"
```

이 명령은 "배치파일이 있는 폴더로 이동"이라는 뜻입니다.

`/d`는 드라이브까지 바꾸기 위한 옵션입니다.

예를 들어 현재 위치가 `D:` 드라이브인데 배치파일은 `C:` 드라이브에 있으면, 그냥 `cd`만으로는 드라이브 이동이 안 될 수 있습니다.

그래서 안전하게:

```bat
cd /d "%~dp0"
```

를 씁니다.

## 29. 실행용 run.bat 예시

빌드 결과물을 특정 폴더에 모아두고 실행하고 싶다면 이런 배치파일을 만들 수 있습니다.

```bat
@echo off
cd /d "%~dp0"
main.exe
pause
```

`pause`를 붙이면 프로그램이 바로 종료됐을 때 창이 닫히지 않고 메시지를 기다립니다.

예:

```text
Press any key to continue . . .
```

OpenGL 프로그램처럼 창이 뜨는 프로그램은 `pause`가 꼭 필요하지 않을 수 있습니다.

하지만 콘솔 로그를 보고 싶을 때는 유용합니다.

## 30. 빌드용 build.bat 예시

매번 긴 CMake 명령을 직접 치기 귀찮다면 배치파일로 만들 수 있습니다.

예를 들어 프로젝트 루트에 `build-mingw.bat`을 만든다면:

```bat
@echo off
cd /d "%~dp0"

cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
if errorlevel 1 (
    echo CMake configure failed.
    pause
    exit /b 1
)

cmake --build build
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

echo Build succeeded.
pause
```

이 배치파일은 다음 일을 합니다.

```text
1. 프로젝트 루트로 이동
2. CMake configure 실행
3. configure 실패 시 중단
4. CMake build 실행
5. build 실패 시 중단
6. 성공 메시지 출력
```

`if errorlevel 1`은 직전 명령이 실패했는지 확인하는 CMD 문법입니다.

## 31. build-and-run.bat 예시

빌드 후 바로 실행하고 싶다면 이런 식으로 만들 수 있습니다.

```bat
@echo off
cd /d "%~dp0"

cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
if errorlevel 1 (
    echo CMake configure failed.
    pause
    exit /b 1
)

cmake --build build
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

build\main.exe
```

다만 OpenGL 프로그램은 실행 위치가 중요할 수 있습니다.

현재 코드에는 `resolvePath` 함수가 있어서:

```text
shaders/shader.vs
resource/Models/CleGLTF.glb
```

같은 파일을 현재 폴더 또는 부모 폴더 기준으로 찾습니다.

그래서 프로젝트 루트에서 실행하면 리소스를 찾기 쉽습니다.

```bat
cd /d "%~dp0"
build\main.exe
```

이렇게 프로젝트 루트에서 `build\main.exe`를 실행하는 방식이 좋습니다.

## 32. 배치파일을 Git에 올릴 때 주의점

배치파일은 프로젝트 자동화에 유용하므로 Git에 올려도 됩니다.

예:

```text
build-mingw.bat
run.bat
build-and-run.bat
```

하지만 다음 파일들은 올리지 않는 것이 좋습니다.

```text
build/
*.exe
*.dll
CMakeCache.txt
```

즉, 배치파일은 올려도 되고, 배치파일이 만든 결과물은 올리지 않는 것이 좋습니다.

현재 `.gitattributes`에는 배치파일 줄바꿈을 Windows 방식으로 유지하는 설정도 있습니다.

```gitattributes
*.bat text eol=crlf
```

Windows CMD 배치파일은 CRLF 줄바꿈이 가장 무난합니다.

## 33. 프로그램 배포용 폴더를 만들 때

가끔 빌드 결과를 `Demo/` 같은 폴더에 모아서 전달하고 싶을 수 있습니다.

예:

```text
Demo/
  main.exe
  run.bat
  shaders/
  resource/
  필요한 DLL들
```

이런 폴더는 실행 테스트나 제출용으로는 유용합니다.

하지만 Git에 계속 올리는 것은 조심해야 합니다.

이유:

```text
exe와 dll은 빌드 결과물입니다.
소스가 바뀔 때마다 같이 바뀝니다.
용량이 커집니다.
충돌이 생기기 쉽습니다.
```

그래서 현재 `.gitignore`에서는 `Demo/`를 제외했습니다.

필요하면 나중에 release 파일로 따로 압축해서 전달하는 방식이 더 깔끔합니다.

GitHub에서는 보통 다음 방식을 씁니다.

```text
소스 코드
  Git repository에 push

실행 파일 묶음
  GitHub Releases에 zip으로 업로드
```

## 34. 빌드 실패를 읽는 방법

빌드가 실패하면 가장 중요한 것은 첫 번째 실제 오류를 찾는 것입니다.

출력이 길어도 아래 단어를 찾으면 됩니다.

```text
error:
fatal error:
undefined reference
cannot find
Could not find
No such file or directory
```

대표적인 오류 해석:

```text
No such file or directory
  헤더 파일이나 소스 파일 경로 문제일 가능성이 큽니다.

undefined reference
  링크 문제일 가능성이 큽니다.

Could not find assimp
  vcpkg toolchain 또는 Assimp 설치 문제일 가능성이 큽니다.

was not found
  실행 시 DLL 문제일 가능성이 큽니다.
```

빌드 오류는 크게 네 종류로 나눠서 보면 이해하기 쉽습니다.

```text
1. CMake configure 오류
  라이브러리를 못 찾거나 generator가 꼬인 경우

2. 컴파일 오류
  C++ 문법 오류, include 경로 문제

3. 링크 오류
  라이브러리 연결 문제

4. 실행 오류
  DLL 누락, 리소스 경로 문제
```

## 35. 현재 프로젝트에서 추천하는 작업 명령

처음 받은 뒤 한 번 빌드:

```powershell
cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
cmake --build build
```

코드만 조금 수정한 뒤 다시 빌드:

```powershell
cmake --build build
```

`CMakeLists.txt`나 `vcpkg.json`을 수정한 뒤:

```powershell
cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-mingw-static
cmake --build build
```

빌드 후 실행:

```powershell
.\build\main.exe
```

프로젝트 루트에서 실행하는 것이 리소스 경로 문제를 줄이는 데 좋습니다.
