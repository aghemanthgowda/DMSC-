# Embedded Linux & the Yocto Project — From Scratch

> A complete, beginner-to-mastery study guide written for Hemanth, who is starting with **zero** prior knowledge of programming, embedded systems, or IT. By the end you will understand exactly how the C++ **Driver Monitoring System (DMS)** in this project was built on a laptop, cross-compiled for an **NXP i.MX 93 EVK** board, and made to run on that board's custom Yocto-built Linux.

---

## What you'll learn

This guide teaches you, one small idea at a time, everything you need to understand and reproduce the workflow behind this project:

- What "embedded systems" and "embedded Linux" actually are, and how a small board differs from your laptop.
- Why a program compiled on your laptop **will not run** on the board, and how "cross-compilation" solves that.
- What the **Yocto Project** is (a system that *builds your own custom Linux*), and its vocabulary: layers, recipes, bitbake, machine config, images, and the BSP.
- What a Yocto build produces: the **bootloader (U-Boot)**, the **Linux kernel**, the **device tree**, and the **root filesystem** — and how that gets flashed onto an SD card or eMMC.
- How the board boots, how you watch it over a **serial console** with `minicom`, and how you log in.
- What the **Yocto SDK / toolchain** is, why you must `source` a special environment-setup script, and how it makes your compiler target the board.
- The **exact** commands used in this project to cross-compile the CMake-based DMS, copy it to the board, and run it.
- The **real problems** that came up in this project (missing OpenCV libraries, no display, camera devices) and how they were fixed.
- How the board's **Ethos-U NPU** (AI accelerator) fits in, and why some models run slowly (motivating the project's `--fast` mode).
- A full **glossary** and a **command cheat sheet** you can keep at your side.

You do not need to memorize anything. Read it top to bottom once; then use the cheat sheet and glossary as references.

---

## Table of contents

