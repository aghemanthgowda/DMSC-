# The Tools & The Vision Science — OpenCV, dlib, YOLO, CMake & C++

> A beginner-to-mastery study guide for Hemanth. No prior programming knowledge is assumed. Every term is defined the first time it appears, and every idea is grounded in the **real source code** of this project: a C++ **Driver Monitoring System (DMS)** — a program that watches a driver through a camera and warns when they look drowsy, distracted, or are using a phone.

## What you'll learn

By the end of this guide you will be able to:

- Explain **what programming is**, what a **programming language** is, and **why this project is written in C++** (and how C++ compares to Python).
- Read the project's actual C++ code: **variables, types, functions, structs, classes, headers vs source files, `#include`, namespaces, references, `const`, `std::vector`**, and what **compiling and linking** mean.
- Understand **CMake** — the tool that turns source code into a runnable program — line by line from this project's real `CMakeLists.txt`.
- Understand **OpenCV**: the `cv::Mat` image, reading a webcam, drawing on frames, and the specific modules this project uses.
- Understand **dlib**: the **HOG face detector** and the **68-point facial landmark** model, including exactly which points are the eyes, mouth, nose, and jaw.
- Understand **the vision science**: the math behind **EAR** (eye closure), **PERCLOS**, **MAR** (yawns), **head pose** (`solvePnP`), and **gaze**.
- Understand **neural networks, YOLO, COCO, and ONNX** — how the phone detector works, and why a cigarette or seat belt is honestly reported as **UNKNOWN**.
- Understand **TCP sockets, HTTP, and MJPEG streaming** — how the dashboard is watched in a browser.
- See **how all these tools fit together** into one flowing pipeline, with a full glossary and cheat-sheet.

This is a long guide (about 25 printed pages). Take it a chapter at a time. You do **not** need to memorize anything — you need to be able to *follow the code and understand why each piece exists*.

---

## Table of contents