1. [Embedded systems & embedded Linux](#1-embedded-systems--embedded-linux)
2. [CPU architecture: why aarch64 ≠ x86-64](#2-cpu-architecture-why-aarch64--x86-64)
3. [Cross-compilation from scratch](#3-cross-compilation-from-scratch)
4. [What the Yocto Project is](#4-what-the-yocto-project-is)
5. [What a Yocto build produces](#5-what-a-yocto-build-produces)
6. [Booting the board](#6-booting-the-board)
7. [The Yocto SDK / toolchain](#7-the-yocto-sdk--toolchain)
8. [Cross-compiling a CMake project with the SDK](#8-cross-compiling-a-cmake-project-with-the-sdk)
9. [Transferring and running on the board](#9-transferring-and-running-on-the-board)
10. [Things that go wrong (and their fixes)](#10-things-that-go-wrong-and-their-fixes)
11. [How the Ethos-U NPU fits in](#11-how-the-ethos-u-npu-fits-in)
12. [Glossary & command cheat sheet](#12-glossary--command-cheat-sheet)

---

# 1. Embedded systems & embedded Linux

## 1.1 What is an "embedded system"?

A **computer** is any machine that follows instructions to process information. When you hear "computer" you probably picture a laptop or desktop — a screen, a keyboard, a mouse, and a big box of electronics. But most computers in the world do not look like that at all. They are tiny, hidden inside other products, doing **one specific job**:

- The chip inside a microwave that runs the timer and turns the magnetron on and off.
- The controller in a car that reads the engine sensors and adjusts fuel injection.
- The board inside a smart doorbell that captures video and streams it to your phone.
- The processor in a fitness watch counting your steps.

These are **embedded systems**: a computer that is *embedded* (built in) as part of a larger device, dedicated to a particular task, rather than being a general-purpose machine you use for anything.

> **Analogy.** A laptop is like a Swiss Army knife — it can do a thousand different things. An embedded system is like a single, purpose-built tool: a bottle opener. It does one job, does it reliably, is cheap, small, and uses very little power.

The **Driver Monitoring System (DMS)** in this project is exactly this kind of application. A DMS is a computer that watches the driver of a vehicle through a camera and detects things like drowsiness, distraction, or looking away from the road. It is meant to live inside a vehicle — an embedded environment — not on a desk.

## 1.2 What is "embedded Linux"?

**Linux** is an **operating system (OS)** — the core software that manages a computer's hardware (its processor, memory, storage, cameras, network) and lets application programs run on top of it. Windows and macOS are also operating systems; Linux is a free, open-source one that is enormously popular in servers, phones (Android is built on Linux), and — crucially for us — embedded devices.

**Embedded Linux** simply means: running Linux as the operating system on an embedded device. Instead of the giant, do-everything Linux you might install on a PC, an embedded Linux system is usually **trimmed down** to include only the pieces that particular device needs. Why bother?

- **Storage is small.** An embedded board might have only a few gigabytes, so you cannot ship a bloated OS.
- **Memory is small.** Less RAM means you cannot run unnecessary background services.
- **It must be reliable.** A device in a car or factory should boot the same way every time and not surprise you.
- **You control everything.** You decide exactly which programs, drivers, and libraries are present.

The i.MX 93 EVK board in this project runs a Linux system that was **custom-built** with the Yocto Project (the subject of Chapter 4). The specific image is NXP's **`imx-image-full`**, from the Yocto release codenamed **scarthgap**, using **Linux kernel version 6.6**.

## 1.3 How an embedded board differs from a laptop

| Aspect | Your laptop | The i.MX 93 EVK board |
|---|---|---|
| Purpose | General-purpose, run anything | Dedicated to a task (e.g., DMS) |
| Processor family | Usually **x86-64** (Intel/AMD) | **aarch64** (Arm Cortex-A55) |
| RAM | 8–64 GB | Often 1–2 GB |
| Storage | 256 GB–2 TB SSD | A few GB on **eMMC** or an SD card |
| Display/keyboard | Built in | Often **none** — you connect over a cable |
| Power | Tens of watts | A couple of watts |
| How you interact | Screen, mouse, keyboard | **Serial console** and/or **network** |
| Operating system | Pre-installed (Windows/macOS/Linux) | You **build and flash** it yourself |

The two biggest surprises for a beginner are:

1. The board usually has **no monitor and no keyboard**. You talk to it through a cable (the *serial console*, Chapter 6) or over the network. In this project the board's network address (its **IP address** — the unique number identifying it on the local network) is **192.168.1.173**.
2. The board's **processor is a completely different kind** from your laptop's. This is the single most important technical idea in this whole guide, so it gets its own chapter next.

## 1.4 What is a development board / EVK?

When a chip company like **NXP** designs a new **SoC** (**System on a Chip** — a single silicon chip that packs the processor, memory controllers, graphics, and many other functions all together), they want engineers to be able to try it out and build products around it. But a bare chip is just a fingernail-sized square of silicon with hundreds of tiny pins; you cannot plug a camera or a network cable into that directly.

So the chip maker builds and sells an **EVK — Evaluation Kit** (also called a **development board** or "dev board"). An EVK is a ready-made circuit board that has the SoC already mounted and wired up to useful connectors: USB ports, Ethernet, camera connectors, an SD-card slot, power, and debug headers. It is a **reference design** — a working example — so engineers can start developing software immediately without designing their own hardware first.

> **Analogy.** The SoC is like a car engine sold on its own. The EVK is a complete go-kart built around that engine so you can actually drive it, test it, and learn how it behaves before you design your own vehicle.

## 1.5 The NXP i.MX 93 EVK specifically

The board used in this project is the **NXP i.MX 93 EVK**. The **i.MX 9** family is a line of application processors from NXP aimed at edge/embedded devices, especially ones doing a bit of machine learning. Key parts relevant to us:

- **CPU: Arm Cortex-A55 cores.** The **CPU (Central Processing Unit)** is the main "brain" that runs your programs. The i.MX 93's CPU uses **Arm Cortex-A55** cores. These are **64-bit** processors implementing the Arm architecture. The names you will see for this architecture are **aarch64**, **arm64**, or **armv8a** — they all mean the same 64-bit Arm instruction set. (Contrast: your laptop is almost certainly **x86-64**, a *different* instruction set from Intel/AMD. Chapter 2 explains why that difference matters so much.)
- **NPU: Arm Ethos-U.** The board also contains an **NPU (Neural Processing Unit)** — a specialized chunk of silicon designed to run the mathematics of neural networks (AI models) very efficiently, far faster and with less power than doing the same math on the CPU. The i.MX 93 uses an **Arm Ethos-U** NPU. For a DMS that runs AI models (to detect faces, eyes, head pose, etc.), an NPU is exactly the kind of accelerator you would love to use. Chapter 11 explains how it fits in and why it is not automatically used.

Two more terms you will meet constantly:

- **Bootloader:** the very first program that runs when the board powers on; it initializes hardware and then loads the operating system. On this board that is **U-Boot** (Chapter 5 and 6).
- **rootfs (root filesystem):** the collection of all the files — programs, libraries, configuration — that make up the running Linux system. When people say "the image," they largely mean the rootfs plus the kernel and bootloader packaged together.

---

# 2. CPU architecture: why aarch64 ≠ x86-64

## 2.1 What a CPU actually understands

Deep down, a CPU does not understand C++, Python, or English. It understands only **machine instructions** — extremely simple numeric commands like "add these two numbers," "load this value from memory," "jump to that instruction." The exact set of instructions a CPU understands, and the precise numeric encoding of each, is called its **instruction set architecture (ISA)**, usually shortened to **architecture** or **arch**.

Different families of CPUs speak **different, incompatible instruction sets**, just like people speak different languages:

- **x86-64** (also called `amd64` or `x64`) — the instruction set of Intel and AMD processors found in almost all laptops, desktops, and cloud servers.
- **aarch64** (also called `arm64` or `armv8a`) — the 64-bit Arm instruction set, used by phones, tablets, Apple Silicon Macs, and embedded boards like the **i.MX 93 EVK**.

## 2.2 Why you can't just copy a program across

When a program is **compiled** (turned into a runnable file — see Chapter 3), the result is a file full of machine instructions for **one specific architecture**. A program compiled for x86-64 contains x86-64 instructions. An aarch64 CPU has no idea what those bytes mean — it is like handing a book written in Japanese to someone who only reads Arabic. The words are there, but the reader cannot execute them.

> **Analogy.** Imagine a recipe written using the exact button-press sequences of *one specific brand* of oven. If you take that recipe to a different brand of oven, the button sequences are meaningless — the ovens do the same *kind* of thing (bake food) but respond to entirely different controls. Machine code is a recipe written in the exact "button presses" of one CPU architecture.

This is why you **cannot** simply build the DMS on your laptop and copy the resulting file to the board. Your laptop produces x86-64 code; the board needs aarch64 code. Copying the wrong one over yields the classic error when you try to run it:

```text
-bash: ./dms: cannot execute binary file: Exec format error
```

That message means: "This file's format/architecture is not something I can execute." The fix is not to copy harder — it is to build the program *for the board's architecture in the first place*. That is **cross-compilation**, Chapter 3.

## 2.3 A quick reality check on your own machines

You can ask any Linux/macOS machine what architecture it is with the `uname -m` command ("machine hardware name"):

```bash
uname -m
```

| Where you run it | Typical output | Meaning |
|---|---|---|
| Your laptop / build PC | `x86_64` | Intel/AMD 64-bit |
| The i.MX 93 EVK board | `aarch64` | Arm 64-bit |

Different answers = incompatible binaries = you must cross-compile. Same answer would mean you *could* (in principle) build natively on the target, but embedded boards are usually far too slow and too limited on storage to compile large C++ projects on themselves — another reason cross-compilation is the norm.

---

# 3. Cross-compilation from scratch

## 3.1 What is a compiler?

Humans write programs in **source code** — text in a programming language such as **C++** (the language the DMS is written in). But CPUs, as we saw, only understand raw machine instructions. A **compiler** is a program that **translates source code into machine code** for a target CPU. The output is an **executable** (also called a **binary**) — a file the CPU can actually run.

```text
   source code (.cpp)  ──►  [ compiler ]  ──►  executable (machine code)
      human-readable                              CPU-runnable
```

The most common C/C++ compilers on Linux are **GCC** (the GNU Compiler Collection) and **Clang**. In the Yocto SDK you will use a GCC that has been specially set up to produce **aarch64** code.

## 3.2 "Host" vs "target"

Two words appear everywhere in cross-development. Learn them now:

- **Host:** the machine where you *do the building* — i.e., where the compiler runs. In this project, the host is your **laptop / build PC (x86-64)**.
- **Target:** the machine where the built program will *actually run*. Here, the target is the **i.MX 93 EVK board (aarch64)**.

## 3.3 What is cross-compilation?

**Native compilation** is when host and target are the same architecture: you build an x86-64 program on an x86-64 PC and run it on that same PC. Simple, but not our situation.

**Cross-compilation** is when host and target are **different** architectures. You run the compiler on the host (fast x86-64 laptop) but tell it to emit machine code for the target (aarch64 board). The compiler that can do this is a **cross-compiler**.

```text
   HOST (x86-64 laptop)                          TARGET (aarch64 board)
   ┌───────────────────────┐                     ┌────────────────────┐
   │  dms.cpp  ──►  cross-  │   scp over network  │                    │
   │              compiler  │ ──────────────────► │  ./dms  runs here   │
   │              (targets  │                     │  (aarch64 CPU)      │
   │               aarch64) │                     │                    │
   └───────────────────────┘                     └────────────────────┘
```

> **Analogy.** You are a translator sitting in London (the host) who is fluent in a language spoken only in another city (the target). You do all the *work of translating* in London, then mail the finished translation to that city, where the locals can read it. You never had to travel there to do the translation.

Why compile ON the laptop FOR the board, instead of on the board itself?

- The laptop is **much faster** and has far more RAM and disk — compiling a big C++ program with OpenCV can be heavy.
- The board may **lack a compiler entirely** (a slim production image often has none).
- You keep your **development tools** on the comfortable machine and ship only the finished binary to the board.

## 3.4 What is a toolchain?

A compiler alone is not enough to produce a working program. You also need:

- an **assembler** (turns a lower-level form into machine code),
- a **linker** (stitches your code together with the libraries it uses into one executable),
- **libraries and headers** for the target,
- utilities like `objdump`, `strip`, `ar`, etc.

This whole coordinated set of build tools is called a **toolchain**. A **cross-toolchain** is a toolchain whose tools all target a *different* architecture than the host. The Yocto **SDK** (Chapter 7) is essentially a packaged cross-toolchain for the i.MX 93, plus the target's libraries.

## 3.5 What is a sysroot?

Your program does not exist in isolation — it calls into **libraries**. The DMS uses **OpenCV** (a big computer-vision library), which in turn relies on the C library, the C++ runtime, image/video libraries, and more. To compile and link successfully, the cross-compiler must see the **target's** versions of all these libraries and their **header files** (the `.h`/`.hpp` files that describe what functions a library offers).

A **sysroot** ("system root") is a directory on the host that mirrors the layout of the target's filesystem — it contains the target's `/usr/include` (headers), `/usr/lib` (libraries), and so on. When cross-compiling, you point the compiler at this sysroot so it uses the *board's* libraries, not your laptop's x86-64 ones.

> **Analogy.** The sysroot is a **scale model** of the board's filesystem kept on your laptop. When the compiler needs to know "what does OpenCV look like on the board?", it consults the model instead of the real board.

In this project the target sysroot lives under the SDK and is referenced by the environment variable **`$SDKTARGETSYSROOT`** (you'll see this in Chapters 7, 8, and 10). There is also a **native** sysroot (`$OECORE_NATIVE_SYSROOT`) that holds the *host-side* tools of the SDK (the cross-compiler itself, CMake helper files, etc.).

---

# 4. What the Yocto Project is

## 4.1 The key idea: not a distro, but a distro *factory*

Beginners often assume Yocto is "a version of Linux" like Ubuntu. **It is not.** Ubuntu, Fedora, and Debian are **Linux distributions ("distros")** — ready-made, pre-packaged Linux systems you download and install.

The **Yocto Project** is a **build system**: a set of tools and metadata that **builds a completely custom Linux distribution for you**, tailored to your exact hardware and your exact needs. You describe *what you want* (which board, which programs, which features), press build, and out comes a bootable image containing precisely that — nothing more, nothing less.

> **Analogy.** Ubuntu is a ready-to-eat meal from the supermarket freezer. Yocto is a fully equipped kitchen with recipes: you decide every ingredient and cook the exact dish your device needs. It is more work, but you get total control — vital for embedded products where size, licensing, and reproducibility matter.

Because it *builds from source*, Yocto can produce an image for **any** supported architecture (including our **aarch64** board) from an x86-64 build machine. In other words, a Yocto build is itself a giant, organized cross-compilation of an entire operating system.

## 4.2 Poky and OpenEmbedded

Two names travel with Yocto:

- **OpenEmbedded (OE):** the underlying build framework and the huge community collection of "recipes" (build instructions) for thousands of software packages. Yocto builds on top of OpenEmbedded's core.
- **Poky:** the Yocto Project's **reference distribution** — a starting-point example that bundles the build tool (bitbake), the OpenEmbedded core metadata, and sensible defaults. Think of Poky as the "hello world" distro you customize from.

You do not need to untangle the exact history. Practically: Yocto = the project/umbrella, Poky = the reference build you start from, OpenEmbedded = the recipe framework underneath. You will notice the board's Linux target is called `...-poky-linux` (as in `armv8a-poky-linux`) — that "poky" is this reference distro.

## 4.3 The core vocabulary

Yocto has its own words. Here they are, each with a plain-English meaning and an analogy to cooking a big meal.

| Yocto term | File/prefix | Plain meaning | Cooking analogy |
|---|---|---|---|
| **Recipe** | `.bb` | Instructions to fetch, configure, compile, and package **one** piece of software | A single **recipe card** (how to make one dish) |
| **Layer** | `meta-*` | A folder grouping many related recipes and configuration | A **chapter** of the cookbook (e.g., "desserts") |
| **bitbake** | — | The **build engine** that reads recipes and executes them | The **chef** who follows the recipes |
| **Machine config** | `<name>.conf` | Describes the target hardware (CPU, features) | The **oven's settings** for your specific oven |
| **Image recipe** | `*-image-*.bb` | A recipe that lists *which packages* go into the final system | The **menu** for the whole dinner |
| **BSP** | layer(s) | **Board Support Package** — everything needed to boot Linux on a specific board | The **appliance manual** for your exact kitchen |

Let's expand the important ones.

### 4.3.1 Recipes (`.bb`)

A **recipe** is a text file (extension `.bb`, for "bitbake") that tells Yocto how to build one component: where to download its source code, how to configure it, how to compile it, and how to package the result. There is a recipe for the Linux kernel, one for U-Boot, one for OpenCV, one for your own application, and so on — thousands of them.

### 4.3.2 Layers (`meta-*`)

A **layer** is a directory (conventionally named starting with `meta-`) that bundles a set of related recipes and configuration together. Layers keep things modular: you can add or remove whole features by including or excluding a layer. Examples:

- `meta` / `meta-poky` — the core (from Poky/OpenEmbedded).
- `meta-imx` — **NXP's layers** for i.MX chips (more below).
- You might add `meta-yourcompany` for your own app.

### 4.3.3 bitbake — the build engine

**bitbake** is the program that actually *does the build*. You give it the name of what you want (usually an image recipe) and it works out the entire dependency tree, downloads sources, and cross-compiles everything in the right order. A typical invocation:

```bash
bitbake imx-image-full
```

That one command can build hundreds of packages, the kernel, the bootloader, and assemble them into a flashable image — potentially taking hours the first time.

### 4.3.4 Machine configuration

The **machine config** tells Yocto exactly which hardware you are targeting so it builds the right kernel, bootloader, and device tree. You select it via the `MACHINE` variable, e.g. something like `MACHINE = "imx93evk"`. This is how Yocto knows to produce **aarch64** binaries with i.MX 93-specific settings.

### 4.3.5 Image recipes

An **image recipe** (named like `something-image-something.bb`) is the "menu": it lists which software packages should be included in the final root filesystem. Different images serve different purposes:

- A **minimal** image: just enough to boot and log in.
- A **full** image: lots of libraries and tools included.

This project uses NXP's **`imx-image-full`**, a comprehensive image that already contains many multimedia and vision libraries — importantly **OpenCV 4.10** — which is exactly why the DMS can run on the board without you having to install OpenCV separately (see Chapter 9).

### 4.3.6 The BSP (Board Support Package)

A **BSP — Board Support Package** — is the collection of software that makes Linux actually run on a *specific* board: the right **bootloader** settings, the **kernel** with the right drivers, and the **device tree** (Chapter 5) describing that board's hardware. Without a BSP, generic Linux has no idea how to talk to *this* board's particular chips. The BSP is what turns "Linux in general" into "Linux that boots on the i.MX 93 EVK."

## 4.4 NXP's meta-imx layers

NXP provides the BSP for i.MX chips as a set of Yocto **layers** collectively referred to as **meta-imx** (with supporting layers such as `meta-freescale`, `meta-freescale-3rdparty`, and others). These layers contain:

- NXP's version of **U-Boot** for i.MX boards,
- NXP's **Linux kernel** (version **6.6** in this scarthgap release) with i.MX drivers,
- device trees for boards like the i.MX 93 EVK,
- recipes for NXP-specific bits like the **Ethos-U NPU** tooling (the `vela` compiler, TensorFlow Lite delegates — see Chapter 11),
- the `imx-image-*` image recipes (including `imx-image-full`).

So the full picture: you take the Yocto/Poky core, add NXP's **meta-imx** layers (the BSP), select the `imx93evk` machine, and bitbake `imx-image-full` — and Yocto cross-builds a complete aarch64 Linux system tailored to this board. That system is what is running on the board at **192.168.1.173**.

---

# 5. What a Yocto build produces

A successful Yocto build creates several distinct pieces. To understand a running board you must know what each one is and what it does. They correspond to the stages a board goes through from power-on to a usable Linux prompt.

## 5.1 The four key outputs

| Output | What it is | Analogy |
|---|---|---|
| **U-Boot** (bootloader) | First software to run at power-on; sets up hardware and loads the kernel | The **ignition + starter motor** of a car |
| **Linux kernel** | The core of the OS; manages CPU, memory, and drives hardware | The **engine** |
| **Device tree** (`.dtb`) | A data file describing this board's hardware to the kernel | The **wiring diagram** the engine consults |
| **Root filesystem (rootfs)** | All the files, programs, and libraries of the running system | The **rest of the car**: seats, dashboard, controls |

### 5.1.1 U-Boot (the bootloader)

**U-Boot** ("Das U-Boot", the Universal Bootloader) is the small program that runs *first*, straight out of power-on. Its jobs: initialize essential hardware (memory, storage, serial port), then find and load the Linux kernel into memory and hand control to it. U-Boot also gives you a mini command prompt (useful for advanced tasks). It is produced by a recipe in NXP's BSP.

### 5.1.2 The Linux kernel

The **kernel** is the heart of the operating system. It manages the CPU cores, memory, and all the device drivers that talk to hardware (camera, network, storage, the NPU). Everything else — your shell, your DMS program — runs as a "user-space" process on top of the kernel. This project's kernel is **version 6.6**.

### 5.1.3 The device tree

Arm embedded systems can't auto-discover all their hardware the way a PC can. Instead, a **device tree** — a compiled data file with the extension **`.dtb`** (Device Tree Blob) — tells the kernel exactly what hardware is on *this* board and where: which pins are the camera, where the Ethernet controller lives, how much RAM there is, and so on. The bootloader loads the right `.dtb` alongside the kernel. Different boards use different device trees even with the *same* kernel.

### 5.1.4 The root filesystem (rootfs)

The **rootfs** is the complete set of files that make up the running Linux system: the shell, system programs, configuration in `/etc`, shared **libraries** in `/usr/lib` (including **OpenCV 4.10**), and so on. When you log into the board and type commands, you are moving around inside the rootfs. The image recipe (`imx-image-full`) decides what goes into it.

## 5.2 Packaging it all: the `.wic` image

For the board to boot, these pieces must be laid out on a storage medium (an SD card or the board's built-in **eMMC** flash) with the right partitions in the right places. Yocto packages this as a single **`.wic`** file — a full **disk image** that already contains the partition table, the bootloader, the kernel + device tree, and the rootfs, arranged exactly as the board expects.

> **Analogy.** A `.wic` file is like a **complete disk photocopy**. Writing it to an SD card doesn't copy files one-by-one; it stamps the entire disk layout — partitions and all — onto the card in one go, producing a ready-to-boot card.

Key storage terms:

- **SD card:** a removable memory card; easy to reflash, great for development.
- **eMMC:** **embedded MultiMediaCard** — flash storage soldered onto the board itself; used for the "final" installed system.

## 5.3 Flashing the image (conceptually)

**Flashing** means writing that `.wic` image onto the SD card or eMMC. Two common tools:

- **`dd`** — a classic low-level Unix tool that copies raw bytes from one place to another. It writes the entire `.wic`, byte for byte, to the card's device node (e.g. `/dev/sdX`). Powerful but unforgiving: **pointing it at the wrong device can erase your laptop's own disk**, so you must be certain of the device name.
- **`bmaptool`** — a smarter, safer, faster tool from the Yocto ecosystem. It reads a companion **block map (`.bmap`)** file that lists only the parts of the image actually containing data, so it skips the empty regions. That makes flashing much quicker and includes integrity checks.

Conceptual examples (device names are illustrative — always confirm yours):

```bash
# Safer/faster, using the block map that ships next to the .wic:
sudo bmaptool copy imx-image-full-imx93evk.wic /dev/sdX

# Classic low-level alternative (double- and triple-check /dev/sdX!):
sudo dd if=imx-image-full-imx93evk.wic of=/dev/sdX bs=4M conv=fsync status=progress
```

Here `if=` is the **in**put file (the image) and `of=` is the **o**utput file (the card's device). After flashing, you put the card in the board (or, for eMMC, use the vendor's flashing procedure), set the boot switches (Chapter 6), and power on.

---

# 6. Booting the board

## 6.1 The boot sequence, step by step

When you power on the i.MX 93 EVK, this chain of events happens:

```text
Power on
   │
   ▼
Chip's built-in ROM code  ──►  reads boot switches to decide WHERE to boot from
   │
   ▼
U-Boot (bootloader)       ──►  initializes RAM, storage, serial console
   │
   ▼
U-Boot loads:  Linux kernel  +  device tree (.dtb)  into memory
   │
   ▼
Linux kernel starts        ──►  brings up drivers, mounts the rootfs
   │
   ▼
Login prompt on the serial console  ──►  you log in as 'root'
```

## 6.2 Boot switches: SD vs eMMC

The board has small physical **DIP switches** (or jumpers) called **boot switches** that tell the chip's ROM **where to look for the bootloader** — for example, "boot from the SD card" versus "boot from the eMMC." During development you typically boot from an **SD card** (easy to reflash on your laptop); for a more permanent setup you flash and boot from **eMMC**. If the board seems dead or doesn't boot, a wrong boot-switch setting is one of the first things to check. (The exact switch positions are printed in the i.MX 93 EVK's quick-start guide.)

## 6.3 The serial console — why `/dev/ttyUSB2` and 115200

Because the board has **no monitor or keyboard by default**, you need another way to see its boot messages and type commands. The oldest, most reliable method is the **serial console**: a simple text link over a cable (here, a USB cable providing a serial connection). Everything the board would "print" — U-Boot messages, kernel logs, the login prompt — comes out over this serial line, and whatever you type goes back in.

- On your laptop, that connection appears as a **device node** — a special file representing the hardware. Here it is **`/dev/ttyUSB2`**. (A single USB debug cable often exposes several ports `ttyUSB0..3`; the console is on one of them — in this project, `ttyUSB2`.)
- Serial links must agree on a **baud rate** — the speed of communication in bits per second. Both sides must use the **same** rate or you get garbage characters. The i.MX 93 EVK console uses **115200** baud, the standard for NXP boards.

### 6.3.1 Watching the boot with `minicom`

**`minicom`** is a terminal program that opens a serial device and shows you its output (there are alternatives like `screen` and `picocom`, but this project uses minicom). You launch it, pointing at the device and baud rate:

```bash
# Open the board's serial console at the correct speed:
sudo minicom -D /dev/ttyUSB2 -b 115200
```

- `-D /dev/ttyUSB2` selects the serial device.
- `-b 115200` sets the baud rate to match the board.

Once connected, power-cycle the board and you'll watch U-Boot and the kernel scroll by in real time — invaluable for diagnosing boot problems.

> **Tip.** To *exit* minicom you press `Ctrl-A` then `X`. To bring up its menu, `Ctrl-A` then `Z`. If you see nothing, check the cable, the device name (`ttyUSB2`), and the baud rate (`115200`).

## 6.4 Logging in as root

When the kernel finishes booting, the serial console shows a **login prompt**. On a development image like `imx-image-full`, you log in as the **`root`** user — the all-powerful administrator account (equivalent to "Administrator" on Windows). Development images often let root log in with no password:

```text
NXP i.MX Release Distro ... imx93evk ttyLP0

imx93evk login: root
root@imx93evk:~#
```

That final `root@imx93evk:~#` is your **shell prompt** on the board. You are now "inside" the board's Linux and can run commands there. You can also reach this same shell over the network with **SSH** (Secure Shell) using the board's IP, which is how files are later copied to it (Chapter 9).

---

# 7. The Yocto SDK / toolchain

## 7.1 What problem the SDK solves

You (usually) don't want every application developer to run full multi-hour Yocto builds just to compile one program. Instead, Yocto can package a **software development kit (SDK)**: a self-contained **cross-toolchain plus the target sysroot**, ready to install on any developer's laptop. With the SDK installed, a developer can cross-compile applications for the board **without touching Yocto again**.

That is exactly the setup in this project: the DMS is built using an SDK installed at:

```text
/opt/fsl-imx-wayland/6.6-scarthgap/
```

("fsl" = Freescale, NXP's predecessor; "wayland" refers to the graphics stack the image uses; "6.6-scarthgap" ties it to kernel 6.6 and the scarthgap release.)

## 7.2 How the SDK is produced

From the Yocto build environment, you generate the SDK for a given image with a special bitbake task, **`populate_sdk`**:

```bash
# Produce an installable SDK matching the imx-image-full image:
bitbake imx-image-full -c populate_sdk
```

- `-c populate_sdk` tells bitbake to run the *populate_sdk* **task** rather than a normal build. This gathers the cross-toolchain and the target sysroot (with the *same* libraries as the image, including OpenCV 4.10) into one bundle.
- The output is a **self-extracting installer script**, a `.sh` file, that anyone can run to install the SDK.

## 7.3 Installing the SDK

You install by simply running the generated `.sh` installer. It asks where to put the SDK and then unpacks the toolchain and sysroot there:

```bash
# Run the installer (name will vary); accept or choose the install path:
./fsl-imx-wayland-glibc-x86_64-imx-image-full-armv8a-imx93evk-toolchain-6.6-scarthgap.sh
```

In this project it was installed to `/opt/fsl-imx-wayland/6.6-scarthgap/`, which is where the crucial environment-setup script lives.

## 7.4 The crucial step: `source` the environment-setup script

Installing the SDK puts the tools on disk, but your shell doesn't yet know to use them. You must **activate** the SDK in your current terminal by **sourcing** its environment-setup script:

```bash
source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-armv8a-poky-linux
```

### 7.4.1 What "source" means

`source` (you may also see it written as a single dot: `. script`) runs a script's commands **in your current shell**, so any environment variables it sets **stick around** in that shell afterward. (Running the script normally would set the variables in a throwaway child shell and lose them.) You must `source` — not just execute — the environment-setup file.

> **Analogy.** Sourcing the SDK is like **putting on a specific pair of work gloves** before a job. Until you put them on, your hands (the shell) work the ordinary way — compiling for the laptop. Once you've put on the gloves, every "build" action you take is now shaped for the board. Open a *new* terminal and you're bare-handed again: you must source it again.

### 7.4.2 What the script actually sets

The environment-setup script sets a batch of **environment variables** — named values your shell and build tools read to know how to behave. The important ones:

| Variable | What it points to / does |
|---|---|
| `CC` | The **C** cross-compiler command (aarch64), with all the right flags |
| `CXX` | The **C++** cross-compiler command (used for the DMS's C++ code) |
| `CFLAGS` / `CXXFLAGS` | Compiler flags targeting the board (CPU tuning, sysroot, etc.) |
| `LD`, `AR`, `STRIP`, ... | The other cross-toolchain tools (linker, archiver, etc.) |
| `SDKTARGETSYSROOT` | Path to the **target sysroot** (the board's headers & libraries) |
| `OECORE_NATIVE_SYSROOT` | Path to the **native (host-side) SDK sysroot** — holds the cross-compiler and CMake helper files |
| `CMAKE_TOOLCHAIN_FILE` info | Points toward `OEToolchainConfig.cmake`, the CMake toolchain file (see 7.4.3) |

After sourcing, when you type `cc`, `gcc`, `cmake`, or when a build system reads `$CC`/`$CXX`, it uses the **cross-compiler** aimed at aarch64 — not your laptop's native compiler.

### 7.4.3 The CMake toolchain file

Projects that use **CMake** (a popular build-configuration tool — the DMS uses it) need to be told about the cross-compiler in CMake's own language. The SDK provides exactly that, a **CMake toolchain file** named **`OEToolchainConfig.cmake`**, living inside the native sysroot at:

```text
$OECORE_NATIVE_SYSROOT/usr/share/cmake/OEToolchainConfig.cmake
```

Passing this file to CMake (Chapter 8) makes CMake use the cross-compiler and the target sysroot automatically.

## 7.5 Verifying the SDK is active

Always confirm before building. Two quick checks:

```bash
# 1. What compiler will builds use? Should print the aarch64 cross-compiler, not plain "gcc".
echo $CC

# 2. What version/target is it? Look for "aarch64" in the output.
$CC --version
```

Expected shapes of the output:

```text
$ echo $CC
aarch64-poky-linux-gcc -mcpu=cortex-a55 ... --sysroot=/opt/fsl-imx-wayland/6.6-scarthgap/sysroots/armv8a-poky-linux

$ $CC --version
aarch64-poky-linux-gcc (GCC) 13.x.x
...
```

Seeing **`aarch64-poky-linux-...`** confirms the SDK is active and your builds will target the board. If `echo $CC` prints nothing, you forgot to `source` the environment-setup script in *this* terminal.

---

# 8. Cross-compiling a CMake project with the SDK

Now we combine everything into the **exact pattern this project uses** to build the DMS. First, the whole thing; then a line-by-line explanation.

```bash
# 1. Activate the SDK in this shell (aarch64 cross-toolchain + sysroot):
source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-armv8a-poky-linux

# 2. Configure the build with CMake, telling it to use the SDK's toolchain file:
cmake -S . -B build-arm64 \
      -DCMAKE_TOOLCHAIN_FILE=$OECORE_NATIVE_SYSROOT/usr/share/cmake/OEToolchainConfig.cmake

# 3. Compile everything (using all CPU cores in parallel):
cmake --build build-arm64 --parallel

# 4. Confirm the result is really an aarch64 board binary:
file build-arm64/dms      # should say: ELF 64-bit LSB executable, ARM aarch64
```

## 8.1 Line-by-line

### Line 1 — `source ...environment-setup-armv8a-poky-linux`

Activates the SDK (Chapter 7). After this, `$CC`, `$CXX`, `$SDKTARGETSYSROOT`, and `$OECORE_NATIVE_SYSROOT` are all set, and CMake will find the cross-compiler. **Skip this and you'll accidentally build an x86-64 binary that won't run on the board.**

### Line 2 — `cmake -S . -B build-arm64 -DCMAKE_TOOLCHAIN_FILE=...`

This is the **configure** step. CMake reads the project's `CMakeLists.txt` and prepares the actual build files.

- `-S .` — the **S**ource directory is the current folder (`.`), where `CMakeLists.txt` lives.
- `-B build-arm64` — put all generated build files into a separate **B**uild directory called `build-arm64`. Keeping build output in its own folder (an "out-of-source build") keeps your source tree clean and lets you have both a laptop build and an ARM build side by side.
- `-DCMAKE_TOOLCHAIN_FILE=$OECORE_NATIVE_SYSROOT/usr/share/cmake/OEToolchainConfig.cmake` — the key flag. `-D` sets a CMake **variable**; here we set `CMAKE_TOOLCHAIN_FILE` to the SDK's **toolchain file**. This is how CMake learns to use the aarch64 cross-compiler and to look for libraries (like OpenCV) inside the **target sysroot** instead of on your laptop. `$OECORE_NATIVE_SYSROOT` expands to the SDK's native-sysroot path that Line 1 set.

### Line 3 — `cmake --build build-arm64 --parallel`

This is the **build/compile** step. `cmake --build build-arm64` runs the compiler over all the source files in the configured build directory. `--parallel` uses multiple CPU cores at once so the build finishes faster. The output includes the DMS executable at `build-arm64/dms`.

### Line 4 — `file build-arm64/dms`

The **`file`** command inspects a file and reports what kind it is. Running it on the freshly built binary is your proof that cross-compilation worked. For a correct board binary you expect something like:

```text
build-arm64/dms: ELF 64-bit LSB executable, ARM aarch64, version 1 (SYSV),
                 dynamically linked, interpreter /lib/ld-linux-aarch64.so.1, ...
```

Decoding that:

| Token | Meaning |
|---|---|
| **ELF** | Executable and Linkable Format — the standard Linux executable format |
| **64-bit** | A 64-bit program |
| **ARM aarch64** | Built for the **Arm 64-bit** architecture — i.e., **the board**, not your x86-64 laptop |
| **dynamically linked** | Uses shared libraries (like OpenCV) present on the board at run time |

If instead you saw `x86-64`, you forgot Line 1 (the `source`). The word **`aarch64`** is the green light that this binary belongs on the i.MX 93 EVK.

---

# 9. Transferring and running on the board

## 9.1 Copying the binary over the network with `scp`

The build happened on your laptop; the binary must now travel to the board. The tool is **`scp` (secure copy)** — it copies files over the network using SSH (the same secure channel used for remote login).

```bash
scp build-arm64/dms root@192.168.1.173:~/deploy/
```

Reading this command:

- `build-arm64/dms` — the **source**: the binary you just built on the laptop.
- `root@192.168.1.173` — the **destination machine and user**: log in as user **`root`** on the board at IP **`192.168.1.173`** (the board's address on the local network).
- `:~/deploy/` — the **destination path** on the board: the `deploy` folder inside root's home directory (`~` means "home"). Make sure it exists first (`ssh root@192.168.1.173 'mkdir -p ~/deploy'` if needed).

> **Analogy.** `scp` is like emailing a file, but directly between two computers over a secure, private line — you specify who (`root`), which computer (`192.168.1.173`), and which folder (`~/deploy/`) it lands in.

## 9.2 Running it on the board

Now log into the board (via `minicom` on `/dev/ttyUSB2`, or `ssh root@192.168.1.173`) and run the program:

```bash
# On the board:
cd ~/deploy
chmod +x dms      # mark the file as executable (give it "run" permission)
./dms             # run it  (the ./ means "the dms in THIS folder")
```

- **`chmod +x dms`** — `chmod` ("change mode") adjusts file permissions; `+x` adds the **executable** permission. Files arriving via `scp` may not be marked runnable, so you grant that here. Without it you'd get a "Permission denied" error.
- **`./dms`** — runs the program. The leading `./` tells the shell to run the `dms` file located **in the current directory** (the shell doesn't run programs from the current folder unless you say so explicitly).

## 9.3 Why the libraries are already there

The DMS depends on **OpenCV 4.10** (a large shared library). You did **not** have to copy OpenCV to the board, because the board's Linux image was built with NXP's **`imx-image-full`**, which **already includes OpenCV 4.10** in its rootfs. So when `./dms` starts and the system looks for the OpenCV shared libraries, it finds them already installed in `/usr/lib` on the board.

This is also why the **sysroot** on your laptop had to match the board's image: you compiled the DMS against the *same* OpenCV 4.10 that lives on the board, so the versions line up and the program loads cleanly. You can confirm the board's OpenCV version at any time:

```bash
# On the board:
opencv_version        # prints e.g. 4.10.0
```

---

# 10. Things that go wrong (and their fixes)

Real embedded work is a series of small obstacles. Here are the actual issues encountered in this project and exactly how they were resolved. Expect problems like these — they are normal.

## 10.1 OpenCV CMake config references missing test libraries

**Symptom.** During the CMake **configure** step (Chapter 8, Line 2), the build fails complaining that certain OpenCV libraries cannot be found — specifically the test/extra modules:

```text
libopencv_ts.so.4.10.0
libopencv_superres.so.4.10.0
```

**Why it happens.** OpenCV ships CMake **config files** in the SDK's sysroot that list *all* the modules OpenCV was configured with, including some (`opencv_ts` = the testing module, `opencv_superres` = super-resolution) whose actual `.so` shared-library files were **not packaged** into the SDK sysroot. CMake reads the list, tries to locate every listed library, and errors out when those two are absent — even though the DMS doesn't actually use them.

**Fix.** Create harmless **symlinks** (symbolic links — pointers that make one filename stand in for another) inside the **target sysroot's** library directory, so the referenced names resolve to an existing library. In practice you point the missing names at a real OpenCV `.so` (or a stub) so CMake's existence check passes:

```bash
# Make sure the SDK is sourced so $SDKTARGETSYSROOT is set:
source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-armv8a-poky-linux

cd $SDKTARGETSYSROOT/usr/lib

# Create symlinks so the missing test-module names resolve to an existing lib:
ln -sf libopencv_core.so.4.10.0 libopencv_ts.so.4.10.0
ln -sf libopencv_core.so.4.10.0 libopencv_superres.so.4.10.0
```

`ln -s` creates a symbolic link; `-f` forces it (overwriting a stale link). After this, `cmake` finds every name it looks for and configuration completes. (This satisfies CMake's file-existence check only; since the DMS never calls into those modules, pointing them at `libopencv_core` is safe.)

> **Analogy.** OpenCV's config is a **guest list** that names two people who never actually showed up. Rather than rewrite the list, you pin a **name tag** on a chair for each missing guest so the doorman's headcount passes.

## 10.2 No display on the board → GUI windows fail

**Symptom.** The DMS tries to open a window to show the camera feed (a typical OpenCV `imshow`, which on this image goes through **Qt/Wayland** — the graphics stack). But the board is being used **headless** (no monitor; you only have the serial console). The program crashes or errors with something like "cannot connect to display" / "no Wayland display" / Qt platform plugin failures.

**Why it happens.** GUI windows need a **display server** and a screen to draw on. **Wayland** is the modern Linux display system; **Qt** is the GUI library OpenCV uses here. With no monitor and no running graphical session on the serial-only board, there is nowhere to put a window, so any attempt to create one fails.

**Fix.** Give the DMS a **headless mode** — a way to run its full processing pipeline **without opening any GUI window**. Instead of displaying frames, in headless mode the program writes results to a file, prints them to the console, and/or streams them elsewhere. This is a command-line switch on the DMS, e.g.:

```bash
# On the board (serial console only): run without trying to open any window
./dms --headless
```

The lesson: on an embedded board you often have **no screen**, so design programs to run "blind," reporting via logs/files/network rather than pop-up windows.

## 10.3 Camera device nodes and GStreamer warnings

**Symptom.** The DMS needs a camera. On Linux, cameras appear as **device nodes** named **`/dev/video0`**, **`/dev/video1`**, etc. (each is a file the program opens to read frames). You may see the program fail to find a camera, or spew **GStreamer** warnings (GStreamer is the multimedia framework OpenCV can use to grab video):

```text
[ WARN ] GStreamer: ... could not link ... / no element ...
Unable to open camera /dev/video0
```

**Why it happens and how to work through it.**

- The camera might be on a **different node** than expected. List what's present:

```bash
# On the board: see which video device nodes exist
ls -l /dev/video*
```

- Not every `/dev/videoN` is a full camera — some are metadata/sub-device nodes. Try each index (`0`, `1`, ...) until frames come through. In OpenCV you select by index (`cv::VideoCapture(0)` opens `/dev/video0`).
- The **GStreamer warnings** are often non-fatal: they mean OpenCV tried one capture backend and fell back to another. If capture still works, you can usually ignore them. If not, ensure the needed GStreamer plugins are present (they generally are in `imx-image-full`) and that you're pointing at the right device index.
- Permissions: make sure the user (root here) can read the device node — as root on a dev image this is normally fine.

Practical approach used in the project: enumerate `/dev/video*`, try the working index, and treat benign GStreamer warnings as noise as long as frames are actually captured.

---

# 11. How the Ethos-U NPU fits in

## 11.1 The promise of the NPU

Recall from Chapter 1 that the i.MX 93 EVK includes an **Arm Ethos-U NPU** — a dedicated accelerator for neural-network math. A DMS leans on AI models (detecting faces, eyes-open/closed, head pose, gaze). Running those models on the NPU can be dramatically faster and more power-efficient than running them on the CPU.

## 11.2 The catch: models must be *prepared* for the NPU

The Ethos-U NPU is not a general-purpose processor; it accelerates a specific kind of model:

- **Quantized models.** **Quantization** means converting a model's numbers from high-precision decimals (floating-point) to small integers (usually 8-bit). This shrinks the model and lets specialized hardware run it fast. The Ethos-U works on **quantized TensorFlow Lite (`.tflite`)** models.
- **The `vela` compiler.** Even a quantized `.tflite` model must be **compiled** by NXP/Arm's **`vela`** tool into a form the Ethos-U can execute. `vela` takes a `.tflite` model and rewrites the parts it can accelerate to run on the NPU (the rest falls back to the CPU). Only after `vela` are you actually using the NPU.

```text
   trained model  ──►  quantize (to int8 .tflite)  ──►  vela compiler  ──►  NPU-ready model
                                                                             (runs on Ethos-U)
```

## 11.3 Why the DMS was slow on the board — and the `--fast` mode

Here is the crucial practical point. The DMS uses **OpenCV's DNN module** (OpenCV's built-in neural-network runner) to run its models. By default, **OpenCV's DNN runs on the CPU** — it does **not** use the Ethos-U NPU. There is no automatic hand-off; OpenCV DNN simply isn't wired to the Ethos-U out of the box.

Consequences:

- Heavy models executed through OpenCV DNN run entirely on the **Cortex-A55 CPU cores**, which are modest. So big/accurate models are **slow** on the board — low frames-per-second, laggy.
- Getting a model onto the NPU would require **converting it** (quantize to `.tflite`, run through `vela`) and running it via a TensorFlow Lite path with the Ethos-U delegate — a different, more involved pipeline than OpenCV DNN.

To keep the DMS **responsive on the board without** that NPU conversion work, the project provides a **`--fast` mode**: it uses lighter-weight models and/or cheaper processing so the whole pipeline keeps up in real time on the CPU alone.

```bash
# On the board: run in the lighter, real-time-friendly mode (CPU-only, no NPU needed)
./dms --fast
```

**In one sentence:** the NPU could accelerate the AI, but only for specially quantized+vela-compiled TensorFlow Lite models — and since the DMS runs its models through OpenCV's CPU-bound DNN module, the `--fast` mode exists to stay real-time on the CPU. (A future optimization would be to port the models to the Ethos-U path.)

---

# 12. Glossary & command cheat sheet

## 12.1 Glossary of every term & acronym

| Term | Meaning |
|---|---|
| **aarch64 / arm64 / armv8a** | The 64-bit Arm CPU instruction set; the architecture of the i.MX 93 EVK. |
| **Architecture (ISA)** | The set of machine instructions a CPU understands; code for one arch won't run on another. |
| **Baud rate** | Serial-link speed in bits/second; both ends must match (here **115200**). |
| **bitbake** | Yocto's build engine that reads recipes and cross-builds everything. |
| **bmaptool** | Fast, safe tool to flash a `.wic` image using a block map (`.bmap`). |
| **Bootloader** | First program at power-on; initializes hardware and loads the OS (here **U-Boot**). |
| **BSP (Board Support Package)** | Everything (bootloader, kernel, device tree) needed to boot Linux on a specific board. |
| **CC / CXX** | Environment variables naming the C / C++ compiler; set to the cross-compiler by the SDK. |
| **CFLAGS / CXXFLAGS** | Compiler flags (targeting options) set by the SDK's environment script. |
| **Clang / GCC** | Common C/C++ compilers; the SDK uses a GCC cross-compiler. |
| **CMake** | A tool that configures how a C/C++ project is built; the DMS uses it. |
| **CMAKE_TOOLCHAIN_FILE** | CMake variable pointing to `OEToolchainConfig.cmake` so CMake cross-compiles. |
| **Compiler** | Translates human source code into CPU machine code. |
| **Cross-compile** | Build on one architecture (host, x86-64) for another (target, aarch64). |
| **Cross-toolchain** | A toolchain whose tools target a different arch than the host. |
| **CPU** | Central Processing Unit — the main processor running your programs. |
| **Device node** | A special file representing hardware, e.g. `/dev/ttyUSB2` (serial) or `/dev/video0` (camera). |
| **Device tree (.dtb)** | Data file describing a board's hardware to the kernel. |
| **Distro (distribution)** | A ready-made Linux system (Ubuntu, Fedora). Yocto *builds* one; it isn't one. |
| **dd** | Low-level tool that copies raw bytes; can flash a `.wic` (dangerous if wrong device). |
| **DMS** | Driver Monitoring System — the C++ application in this project. |
| **DNN module** | OpenCV's neural-network runner; runs on **CPU** by default (not the NPU). |
| **eMMC** | Flash storage soldered onto the board; used for the installed system. |
| **ELF** | Executable and Linkable Format — the Linux executable file format. |
| **Embedded system** | A computer built into a larger device for a dedicated task. |
| **Embedded Linux** | Running Linux, usually trimmed-down, on an embedded device. |
| **Environment variable** | A named value in your shell that programs read (e.g. `$CC`, `$SDKTARGETSYSROOT`). |
| **EVK (Evaluation Kit)** | A ready-made development board built around an SoC for evaluation and development. |
| **Ethos-U** | The Arm NPU on the i.MX 93; accelerates quantized TFLite models. |
| **file (command)** | Reports what kind a file is; used to confirm a binary is `ARM aarch64`. |
| **Flashing** | Writing an OS image onto storage (SD/eMMC). |
| **GStreamer** | Multimedia framework OpenCV can use to capture video; may emit benign warnings. |
| **Headless** | Running without a display/GUI; the mode the DMS uses on the serial-only board. |
| **Host** | The machine where you build (the x86-64 laptop). |
| **Image** | The bootable OS package a Yocto build produces (kernel + rootfs + bootloader). |
| **imx-image-full** | NXP's comprehensive Yocto image; includes OpenCV 4.10. |
| **IP address** | A machine's numeric address on a network (board = **192.168.1.173**). |
| **Kernel** | The core of the OS; manages hardware and runs programs (here Linux **6.6**). |
| **Layer (meta-\*)** | A folder grouping related Yocto recipes and config. |
| **Linker** | Combines compiled code with libraries into one executable. |
| **Linux** | The open-source operating system used on the board. |
| **meta-imx** | NXP's Yocto layers providing the i.MX BSP. |
| **minicom** | Terminal program to view/use a serial console. |
| **Native compile** | Building for the same architecture you build on. |
| **NPU (Neural Processing Unit)** | Hardware accelerator for neural-network math (here Ethos-U). |
| **NXP** | The chip maker behind the i.MX 9 family and its BSP. |
| **OECORE_NATIVE_SYSROOT** | SDK variable: path to the host-side (native) sysroot with the cross-compiler & CMake files. |
| **OEToolchainConfig.cmake** | The SDK's CMake toolchain file that enables cross-compilation. |
| **OpenCV** | Computer-vision library the DMS uses (version **4.10** on the board). |
| **OpenEmbedded (OE)** | The build framework and recipe collection underlying Yocto. |
| **Operating system (OS)** | Core software managing hardware and running applications. |
| **Poky** | Yocto's reference distribution you customize from; source of `-poky-linux`. |
| **populate_sdk** | The bitbake task that generates an installable SDK. |
| **Quantization** | Converting model numbers to small integers (int8) so accelerators can run them. |
| **Recipe (.bb)** | Instructions to build one software component in Yocto. |
| **rootfs (root filesystem)** | All files/programs/libraries of the running Linux system. |
| **scp** | Secure copy — transfer files to/from the board over SSH. |
| **SD card** | Removable memory card; convenient for development booting. |
| **SDK** | Software Development Kit — the packaged cross-toolchain + sysroot. |
| **SDKTARGETSYSROOT** | SDK variable: path to the **target** sysroot (board's headers & libraries). |
| **Serial console** | A text link (over `/dev/ttyUSB2`) to see boot output and log in. |
| **SoC (System on a Chip)** | A single chip integrating CPU, memory controllers, graphics, NPU, etc. |
| **source (command)** | Run a script in the current shell so its variables persist (needed for the SDK setup). |
| **SSH** | Secure Shell — encrypted remote login/networking (underlies `scp`). |
| **Symlink** | A symbolic link; a filename that points to another file. |
| **sysroot** | A host-side mirror of the target's filesystem (headers + libraries) for cross-building. |
| **Target** | The machine the program will run on (the aarch64 board). |
| **TensorFlow Lite (.tflite)** | Lightweight model format runnable on the NPU after `vela`. |
| **Toolchain** | The coordinated set of build tools (compiler, assembler, linker, etc.). |
| **U-Boot** | The board's bootloader. |
| **uname** | Command to print system info; `uname -m` shows the architecture, `uname -a` shows all. |
| **vela** | Arm/NXP compiler that prepares a `.tflite` model to run on the Ethos-U NPU. |
| **Wayland** | Modern Linux display server; GUI windows need it (absent on the headless board). |
| **.wic** | A full disk image file laid out for flashing to SD/eMMC. |
| **x86-64 / amd64** | The Intel/AMD 64-bit instruction set (your laptop's architecture). |
| **Yocto Project** | A build system that creates a custom Linux distribution for your hardware. |

## 12.2 Command cheat sheet

### bitbake basics (on the Yocto build machine)

| Goal | Command |
|---|---|
| Build the full image | `bitbake imx-image-full` |
| Build one recipe | `bitbake <recipe-name>` |
| Generate the installable SDK | `bitbake imx-image-full -c populate_sdk` |
| Clean a recipe's build | `bitbake -c cleanall <recipe-name>` |

### Activating the SDK (on your dev laptop)

| Goal | Command |
|---|---|
| Activate the cross-toolchain in this shell | `source /opt/fsl-imx-wayland/6.6-scarthgap/environment-setup-armv8a-poky-linux` |
| Check which compiler is active | `echo $CC` |
| Check compiler version/target (look for `aarch64`) | `$CC --version` |
| See the target sysroot path | `echo $SDKTARGETSYSROOT` |
| See the native sysroot path | `echo $OECORE_NATIVE_SYSROOT` |

### Cross-compiling the CMake project

| Goal | Command |
|---|---|
| Configure with the SDK toolchain file | `cmake -S . -B build-arm64 -DCMAKE_TOOLCHAIN_FILE=$OECORE_NATIVE_SYSROOT/usr/share/cmake/OEToolchainConfig.cmake` |
| Build (all cores) | `cmake --build build-arm64 --parallel` |
| Verify it's an ARM binary | `file build-arm64/dms` → expect `ELF 64-bit LSB executable, ARM aarch64` |

### Fixing the OpenCV missing-libs issue

| Goal | Command |
|---|---|
| Go to target sysroot libs | `cd $SDKTARGETSYSROOT/usr/lib` |
| Symlink the missing test module | `ln -sf libopencv_core.so.4.10.0 libopencv_ts.so.4.10.0` |
| Symlink the missing superres module | `ln -sf libopencv_core.so.4.10.0 libopencv_superres.so.4.10.0` |

### Transferring & running on the board

| Goal | Command |
|---|---|
| Copy the binary to the board | `scp build-arm64/dms root@192.168.1.173:~/deploy/` |
| Log into the board over the network | `ssh root@192.168.1.173` |
| Make it executable (on the board) | `chmod +x dms` |
| Run it (on the board) | `./dms` |
| Run headless (no GUI) | `./dms --headless` |
| Run the lightweight real-time mode | `./dms --fast` |

### Serial console with minicom

| Goal | Command |
|---|---|
| Open the board's serial console | `sudo minicom -D /dev/ttyUSB2 -b 115200` |
| Exit minicom | `Ctrl-A` then `X` |
| minicom menu | `Ctrl-A` then `Z` |

### Checking the board (run these on the board)

| Goal | Command | What you learn |
|---|---|---|
| See the OS/distro details | `cat /etc/os-release` | Distro name & version of the image |
| See kernel & arch | `uname -a` | Kernel **6.6**, and `aarch64` |
| Just the architecture | `uname -m` | `aarch64` |
| OpenCV version | `opencv_version` | e.g. `4.10.0` |
| List camera device nodes | `ls -l /dev/video*` | Which `/dev/videoN` exist |

---

## Closing note

You now have the full mental model behind this project: an **embedded board** (i.MX 93 EVK, aarch64, with an Ethos-U NPU) running a **custom Linux** built by the **Yocto Project** (Poky + OpenEmbedded + NXP's meta-imx BSP), producing **U-Boot + kernel 6.6 + device tree + rootfs** in a `.wic` image. You reach the board over the **serial console** (`/dev/ttyUSB2` at 115200) and the **network** (`192.168.1.173`). You **cross-compile** the C++ DMS on your laptop using the **Yocto SDK** at `/opt/fsl-imx-wayland/6.6-scarthgap/`, activated by `source`-ing its environment-setup script, and driven through **CMake** with `OEToolchainConfig.cmake`. You verify the binary with `file`, `scp` it over, and run it — falling back to `--headless` and `--fast` modes because the board has no display and the NPU isn't used by OpenCV's CPU-bound DNN.

Re-read any chapter as needed, and keep the cheat sheet close. Welcome to embedded Linux, Hemanth.