1. [What is programming, and why C++?](#chapter-1--what-is-programming-and-why-c)
2. [C++ essentials, through the project's own code](#chapter-2--c-essentials-through-the-projects-own-code)
3. [Build systems and CMake, from scratch](#chapter-3--build-systems-and-cmake-from-scratch)
4. [OpenCV in depth](#chapter-4--opencv-in-depth)
5. [dlib in depth: HOG faces and 68 landmarks](#chapter-5--dlib-in-depth-hog-faces-and-68-landmarks)
6. [The vision science: EAR, PERCLOS, MAR, head pose, gaze](#chapter-6--the-vision-science)
7. [Neural networks and object detection: YOLO, COCO, ONNX](#chapter-7--neural-networks-and-object-detection)
8. [Networking: TCP, HTTP, and MJPEG streaming](#chapter-8--networking-tcp-http-and-mjpeg-streaming)
9. [How all the tools fit together](#chapter-9--how-all-the-tools-fit-together)
10. [Glossary and cheat-sheet](#chapter-10--glossary-and-cheat-sheet)

---

## Chapter 1 — What is programming, and why C++?

### 1.1 What is a computer program?

A **computer** is a machine that does one thing extraordinarily fast: it follows instructions. A **program** is a list of those instructions, written down so precisely that a machine can carry them out with no guessing. When you open this project and run it, the program tells the computer: *"grab a picture from the camera, look for a face, measure the eyes, decide if the driver is drowsy, draw the result on screen, repeat 30 times a second."*

The catch: computers ultimately only understand **machine code** — raw numbers that represent operations like "add these two numbers" or "copy this value into that memory slot." Writing machine code by hand is inhuman. So we write in a **programming language** instead: a structured, human-readable notation with rules (its *syntax*) and meanings (its *semantics*). A separate tool then converts our human-readable text into machine code.

> **Analogy.** A programming language is like a recipe written in clear English. The computer is a very literal cook who only understands a private shorthand. A **compiler** (Chapter 3) is the translator that rewrites your English recipe into that shorthand before the cook starts.

### 1.2 What is a programming language?

A programming language gives you words and grammar to express ideas like:

- **Store a value** and give it a name ("let `earThreshold` be 0.21").
- **Make decisions** ("*if* the eye ratio is below the threshold, *then* mark the eyes as closed").
- **Repeat work** ("*while* the camera is running, process each frame").
- **Group instructions** into reusable named blocks called **functions**.

There are hundreds of languages. This project uses two ideas you'll hear about constantly: **C++** (the language the whole program is written in) and, indirectly, **Python** (used only to *export* some AI model files — never to run the live system).

### 1.3 Why does this project use C++?

A Driver Monitoring System has hard requirements that shaped the choice of language:

| Requirement | Why it matters here | Why C++ fits |
|---|---|---|
| **Speed** | The system must process ~30 camera frames every second and still have time to run face detection, landmark math, and sometimes a neural network — all before the next frame arrives. | C++ compiles directly to machine code with no interpreter in between, so it is among the fastest general-purpose languages. |
| **Runs on a small embedded board** | The production target mentioned in the code comments is an **i.MX 93** — a low-power embedded computer, not a big desktop. | C++ programs are small, self-contained, and run well on constrained hardware. |
| **Direct hardware access** | The program talks to a camera and, on the board, potentially a neural-network accelerator (NPU). | C++ can call operating-system and hardware interfaces directly. |
| **Predictable memory use** | An always-on safety system can't afford random pauses. | C++ gives the programmer precise control over memory; there is no unpredictable "garbage collector" pausing the program. |

You can see these motivations written directly into the project. In `src/main.cpp` the `--fast` option is described as making things *"smoother/low-latency on slow devices (i.MX 93)"* — real evidence that running on a small board drove the design.

### 1.4 A brief comparison to Python

You may have heard of **Python**. It is a wonderful *beginner* language and dominates AI research. The difference:

- **Python is interpreted.** A program called the *interpreter* reads your Python line by line and executes it as it goes. This is flexible and forgiving, but slower.
- **C++ is compiled.** The whole program is translated *once*, ahead of time, into machine code that then runs on its own at full speed.

This project uses that division exactly as you'd expect. The heavy real-time loop is **C++**. Python appears only in the documentation (`docs/MODELS.md`) for a *one-time* offline job: exporting a neural-network model to a file. For example the docs suggest `pip install ultralytics && yolo export model=yolov8n.pt format=onnx` — that runs Python once on a developer's laptop to *produce a file*, which the C++ program then loads and runs fast. **Python builds the model; C++ runs it in the car.**

---

## Chapter 2 — C++ essentials, through the project's own code

This chapter teaches just enough C++ to *read* the project. We'll use real snippets. Don't worry about writing C++ yet — aim to recognize what each piece is doing.

### 2.1 Variables and types

A **variable** is a named box that holds a value. In C++ every variable has a **type** — the kind of value it can hold. The type is written *before* the name. Here are the four types you'll meet most:

| Type | Holds | Example value |
|---|---|---|
| `int` | a whole number (integer) | `67`, `0`, `-1` |
| `double` | a number with a decimal point (a "double-precision" real number) | `0.21`, `25.0` |
| `bool` | a truth value: only `true` or `false` | `true` |
| `std::string` | a piece of text | `"models/pfld.onnx"` |

Look at the project's central settings file, `include/dms/Config.hpp`. Every knob the system can tune is a variable with a type and a starting value:

```cpp
double earThreshold = 0.21;          // fallback EAR threshold before calibration
int classIdPhone = 67;
bool mirror = true;                  // selfie-mirrored view
std::string headlessImagePath = "dms_frame.jpg"; // dashboard snapshot target
```

Reading line by line:

- `double earThreshold = 0.21;` — make a decimal-number box named `earThreshold`, put `0.21` in it. (The `//` starts a **comment** — text for humans that the computer ignores.)
- `int classIdPhone = 67;` — a whole-number box named `classIdPhone` holding `67`. (67 is the ID for "cell phone" — Chapter 7.)
- `bool mirror = true;` — a true/false box; when true, the video is flipped like a selfie mirror.
- `std::string headlessImagePath = "dms_frame.jpg";` — a text box holding a filename.

The `;` (semicolon) at the end of each line means "this instruction is finished" — like a full stop in a sentence.

### 2.2 Functions

A **function** is a named, reusable block of instructions. You *call* it to run it, optionally hand it some input values (**arguments**/**parameters**), and it can *return* a result. A function's first line (its **signature**) states: the type it returns, its name, and its parameters in parentheses.

Here is a small, real function from `src/FaceTracker.cpp`:

```cpp
double dist(const cv::Point2f& a, const cv::Point2f& b) {
    return cv::norm(a - b);
}
```

- `double` — this function **returns a `double`** (a decimal number).
- `dist` — its name (short for "distance").
- `(const cv::Point2f& a, const cv::Point2f& b)` — it takes **two** inputs, both 2-D points named `a` and `b`. (`cv::Point2f` is OpenCV's type for a point with an x and y coordinate; more on `cv::` and `&` and `const` below.)
- `return cv::norm(a - b);` — it computes the straight-line distance between the two points and hands that number back.

So `dist(p, q)` gives you the distance between points `p` and `q`. This one tiny function is the workhorse behind the eye and mouth measurements later.

### 2.3 Structs — grouping related values

Often several values belong together. A **struct** is a custom type that bundles multiple variables (its **members** or **fields**) under one name. Think of a struct as a labeled form with several blanks.

The project's per-frame results live in a struct in `include/dms/Types.hpp`:

```cpp
struct FaceObservation {
    bool faceDetected = false;
    int faceCount = 0;                    // number of faces seen this frame
    double confidence = 0.0;              // detector confidence for the driver face
    cv::Rect face;                        // driver face box (largest / tracked)
    std::vector<cv::Point2f> landmarks;   // 68 points when hasLandmarks == true
    double ear = -1.0;
    double mar = -1.0;                    // mouth-aspect-ratio
    double yaw = 0.0;                     // degrees, + = turned to driver's right
    // ... (more fields)
};
```

Every time the system looks at one camera frame, it fills in one `FaceObservation` — "did I see a face? how confident? where is it? what are the 68 landmark points? what's the eye ratio, mouth ratio, head angle?" Elsewhere in the code you'll see `obs.ear` or `obs.faceDetected`: the **dot** (`.`) reaches inside a struct to one of its fields. So `obs.faceDetected` means "the `faceDetected` field of the observation named `obs`."

Notice the `= false`, `= 0`, `= -1.0` defaults. If nobody fills a field in, it starts at a sensible value. The `-1.0` default for `ear` is a deliberate signal meaning *"not measured yet"* (a real eye ratio is always positive), which the later code checks with `if (obs.ear >= 0.0)`.

### 2.4 Classes — data plus the behavior that acts on it

A **class** is like a struct but it also bundles **functions** (here called **methods**) that operate on that data. A class is the blueprint; an actual thing built from the blueprint is an **object**. Classes usually keep some data **private** (hidden, internal) and expose a few **public** methods (the controls you're allowed to use).

The `FaceTracker` (in `include/dms/FaceTracker.hpp`) is a class that turns a camera frame into a `FaceObservation`:

```cpp
class FaceTracker {
public:
    explicit FaceTracker(const Config& cfg);
    bool init();
    FaceObservation process(const cv::Mat& frameBGR);
private:
    Config cfg_;
    cv::CascadeClassifier faceCascade_;
    // ... internal state ...
};
```

- `public:` — everything below is what the *outside world* may use.
  - `FaceTracker(const Config& cfg);` — the **constructor**: a special method that runs when a `FaceTracker` is created, handed the configuration.
  - `bool init();` — set up (load the face-detection models); returns `true` if it succeeded.
  - `FaceObservation process(const cv::Mat& frameBGR);` — the main job: give it a frame, get back an observation.
- `private:` — internal parts only the class itself touches. `cfg_` holds its copy of the configuration; `faceCascade_` is one of the detectors. (The trailing underscore `cfg_` is just a naming habit meaning "this is a private member.")

In `main.cpp` you can see the class *used*:

```cpp
FaceTracker tracker(cfg);        // build one, handing it the config
if (!tracker.init()) return 1;   // set it up; quit if that fails
// ... later, every frame:
FaceObservation obs = tracker.process(frame);
```

That's the whole point of a class: `main.cpp` doesn't need to know *how* faces are found — it just says `tracker.process(frame)` and gets a tidy result. This hiding of complexity behind a simple interface is called **encapsulation**, and it's why big programs stay manageable.

### 2.5 Header files (`.hpp`) vs source files (`.cpp`)

C++ splits code into two kinds of files:

- **Header files** (`.hpp` or `.h`) — the **declarations**: *what* exists. The names, types, and signatures of functions, structs, and classes — like a table of contents or a menu. Example: `include/dms/FaceTracker.hpp` announces that a `FaceTracker` class exists with a `process` method.
- **Source files** (`.cpp`) — the **definitions**: *how* it actually works. The real instruction bodies. Example: `src/FaceTracker.cpp` contains the actual code of `process`.

> **Analogy.** The header is the *menu* (dish names and what they contain). The source file is the *kitchen* (the actual cooking). Other parts of the program only read the menu; they don't need to be in the kitchen.

Why split at all? So that many source files can each *read the same menu* without copying the kitchen everywhere. When `main.cpp` wants to use a `FaceTracker`, it only needs to read the header to know the class's shape — it doesn't need the full source.

### 2.6 `#include` — pulling in a header

At the top of nearly every file you'll see lines starting with `#include`. This literally means "paste the contents of that header here so I can use the names it declares." From `src/FaceTracker.cpp`:

```cpp
#include "dms/FaceTracker.hpp"
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
```

- `#include "dms/FaceTracker.hpp"` — quotes mean "one of *our* headers." This is how the source file gets the menu it must match.
- `#include <opencv2/imgproc.hpp>` — angle brackets mean "a header from a **library** installed on the system" (here, OpenCV's image-processing module). A **library** is a bundle of pre-written, reusable code (Chapters 4, 5).

You'll also see this at the very top of every header:

```cpp
#pragma once
```

This is an **include guard**. It tells the compiler "if you've already pasted this header once, don't paste it again." Without it, a header included twice would define the same things twice and cause errors.

### 2.7 Namespaces — the project's `dms`

When many libraries are combined, two of them might use the same name (say, both define `Config`). A **namespace** is a labeled container for names that keeps them from colliding — like area codes for phone numbers. This whole project lives inside a namespace called `dms` (driver monitoring system):

```cpp
namespace dms {
    struct Config { /* ... */ };
} // namespace dms
```

Everything between `namespace dms {` and the closing `}` belongs to `dms`. From *inside* the namespace you can write `Config` directly. From *outside* you'd write `dms::Config`. The `::` is the **scope-resolution operator** — read it as "belonging to." You've already seen `cv::Mat` and `cv::Point2f`: those live in OpenCV's namespace, `cv`. And `std::string`, `std::vector` live in `std`, the C++ **standard library** (the toolbox that ships with the language).

`main.cpp` opens with `using namespace dms;`, which is a shortcut saying "assume `dms::` on our names so I can drop the prefix." That's why `main.cpp` can write `FaceTracker` and `Config` without `dms::`.

### 2.8 References (`&`) and `const`

Two small symbols appear everywhere and matter a lot for performance — which is central to a real-time system.

**A reference (`&`)** means "another name for an existing thing, *not a copy*." Look back at `process(const cv::Mat& frameBGR)`. A camera frame is a big image (hundreds of thousands of pixels). If we *copied* it every time we passed it to a function, we'd waste huge amounts of time. The `&` says "just let the function look at the original frame in place." This is called **pass by reference** and it's fast because nothing is duplicated.

**`const`** means "read-only — promise not to change this." So `const cv::Mat& frameBGR` reads as: *"a reference to a frame that I promise not to modify."* This is the ideal way to hand a big value to a function: `&` avoids the copy (fast), and `const` guarantees safety (the function can't accidentally alter your image). You'll see this pattern constantly:

```cpp
FaceObservation process(const cv::Mat& frameBGR);        // read the frame, don't copy or change it
double dist(const cv::Point2f& a, const cv::Point2f& b); // read two points cheaply
```

Sometimes you *do* want a function to change something you pass in. Then you use a **non-const reference**. In `DrowsinessDetector.cpp`:

```cpp
bool decideEyesClosed(double rawEar, double t, double& smoothedOut, double& thresholdOut);
```

Here `double& smoothedOut` is a reference *without* `const`: the function will *write* the smoothed EAR value back into the caller's variable through it. This is a common C++ way to return more than one value from a function — hand it references to fill in.

### 2.9 `std::vector` — a resizable list

Very often you don't have *one* of something, you have *a list*. A `std::vector` is C++'s standard growable array — an ordered list that can hold any number of items of one type. The item type goes in angle brackets:

- `std::vector<cv::Point2f>` — a list of 2-D points (used for the 68 landmarks).
- `std::vector<cv::Rect>` — a list of rectangles (used for detected eye boxes or object boxes).
- `std::vector<int>` — a list of whole numbers.

From `Types.hpp`:

```cpp
std::vector<cv::Point2f> landmarks;   // 68 points when hasLandmarks == true
```

You reach into a vector by position (its **index**), counting **from 0**. So `landmarks[0]` is the first point, `landmarks[36]` is the 37th, and `landmarks.size()` tells you how many it holds. In `FaceTracker.cpp`, after the model runs, the code checks `shapes[0].size() == 68` — "did we really get 68 points?" — before trusting them.

### 2.10 Compiling and linking

Finally, how does human-readable C++ become a running program? Two steps.

1. **Compiling.** A tool called a **compiler** reads each `.cpp` file (plus the headers it includes) and translates it into machine code, producing an **object file** (one blob of machine code per source file). If your grammar is wrong, the compiler stops here with an error.
2. **Linking.** A tool called the **linker** stitches all those object files together — plus the machine code of the libraries you used (OpenCV, dlib) — into one final **executable**: the actual runnable program. If you *used* a function but never provided or connected its code, the linker complains ("undefined reference").

> **Analogy.** Compiling is translating each *chapter* of a book into the reader's language separately. Linking is binding all the translated chapters (and the referenced appendices from other books) into one finished volume you can actually read cover to cover.

Doing this by hand for the 20-odd source files in this project — remembering every compiler flag and every library to link — would be miserable and error-prone. That is exactly the job of a **build system**, which is our next chapter.

---

## Chapter 3 — Build systems and CMake, from scratch

### 3.1 What is a build system?

A **build system** is a tool that *automates* compiling and linking. You describe your project once — "here are my source files, here are the libraries I need" — and the build system figures out the exact compiler and linker commands, in the right order, and runs them. It also only rebuilds what changed, saving time.

The build system this project uses is **CMake**. CMake is slightly special: it doesn't build directly. Instead it *generates* the instructions for whatever native build tool your machine has (on Linux that's usually `make`; on Windows, Visual Studio). This is why CMake is popular for **cross-platform** projects — the same `CMakeLists.txt` produces a working build on Linux, Windows, or the embedded board.

### 3.2 The recipe file: `CMakeLists.txt`

You describe your build in a file named exactly `CMakeLists.txt`. Let's read this project's real one (`/home/user/dmsc-/CMakeLists.txt`) in pieces.

```cmake
cmake_minimum_required(VERSION 3.16)
project(driver_monitoring_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

- `cmake_minimum_required(VERSION 3.16)` — "you need at least CMake 3.16 to build this."
- `project(driver_monitoring_system LANGUAGES CXX)` — names the project and says it's written in C++ (`CXX` is CMake's label for C++).
- `set(CMAKE_CXX_STANDARD 17)` — use the **C++17** version of the language (C++ has revisions: C++11, C++14, C++17...). This project needs C++17 features. The next two `set` lines make that requirement strict and turn off compiler-specific extensions, so the code stays standard and portable.

### 3.3 Finding libraries: `find_package`

Your program depends on outside libraries. `find_package` asks CMake to locate an installed library and learn where its headers and compiled code live. From the project:

```cmake
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs objdetect highgui videoio calib3d dnn)
```

Read this as: *"Find OpenCV on this machine. It is `REQUIRED` (fail the build if it's missing). Specifically I need these modules (`COMPONENTS`): core, imgproc, imgcodecs, objdetect, highgui, videoio, calib3d, dnn."* Each of those is a piece of OpenCV this project uses — Chapter 4 explains what each does. After this line succeeds, CMake gives you two handy variables to use later: `OpenCV_INCLUDE_DIRS` (where OpenCV's headers are) and `OpenCV_LIBS` (its compiled library files).

Further down, dlib is found — but with a clever twist:

```cmake
find_package(dlib CONFIG QUIET)
if(dlib_FOUND)
    set(DMS_DLIB_TARGET dlib::dlib)
    message(STATUS "DMS: dlib found via find_package")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/dlib/dlib/CMakeLists.txt")
    add_subdirectory("${CMAKE_SOURCE_DIR}/dlib/dlib" dlib_build)
    set(DMS_DLIB_TARGET dlib::dlib)
    message(STATUS "DMS: dlib compiled from source at ./dlib")
endif()
```

This says: *first* try to find an already-installed dlib (`find_package(dlib ...)`). If that fails but there's a copy of dlib's **source code** sitting in a `dlib/` folder inside the project, then `add_subdirectory(...)` tells CMake to **compile dlib itself as part of this build**. (`QUIET` means "don't complain loudly if not found," `message(STATUS ...)` prints a friendly note.) Chapter 5 explains *why* dlib is often compiled from source rather than just installed.

### 3.4 Declaring the program: `add_executable`

Once CMake knows the language and the libraries, you tell it what to build and from which files:

```cmake
add_executable(dms
    src/main.cpp
    src/Types.cpp
    src/ConfigManager.cpp
    src/FaceTracker.cpp
    src/GazeEstimator.cpp
    src/MjpegServer.cpp
    src/DrowsinessDetector.cpp
    src/DistractionDetector.cpp
    src/ObjectDetector.cpp
    # ... more source files ...
)
```

`add_executable(dms ...)` says: *"Build one runnable program called `dms`, compiled from this list of `.cpp` source files."* This is the list of "chapters" the compiler will translate and the linker will bind together into the final `dms` executable. Notice these are all `.cpp` (source) files — the headers are pulled in automatically by the `#include` lines inside them.

### 3.5 Connecting the pieces: `target_include_directories` and `target_link_libraries`

Two final jobs: tell the compiler where to find headers, and tell the linker which libraries to bind in.

```cmake
target_include_directories(dms PRIVATE ${CMAKE_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(dms PRIVATE ${OpenCV_LIBS})
```

- `target_include_directories(dms PRIVATE ...)` — "when compiling `dms`, also look for headers in our `include/` folder and in OpenCV's include folders." This is what makes `#include "dms/FaceTracker.hpp"` and `#include <opencv2/...>` resolve. (`${...}` inserts the value of a CMake variable; `PRIVATE` means this setting applies only to building `dms` itself.)
- `target_link_libraries(dms PRIVATE ${OpenCV_LIBS})` — "**link** the compiled OpenCV code into `dms`." This is the *linking* step from Chapter 2.10, expressed in the build recipe: without it, the program would compile but the linker couldn't find OpenCV's actual functions.

The recipe also links the **threads** library (for the streaming server's background thread) and, *only when dlib was found*, links dlib and defines a marker:

```cmake
find_package(Threads REQUIRED)
target_link_libraries(dms PRIVATE Threads::Threads)

if(DMS_DLIB_AVAILABLE)
    target_compile_definitions(dms PRIVATE DMS_HAVE_DLIB)
    target_link_libraries(dms PRIVATE ${DMS_DLIB_TARGET})
endif()
```

`target_compile_definitions(dms PRIVATE DMS_HAVE_DLIB)` defines a compile-time flag named `DMS_HAVE_DLIB`. Remember the `#ifdef DMS_HAVE_DLIB` lines in `FaceTracker.hpp` and `.cpp`? Those mean "**only compile this block if `DMS_HAVE_DLIB` is defined.**" So CMake and the source code cooperate: if dlib is available, CMake flips this switch on, and the dlib code paths get compiled in. If not, they're skipped and the program still builds (falling back to simpler detection). This is how one codebase gracefully adapts to what's installed — the same idea appears for the optional OpenCV `face` module (`DMS_HAVE_FACE`).

### 3.6 Out-of-source builds: the `build/` directory

When you actually build, you don't scatter the generated machine-code files among your source. Instead you make a separate folder — conventionally named `build/` — and run CMake from there. This is an **out-of-source build**. Typical commands:

```bash
mkdir build            # make a fresh folder for all generated files
cd build               # go into it
cmake ..               # ".." = "the project is in the parent folder"; generate the build
cmake --build .        # actually compile + link, producing the `dms` executable
```

- `cmake ..` reads the `CMakeLists.txt` in the parent directory (`..`) and *generates* the native build files inside `build/`.
- `cmake --build .` runs the compiler and linker.

Why keep it separate? Because everything generated — object files, the executable, CMake's own bookkeeping — lands in `build/`, leaving your actual source code clean. To start over completely, you just delete the `build/` folder; nothing precious is in there. Your source files are never touched by the build.

---

## Chapter 4 — OpenCV in depth

### 4.1 What is OpenCV?

**OpenCV** ("Open Source Computer Vision Library") is a huge, free library for working with images and video. **Computer vision** is the field of getting computers to extract meaning from pictures — where a face is, what an object is, how bright a scene is. OpenCV gives you thousands of ready-made building blocks so you don't reinvent them: reading a webcam, resizing an image, converting colors, drawing shapes, running neural networks, and much more. Everything OpenCV provides lives in the `cv::` namespace.

### 4.2 The heart of OpenCV: `cv::Mat`

The single most important OpenCV type is **`cv::Mat`** — short for "matrix." A `cv::Mat` is how OpenCV stores an image: a grid of numbers. Think of a photo as a spreadsheet where each cell is one **pixel** (picture element — a single dot of the image), and the number in the cell is that dot's brightness/color.

Key properties of a `cv::Mat`:

- **rows** and **cols** — the image's height and width in pixels. A 640×480 frame has 480 rows and 640 cols.
- **channels** — how many numbers per pixel. A grayscale image has **1 channel** (just brightness). A color image has **3 channels**.
- **BGR color order.** Here's a famous OpenCV quirk: color images are stored as **B, G, R** — **Blue, Green, Red** — *not* the usual R, G, B. Each channel is a number 0–255 (0 = none, 255 = full). So a bright red pixel is stored as (Blue=0, Green=0, Red=255). This is why the project's frame variables are named `frameBGR` — a constant, honest reminder of the byte order.

You've already seen `cv::Mat` everywhere: `FaceObservation process(const cv::Mat& frameBGR)` takes a color frame; `void publish(const cv::Mat& frameBGR)` in the streaming server takes the finished dashboard image. In `FaceTracker.cpp`, converting color to gray is a one-liner:

```cpp
cv::Mat gray;
cv::cvtColor(frameBGR, gray, cv::COLOR_BGR2GRAY);
```

- `cv::Mat gray;` — make an empty image to hold the result.
- `cv::cvtColor(frameBGR, gray, cv::COLOR_BGR2GRAY);` — **c**on**v**er**t** **color**: take the 3-channel BGR frame and produce a 1-channel gray version in `gray`. Many detectors work on grayscale because brightness alone is enough to find edges and shapes, and it's faster (one number per pixel instead of three).

### 4.3 Reading a webcam: `cv::VideoCapture`

To get frames from a camera, OpenCV gives you **`cv::VideoCapture`**. From `main.cpp`:

```cpp
cv::VideoCapture cap(cfg.cameraIndex);
if (!cap.isOpened()) {
    std::cerr << "Could not open camera index " << cfg.cameraIndex << " ...\n";
    return 1;
}
cap.set(cv::CAP_PROP_FRAME_WIDTH, cfg.captureWidth);
cap.set(cv::CAP_PROP_FRAME_HEIGHT, cfg.captureHeight);
```

- `cv::VideoCapture cap(cfg.cameraIndex);` — open camera number `cameraIndex` (0 is usually the first/built-in camera). `cap` now represents the live camera.
- `cap.isOpened()` — did it actually open? If not, print an error to `std::cerr` (the error output) and quit with code 1 (a non-zero exit code means "something went wrong").
- `cap.set(cv::CAP_PROP_FRAME_WIDTH, ...)` — request a capture size (here 640×480). `CAP_PROP_FRAME_WIDTH` is a named setting ("property").

Later, grabbing the next frame is conceptually `cap.read(frame)` — it fills a `cv::Mat` with the newest picture. (This project wraps that in a small `CameraGrabber` that reads on a background thread so a slow board always processes the *newest* frame instead of falling behind — but underneath it's still `cv::VideoCapture`.)

The same `main.cpp` also shows OpenCV image transforms used to handle real-world camera mounting:

```cpp
if (cfg.cameraRotation == 90) cv::rotate(frame, frame, cv::ROTATE_90_CLOCKWISE);
if (cfg.mirror) cv::flip(frame, frame, 1);
```

`cv::rotate` spins the whole image (for a camera mounted sideways); `cv::flip(frame, frame, 1)` mirrors it left-to-right so the video looks like a selfie.

### 4.4 Drawing on images

Once you've computed something, you usually want to *show* it by drawing on the frame. OpenCV's drawing functions all take a `cv::Mat` to draw onto, a shape, a **color** (a `cv::Scalar`, given in BGR order), and a **thickness**. The three you'll see most:

- **`cv::rectangle(img, rect, color, thickness)`** — draw a box. Used to outline the detected face or a phone.
- **`cv::circle(img, center, radius, color, thickness)`** — draw a circle. Used to mark landmark points or a pupil.
- **`cv::putText(img, text, position, font, scale, color, thickness)`** — write text onto the image. Used for the on-screen status labels ("DROWSY", the EAR value, the FPS counter).

A color like `cv::Scalar(0, 0, 255)` is (Blue=0, Green=0, Red=255) — pure red. Remember: BGR order. These calls are how the raw numbers behind the scenes become the visible dashboard the driver (or a tester) sees.

### 4.5 The OpenCV modules this project uses

OpenCV is split into **modules** — themed sub-libraries you can pick from. Recall the `find_package` line requested exactly eight. Here is what each does and where the project uses it:

| Module | What it does | Where this project uses it |
|---|---|---|
| **core** | The foundation: `cv::Mat`, `cv::Point`, `cv::Rect`, `cv::Scalar`, basic math like `cv::norm`. | Everywhere — every image and point. |
| **imgproc** | *Image processing*: color conversion, resize, blur, threshold, drawing. | `cv::cvtColor`, `cv::resize`, `cv::GaussianBlur`, `cv::threshold` in the trackers and gaze estimator. |
| **imgcodecs** | *Image codecs*: encode/decode image files (JPEG, PNG). | `cv::imencode(".jpg", ...)` to compress the dashboard for streaming; `cv::imwrite` to save snapshots. |
| **objdetect** | *Object detection*: classic detectors, incl. **Haar cascades** for faces/eyes. | `cv::CascadeClassifier` — the always-available fallback face/eye detector. |
| **highgui** | *High-level GUI*: create windows, show images, read keys. | `cv::namedWindow`, `cv::imshow`, `cv::waitKey` to display the live window. |
| **videoio** | *Video input/output*: cameras and video files. | `cv::VideoCapture` — the webcam. |
| **calib3d** | *Camera calibration & 3-D geometry*: relate 3-D world points to 2-D image points. | `cv::solvePnP`, `cv::Rodrigues` for **head-pose** estimation (Chapter 6). |
| **dnn** | *Deep neural networks*: load and run trained AI models. | `cv::dnn::readNetFromONNX`, `blobFromImage`, `forward`, `NMSBoxes` for the **YOLO phone detector** and the optional landmark model (Chapter 7). |

The beauty is that all eight ship inside OpenCV. The `dnn` module in particular means this project can run a neural network **without any separate AI runtime installed** — a key design point we'll return to.

### 4.6 Showing a window: `cv::imshow` and `cv::waitKey`

To display the live dashboard, `main.cpp` uses the highgui module:

```cpp
cv::imshow(win, canvas);
const int key = cv::waitKey(1) & 0xFF;
if (key == 'q' || key == 27) break;
```

- `cv::imshow(win, canvas);` — show the image `canvas` in the window named `win`. Call it every frame and you get live video.
- `cv::waitKey(1)` — wait up to 1 millisecond for a key press and return which key (if any). **This call is also what actually lets the window refresh** — a highgui window only updates during `waitKey`. It returns the key code; `& 0xFF` keeps just the meaningful low byte.
- `if (key == 'q' || key == 27) break;` — if the user pressed `q` or the Escape key (code 27), stop the loop. (Other keys toggle developer mode or trigger recalibration.)

### 4.7 Compressing an image: `cv::imencode`

The streaming server needs to send images over the network. Sending a raw `cv::Mat` would be enormous (a 640×480 color frame is ~900,000 bytes). So it's compressed to **JPEG** first. From `MjpegServer.cpp`:

```cpp
std::vector<uchar> jpeg;
const std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, jpegQuality_};
if (!cv::imencode(".jpg", frameBGR, jpeg, params)) return;
```

- `cv::imencode(".jpg", frameBGR, jpeg, params)` — **encode** the frame *in memory* as a JPEG. The `.jpg` says which format; the compressed bytes are written into the `jpeg` vector (`std::vector<uchar>` — a list of raw bytes; `uchar` = unsigned char = one byte 0–255).
- `params` sets the JPEG **quality** (1–100; higher = better image but bigger). The project defaults to 80.

Unlike `cv::imwrite` (which writes a file to disk), `imencode` compresses into memory so the bytes can be shipped straight over the network (Chapter 8). A typical JPEG frame is only ~20–50 KB — small enough to stream smoothly.

---

## Chapter 5 — dlib in depth: HOG faces and 68 landmarks

### 5.1 What is dlib?

**dlib** is another free C++ library, focused on machine learning and computer vision. This project uses dlib for the two most important perception steps, because dlib's tools here are more accurate than the classic OpenCV fallbacks:

1. **Finding the face** in the frame (the HOG face detector).
2. **Placing 68 precise points** on the face — the corners of the eyes, the outline of the lips, the bridge of the nose, the jawline (the 68-point shape predictor).

Those 68 points are the raw material for *every* downstream measurement: eye closure, yawns, head angle, gaze.

### 5.2 The HOG face detector (`frontal_face_detector`)

Before you can measure eyes, you must find the face. dlib's default face finder is built on **HOG** — **Histogram of Oriented Gradients**. Let's unpack that in plain words:

- A **gradient** is a change in brightness from one pixel to its neighbor — i.e., an **edge**. Where the image goes from light to dark (like the border of a nostril or an eyebrow), there's a strong gradient.
- "**Oriented**" means we also record the *direction* of each edge (horizontal, vertical, diagonal).
- A "**histogram**" is a tally. HOG chops the image into small cells and, in each cell, tallies up how many edges point in each direction.

The result is a compact "sketch" of the image's edge pattern. Human faces have a very consistent edge pattern (eyes, nose, mouth in fixed relative positions), so a detector trained on many faces can slide this HOG template across the image and light up wherever that face-like edge pattern appears. It's robust and needs no color — just shapes of light and dark.

In the code (`FaceTracker.cpp`), you create and run it like this:

```cpp
detector_ = dlib::get_frontal_face_detector();
// ... each frame:
dlib::cv_image<dlib::bgr_pixel> dsmall(small);
std::vector<dlib::rect_detection> dets;
detector_(dsmall, dets);  // fills 'dets' with detected face rectangles + confidence
```

- `dlib::get_frontal_face_detector()` — get dlib's ready-made, pre-trained HOG face detector. ("Frontal" = it expects a roughly face-on view.)
- `dlib::cv_image<dlib::bgr_pixel> dsmall(small);` — wrap an OpenCV `cv::Mat` so dlib can read it (a thin adapter; no copy of the pixels).
- `detector_(dsmall, dets);` — run the detector on the image; it fills `dets` with any faces found, each a rectangle plus a confidence score.

Notice the code runs detection on a *smaller* copy of the image (`small`) and only **every Nth frame** (a comment says this "roughly triples throughput on a laptop CPU"). Finding the face is the expensive step; measuring landmarks is cheap. So the project finds the face occasionally and re-measures the 68 points every frame — a classic real-time optimization. This is exactly the `faceDetectEveryNFrames` setting from `Config.hpp`.

### 5.3 The 68-point facial landmark predictor

Once you have a face rectangle, dlib's **shape predictor** places 68 labeled dots on the facial features inside it. The model that does this is a file named **`shape_predictor_68_face_landmarks.dat`** — a pre-trained model you download once (see `docs/MODELS.md` and `main.cpp`, which auto-detects `models/shape_predictor_68_face_landmarks.dat`). A `.dat` file is just the learned parameters saved to disk; dlib loads them and can then predict points on any new face.

Running it (`FaceTracker.cpp`):

```cpp
dlib::deserialize(cfg_.facemarkModel) >> predictor_;   // load the .dat once, at startup
// ... each frame:
const dlib::full_object_detection shape = predictor_(dfull, full);
if (shape.num_parts() == 68) {
    for (unsigned i = 0; i < 68; ++i) {
        obs.landmarks.emplace_back(static_cast<float>(shape.part(i).x()),
                                   static_cast<float>(shape.part(i).y()));
    }
    obs.hasLandmarks = true;
}
```

- `dlib::deserialize(...) >> predictor_;` — load the trained model file into `predictor_` (done once in `init()`).
- `predictor_(dfull, full)` — given the full frame `dfull` and the face rectangle `full`, predict the 68 points. `shape.part(i)` is the i-th point; `.x()` and `.y()` are its pixel coordinates.
- The loop copies all 68 points into `obs.landmarks` (our `std::vector<cv::Point2f>`), and sets `obs.hasLandmarks = true`. From here on, the rest of the system has 68 dots to compute from.

### 5.4 What the 68 points are — the index map

The 68 points follow a fixed, standard ordering known as the **iBUG 68** scheme. Because the order is fixed, the code can rely on specific index numbers always meaning the same feature. Remembering that vectors count from 0, here is the full map:

| Indices | Facial feature | Notes |
|---|---|---|
| **0–16** | Jawline | 17 points tracing the chin/jaw from ear to ear. Point **8** is the bottom of the chin. |
| **17–21** | Right eyebrow | (driver's right) |
| **22–26** | Left eyebrow | |
| **27–30** | Nose bridge | Point **30** is the **nose tip**. |
| **31–35** | Lower nose | The nostrils' base line. |
| **36–41** | **Right eye** | 6 points around the eye. |
| **42–47** | **Left eye** | 6 points around the eye. |
| **48–59** | Outer lip | The outer edge of the lips. Point **48** = right mouth corner, **54** = left mouth corner. |
| **60–67** | Inner lip | The inner edge (the mouth opening). |

The project uses exactly these indices. From `FaceTracker.cpp`:

```cpp
static const int L[6] = {36, 37, 38, 39, 40, 41};   // right eye's 6 points
static const int R[6] = {42, 43, 44, 45, 46, 47};   // left eye's 6 points
```

and for the mouth:

```cpp
const double vertical = dist(lm[61], lm[67]) + dist(lm[62], lm[66]) + dist(lm[63], lm[65]);
const double width = dist(lm[48], lm[54]);
```

Here `lm[61]`, `lm[67]`, etc. are **inner-lip** points (the mouth opening), and `lm[48]`, `lm[54]` are the mouth **corners** — precisely as the index map says. And for head pose, the code picks landmarks `30` (nose tip), `8` (chin), `36` and `45` (outer eye corners), `48` and `54` (mouth corners) — six well-spread, stable points. Understanding the index map is the key that unlocks Chapter 6.

### 5.5 Why dlib is compiled from source in this project

Recall from Chapter 3 that the `CMakeLists.txt` will **compile dlib itself** if it finds dlib's source in a `dlib/` folder. Why not just install a pre-built dlib like we do with OpenCV? The project's own comments explain it:

```cmake
# dlib is optional and found three ways ...
#   2) a local dlib SOURCE checkout compiled as part of this build - the most
#      reliable path on Windows (no vcpkg, no toolchain file):
#         git clone https://github.com/davisking/dlib.git
#      then just build; CMake compiles dlib automatically.
```

Reasons compiling from source is preferred here:

- **Reliability across platforms.** A pre-built library must exactly match your compiler and settings, which is fiddly (especially on Windows). Compiling dlib *with your project* guarantees they match — same compiler, same C++ version, same options.
- **Performance tuning.** When dlib is compiled on the target machine, it can be optimized for that machine's CPU features, making detection faster.
- **No extra package tools needed.** You just `git clone` dlib into the folder and build; CMake's `add_subdirectory` handles the rest. No separate installer or package manager required.

The trade-off is a longer *first* build (you're compiling a whole extra library), but it's a one-time cost that buys a build that "just works." And if dlib truly isn't present, the code's `#ifdef DMS_HAVE_DLIB` guards let the program still build and run using the OpenCV Haar-cascade fallback — just with fewer features.

---

## Chapter 6 — The vision science

Now we reach the mathematical heart: how do 68 dots become "the driver is drowsy"? Each detector turns geometry into a single meaningful number, then compares it to a threshold. Crucially, this project doesn't use *fixed* thresholds blindly — it *calibrates* to each driver. We'll see how.

### 6.1 EAR — Eye Aspect Ratio (are the eyes open?)

The **Eye Aspect Ratio (EAR)** is a beautifully simple idea: an open eye is *tall*; a closed eye is *flat*. If we measure the eye's height and divide by its width, we get a number that is relatively large when the eye is open and drops toward zero when it closes. Because it's a *ratio*, it doesn't care how far the face is from the camera — a small distant eye and a large near eye give the same EAR when equally open.

Each eye has 6 landmarks. Label them p1..p6 going around the eye (outer corner, top-outer, top-inner, inner corner, bottom-inner, bottom-outer). The formula is:

```text
        ‖p2 − p6‖ + ‖p3 − p5‖
EAR =  ─────────────────────────
              2 · ‖p1 − p4‖
```

In words: add the **two vertical distances** across the eyelids (top-to-bottom), and divide by **twice the horizontal width** (corner-to-corner). Here `‖a − b‖` means "distance between points a and b."

This maps *exactly* onto the project's code in `FaceTracker.cpp`:

```cpp
double eyeAspectRatio(const std::vector<cv::Point2f>& lm, const int idx[6]) {
    const double vertical = dist(lm[idx[1]], lm[idx[5]]) + dist(lm[idx[2]], lm[idx[4]]);
    const double horizontal = dist(lm[idx[0]], lm[idx[3]]);
    if (horizontal < 1e-6) return 0.0;
    return vertical / (2.0 * horizontal);
}
```

- `vertical` = distance(point1, point5) + distance(point2, point4) — the two eyelid gaps. (Indices `idx[1]`,`idx[5]` etc. — recall the eye's points are 36–41 or 42–47.)
- `horizontal` = distance(point0, point3) — the eye's width.
- `if (horizontal < 1e-6) return 0.0;` — a safety check: never divide by (nearly) zero.
- `return vertical / (2.0 * horizontal);` — the EAR formula, exactly as above.

The system computes EAR for both eyes and averages them:

```cpp
obs.earLeft = eyeAspectRatio(obs.landmarks, L);
obs.earRight = eyeAspectRatio(obs.landmarks, R);
obs.ear = 0.5 * (obs.earLeft + obs.earRight);
```

**Why EAR drops when eyes close:** as the eyelids come together, the vertical gaps (`p2−p6`, `p3−p5`) shrink toward zero while the width stays about the same. So the numerator collapses and EAR plunges. A typical open eye is around 0.3; a closed eye is near 0.1 or below. The default `earThreshold = 0.21` in `Config.hpp` sits between those.

**Self-calibration (this is the clever part).** People's eyes differ, and glasses and camera angles shift EAR. So instead of trusting one fixed number, `DrowsinessDetector.cpp` *learns each driver's own "eyes-open" baseline* — the recent maximum EAR — and sets the closed-threshold as a fraction of it:

```cpp
double baseline = 0.0;
for (const auto& e : earBaseline_) baseline = std::max(baseline, e.second);
double threshold = cfg_.earThreshold;
if (baseline > 0.15) {
    threshold = std::clamp(baseline * cfg_.earCloseRatio, 0.15, 0.30);
}
```

- It finds `baseline` = the largest recent smoothed EAR (that's "wide open" for *this* driver).
- Then `threshold = baseline * earCloseRatio` (the config's `earCloseRatio = 0.62`) — i.e. "call the eyes closed once EAR falls to 62% of this person's open value." `std::clamp(..., 0.15, 0.30)` keeps that threshold in a sane band.

It also **smooths** the raw EAR (averages the last few frames) to fight jitter, and uses **hysteresis** — it enters "closed" when EAR drops below the threshold but only leaves "closed" once EAR rises clearly *above* it (`threshold * 1.12`). Hysteresis prevents flickering right at the edge (like a thermostat that doesn't rapidly click on/off around the set temperature).

### 6.2 Blinks, sustained closure, and microsleep

A drop in EAR alone isn't dangerous — everyone blinks. What matters is *how long* the eyes stay closed. `DrowsinessDetector.cpp` times each closure from the moment EAR crosses the threshold:

```cpp
if (eyesClosed && !closed_) {              // just closed
    closed_ = true;
    closureStart_ = tSeconds;
} else if (!eyesClosed && closed_) {       // just reopened
    const double dur = tSeconds - closureStart_;
    if (dur >= cfg_.blinkMinSeconds && dur <= cfg_.blinkMaxSeconds) {
        ++blinkCount_;                      // a normal blink
    }
    if (dur >= cfg_.longBlinkSeconds) ++longBlink_;   // a fatigue-suggesting long blink
    closed_ = false;
}
```

- A closure lasting between `blinkMinSeconds` (0.06s) and `blinkMaxSeconds` (0.40s) counts as an ordinary **blink**.
- A closure longer than `longBlinkSeconds` (0.50s) is a **long blink** — a known drowsiness cue.
- A closure that *keeps going* past `eyeClosedDrowsySeconds` (0.60s) marks **Drowsy**, and past `eyeClosedAlarmSeconds` (1.20s) is a **microsleep** — a brief involuntary sleep — classified **Critical**. The scoring code enforces this explicitly:

```cpp
if (r.closureSeconds >= cfg_.eyeClosedAlarmSeconds || r.score >= 80.0) {
    r.level = DrowsyLevel::Critical;
}
```

A microsleep at highway speed means traveling many meters with eyes shut — which is exactly why it's the top alarm level.

### 6.3 PERCLOS — the percentage of eye closure over time

A single closure is one event; **fatigue** shows up as a *pattern* over time. **PERCLOS** stands for the **PERcentage of eye CLOSure** — the fraction of time, over a rolling window, that the eyes are closed. It's one of the most validated drowsiness measures in the research literature.

`DrowsinessDetector.cpp` keeps a rolling 60-second window of per-frame open/closed samples and computes the fraction closed:

```cpp
if (obs.faceDetected) {
    window_.emplace_back(tSeconds, eyesClosed);       // record (time, closed?) each frame
}
while (!window_.empty() && tSeconds - window_.front().first > cfg_.perclosWindowSeconds) {
    window_.pop_front();                              // forget samples older than 60s
}
int closedCount = 0;
for (const auto& s : window_) closedCount += s.second ? 1 : 0;
r.perclos = static_cast<double>(closedCount) / static_cast<double>(window_.size());
```

- It appends `(time, eyesClosed)` for each frame, and discards samples older than `perclosWindowSeconds` (60s) — that's the "rolling window."
- `closedCount / window.size()` is the fraction of recent frames with eyes closed — a value from 0 to 1.

The thresholds in `Config.hpp`: `perclosWarn = 0.15` (15% of the last minute closed → building fatigue) and `perclosAlarm = 0.30` (30% → drowsy). So even a driver who never has one long microsleep, but whose eyes are drifting shut 30% of the time, gets flagged.

### 6.4 MAR — Mouth Aspect Ratio (yawns)

Yawning is another fatigue signal, and it's measured just like EAR but for the mouth: the **Mouth Aspect Ratio (MAR)**. A closed mouth is flat (MAR near 0); a yawn is a tall opening (MAR rises). From `FaceTracker.cpp`:

```cpp
double mouthAspectRatio(const std::vector<cv::Point2f>& lm) {
    const double vertical =
        dist(lm[61], lm[67]) + dist(lm[62], lm[66]) + dist(lm[63], lm[65]);
    const double width = dist(lm[48], lm[54]);
    if (width < 1e-6) return 0.0;
    return vertical / (3.0 * width);
}
```

- `vertical` = the sum of **three** inner-lip gaps (points 61↔67, 62↔66, 63↔65) — how far the lips are apart.
- `width` = distance between the **outer** mouth corners (48↔54). The comment explains a subtle engineering choice: using the *stable outer* width as the denominator (rather than the wobbly inner corners) makes MAR ~0 for a closed mouth and rise smoothly as it opens — far less noisy.
- Divide by `3.0 * width` (three gaps, so divide by three times the width to normalize).

Turning MAR into a yawn (`DrowsinessDetector.cpp`) reuses the same smart ideas as EAR: **smoothing**, an **adaptive baseline** (here the rolling *minimum* MAR, since the mouth is closed most of the time), **hysteresis** (separate open/close thresholds), a **duration** requirement, and **dip tolerance** so brief landmark jitter during a yawn doesn't reset the timer:

```cpp
const double openThr = std::max(cfg_.marThreshold, baseline + cfg_.marOpenDelta);
// ... mouth considered open when smoothed MAR >= openThr ...
if (mouthOpen_ && !yawnCounted_ && tSeconds - mouthOpenStart_ >= cfg_.yawnMinSeconds) {
    ++yawnCount_;         // a real yawn: mouth held open long enough
    yawnCounted_ = true;
}
```

A "yawn" is only counted when the mouth stays open at least `yawnMinSeconds` (0.55s) — that distinguishes a yawn from talking or a quick "oh."

### 6.5 Head pose — yaw, pitch, roll via `cv::solvePnP`

To know if the driver is looking at the road or off to the side/down at a lap, we estimate **head pose** — the head's orientation in three angles:

- **Yaw** — turning left/right (shaking "no").
- **Pitch** — nodding up/down ("yes").
- **Roll** — tilting the head toward a shoulder.

The problem: the camera gives a flat 2-D image, but head orientation is a 3-D fact. The tool that bridges them is **`cv::solvePnP`** (from OpenCV's `calib3d` module). **PnP** means "**P**erspective-**n**-**P**oint": given a set of known **3-D model points** and where those same points appear in the **2-D image**, solve for the rotation and position of the object that would line them up. It's like reverse-engineering the camera angle from how a familiar shape looks distorted.

The project (`FaceTracker.cpp`) uses six landmarks and a generic 3-D face model:

```cpp
const std::vector<cv::Point3d> model = {
    {0.0, 0.0, 0.0},        // nose tip        (landmark 30)
    {0.0, -63.6, -12.5},    // chin            (landmark 8)
    {-43.3, 32.7, -26.0},   // left eye corner (landmark 36)
    {43.3, 32.7, -26.0},    // right eye corner(landmark 45)
    {-28.9, -28.9, -24.1},  // left mouth      (landmark 48)
    {28.9, -28.9, -24.1},   // right mouth     (landmark 54)
};
const std::vector<cv::Point2d> image = {
    lm[30], lm[8], lm[36], lm[45], lm[48], lm[54]};
```

- `model` — six points in an *idealized 3-D head* (a fixed reference face; the numbers are millimeter-like offsets from the nose).
- `image` — where those same six features *actually landed* in this frame (our landmarks 30, 8, 36, 45, 48, 54).

Then:

```cpp
cv::Mat rvec, tvec;
if (!cv::solvePnP(model, image, cameraMatrix, distCoeffs, rvec, tvec)) return;
cv::Mat rot;
cv::Rodrigues(rvec, rot);
// ... decompose into Euler angles ...
obs.pitch = normalize(euler.at<double>(0));
obs.yaw   = normalize(euler.at<double>(1));
obs.roll  = normalize(euler.at<double>(2));
```

- `cv::solvePnP(...)` computes `rvec` (rotation) and `tvec` (position) that best explain the 2-D points given the 3-D model and the camera parameters (`cameraMatrix` is built from the frame size — a reasonable estimate when the camera isn't precisely calibrated).
- `cv::Rodrigues(rvec, rot)` converts the compact rotation into a full rotation matrix, which is then decomposed into the three human-readable **Euler angles** — yaw, pitch, roll.

Once it has angles, `DistractionDetector.cpp` decides where the head is pointing using the thresholds from `Config.hpp` (`headAwayYawDegrees = 25`, `headDownPitchDegrees = 18`):

```cpp
if (emaYaw_ > cfg_.headAwayYawDegrees) r.headDir = "right";
else if (emaYaw_ < -cfg_.headAwayYawDegrees) r.headDir = "left";
else if (emaPitch_ < -cfg_.headDownPitchDegrees) r.headDir = "down";
// ...
else r.headDir = "forward";
```

(`emaYaw_` is a *smoothed* yaw — an **exponential moving average**, which gently follows the value while ignoring single-frame noise.)

**Why an angled camera mount needs calibration.** In a real car the camera is rarely dead-ahead of the driver — it's off to the side or above. So even when the driver looks straight at the road, the *raw* head pose reads as "turned." If we didn't correct for this, the system would scream "distracted!" constantly. The fix, in `main.cpp`, is a short **neutral-pose calibration**: for the first few seconds it averages the driver's normal "looking at the road" pose and stores it as the zero reference, then subtracts it from every later reading:

```cpp
if (cfg.headPoseCalibrate && calibratingNow && obs.hasHeadPose) {
    sumYaw += obs.yaw; sumPitch += obs.pitch; sumRoll += obs.roll; ++sumN;   // average during calibration
}
// ... after calibration:
if (neutSet && obs.hasHeadPose) {
    obs.yaw -= neutYaw; obs.pitch -= neutPitch; obs.roll -= neutRoll;        // make "forward" relative to the mount
}
```

Now "forward" means "the driver's normal road-watching pose *for this mount*," and only *departures* from it count as looking away. This is why `Config.hpp` documents `headPoseCalibrate` as making a side-mounted camera not read the neutral pose as "looking away."

### 6.6 Gaze / pupil approximation

Head pose tells you where the *head* points, but the eyes can look elsewhere. `GazeEstimator.cpp` makes a **coarse** estimate of eye direction by locating the darkest blob (the iris/pupil) inside each eye region and measuring how far off-center it sits.

The code is honest that this is *approximate* — the comment in `Types.hpp` notes "dlib's 68 points have no iris, so this is a coarse cue." The steps:

```cpp
cv::Mat eye = gray(roi);                                   // crop just the eye region
cv::GaussianBlur(eye, blurred, cv::Size(5, 5), 0);         // blur to reduce speckle
double minVal, maxVal; cv::Point minLoc, maxLoc;
cv::minMaxLoc(blurred, &minVal, &maxVal, &minLoc, &maxLoc); // find darkest spot
cv::threshold(blurred, darkMask, minVal + (maxVal - minVal) * 0.25, 255, cv::THRESH_BINARY_INV);
cv::Moments m = cv::moments(darkMask, true);               // centroid of the dark region
```

- Crop the eye using the six eye landmarks, blur it (the iris is the darkest thing there), find the darkest region, and take that region's **centroid** (its center of mass) as the pupil position — steadier than a single darkest pixel.
- Then it computes the pupil's offset from the eye-box center, normalized to a −1..+1 range:

```cpp
dx = (static_cast<double>(center.x) / roi.width - 0.5) * 2.0;   // -1 = far left, +1 = far right
dy = (static_cast<double>(center.y) / roi.height - 0.5) * 2.0;
```

If the smoothed offset exceeds `gazeOffThreshold` (0.28), the gaze is labeled "left/right/up/down"; otherwise "forward." Because it's approximate, gaze only *contributes* to the distraction score — it never triggers a hard alarm on its own. The system deliberately gates it: gaze is skipped entirely if the eyes are closed (`obs.eyesClosed`), because a closed eye gives a meaningless dark blob.

### 6.7 The philosophy: honesty over false confidence

A theme runs through all of Chapter 6: the system *knows what it can't know*. `Config.hpp` documents a whole "monitoring quality" gate — if the face is too small, the light too dim, or the head turned too far for landmarks to be reliable, it reports "**MONITORING QUALITY LOW**" instead of asserting drowsiness. As the config comment puts it, "it never false-alarms when it cannot actually see the driver's eyes." A safety system that cries wolf gets ignored; one that's honest about uncertainty gets trusted.

---

## Chapter 7 — Neural networks and object detection

Everything so far used geometry on landmarks. But to spot a **phone** in the frame, you need something different — a **neural network**. This chapter builds up the ideas from zero.

### 7.1 What is a neural network, and what is a "model"?

A **neural network** is a kind of program that *learns from examples* instead of being told exact rules. It's loosely inspired by brain cells: many simple units ("neurons") connected in layers, each connection having a tunable strength (a **weight**). You show the network thousands of labeled example images ("this is a phone," "this is not"), and a training process gradually adjusts all the weights until the network's guesses match the labels. This adjusting phase is **training**.

A **model** is the *finished, trained network* — specifically, the millions of learned weight numbers, saved to a file. Once trained, running the model on a *new* image to get a prediction is called **inference**. Key point for this project: **training happens once, offline, on a powerful machine (often in Python); inference happens live, in the car, in C++.** The DMS never trains; it only runs a pre-trained model file.

### 7.2 What is YOLO?

**YOLO** stands for "**You Only Look Once**." It's a famous family of neural networks for **object detection** — not just "is there a phone somewhere?" but "there's a phone *here*, at this exact box, and I'm 78% sure." The "look once" name refers to its speed trick: older detectors scanned an image many times at many positions; YOLO processes the whole image in a *single* pass, which makes it fast enough for real-time video.

For each object it finds, YOLO outputs three things:

1. A **bounding box** — four numbers (center-x, center-y, width, height) locating the object.
2. A **class ID** — a number saying *what* it is (e.g., 67 = cell phone).
3. A **confidence score** — how sure it is (0 to 1).

### 7.3 COCO — and why cigarettes and seat belts are UNKNOWN

A model only knows the categories it was trained on. The standard YOLO models are trained on a public dataset called **COCO** ("**C**ommon **O**bjects in **CO**ntext") — about 80 everyday object categories, each with a fixed ID number. In COCO, **"cell phone" is class 67** and **"person" is class 0**. That's why `Config.hpp` says:

```cpp
int classIdPhone = 67;
int classIdPerson = 0;
int classIdCigarette = -1;
int classIdSeatbelt = -1;
```

Here is the honest and important part. **COCO has no "cigarette" class and no "seat belt" class.** They simply aren't among its 80 categories. So a standard YOLO model *cannot* detect them — and this project refuses to fake it. The IDs are set to `-1`, a sentinel meaning "no such class in the loaded model," and the corresponding features report **UNKNOWN**. `docs/MODELS.md` states the rule plainly: *"Never set an invalid COCO id to fake a class."* The `Types.hpp` enums encode this honesty directly — `SmokingState` and `SeatBeltState` both default to `Unknown`, with comments like "defaults to Unknown so we never fake a detection." If someone later trains a custom model that *does* include those classes, they just set the real class IDs in the config and the existing logic starts working — no code changes needed.

### 7.4 What is ONNX?

Different AI tools (PyTorch, TensorFlow, etc.) each save models in their own format. **ONNX** ("**O**pen **N**eural **N**etwork **E**xchange") is a *universal* format — a common file type (`.onnx`) that many tools can export to and many runtimes can load. It's like PDF for neural networks: export once, run anywhere. This project loads YOLO models as `.onnx` files (`models/yolov8n.onnx`, etc.), which is why the docs show exporting with `yolo export ... format=onnx`.

### 7.5 Running YOLO with OpenCV's `dnn` — no separate runtime

A crucial design decision: this project runs the neural network using **OpenCV's own `dnn` module** — the same OpenCV you already built for images. There is **no separate "ONNX Runtime" or TensorFlow to install**. As `docs/MODELS.md` says: *"if you built the project, you already have everything except the model file."* For an embedded product, one fewer dependency is a big win.

Let's walk the real inference pipeline in `ObjectDetector.cpp`. Loading the model once (`init()`):

```cpp
net_ = cv::dnn::readNetFromONNX(cfg_.phoneModel);
net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
```

- `cv::dnn::readNetFromONNX(path)` — load the `.onnx` model file into a `cv::dnn::Net` (a runnable network object).
- The next two lines say "run it using OpenCV's engine, on the CPU."

Now the per-frame work in `runYolo`. It has four classic stages: **letterbox → blob → forward → decode → NMS.**

**Stage 1 — Letterboxing.** YOLO wants a fixed square input (here 640×640), but camera frames are rectangular (640×480). If you just squished the frame into a square, objects would distort. **Letterboxing** resizes the frame to fit *while keeping its shape*, padding the leftover space with a neutral gray:

```cpp
const double r = std::min((double)S / frameBGR.cols, (double)S / frameBGR.rows);  // scale that fits
const int newW = (int)std::round(frameBGR.cols * r), newH = (int)std::round(frameBGR.rows * r);
const int padX = (S - newW) / 2, padY = (S - newH) / 2;                            // centering pad
cv::Mat resized, canvas(S, S, frameBGR.type(), cv::Scalar(114, 114, 114));         // gray canvas
cv::resize(frameBGR, resized, cv::Size(newW, newH));
resized.copyTo(canvas(cv::Rect(padX, padY, newW, newH)));                          // paste centered
```

The `114,114,114` is the gray fill (a common YOLO convention). We remember `r`, `padX`, `padY` so we can later map boxes *back* to the original frame's coordinates.

**Stage 2 — Blob.** A neural network doesn't take a `cv::Mat` directly; it takes a **blob** — a specially formatted, normalized numeric bundle. `blobFromImage` does the conversion:

```cpp
cv::dnn::blobFromImage(canvas, blob, 1.0 / 255.0, cv::Size(S, S), cv::Scalar(), true, false);
```

- `1.0 / 255.0` — **scale** the pixel values from 0–255 down to 0.0–1.0 (networks expect small numbers).
- `cv::Size(S, S)` — the input size (640×640).
- `true` — **swap R and B**: remember OpenCV is BGR, but the model was trained on RGB, so this swaps them.
- The result `blob` is what the network will consume.

**Stage 3 — Forward.** Push the blob through the network and read the raw output:

```cpp
net_.setInput(blob);
cv::Mat out = net_.forward();  // YOLOv8: shape [1, 84, 8400]
```

`forward()` runs the actual inference — the "look once." The output is a big grid of numbers: for YOLOv8 on COCO it's 8400 candidate detections, each described by 84 numbers (4 box numbers + 80 class scores). The code even auto-detects whether it's a YOLOv5 (85 numbers, with an extra "objectness") or YOLOv8 (84) output.

**Stage 4 — Decode.** Loop over every candidate, find its best class and score, drop weak ones, and map its box back to original-frame coordinates (undoing the letterbox with `r`, `padX`, `padY`):

```cpp
for (int c = 0; c < numClasses; ++c) {
    if (d[classStart + c] > best) { best = d[classStart + c]; bestId = c; }   // strongest class
}
if (best < cfg_.yoloConfidence) continue;                                    // ignore low confidence
const float cx = d[0], cy = d[1], w = d[2], h = d[3];
const int x = (int)((cx - w / 2 - padX) / r);                                // undo padding + scale
const int y = (int)((cy - h / 2 - padY) / r);
```

Candidates below `yoloConfidence` (0.45) are discarded.

**Stage 5 — NMS (Non-Maximum Suppression).** A real object usually triggers *several* overlapping boxes. **NMS** cleans this up: among boxes that overlap a lot and claim the same thing, keep only the highest-scoring one and suppress the rest. "Overlap" is measured by **IoU** (Intersection-over-Union — the shared area divided by the combined area; 1.0 = identical boxes, 0 = no overlap).

```cpp
std::vector<int> keep;
cv::dnn::NMSBoxes(boxes, scores, (float)cfg_.yoloConfidence, (float)cfg_.nmsThreshold, keep);
```

`NMSBoxes` returns the indices to `keep`; `nmsThreshold` (0.45) is the IoU cutoff. What's left is the final, de-duplicated list of detections.

### 7.6 From a detection to "phone in use" — evidence and temporal confirmation

Finding a phone in *one* frame isn't enough — a single frame could be a fluke. `ObjectDetector::detect` adds **temporal confirmation**: it requires the phone to be seen in several recent detection cycles before believing it, then runs a small **state machine**:

```cpp
phoneHits_.push_back(phoneNow);                                  // record hit/miss each cycle
// keep only the last few...
const int hits = std::accumulate(phoneHits_.begin(), phoneHits_.end(), 0,
                                 [](int a, bool b) { return a + (b ? 1 : 0); });
if (hits >= cfg_.phoneConfirmFrames) {
    r.state = heldByHand ? PhoneState::UsageConfirmed : PhoneState::Detected;
} else if (hits > 0) {
    r.state = PhoneState::Possible;
} else {
    r.state = PhoneState::NoPhone;
}
```

The states escalate `NoPhone → Possible → Detected → UsageConfirmed`. It even *fuses* with hand position — if the phone box sits near a detected hand (normalized by face width, via `handPhoneDistanceFraction`), it upgrades to `UsageConfirmed`. For performance it only runs YOLO every `phoneDetectEveryNFrames` (4th) frame and reuses results in between — the same "expensive step, run it occasionally" idea as the face detector.

And the golden rule, stated in comments in both `main.cpp` and `ObjectDetector.hpp`: **phone use is decided ONLY by actually detecting a phone object — never inferred from head pose.** Turning your head to check a mirror must never be mislabeled "phone." Head pose feeds *distraction*; only a real detected phone box feeds *phone*.

---

## Chapter 8 — Networking: TCP, HTTP, and MJPEG streaming

The board that runs this system often has **no monitor attached** — it's tucked behind a dashboard. So how do you *watch* the live dashboard during a demo? The project includes a tiny web server (`MjpegServer.cpp`) that streams the annotated video to any browser on the same network. To understand it, we need three networking ideas, kept brief.

### 8.1 TCP sockets (very briefly)

Computers on a network talk through **sockets** — think of a socket as one end of a telephone line between two programs. **TCP** (Transmission Control Protocol) is the most common way to make that line *reliable*: bytes arrive in order and nothing is silently lost. One program **listens** on a numbered **port** (like a phone extension) and waits; another **connects** to that port; then they exchange bytes.

`MjpegServer.cpp` does exactly this with standard socket calls:

```cpp
listenFd_ = ::socket(AF_INET, SOCK_STREAM, 0);   // create a TCP socket
::bind(listenFd_, ...);                          // claim the port (e.g. 8080)
::listen(listenFd_, 8);                           // start listening for callers
// ... on a background thread:
const int fd = ::accept(listenFd_, ...);          // answer an incoming connection
```

- `socket(AF_INET, SOCK_STREAM, 0)` — make a TCP socket (`SOCK_STREAM` = TCP). It returns a **file descriptor** (`fd`) — just a number that names this connection.
- `bind` — attach it to the chosen port (`streamPort`, default 8080).
- `listen` then `accept` — wait for and answer incoming viewers. This runs on a **background thread** (a separate line of execution) so it doesn't freeze the main video loop — which is why `CMakeLists.txt` linked the Threads library.

### 8.2 HTTP (very briefly)

**HTTP** (HyperText Transfer Protocol) is the language web browsers speak. It's simple text: the browser sends a **request** line like `GET /stream`, and the server sends back a **response** — some header lines describing the content, a blank line, then the content itself. The server reads the request to decide what to serve:

```cpp
if (req.find("GET /stream") == std::string::npos) {
    sendAll(fd, kIndexPage, sizeof(kIndexPage) - 1);   // any other path → tiny landing page
    ::close(fd);
    continue;
}
```

- If the request is *not* `GET /stream`, send back `kIndexPage` — a minimal HTML page that just embeds `<img src="/stream">`.
- If it *is* `GET /stream`, the browser gets the live video feed (next section).

### 8.3 MJPEG — motion JPEG over HTTP

Here's the elegant trick for streaming video to a plain browser with no plugins. **MJPEG** ("Motion JPEG") is simply *a sequence of JPEG images sent one after another*, each replacing the last — like a digital flip-book. HTTP has a built-in mechanism for this called **`multipart/x-mixed-replace`**: it tells the browser "I'm going to keep sending you images on this one connection; each new one *replaces* the previous." The server announces it once in the response header:

```cpp
const char kMultipartHeader[] =
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=--dmsframe\r\n"
    "\r\n";
```

The `boundary=--dmsframe` is a marker string that separates one image from the next. Then, for every processed video frame, `publish()` JPEG-encodes the dashboard (Chapter 4.7) and writes it to each connected viewer, wrapped in a boundary + a small header stating the image's type and size:

```cpp
std::string head = "--dmsframe\r\nContent-Type: image/jpeg\r\nContent-Length: " +
                   std::to_string(jpeg.size()) + "\r\n\r\n";
// send the boundary+header, then the JPEG bytes, then a trailing CRLF, to each client:
sendAll(fd, head.data(), head.size()) &&
sendAll(fd, (const char*)jpeg.data(), jpeg.size()) &&
sendAll(fd, "\r\n", 2);
```

The browser sees a never-ending series of JPEGs on one connection and displays them in place — smooth live video. If a viewer disconnects, `sendAll` fails and the server quietly drops that client from its list. It also disables **Nagle's algorithm** (`TCP_NODELAY`) so frames go out immediately for low latency, and only bothers encoding at all if someone is actually watching (`if (clients_.empty()) return;`). The upshot: from any laptop or phone on the same network, you open `http://<board-ip>:8080/` and watch the DMS live — no app, no monitor on the board.

---

## Chapter 9 — How all the tools fit together

You've now met every tool. Let's assemble the whole machine. Here is the **data-flow word-diagram** for one trip around the main loop in `main.cpp`, which repeats ~30 times per second:

```text
   ┌─────────────┐
   │   CAMERA    │   physical webcam
   └──────┬──────┘
          │  cv::VideoCapture  (OpenCV: videoio)
          ▼
   ┌─────────────────────────┐
   │  cv::Mat frame (BGR)     │  rotate / mirror if configured (OpenCV: core, imgproc)
   └──────┬──────────────────┘
          │
          ▼
   ┌─────────────────────────────────────────────┐
   │  FaceTracker.process(frame)                  │
   │    • dlib HOG face detector  → face box      │  (dlib)
   │    • dlib 68-point predictor → 68 landmarks  │  (dlib)
   │    • EAR / MAR from landmarks                 │  (geometry, Ch.6)
   │    • head pose via cv::solvePnP              │  (OpenCV: calib3d)
   │  → fills a FaceObservation struct            │
   └──────┬───────────────────────────────────────┘
          │  obs (FaceObservation: landmarks, ear, mar, yaw/pitch/roll)
          ├───────────────► GazeEstimator.estimate() → pupil offset (OpenCV: imgproc)
          │
          ├───────────────► ObjectDetector.detect()  ──────────────┐
          │                   YOLO .onnx via cv::dnn                │  (OpenCV: dnn)
          │                   letterbox→blob→forward→NMS            │  runs every Nth frame
          │                   → PhoneResult (evidence-based)        │
          │                                                          │
          ▼                                                          ▼
   ┌──────────────────────┐   ┌──────────────────────┐   ┌─────────────────────┐
   │ DrowsinessDetector   │   │ DistractionDetector  │   │  (phone feeds        │
   │  EAR→closure/PERCLOS │   │  head pose + gaze     │   │   distraction only)  │
   │  MAR→yawns           │   │  + phone presence     │   └─────────────────────┘
   └──────────┬───────────┘   └──────────┬───────────┘
              │  drowsy score            │  distraction score
              └────────────┬─────────────┘
                           ▼
              ┌──────────────────────────┐
              │   RiskEngine (fusion)     │  weighted blend → 0..100 risk, DriverState
              │   wDrowsiness .40, etc.   │  (weights from Config.hpp)
              └────────────┬──────────────┘
                           ▼
              ┌──────────────────────────┐
              │  Dashboard.render()       │  draws boxes/points/text (OpenCV: imgproc)
              │  → annotated cv::Mat       │
              └────────────┬──────────────┘
                           ├──────────────► cv::imshow window (OpenCV: highgui)
                           ├──────────────► cv::imwrite snapshot (headless)  (imgcodecs)
                           └──────────────► MjpegServer.publish() → browser  (Ch.8 + imgcodecs)
```

Reading it as a story: the **camera** gives OpenCV a **BGR frame**. **dlib** finds the face and its **68 landmarks**. Simple **geometry** turns landmarks into **EAR** and **MAR**; **OpenCV's `solvePnP`** turns them into **head pose**. All of that lands in one **`FaceObservation`**. Separately, **YOLO (via OpenCV's `dnn`)** looks for a **phone**. The **DrowsinessDetector** and **DistractionDetector** convert these signals into scores; the **RiskEngine** blends them (using the weights `wDrowsiness = 0.40`, `wDistraction = 0.30`, `wPhone = 0.20`, `wYawn = 0.10` from `Config.hpp`) into one overall **risk** and **driver state**. Finally OpenCV **draws** the dashboard and shows it three ways: a **window**, a saved **image**, and a **browser stream**.

**Where YOLO plugs in:** notice the phone path is *parallel and optional*. If no YOLO model is present, `ObjectDetector` simply reports `NO_PHONE` and everything else works unchanged. And phone detection feeds only the distraction/phone logic — never the drowsiness math, and never inferred from head pose. Every tool has one clear job, and they connect through simple structs (`FaceObservation`, `PhoneResult`, `GazeResult`) — exactly the encapsulation idea from Chapter 2.

---

## Chapter 10 — Glossary and cheat-sheet

### 10.1 Glossary of terms

| Term | Plain-English meaning |
|---|---|
| **Program** | A list of precise instructions a computer follows. |
| **Programming language** | Human-readable notation for writing programs (this project: C++). |
| **Compiler** | Tool that translates C++ source into machine code. |
| **Linker** | Tool that binds compiled pieces + libraries into one executable. |
| **Executable** | The final runnable program (here, `dms`). |
| **Library** | Reusable pre-written code you build on (OpenCV, dlib). |
| **Variable** | A named box holding a value. |
| **Type** | The kind of value a variable holds (`int`, `double`, `bool`, `std::string`). |
| **Function** | A named, reusable block of instructions that can take inputs and return a result. |
| **Struct** | A custom type bundling several related values (fields). |
| **Class** | A struct that also bundles the functions (methods) acting on its data. |
| **Object** | A concrete thing built from a class blueprint. |
| **Method** | A function that belongs to a class. |
| **Encapsulation** | Hiding internal details behind a simple public interface. |
| **Header (`.hpp`)** | Declarations — *what* exists (the menu). |
| **Source (`.cpp`)** | Definitions — *how* it works (the kitchen). |
| **`#include`** | Paste in a header so its names are usable. |
| **Namespace** | A labeled container of names to avoid clashes (`dms`, `cv`, `std`). |
| **Reference (`&`)** | Another name for an existing value — no copy (fast). |
| **`const`** | Read-only; a promise not to modify. |
| **`std::vector`** | A resizable list of items of one type. |
| **Build system** | Tool that automates compiling + linking (CMake). |
| **CMake** | The build system this project uses; recipe in `CMakeLists.txt`. |
| **Out-of-source build** | Building into a separate `build/` folder, keeping source clean. |
| **Computer vision** | Getting computers to extract meaning from images. |
| **OpenCV** | Library for images, video, drawing, and running neural nets (`cv::`). |
| **`cv::Mat`** | OpenCV's image: a grid of pixels (rows × cols × channels). |
| **Pixel** | One dot of an image. |
| **BGR** | OpenCV's color order: Blue, Green, Red (each 0–255). |
| **Channel** | One number per pixel (grayscale = 1, color = 3). |
| **Haar cascade** | Classic OpenCV face/eye detector; the always-available fallback. |
| **dlib** | ML/vision library used for the face detector and 68 landmarks. |
| **HOG** | Histogram of Oriented Gradients — the edge-pattern method behind dlib's face detector. |
| **Landmark** | One of the 68 labeled facial points. |
| **iBUG 68** | The standard ordering of the 68 landmark points. |
| **EAR** | Eye Aspect Ratio — eye height ÷ width; drops when eyes close. |
| **PERCLOS** | Percentage of eye closure over a rolling time window. |
| **Microsleep** | A brief involuntary sleep; a critical alarm. |
| **MAR** | Mouth Aspect Ratio — mouth opening ÷ width; rises during a yawn. |
| **Hysteresis** | Separate enter/leave thresholds to prevent flickering. |
| **Head pose** | Head orientation as yaw / pitch / roll angles. |
| **Yaw / Pitch / Roll** | Turn left-right / nod up-down / tilt to shoulder. |
| **`solvePnP`** | OpenCV routine that finds 3-D orientation from matched 3-D↔2-D points. |
| **Euler angles** | Orientation expressed as three rotation angles. |
| **EMA** | Exponential Moving Average — a lightweight way to smooth a noisy signal. |
| **Gaze** | Approximate eye-looking direction from pupil offset. |
| **Neural network** | A program that learns from examples via tunable weights. |
| **Model** | A trained network (its learned weights) saved to a file. |
| **Training / Inference** | Learning the weights (once, offline) / using the model on new input (live). |
| **YOLO** | "You Only Look Once" — fast single-pass object detector. |
| **Object detection** | Finding *what* objects are *where* (box + class + confidence). |
| **Bounding box** | The rectangle locating a detected object. |
| **Class ID** | A number identifying an object category. |
| **COCO** | Standard 80-category dataset; "cell phone" = 67, "person" = 0. |
| **ONNX** | Universal neural-network file format (`.onnx`). |
| **`cv::dnn`** | OpenCV's module that loads and runs neural nets — no separate runtime. |
| **Blob** | The normalized numeric bundle fed into a neural network. |
| **Letterboxing** | Resizing to a square while keeping aspect ratio, padding the rest. |
| **NMS** | Non-Maximum Suppression — removing duplicate overlapping detections. |
| **IoU** | Intersection-over-Union — how much two boxes overlap. |
| **State machine** | Logic that moves through defined states (e.g. NoPhone→…→UsageConfirmed). |
| **Socket** | One end of a network connection between programs. |
| **TCP** | Reliable, ordered byte-stream network protocol. |
| **Port** | A numbered endpoint on a machine (this project: 8080). |
| **Thread** | A separate line of execution running alongside the main loop. |
| **HTTP** | The text protocol browsers use to request and receive content. |
| **MJPEG** | Motion JPEG — a live video made of back-to-back JPEG images. |
| **`multipart/x-mixed-replace`** | HTTP mode where each new image replaces the previous (enables MJPEG). |

### 10.2 Cheat-sheet: key OpenCV calls

| Call | What it does |
|---|---|
| `cv::VideoCapture cap(index)` | Open a webcam. |
| `cap.read(frame)` / `cap.set(prop, val)` | Grab a frame / set a capture property. |
| `cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY)` | Convert color image to grayscale. |
| `cv::resize(src, dst, size)` | Resize an image. |
| `cv::rotate(...)` / `cv::flip(...)` | Rotate / mirror an image. |
| `cv::GaussianBlur(...)` / `cv::threshold(...)` | Blur / black-and-white split (used in gaze). |
| `cv::rectangle / cv::circle / cv::putText` | Draw a box / circle / text label. |
| `cv::solvePnP(...)` / `cv::Rodrigues(...)` | Head-pose geometry (calib3d). |
| `cv::dnn::readNetFromONNX(path)` | Load a neural-network model. |
| `cv::dnn::blobFromImage(...)` | Prepare an image as network input. |
| `net.setInput(blob)` / `net.forward()` | Run inference. |
| `cv::dnn::NMSBoxes(...)` | Remove duplicate overlapping detections. |
| `cv::imencode(".jpg", img, buf, params)` | Compress an image to JPEG in memory. |
| `cv::imwrite(path, img)` | Save an image to a file. |
| `cv::imshow(win, img)` / `cv::waitKey(1)` | Show a window / refresh + read a key. |

### 10.3 Cheat-sheet: key dlib calls

| Call | What it does |
|---|---|
| `dlib::get_frontal_face_detector()` | Get the pre-trained HOG face detector. |
| `dlib::cv_image<dlib::bgr_pixel>(mat)` | Wrap an OpenCV frame for dlib to read. |
| `detector_(image, dets)` | Run face detection; fill `dets` with boxes + confidence. |
| `dlib::deserialize(path) >> predictor_` | Load the 68-point `.dat` shape-predictor model. |
| `predictor_(image, faceRect)` | Predict the 68 landmarks inside a face box. |
| `shape.part(i).x()` / `.y()` | Get the i-th landmark's coordinates. |

### 10.4 Cheat-sheet: key CMake commands

| Command | What it does |
|---|---|
| `cmake_minimum_required(VERSION x)` | Require a minimum CMake version. |
| `project(name LANGUAGES CXX)` | Name the project; it's C++. |
| `set(CMAKE_CXX_STANDARD 17)` | Use the C++17 language version. |
| `find_package(OpenCV REQUIRED COMPONENTS ...)` | Locate a library and its modules. |
| `add_subdirectory(path)` | Build another source tree (e.g. dlib) as part of this build. |
| `add_executable(dms src...)` | Declare the program and its source files. |
| `target_include_directories(dms ...)` | Tell the compiler where headers live. |
| `target_link_libraries(dms ...)` | Link libraries into the program. |
| `target_compile_definitions(dms PRIVATE FLAG)` | Define a compile flag (e.g. `DMS_HAVE_DLIB`). |
| `message(STATUS "...")` | Print a note during configuration. |

### 10.5 Cheat-sheet: the 68-landmark index map

| Indices | Feature | Key single points |
|---|---|---|
| 0–16 | Jawline | **8** = chin bottom |
| 17–21 | Right eyebrow | |
| 22–26 | Left eyebrow | |
| 27–30 | Nose bridge | **30** = nose tip |
| 31–35 | Lower nose | |
| 36–41 | **Right eye** | used for EAR (right) |
| 42–47 | **Left eye** | used for EAR (left) |
| 48–59 | Outer lip | **48** = right corner, **54** = left corner |
| 60–67 | Inner lip | **61,62,63 ↔ 67,66,65** = MAR vertical gaps |

Used by the code as: EAR eyes `L={36..41}`, `R={42..47}`; MAR verticals `61-67, 62-66, 63-65` over outer width `48-54`; head pose points `30, 8, 36, 45, 48, 54`.

---

### Closing note

Every number, formula, and function in this guide comes straight from the project's real files — `Config.hpp`, `Types.hpp`, `FaceTracker.cpp`, `ObjectDetector.cpp`, `DrowsinessDetector.cpp`, `DistractionDetector.cpp`, `GazeEstimator.cpp`, `MjpegServer.cpp`, `main.cpp`, `CMakeLists.txt`, and `docs/MODELS.md`. If you can now open any one of those files and follow what it's doing and *why*, you've reached the goal. Re-read a chapter whenever a term feels fuzzy; understanding compounds. Welcome to computer vision, Hemanth.
