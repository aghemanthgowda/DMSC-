# Linux & Ubuntu — From Zero to Confident

Welcome, Hemanth! This guide starts at absolute zero. It assumes you have **never** written code, never used a "terminal", and maybe have only ever used a phone or a Windows computer by clicking on things. That is completely fine. By the end of this guide you will understand what Linux and Ubuntu are, and you will be able to move around, run commands, install software, and talk to the embedded board used in the Driver Monitoring System (DMS) project — all with confidence.

Read this slowly. Try every example on a real Ubuntu computer as you go. You learn this by **doing**, not by reading alone.

## What you'll learn

By working through this guide you will be able to:

- Explain what an operating system, Linux, and Ubuntu are — in your own words.
- Open a terminal and understand what you are looking at.
- Move around the file system, create and delete files and folders.
- Read and edit text files without a mouse.
- Understand permissions, `sudo`, and why they matter.
- Install the exact software libraries this project needed.
- Watch what your computer is doing (processes, memory, disk).
- Connect to another computer over the network with `ssh` and copy files with `scp`.
- Talk to the NXP i.MX 93 board over a **serial console** using `minicom`.
- Use the Bash shell like a pro: variables, pipes, wildcards, and shortcuts.

---

## Table of Contents

1. [Chapter 1 — Big Picture: Operating Systems, Linux, and Ubuntu](#chapter-1--big-picture-operating-systems-linux-and-ubuntu)
2. [Chapter 2 — The Ubuntu Desktop and Opening a Terminal](#chapter-2--the-ubuntu-desktop-and-opening-a-terminal)
3. [Chapter 3 — The Terminal in Depth](#chapter-3--the-terminal-in-depth)
4. [Chapter 4 — The Linux Filesystem Hierarchy](#chapter-4--the-linux-filesystem-hierarchy)
5. [Chapter 5 — Essential Commands](#chapter-5--essential-commands)
6. [Chapter 6 — Viewing and Editing Files (nano and vim)](#chapter-6--viewing-and-editing-files-nano-and-vim)
7. [Chapter 7 — Permissions, Ownership, and sudo](#chapter-7--permissions-ownership-and-sudo)
8. [Chapter 8 — Package Management with APT](#chapter-8--package-management-with-apt)
9. [Chapter 9 — Processes and the System](#chapter-9--processes-and-the-system)
10. [Chapter 10 — Networking Basics for This Project](#chapter-10--networking-basics-for-this-project)
11. [Chapter 11 — Serial Consoles and minicom](#chapter-11--serial-consoles-and-minicom)
12. [Chapter 12 — The Bash Shell Essentials](#chapter-12--the-bash-shell-essentials)
13. [Chapter 13 — Keyboard Shortcuts Cheat Sheet](#chapter-13--keyboard-shortcuts-cheat-sheet)
14. [Chapter 14 — Applications You'll Use](#chapter-14--applications-youll-use)
15. [Chapter 15 — Command Cheat Sheet, Common Mistakes, and Practice](#chapter-15--command-cheat-sheet-common-mistakes-and-practice)

---

## Chapter 1 — Big Picture: Operating Systems, Linux, and Ubuntu

Before touching a single command, let's build a mental picture of what's going on inside a computer. If this picture is clear, everything else becomes much easier.

### 1.1 What is an operating system?

A computer is just a pile of hardware: a **processor** (the "brain" that does calculations, also called the CPU), **memory** (fast, temporary storage called RAM), a **disk** (slow, permanent storage where your files live), a screen, a keyboard, and so on. On its own, this hardware does nothing useful. It needs a manager.

An **operating system** (often shortened to **OS**) is the master program that manages all of that hardware and lets other programs run on top of it. Think of it as the **manager of a big kitchen**:

- The cooks (your programs — a web browser, a game, the DMS software) want to get work done.
- The manager (the OS) decides who gets to use the stove (the CPU), how much counter space each cook gets (the memory), and where ingredients are stored (the disk).
- The cooks don't talk to the stove directly; they ask the manager.

Without an operating system, every program would have to know the exact details of your specific keyboard, your specific disk, your specific screen. That would be impossible. The OS hides all those messy details and gives every program a clean, shared way to use the hardware.

Examples of operating systems you may have heard of: **Windows** (from Microsoft), **macOS** (from Apple, on Mac computers), **Android** and **iOS** (on phones), and **Linux**.

### 1.2 What is Linux?

**Linux** is an operating system, like Windows or macOS — but with two important differences:

1. It is **free**. You do not pay for it.
2. It is **open source**. "Open source" means the source code — the actual instructions that make up the software — is public. Anyone in the world can read it, learn from it, and improve it. Windows is "closed source": only Microsoft can see and change its code.

Linux was started in 1991 by a student named Linus Torvalds. Today it is maintained by thousands of people and companies around the world. It runs almost everything important behind the scenes: most websites, most cloud servers, most Android phones (Android is built on Linux), supercomputers, cars, TVs — and **embedded boards like the NXP i.MX 93 used in this DMS project**. That is a big reason to learn it: professional embedded and server work runs on Linux.

### 1.3 What is the "kernel"?

Here is a subtle but important point. The word "Linux" technically refers to just one piece: the **kernel**.

The **kernel** is the innermost core of the operating system. It is the part that talks directly to the hardware. Going back to the kitchen analogy, the kernel is the manager standing right next to the stove and the fridge, physically handing out access. When a program wants to save a file, read the camera, or use memory, the request ultimately goes to the kernel.

The kernel handles:

- **Processes** — running programs (deciding which one uses the CPU right now).
- **Memory** — giving each program its slice of RAM and keeping them from stepping on each other.
- **Devices** — talking to hardware like the disk, the camera (`/dev/video0`), and the serial port (`/dev/ttyUSB2`). We will meet these device names soon.
- **The filesystem** — organizing your files on disk.

You almost never talk to the kernel directly. You talk to programs, and those programs ask the kernel. But it's good to know the kernel is the beating heart underneath.

### 1.4 What is a "distribution"?

The kernel by itself is not enough to be useful. You also need:

- A way to type commands (a **shell**).
- Basic tools (to copy files, edit text, connect to networks).
- A way to install more software (a **package manager**).
- Often a graphical desktop (windows, icons, a mouse pointer).

A **distribution** (usually shortened to **distro**) is a complete, ready-to-use bundle: the Linux kernel **plus** all those extra tools and programs, packaged together so you can install it and start working. Think of it like this:

- The **kernel** is the engine.
- A **distribution** is the whole car — engine, seats, steering wheel, dashboard — built around that engine and ready to drive.

There are many distributions, each with different choices and personalities: **Ubuntu**, **Debian**, **Fedora**, **Arch**, **Linux Mint**, and many more. They all share the same Linux kernel underneath, but they package different tools and look different.

### 1.5 What is Ubuntu?

**Ubuntu** is one specific Linux distribution, made by a company called Canonical. It is one of the most popular distributions in the world, especially for beginners, because:

- It is easy to install and use.
- It has a friendly graphical desktop.
- It has a huge amount of software available and lots of help online.
- It uses a package manager called **APT**, which we will use heavily in this guide to install the project's libraries.

When you used "Ubuntu on your laptop" for the DMS project, you were using the Linux kernel wrapped inside the Ubuntu distribution. So:

- **Linux** = the kernel (and loosely, the family of these operating systems).
- **Ubuntu** = a specific, complete, beginner-friendly Linux distribution built on the Linux kernel.

### 1.6 What is a shell / terminal?

A **shell** is a program that reads text commands you type and asks the operating system to carry them out. It is a way to control the computer by **typing** instead of clicking.

A **terminal** (also called a "terminal emulator" or "console") is the window where you see and type into the shell. Loosely, people say "the terminal" and "the shell" to mean the same thing: the black window where you type commands.

The most common shell on Ubuntu is called **Bash** (short for "Bourne Again SHell"). We will spend a lot of time in Bash, because that is where real Linux power lives.

Why bother typing commands when you could click? Because:

- It is **faster** for many tasks once you learn it.
- It is **precise** — you say exactly what you want.
- It can be **automated** and repeated.
- On the **embedded board and over the serial console, there is no mouse and no desktop** — text commands are the *only* way to work. This is exactly why you needed it for the DMS board.

### 1.7 Linux vs. Ubuntu vs. Windows — a comparison

Let's put the three side by side.

| Topic | Windows | Linux (general) | Ubuntu (a Linux distro) |
|---|---|---|---|
| Made by | Microsoft (a company) | Community + many companies | Canonical (a company) + community |
| Cost | Paid | Free | Free |
| Source code | Closed (private) | Open (public) | Open (public) |
| Main way to control | Mostly clicking (GUI) | Clicking **and** terminal | Clicking **and** terminal |
| Installing software | Download .exe and double-click, or Microsoft Store | Package manager | `apt` package manager + App store |
| Common in | Home PCs, offices | Servers, phones, embedded, cloud | Desktops, servers, learning |
| File paths use | Backslash `\` (e.g. `C:\Users`) | Forward slash `/` (e.g. `/home`) | Forward slash `/` |
| Drive letters | Yes (`C:`, `D:`) | No — one single tree from `/` | No — one single tree from `/` |

A few key mental shifts coming from Windows:

- On Windows, paths look like `C:\Users\Hemanth\file.txt`. On Linux there are **no drive letters**. Everything hangs off a single top folder called `/` (pronounced "root" or "slash"), and paths use forward slashes: `/home/hemanth/file.txt`.
- On Windows, file names ignore case (`File.txt` and `file.txt` are the same). On Linux, **case matters**: `File.txt` and `file.txt` are two different files. Be careful and consistent.
- On Linux, a file's name does **not** need an extension like `.txt` or `.exe` to be usable. Extensions are just a convention for humans.

Keep these differences in mind and you'll avoid a lot of early confusion.

---

## Chapter 2 — The Ubuntu Desktop and Opening a Terminal

When Ubuntu starts, you log in with your username and password, and you arrive at the **desktop**. The desktop environment Ubuntu uses is called **GNOME**. You don't need to memorize that name, but you'll see it mentioned online.

### 2.1 What you see on the desktop

- **Wallpaper**: the background image.
- **Top bar**: usually shows the clock in the middle, and system icons (network, sound, battery, power) on the right.
- **Dock**: a strip of application icons, usually on the left side of the screen. It holds shortcuts to common apps like Firefox (web browser), Files, and the Terminal.
- **Activities / Show Applications**: a button (often top-left, or the "Super" key — see below) that shows all your open windows and lets you search for apps. The "Show Applications" button (a grid of dots, usually at the bottom of the dock) shows every installed app.

The **Super key** is the key with the Windows logo on most keyboards (on a Mac keyboard it's the Command key). In Ubuntu, tapping **Super** opens the Activities overview where you can search for anything. Try tapping it, typing `term`, and pressing Enter — that opens the Terminal.

### 2.2 Opening a terminal — four ways

This is the single most important skill in this guide. Here are several ways to open a terminal; pick whichever you like.

| Method | Steps |
|---|---|
| Keyboard shortcut (fastest) | Press **Ctrl + Alt + T** |
| Search | Tap **Super**, type `terminal`, press **Enter** |
| Show Applications | Click the grid-of-dots icon in the dock, find **Terminal**, click it |
| Right-click (sometimes) | Right-click on the desktop and choose "Open Terminal" if available |

The classic and most reliable one to memorize is **Ctrl + Alt + T**. Do it now. A window opens with some text and a blinking cursor waiting for you. That's the shell, ready for your commands. In the next chapter we'll understand exactly what we're looking at.

### 2.3 A word about staying calm

The terminal can feel intimidating because it just sits there silently. Remember: it is patient. It does **nothing** until you press Enter. You can type, look at it, delete it, and think. Nothing happens until you commit with Enter. And most commands you'll learn here are safe to explore. The few dangerous ones are clearly marked in this guide with a warning.

---

## Chapter 3 — The Terminal in Depth

Let's slow down and really understand the terminal window.

### 3.1 The prompt

When you open the terminal, you see something like this:

```text
hemanth@laptop:~$
```

That line is called the **prompt**. It is the shell telling you "I'm ready — type here." Let's break it apart piece by piece:

| Part | Meaning |
|---|---|
| `hemanth` | Your **username** — who you are logged in as |
| `@` | Just a separator, read as "at" |
| `laptop` | The **hostname** — the name of this computer |
| `:` | A separator |
| `~` | Your **current location** (current directory). `~` is short for your home folder |
| `$` | Marks the end of the prompt. `$` means a normal user. `#` would mean the all-powerful root user |

So `hemanth@laptop:~$` reads as: "User *hemanth*, on the computer *laptop*, currently in the *home* folder, ready for a command." After the `$` is where your typing appears.

**Important:** In this guide and in most tutorials, code examples show the command **without** the prompt, or with just a `$`. When you see:

```bash
pwd
```

it means "type `pwd` and press Enter." Do **not** type the prompt part yourself.

### 3.2 Running your first command

Type this and press Enter:

```bash
whoami
```

Expected output:

```text
hemanth
```

`whoami` simply prints the username you are logged in as. Congratulations — you just ran a Linux command. The shell read your word, asked the OS "who is this user?", and printed the answer.

### 3.3 Anatomy of a command: program, arguments, and flags

Almost every command has the same shape:

```text
program   [flags]   [arguments]
```

- The **program** (or command) is the first word — the thing you want to run, e.g. `ls`.
- **Arguments** are the extra words you give it — usually *what* to act on, e.g. a filename or folder.
- **Flags** (also called **options** or **switches**) change *how* the program behaves. They usually start with a dash `-` (short form) or two dashes `--` (long form).

Example:

```bash
ls -l /home
```

Here:

- `ls` is the program (it lists files).
- `-l` is a flag (it means "long format" — show extra detail).
- `/home` is the argument (the folder to list).

Short flags can often be combined. `ls -l -a` is the same as `ls -la`. Order of separate flags usually doesn't matter.

A very useful universal flag is `--help`. Almost every program prints a short help message if you add it:

```bash
ls --help
```

That prints a wall of text explaining every flag `ls` accepts. Don't try to read it all — just skim for what you need.

### 3.4 The current working directory

At every moment, your shell is "sitting inside" one folder. That folder is called the **current working directory** (often shortened to **CWD** or just "the working directory"). Think of it as *where you are standing* in the file system. When you run a command without specifying a location, it usually acts on your current directory.

To ask "where am I right now?", use `pwd` (which stands for **p**rint **w**orking **d**irectory):

```bash
pwd
```

Expected output:

```text
/home/hemanth
```

That output is a **path** — a chain of folders separated by `/`, describing exactly where you are, starting from the top (`/`). We'll cover paths in detail in Chapter 4 and Chapter 5. For now, just know: `pwd` answers "where am I?"

### 3.5 Typing tips right away

Two habits will make your life dramatically easier from day one:

- **Tab completion**: Start typing a command or a filename, then press the **Tab** key. The shell will try to finish the word for you. If several things match, press Tab twice to see the list. This saves typing and prevents spelling mistakes. Try typing `pw` then Tab — it completes to `pwd`.
- **Up-arrow history**: Press the **Up arrow** key to bring back the previous command you ran. Keep pressing it to go further back. This saves you from retyping. The **Down arrow** goes the other way.

We'll return to these and many more shortcuts in Chapter 12 and Chapter 13.

---

## Chapter 4 — The Linux Filesystem Hierarchy

On Linux, all files and folders live in **one single tree** that starts at the top with `/`. There are no `C:` or `D:` drives. Even a USB stick or a second hard disk gets "attached" (the word is **mounted**) somewhere inside this one tree. Understanding this tree is essential.

### 4.1 The idea of a tree

Picture an upside-down tree. The trunk at the top is `/`. Big branches come off it (`/home`, `/etc`, `/usr`, and so on). Smaller branches come off those. The leaves are your actual files. A **folder** is called a **directory** in Linux language — the two words mean the same thing.

Here is a simplified picture:

```text
/
├── home/
│   └── hemanth/        <- your personal space ("home directory")
│       ├── Documents/
│       ├── Downloads/
│       └── DMSC-/      <- your project folder
├── etc/                <- system configuration files
├── usr/                <- installed programs and libraries
├── dev/                <- devices (camera, serial ports...)
├── tmp/                <- temporary files
├── opt/                <- optional / add-on software
├── var/                <- changing data (logs, etc.)
└── root/               <- the home of the "root" superuser
```

### 4.2 The main top-level directories

Here is what each important top-level directory is for. You do not need to memorize all of this, but you should recognize these names when you see them.

| Directory | Name / say it as | What it holds | Why you care |
|---|---|---|---|
| `/` | "root" or "slash" | The very top of everything | Every path ultimately starts here |
| `/home` | "home" | Personal folders, one per user | Your files live in `/home/hemanth` |
| `/root` | "root's home" | The home folder of the **root** superuser | Different from `/`! It's root's personal space |
| `/etc` | "etc" (et-see) | System-wide **configuration** files (text settings) | Network config, service settings live here |
| `/usr` | "user" | Installed programs, libraries, shared data | Most software you install lands under here |
| `/dev` | "dev" | **Device** files representing hardware | Camera and serial port appear here |
| `/tmp` | "temp" | **Temporary** files, wiped on reboot | Scratch space; don't keep anything important here |
| `/opt` | "opt" | **Optional**, self-contained add-on software | Some third-party tools install here |
| `/var` | "var" | **Variable** data that changes: logs, caches, mail | System logs live in `/var/log` |
| `/bin`, `/sbin` | "bin", "s-bin" | Essential command programs | `ls`, `cp` and friends live here |
| `/lib` | "lib" | Shared **libraries** programs need | Supporting code for programs |
| `/mnt`, `/media` | "mount", "media" | Where external disks/USB get attached | Your USB stick may appear under `/media` |

Note the important difference: `/` is the top of the whole tree, while `/root` is just the personal home folder belonging to the special **root** user. They are not the same thing, even though both involve the word "root". (This is a classic beginner confusion — now you won't fall for it.)

### 4.3 Your home directory

The most important folder for you day-to-day is your **home directory**: `/home/hemanth`. This is *your* space. You own it. You can freely create, edit, and delete files here without special permission. When you open a terminal, you usually start here.

There is a handy shortcut for your home directory: the tilde character `~`. Anywhere you could type `/home/hemanth`, you can instead type `~`. So `~/Downloads` means `/home/hemanth/Downloads`. That's why the prompt showed `~` — it means "you are in your home directory."

### 4.4 Devices in /dev — the ones this project uses

This is where Linux does something clever that surprises newcomers: **hardware devices appear as files** inside `/dev`. You can "read from" and "write to" a piece of hardware as if it were a file. This is a famous Linux idea: *"everything is a file."*

Two device files matter directly for the DMS project:

- **`/dev/video0`** — This is the **camera**. When you plug in a USB camera (or use the board's built-in camera), Linux creates a device file called `/dev/video0` (the next camera would be `/dev/video1`, and so on). The DMS software opens `/dev/video0` to grab video frames of the driver's face. When your program "reads from the camera," it is really reading from this file. To use the camera, your user must be allowed to access it — that's handled by the **`video` group**, which we cover in Chapter 7.

- **`/dev/ttyUSB2`** — This is a **serial port** provided by a USB-to-serial adapter. "Serial" means data travels one bit after another over a simple wire. When you connect a USB serial cable between your laptop and the i.MX 93 board, Linux creates a device file like `/dev/ttyUSB2`. The program `minicom` opens this file to send your keystrokes to the board and show the board's text output. (The exact number can vary — it might be `/dev/ttyUSB0`, `/dev/ttyUSB1`, or `/dev/ttyUSB2` depending on what's plugged in.) We use this heavily in Chapter 11.

You will rarely "open" these files by hand. Instead, programs like the DMS app or `minicom` open them for you. But now you understand what they are: **files that stand in for real hardware.**

To see them, you can list them:

```bash
ls -l /dev/video0
ls -l /dev/ttyUSB2
```

Example output:

```text
crw-rw----+ 1 root video    81, 0 Aug 11 09:12 /dev/video0
crw-rw---- 1 root dialout 188, 2 Aug 11 09:12 /dev/ttyUSB2
```

Notice the words `video` and `dialout` — those are the **groups** that own these devices. Your user needs to belong to the right group to use them. More on that in Chapter 7.

---

## Chapter 5 — Essential Commands

This is the biggest and most practical chapter. Here you learn the everyday commands to move around, look at things, and manage files. **Type every example yourself.** Reading is not enough; your fingers need the practice.

Throughout, remember: after typing a command you must press **Enter** to run it.

### 5.1 pwd — where am I?

```bash
pwd
```

```text
/home/hemanth
```

`pwd` = **p**rint **w**orking **d**irectory. Whenever you feel lost, run `pwd`.

### 5.2 ls — list what's here

`ls` = **l**i**s**t. It shows the files and folders in a directory.

```bash
ls
```

```text
Desktop  Documents  Downloads  Music  Pictures  DMSC-
```

By default `ls` lists the current directory. You can give it a path to list somewhere else:

```bash
ls /etc
```

#### Useful ls flags

| Flag | Meaning | Effect |
|---|---|---|
| `-l` | long | One item per line, with details (permissions, owner, size, date) |
| `-a` | all | Show **hidden** files too (names starting with `.`) |
| `-h` | human | Show sizes in KB/MB/GB instead of raw bytes (use with `-l`) |

Long listing:

```bash
ls -l
```

```text
total 24
drwxr-xr-x 2 hemanth hemanth 4096 Aug 10 14:02 Desktop
drwxr-xr-x 5 hemanth hemanth 4096 Aug 11 09:00 DMSC-
-rw-r--r-- 1 hemanth hemanth  220 Aug 09 18:44 notes.txt
```

Each line packs a lot in. Reading the last line left to right: `-rw-r--r--` are the **permissions** (Chapter 7), `1` is a link count, `hemanth hemanth` are the **owner** and **group**, `220` is the size in bytes, `Aug 09 18:44` is the last-modified time, and `notes.txt` is the name. A line starting with `d` (like `Desktop`) is a **directory**; a line starting with `-` is a regular file.

Show hidden files. On Linux, any file whose name starts with a dot `.` is **hidden** by default (often config files):

```bash
ls -a
```

```text
.  ..  .bashrc  .config  Desktop  Documents  DMSC-  notes.txt
```

Notice `.` and `..` — those are special. `.` means "this directory" and `..` means "the directory above this one" (the parent). We use these constantly for navigation.

Combine flags for a friendly, complete listing:

```bash
ls -lah
```

```text
total 32K
drwxr-xr-x 8 hemanth hemanth 4.0K Aug 11 09:00 .
drwxr-xr-x 3 root    root    4.0K Aug 01 08:00 ..
-rw-r--r-- 1 hemanth hemanth 220  Aug 09 18:44 .bashrc
drwxr-xr-x 5 hemanth hemanth 4.0K Aug 11 09:00 DMSC-
```

`-lah` = long + all + human-readable. This is many people's favorite everyday `ls`.

### 5.3 cd — change directory (move around)

`cd` = **c**hange **d**irectory. This is how you "walk" into a different folder.

```bash
cd Downloads
pwd
```

```text
/home/hemanth/Downloads
```

To understand `cd`, you must understand **paths**. There are two kinds.

#### Absolute vs. relative paths

- An **absolute path** starts from the top `/` and spells out the full route, e.g. `/home/hemanth/Downloads`. It works no matter where you currently are. Think of it like a full postal address with country, city, and street.
- A **relative path** starts from where you currently are. If you are in `/home/hemanth`, then `Downloads` (relative) means `/home/hemanth/Downloads`. Think of it like saying "the room next door" — it only makes sense relative to where you're standing.

#### Special path shortcuts

| Symbol | Means | Example |
|---|---|---|
| `.` | The current directory | `./run.sh` runs a script in the current folder |
| `..` | The parent directory (one level up) | `cd ..` goes up one folder |
| `~` | Your home directory | `cd ~` or just `cd` goes home |
| `/` | The top (root) of the filesystem | `cd /` goes to the very top |
| `-` | The previous directory you were in | `cd -` jumps back |

Worked navigation example. Follow along:

```bash
cd ~                 # go to home: /home/hemanth
cd DMSC-             # relative move into the project folder
pwd                  # /home/hemanth/DMSC-
cd ..                # up one level, back to /home/hemanth
cd /etc              # absolute move to /etc
cd -                 # jump back to /home/hemanth
cd                   # bare cd also goes home
```

`cd` with no argument always takes you home. `cd ..` from `/home/hemanth` takes you to `/home`. Two levels up would be `cd ../..`.

### 5.4 mkdir — make a directory

`mkdir` = **m**a**k**e **dir**ectory.

```bash
mkdir practice
ls
```

```text
Desktop  Documents  Downloads  practice  DMSC-
```

To create nested folders in one go, add the `-p` flag (it creates parents as needed):

```bash
mkdir -p project/src/utils
```

That creates `project`, then `src` inside it, then `utils` inside that — all at once. Without `-p`, `mkdir` fails if the parent folders don't already exist.

### 5.5 rmdir — remove an empty directory

`rmdir` = **r**e**m**ove **dir**ectory. It only removes **empty** directories, which makes it safe.

```bash
rmdir practice
```

If the folder has anything inside it, `rmdir` refuses:

```text
rmdir: failed to remove 'practice': Directory not empty
```

That refusal is a safety feature. To remove a non-empty folder you'd use `rm -r` — see the warning in section 5.10.

### 5.6 touch — create an empty file (or update its timestamp)

`touch` creates a new, empty file if it doesn't exist:

```bash
touch notes.txt
ls -l notes.txt
```

```text
-rw-r--r-- 1 hemanth hemanth 0 Aug 11 09:20 notes.txt
```

The size is `0` because it's empty. If the file already exists, `touch` just updates its "last modified" time to now (that's the literal meaning — you "touched" it).

### 5.7 cp — copy files and folders

`cp` = **c**o**p**y. The shape is `cp SOURCE DESTINATION`.

```bash
cp notes.txt notes_backup.txt
ls
```

```text
notes.txt  notes_backup.txt
```

To copy a whole folder, add `-r` (recursive — meaning "include everything inside, going deep"):

```bash
cp -r project project_copy
```

You can copy into another folder:

```bash
cp notes.txt ~/Documents/
```

That copies `notes.txt` into your Documents folder, keeping the same name.

### 5.8 mv — move or rename

`mv` = **m**o**v**e. It does two jobs, depending on the destination:

Move a file into another folder:

```bash
mv notes.txt ~/Documents/
```

**Rename** a file (move it "to a new name in the same place"):

```bash
mv notes_backup.txt notes_old.txt
```

There is no separate "rename" command in Linux — renaming *is* moving. Unlike `cp`, you do not need `-r` to move a folder.

### 5.9 rm — remove (delete) files

`rm` = **r**e**m**ove. It **deletes** files. There is no Recycle Bin here — deletion is usually permanent.

```bash
rm notes_old.txt
```

To delete a folder and everything in it, you need `-r` (recursive):

```bash
rm -r project_copy
```

### 5.10 The danger of rm -rf — read this carefully

> **WARNING — the most dangerous command for beginners.**
>
> `rm -rf something` means: **r**e**m**ove, **r**ecursively (go into every subfolder), **f**orce (don't ask for confirmation, ignore errors). It deletes the target and *everything inside it* instantly and permanently. There is no undo and no trash bin.

A safe example:

```bash
rm -rf practice_folder
```

The famous catastrophe to **never** run:

```bash
# DO NOT RUN THIS — shown only so you recognize and fear it
rm -rf /
```

That would attempt to erase the **entire system** starting from the top of the tree. Modern systems block that exact one, but variations can still wreck your machine.

How to stay safe with `rm`:

- **Always run `pwd` and `ls` first** so you know exactly where you are and what's there.
- **Avoid `-f`** unless you truly need it. Plain `rm -r` will at least stop on some errors.
- **Be extremely careful with `*`** (the wildcard). `rm -rf *` deletes everything in the current folder. A stray space, as in `rm -rf / home` instead of `rm -rf /home`, is a disaster.
- When unsure, use the `-i` flag (**i**nteractive) which asks before each delete: `rm -i notes.txt`.

Treat `rm -rf` like a sharp knife: useful, but you keep your fingers clear.

### 5.11 cat — show a file's contents

`cat` prints the whole content of a file to the screen at once. (The name comes from "con**cat**enate", because it can also join files, but you'll mostly use it to view.)

```bash
cat notes.txt
```

```text
Remember to test the camera on /dev/video0
Board IP is 192.168.1.173
```

`cat` is best for **short** files, because it dumps everything at once. For long files, use `less` (next).

### 5.12 less — read a long file page by page

`less` opens a file in a scrollable viewer, so long files don't fly past you.

```bash
less /var/log/syslog
```

Once inside `less`:

| Key | Action |
|---|---|
| Arrow keys / Page Up / Page Down | Scroll |
| Space | Forward one page |
| `/word` then Enter | Search for "word" |
| `n` | Next search match |
| `q` | **Quit** and return to the shell |

The key thing to remember: press **`q`** to get out of `less`. (Fun fact: the name is a joke — the older tool was `more`, and "less is more".)

### 5.13 head and tail — the top and bottom of a file

`head` shows the first lines of a file (default 10):

```bash
head notes.txt
```

`tail` shows the last lines (default 10):

```bash
tail notes.txt
```

Control how many lines with `-n`:

```bash
head -n 3 notes.txt
tail -n 5 notes.txt
```

A very handy variant is `tail -f` (**f**ollow). It shows the end of a file and keeps printing new lines as they are added — perfect for watching a log while a program runs:

```bash
tail -f /var/log/syslog
```

Press **Ctrl + C** to stop following.

### 5.14 find — search for files by name

`find` searches through a folder and all its subfolders for files matching a rule. The basic shape is `find WHERE -name PATTERN`.

```bash
find ~ -name "notes.txt"
```

```text
/home/hemanth/notes.txt
/home/hemanth/Documents/notes.txt
```

The `*` wildcard matches "anything". Find all C++ source files under the project:

```bash
find ~/DMSC- -name "*.cpp"
```

Find only directories named `build`:

```bash
find ~/DMSC- -type d -name "build"
```

`-type f` finds files, `-type d` finds directories. `find` is powerful; start with `-name` and grow from there.

### 5.15 grep — search *inside* files for text

Where `find` searches for file **names**, `grep` searches for **text inside** files. It is one of the most useful tools you'll ever learn.

```bash
grep "video0" notes.txt
```

```text
Remember to test the camera on /dev/video0
```

`grep` printed the line that contained `video0`. Useful flags:

| Flag | Meaning |
|---|---|
| `-i` | Ignore case (match `Video0`, `VIDEO0`, etc.) |
| `-r` | Recursive — search all files in a folder tree |
| `-n` | Show the line number of each match |
| `-w` | Match whole words only |

Search a whole project for where the camera device is used:

```bash
grep -rn "video0" ~/DMSC-
```

```text
/home/hemanth/DMSC-/src/camera.cpp:42:    cap.open("/dev/video0");
```

That tells you the file, the line number (`42`), and the matching line. Enormously useful for understanding a codebase.

### 5.16 wc — count lines, words, characters

`wc` = **w**ord **c**ount. By itself it prints three numbers: lines, words, bytes.

```bash
wc notes.txt
```

```text
  2  13  71 notes.txt
```

That's 2 lines, 13 words, 71 bytes. Most often you'll want just the line count with `-l`:

```bash
wc -l notes.txt
```

```text
2 notes.txt
```

### 5.17 echo — print text

`echo` simply prints whatever you give it. It seems trivial, but it's used constantly, especially with variables (Chapter 12).

```bash
echo "Hello, Hemanth"
```

```text
Hello, Hemanth
```

Print the value of a variable (the `$` means "the value of"):

```bash
echo $HOME
```

```text
/home/hemanth
```

### 5.18 man — the manual

`man` = **man**ual. Almost every command has a built-in manual page. To read the manual for `ls`:

```bash
man ls
```

This opens in the `less` viewer, so scroll with arrows and press **`q`** to quit. Manual pages are dense but authoritative. When you forget what a flag does, `man` is the official answer.

### 5.19 --help — the quick reference

For a shorter, faster reference than `man`, add `--help` to most commands:

```bash
cp --help
```

This prints a compact summary of usage and flags right in your terminal (no `q` needed). Use `--help` for a quick reminder, `man` for the full story.

### 5.20 history — what did I type before?

The shell remembers the commands you've run. `history` lists them, numbered:

```bash
history
```

```text
  1  pwd
  2  ls -la
  3  cd DMSC-
  4  grep -rn "video0" .
```

You can re-run a numbered command with `!` — for example `!3` re-runs command number 3. And remember, the **Up arrow** walks back through this same history one command at a time. `Ctrl + R` searches it (Chapter 12/13).

### 5.21 clear — clean the screen

When the terminal gets cluttered, clear it:

```bash
clear
```

The screen wipes clean with a fresh prompt at the top. (The keyboard shortcut **Ctrl + L** does the same thing instantly.) Your history and files are untouched — it just tidies the view.

### 5.22 tree — see the folder structure

`tree` shows a directory and its contents as a nice indented tree, which is great for understanding a project's layout.

```bash
tree ~/DMSC-
```

```text
/home/hemanth/DMSC-
├── CMakeLists.txt
├── deploy/
│   └── dms_app
├── src/
│   ├── main.cpp
│   └── camera.cpp
└── README.md
```

`tree` is not always installed by default. If you get "command not found", install it (you'll learn exactly how in Chapter 8):

```bash
sudo apt install tree
```

Limit how deep it goes with `-L`:

```bash
tree -L 1 ~/DMSC-
```

That shows only the top level.

---

## Chapter 6 — Viewing and Editing Files (nano and vim)

Sooner or later you must **edit** a text file — a config file, a note, a bit of source code — right from the terminal, with no mouse. Linux gives you text editors that run inside the terminal itself. The two most famous are **nano** (beginner-friendly) and **vim** (powerful but tricky). Learn nano well; learn just enough vim to escape it.

### 6.1 nano — the friendly editor

`nano` is the editor to start with. It shows helpful hints at the bottom of the screen the whole time.

Open (or create) a file:

```bash
nano notes.txt
```

You are now inside nano. You can just start typing — text goes in where the cursor is, exactly like you'd expect. Move around with the **arrow keys**.

At the bottom you'll see hints like `^O Write Out` and `^X Exit`. The `^` symbol means the **Ctrl** key. So `^O` means **Ctrl + O**.

#### Essential nano keys

| Keys | Shown as | What it does |
|---|---|---|
| Ctrl + O | `^O` | **Write Out** = save. After pressing it, nano asks for the filename — just press **Enter** to keep the same name |
| Ctrl + X | `^X` | **Exit** nano. If you have unsaved changes, it asks whether to save |
| Ctrl + K | `^K` | **Cut** the current line |
| Ctrl + U | `^U` | **Uncut** = paste the line you cut |
| Ctrl + W | `^W` | **Where is** = search for text |
| Ctrl + \ | `^\` | Search and replace |
| Ctrl + G | `^G` | **Get help** — full list of commands |
| Ctrl + C | `^C` | Show the current cursor position (line/column) |
| Alt + U | `M-U` | Undo |

The typical workflow: open with `nano file`, type your changes, press **Ctrl + O** then **Enter** to save, then **Ctrl + X** to quit. That's it. nano is genuinely easy — the on-screen hints mean you never have to memorize much.

### 6.2 A worked nano session

Let's edit a file end to end.

```bash
nano hello.txt
```

Now type:

```text
Hello, this is my first edited file.
The DMS board is at 192.168.1.173.
```

Then:

1. Press **Ctrl + O** (save). nano shows `File Name to Write: hello.txt` at the bottom.
2. Press **Enter** to confirm. nano says something like `[ Wrote 2 lines ]`.
3. Press **Ctrl + X** to exit.

Back at the shell, check your work:

```bash
cat hello.txt
```

```text
Hello, this is my first edited file.
The DMS board is at 192.168.1.173.
```

You just created and edited a file entirely from the terminal.

### 6.3 vim — powerful, but first learn to quit it

`vim` (and its older form `vi`) is another terminal editor. It is extremely powerful and popular among experienced developers, but it is famously confusing for beginners because it has **modes**, and when you first open it, typing does *not* insert text the way you expect.

The single most important vim survival skill is: **how to quit it.** Many beginners get stuck inside vim with no idea how to leave. Here is your escape hatch.

#### How to quit vim (memorize this)

1. Press the **Esc** key. This makes sure you are in "Normal mode" (the command mode).
2. Type a colon `:` — a small `:` appears at the bottom-left.
3. Then type one of these and press **Enter**:

| You type | Meaning |
|---|---|
| `:q` then Enter | **Quit** (works only if you made no changes) |
| `:q!` then Enter | **Quit and throw away changes** — the reliable escape when stuck |
| `:wq` then Enter | **Write (save) and quit** |

So the universal "get me out of here" sequence is: **Esc**, then type `:q!`, then **Enter**. That discards changes and exits no matter what state you're in.

#### The tiniest bit of vim usage

If you *do* want to edit in vim:

```bash
vim hello.txt
```

- vim opens in **Normal mode**, where letters are commands, not text.
- Press **`i`** to enter **Insert mode** — now you can type text normally. You'll see `-- INSERT --` at the bottom.
- Press **Esc** to leave Insert mode and go back to Normal mode.
- From Normal mode, save and quit with `:wq` then Enter.

That's enough vim to survive. For real editing as a beginner, prefer **nano**. Learn vim later, on purpose, when you have time. For now, the goal is simply: *if vim opens by accident, you know how to get out* (**Esc**, `:q!`, **Enter**).

---

## Chapter 7 — Permissions, Ownership, and sudo

Linux is a **multi-user** system: many people (and programs) can share one machine safely. To keep everyone's files private and the system secure, every file has an **owner** and a set of **permissions** that say who can do what. Understanding this is essential — especially for the camera and serial devices in this project.

### 7.1 Users, groups, and others

Every file is associated with:

- A **user** who owns it (the **owner**).
- A **group** — a named collection of users. Groups let you give the same access to several people (or to devices, like the `video` group for cameras).
- **Others** — everyone else on the system.

So permissions are described for three categories: **owner (u)**, **group (g)**, and **others (o)**.

### 7.2 What rwx means

For each of those three categories, there are three possible permissions:

| Letter | Name | On a **file** it means | On a **directory** it means |
|---|---|---|---|
| `r` | read | You can read the file's contents | You can list what's inside |
| `w` | write | You can change/delete the file | You can add/remove files inside |
| `x` | execute | You can run the file as a program | You can enter (cd into) the directory |

If a permission is not granted, a dash `-` appears instead of the letter.

### 7.3 Reading the permission string

Recall the long listing from `ls -l`:

```bash
ls -l hello.txt
```

```text
-rw-r--r-- 1 hemanth hemanth 71 Aug 11 09:30 hello.txt
```

Focus on the first block: `-rw-r--r--`. Read it in chunks:

| Position | Value | Meaning |
|---|---|---|
| 1st character | `-` | File type: `-` = regular file, `d` = directory, `c`/`b` = device, `l` = link |
| Characters 2–4 | `rw-` | **Owner** permissions: read + write, no execute |
| Characters 5–7 | `r--` | **Group** permissions: read only |
| Characters 8–10 | `r--` | **Others** permissions: read only |

So `-rw-r--r--` means: it's a regular file; the owner (`hemanth`) can read and write it; the group can only read it; everyone else can only read it. The two `hemanth` words after the permissions are the **owner** and the **group**.

For a directory it starts with `d`:

```text
drwxr-xr-x 2 hemanth hemanth 4096 Aug 11 09:00 project
```

`drwxr-xr-x`: it's a directory; owner can read/write/enter; group can read/enter; others can read/enter.

### 7.4 chmod — change permissions

`chmod` = **ch**ange **mod**e. It sets who can do what. There are two ways to use it: **symbolic** and **numeric**.

#### Symbolic mode (letters)

You specify **who** (`u`ser, `g`roup, `o`thers, `a`ll), an action (`+` add, `-` remove, `=` set exactly), and **which** permission (`r`, `w`, `x`).

```bash
chmod u+x run.sh      # give the owner permission to execute run.sh
chmod g-w notes.txt   # remove write permission from the group
chmod o-r secret.txt  # others can no longer read it
chmod a+r public.txt  # everyone can read it
```

A very common one: make a script runnable by you:

```bash
chmod u+x deploy.sh
```

#### Numeric mode (numbers)

Each permission has a number: **read = 4**, **write = 2**, **execute = 1**. Add them up for each category:

| Sum | Permissions | Meaning |
|---|---|---|
| 7 | `rwx` | read + write + execute (4+2+1) |
| 6 | `rw-` | read + write (4+2) |
| 5 | `r-x` | read + execute (4+1) |
| 4 | `r--` | read only |
| 0 | `---` | nothing |

You give three digits: owner, group, others.

```bash
chmod 644 notes.txt   # owner rw- (6), group r-- (4), others r-- (4)
chmod 755 run.sh      # owner rwx (7), group r-x (5), others r-x (5)
chmod 700 private/    # owner rwx, group nothing, others nothing
```

`644` is the typical setting for a normal file (owner can edit, everyone can read). `755` is typical for a program or folder (owner can change, everyone can run/enter). These two show up everywhere.

### 7.5 chown — change ownership

`chown` = **ch**ange **own**er. It changes which user (and optionally group) owns a file. Because you're taking a file away from one user and giving it to another, this almost always needs `sudo` (next section).

```bash
sudo chown hemanth notes.txt              # make hemanth the owner
sudo chown hemanth:hemanth notes.txt      # set owner AND group to hemanth
sudo chown -R hemanth ~/DMSC-             # recursively, whole folder tree
```

The `-R` flag applies the change to everything inside a directory.

### 7.6 What is root / sudo, and why it's both needed and dangerous

There is one special, all-powerful user on every Linux system called **root** (also called the **superuser** or **administrator**). Root can do *anything*: read any file, change any setting, delete anything, install system software. Its home is `/root`, and in a shell its prompt ends with `#` instead of `$`.

For safety, you normally log in as a **normal user** (`hemanth`), who is *not* allowed to touch system-wide things. This protects the system from mistakes and from malicious programs. But sometimes you genuinely need root power — for example, to install software or change a system setting.

That's what **`sudo`** is for. `sudo` means roughly "**s**uper**u**ser **do**" — run *this one command* as root. You put `sudo` in front of a command:

```bash
sudo apt install tree
```

The first time in a session, `sudo` asks for **your** password (not a separate root password). As you type, **nothing appears on screen** — no dots, no stars. That's normal and intentional; just type your password and press Enter.

> **WARNING — respect sudo.**
>
> A command with `sudo` runs with unlimited power. A mistake as a normal user usually harms only your own files. A mistake **with `sudo`** can break the entire operating system. Golden rules:
> - Only use `sudo` when a command truly needs it (installing software, editing system files, accessing certain hardware).
> - **Read the command carefully** before pressing Enter with `sudo`.
> - Never blindly paste `sudo` commands from the internet that you don't understand.
> - `sudo rm -rf` on the wrong path can destroy your system. Combine the earlier `rm` warning with this one and be very careful.

Use `sudo` like a key to a dangerous room: necessary sometimes, but you don't leave the door open or wave the key around.

### 7.7 The "video" group — letting your user use the camera

Remember `/dev/video0` from Chapter 4? It's owned by the `video` group:

```bash
ls -l /dev/video0
```

```text
crw-rw----+ 1 root video 81, 0 Aug 11 09:12 /dev/video0
```

Read this with your new permission knowledge: the owner is `root`, the group is `video`, and the permissions are `crw-rw----`. That means the **owner (root)** can read/write it, the **group (video)** can read/write it, and **others get nothing**. So a normal user like `hemanth` can only use the camera if `hemanth` is a **member of the `video` group**.

To add yourself to the `video` group (needed so the DMS app can open the camera without running as root):

```bash
sudo usermod -aG video hemanth
```

Breaking that down: `usermod` modifies a user account; `-a` means **a**dd (append, don't replace existing groups — very important, forgetting `-a` can remove you from other groups!); `-G video` names the group; `hemanth` is your username.

Check which groups you belong to:

```bash
groups
```

```text
hemanth adm cdrom sudo dip plugdev video dialout
```

> **Important:** Group changes only take effect after you **log out and log back in** (or reboot). Running `groups` right after `usermod` may not show the new group yet — that's expected.

Notice `dialout` in that list too. Just as `video` grants camera access, the **`dialout` group** grants access to **serial ports** like `/dev/ttyUSB2`. That's why you might also run:

```bash
sudo usermod -aG dialout hemanth
```

so you can use `minicom` and serial devices without `sudo`. These two group memberships — `video` and `dialout` — are exactly what this project needs for the camera and the board's serial console.

---

## Chapter 8 — Package Management with APT

On Windows you install software by downloading `.exe` files from various websites. On Ubuntu there's a better, safer way: a **package manager**. Ubuntu's is called **APT**.

### 8.1 What is a package and a package manager?

A **package** is a bundle containing a piece of software plus information about it (its name, version, and what other software it needs to work — those needs are called **dependencies**). Instead of hunting websites, you ask APT for a package by name and it fetches and installs it for you, along with all its dependencies, from trusted collections called **repositories**.

**APT** (Advanced Package Tool) is the program you use. Think of it as an **app store you drive from the terminal**: one command to install, one to remove, one to update. Because everything comes from Ubuntu's trusted repositories, it's far safer than downloading random installers.

Because installing system software affects the whole computer, APT commands need `sudo`.

### 8.2 The core APT commands

| Command | What it does |
|---|---|
| `sudo apt update` | Refresh the **list** of available packages and versions (does *not* install anything) |
| `sudo apt upgrade` | Actually **install newer versions** of packages you already have |
| `sudo apt install NAME` | Install a package (and its dependencies) |
| `sudo apt remove NAME` | Uninstall a package |
| `sudo apt search TEXT` | Search for packages matching text (no `sudo` needed) |
| `apt show NAME` | Show details about a package |

#### apt update — refresh the catalog

```bash
sudo apt update
```

```text
Hit:1 http://archive.ubuntu.com/ubuntu noble InRelease
Get:2 http://security.ubuntu.com/ubuntu noble-security InRelease
Reading package lists... Done
12 packages can be upgraded. Run 'apt list --upgradable' to see them.
```

Think of `update` as "check the store's latest catalog." It only refreshes information; it does not change your installed software. **Always run `apt update` before installing**, so APT knows about the newest versions.

#### apt upgrade — install the newer versions

```bash
sudo apt upgrade
```

This downloads and installs newer versions of everything already on your system. It may ask "Do you want to continue? [Y/n]" — type `y` and Enter. Running `update` then `upgrade` regularly keeps your system healthy and secure.

#### apt install — add software

```bash
sudo apt install tree
```

```text
Reading package lists... Done
The following NEW packages will be installed:
  tree
Need to get 47.9 kB of archives.
Do you want to continue? [Y/n] y
Setting up tree ...
```

You can install several packages at once by listing them:

```bash
sudo apt install git curl htop
```

#### apt remove — uninstall software

```bash
sudo apt remove tree
```

To also delete its configuration files, there's `sudo apt purge tree`. To clean up dependencies no longer needed by anything, there's `sudo apt autoremove`.

#### apt search — find a package

```bash
apt search opencv
```

This lists packages whose name or description mentions "opencv". Use it when you know *what* you want but not the exact package name.

### 8.3 What are "-dev" packages?

This is important for building software like the DMS project. Many libraries come in **two** package forms:

- The **runtime** package (e.g. `libsqlite3-0`) — just enough to *run* programs that already use the library.
- The **development** package, whose name ends in **`-dev`** (e.g. `libsqlite3-dev`) — this adds the extra files a programmer needs to **build/compile** new software against that library: the **header files** (files ending in `.h` or `.hpp` that describe the library's functions) and things the compiler links against.

Rule of thumb: **if you are writing or compiling C/C++ code that uses a library, you need that library's `-dev` package.** If you are only running finished software, you don't. Because the DMS project was compiled from C++ source, it needed the `-dev` versions of every library it used.

### 8.4 The exact libraries this project installed

Here are the packages this DMS project needed, with what each one is for. This is a real, working set of build dependencies for a C++ + OpenCV + SQLite application.

| Package | What it provides |
|---|---|
| `build-essential` | The core toolkit for compiling C/C++: the **compiler** (`g++`), `make`, and standard libraries. The foundation for building any C++ program |
| `cmake` | A build-system generator that reads a `CMakeLists.txt` file and produces the instructions to compile the project. Widely used for C++ |
| `libopencv-dev` | **OpenCV** development files — the computer-vision library used to open the camera, read frames, and process the driver's face |
| `libsqlite3-dev` | **SQLite** development files — a small, file-based database used to store data/logs locally |
| `libjpeg-dev` | Development files for reading/writing **JPEG** images |
| `libpng-dev` | Development files for reading/writing **PNG** images |
| `liblapack-dev` | **LAPACK**: a library of advanced linear-algebra routines (used by math-heavy vision code) |
| `libblas-dev` | **BLAS**: basic linear-algebra building blocks that LAPACK and OpenCV rely on |

The single command that installs them all at once:

```bash
sudo apt update
sudo apt install build-essential cmake libopencv-dev libsqlite3-dev \
  libjpeg-dev libpng-dev liblapack-dev libblas-dev
```

(The `\` at the end of the first line is a line-continuation: it tells the shell "this command continues on the next line." You can also just type it all on one long line.)

After this, your Ubuntu laptop has everything needed to compile the DMS C++ application. That is the practical payoff of understanding APT.

---

## Chapter 9 — Processes and the System

Every running program is called a **process**. When you launch the DMS app, or a browser, or even `less`, the operating system creates a process for it and gives it a number called a **PID** (Process ID). This chapter is about seeing what's running and controlling it, plus checking your system's health.

### 9.1 ps — list processes

`ps` = **p**rocess **s**tatus. By itself it shows just your current terminal's processes. The common way to see everything is `ps aux`:

```bash
ps aux
```

```text
USER       PID %CPU %MEM    VSZ   RSS TTY   STAT START   TIME COMMAND
hemanth   1523  0.0  0.1  22000  5400 pts/0 Ss   09:00   0:00 bash
hemanth   2891 12.3  4.5 998000 360000 pts/0 Rl  09:40   0:15 ./dms_app
```

Each line is one process. The useful columns: **PID** (the process's ID number), **%CPU** and **%MEM** (how much processor and memory it's using), and **COMMAND** (what it is). Here you can see the DMS app (`./dms_app`) running with PID `2891`, using 12.3% CPU.

To find a specific process, combine `ps` with `grep` (using a **pipe**, covered in Chapter 12):

```bash
ps aux | grep dms
```

### 9.2 top and htop — live activity monitor

`ps` is a snapshot. `top` is a **live, updating** view of processes, refreshing every couple of seconds — like Task Manager on Windows.

```bash
top
```

```text
top - 09:45:01 up  1:20,  1 user,  load average: 0.42, 0.31, 0.20
Tasks: 210 total,   1 running, 209 sleeping
%Cpu(s):  8.3 us,  2.1 sy, 89.4 id
MiB Mem :   7842.0 total,   3120.5 free,   2011.2 used
  PID USER      %CPU %MEM     TIME+ COMMAND
 2891 hemanth   12.3  4.5   0:22.10 dms_app
 1102 hemanth    3.1  2.0   0:45.30 gnome-shell
```

The list re-sorts by CPU use so the busiest program floats to the top. Press **`q`** to quit `top`.

`htop` is a friendlier, colorful version of `top` with easier scrolling and a mouse. It's not installed by default:

```bash
sudo apt install htop
htop
```

In `htop` you can scroll with arrows and press **F10** or **`q`** to quit. Many people prefer it.

### 9.3 kill — stop a process

Sometimes a program hangs or you need to stop one that's running in the background. `kill` sends a stop signal to a process **by its PID**:

```bash
kill 2891
```

That politely asks process 2891 to shut down. If it refuses to die (truly stuck), force it:

```bash
kill -9 2891
```

`-9` sends the "cannot be ignored" signal. Use it only as a last resort — it gives the program no chance to clean up. If you know the program's *name* rather than its PID, `killall` works by name: `killall dms_app`.

### 9.4 Foreground, background, and job control

When you run a command normally, it runs in the **foreground**: it takes over the terminal until it finishes, and you can't type anything else. Sometimes you want a program to run in the **background** so you get your prompt back.

#### & — start in the background

Add `&` to the end of a command to launch it in the background:

```bash
./dms_app &
```

```text
[1] 2891
```

`[1]` is the **job number** (a simple counter for this terminal) and `2891` is the PID. You get your prompt back immediately while `dms_app` keeps running.

#### Ctrl-C — stop the foreground program

If a program is running in the foreground and you want to **stop/cancel** it, press **Ctrl + C**. This is your everyday "make it stop" — for example, to stop `tail -f` or a program stuck in a loop.

#### Ctrl-Z — pause the foreground program

Press **Ctrl + Z** to **pause** (suspend) the foreground program and get your prompt back. The program is frozen, not stopped:

```text
^Z
[1]+  Stopped                 ./dms_app
```

#### jobs, fg, bg — manage suspended/background jobs

- `jobs` — list the jobs running or suspended in this terminal.
- `fg` — bring a job back to the **f**ore**g**round (resume it and give it the terminal).
- `bg` — resume a suspended job in the **b**ack**g**round.

A typical sequence:

```bash
./dms_app          # runs in foreground
# press Ctrl+Z to pause it
jobs               # see it listed as "Stopped"
bg                 # resume it, but in the background
jobs               # now it shows as "Running"
fg                 # pull it back to the foreground
```

```text
[1]+  Stopped                 ./dms_app
[1]+ ./dms_app &
[1]+  Running                 ./dms_app &
./dms_app
```

Job control feels abstract at first, but it's genuinely useful: pause a long task, do something else, then resume it.

### 9.5 Checking system health

These commands answer "how is my computer doing?"

#### df -h — disk space

`df` = **d**isk **f**ree. The `-h` flag makes sizes human-readable (GB, MB).

```bash
df -h
```

```text
Filesystem      Size  Used Avail Use% Mounted on
/dev/sda2       234G   98G  124G  45% /
/dev/sda1       511M  6.1M  505M   2% /boot/efi
```

Look at the `/` line: this system has a 234 GB disk, 98 GB used, 124 GB free (45% full). Run this when you fear you're running out of space.

#### free -h — memory (RAM)

```bash
free -h
```

```text
               total        used        free      shared  buff/cache   available
Mem:           7.7Gi       2.0Gi       3.0Gi       210Mi       2.7Gi       5.3Gi
Swap:          2.0Gi          0B       2.0Gi
```

`Mem` is your RAM. Focus on the **available** column — that's how much memory programs can still use. `Swap` is emergency overflow space on disk used when RAM fills up.

#### uname -a — what system is this?

`uname` prints information about the operating system and kernel. `-a` means "all":

```bash
uname -a
```

```text
Linux laptop 6.8.0-40-generic #40-Ubuntu SMP x86_64 GNU/Linux
```

That tells you it's Linux, the hostname (`laptop`), the kernel version (`6.8.0-40-generic`), and the architecture (`x86_64`, meaning a 64-bit Intel/AMD PC). On the i.MX 93 board it would show something like `aarch64` instead — a different, ARM-based processor. That difference (x86_64 vs aarch64) is exactly why the project needed a special toolchain to build for the board.

#### lscpu — details about the processor

```bash
lscpu
```

```text
Architecture:            x86_64
CPU(s):                  8
Model name:              Intel(R) Core(TM) i5-1135G7
CPU max MHz:             4200.0
```

`lscpu` tells you how many CPU cores you have and what kind. Useful when building software (more cores = faster compiling) or just understanding your machine.

---

## Chapter 10 — Networking Basics for This Project

To deploy the DMS software you had to get files from your laptop *onto* the i.MX 93 board across a network, and log into the board remotely. This chapter covers the small, practical set of networking commands that made that possible.

### 10.1 What is an IP address?

Every device on a network has an **IP address** — a numeric label like `192.168.1.173` that identifies it, similar to a house address. To send files to the board or connect to it, you need to know its IP address. Addresses starting with `192.168.` are **private** addresses used inside home/office networks.

### 10.2 Finding your own IP address

Two easy ways.

`hostname -I` prints your machine's IP address(es), simply:

```bash
hostname -I
```

```text
192.168.1.42
```

(That capital `-I` matters.) For more detail, `ip addr` shows every network interface and its address:

```bash
ip addr
```

```text
1: lo: <LOOPBACK,UP> ...
    inet 127.0.0.1/8 scope host lo
2: wlp2s0: <BROADCAST,MULTICAST,UP> ...
    inet 192.168.1.42/24 scope global wlp2s0
```

Look for the line under your real interface (here `wlp2s0`, a Wi-Fi adapter) that says `inet` — that's your IP: `192.168.1.42`. The `lo` interface with `127.0.0.1` is the "loopback" — it always means "this same computer" and isn't a real network address.

### 10.3 ping — is the other device reachable?

`ping` checks whether another device is reachable on the network by sending it small test messages and timing the replies. It's the first thing to try when a connection isn't working.

```bash
ping 192.168.1.173
```

```text
PING 192.168.1.173 (192.168.1.173) 56(84) bytes of data.
64 bytes from 192.168.1.173: icmp_seq=1 ttl=64 time=0.45 ms
64 bytes from 192.168.1.173: icmp_seq=2 ttl=64 time=0.39 ms
```

Replies like these mean the board is reachable — good. If instead you see `Destination Host Unreachable` or it just hangs with no replies, the board is not reachable (wrong IP, not on the network, or not powered). Press **Ctrl + C** to stop pinging.

### 10.4 ssh — log into another computer remotely

**SSH** stands for **S**ecure **SH**ell. It lets you open a shell (a terminal session) on *another* computer over the network, securely. Instead of sitting at the board with its own keyboard, you control it from your laptop's terminal. Everything is encrypted, so it's safe even over untrusted networks.

The basic shape is `ssh USER@ADDRESS`:

```bash
ssh root@192.168.1.173
```

The first time you connect to a new machine, SSH asks you to confirm its identity:

```text
The authenticity of host '192.168.1.173' can't be established.
ED25519 key fingerprint is SHA256:abc123...
Are you sure you want to continue connecting (yes/no)? yes
```

Type `yes` and Enter. Then it asks for the password of that user *on the board* (here, root's password). After logging in, your prompt changes to show you're now on the board — commands you type run **on the board**, not your laptop. Type `exit` to return to your laptop.

#### Password vs. key authentication

There are two ways SSH can verify it's really you:

| Method | How it works | Trade-off |
|---|---|---|
| **Password** | You type the remote user's password each time | Simple; but you type it every login, and passwords can be weak |
| **Key** | You generate a pair of files: a **private key** (secret, stays on your laptop) and a **public key** (copied to the board). They mathematically match | More secure and no password typing after setup; a little setup needed |

For quick work, password login is fine. For repeated or automated access, key-based login is preferred. To create a key pair you'd run `ssh-keygen`, and to install your public key on the board you'd run `ssh-copy-id root@192.168.1.173`. After that, `ssh` logs in without asking for a password.

### 10.5 scp — copy files over the network

**SCP** = **S**ecure **C**o**P**y. It copies files between computers over the same secure SSH connection. This is exactly how the built DMS software got from your laptop onto the board.

The shape is `scp SOURCE USER@ADDRESS:DESTINATION`. The **real command this project used** to copy the whole `deploy` folder to the board's root home directory:

```bash
scp -r ~/DMSC-/deploy root@192.168.1.173:
```

Let's dissect it carefully, because it packs several ideas together:

| Piece | Meaning |
|---|---|
| `scp` | The secure-copy program |
| `-r` | **R**ecursive — copy the folder *and everything inside it* (needed for a directory) |
| `~/DMSC-/deploy` | The **source**: the local `deploy` folder inside your project (`~` is your home) |
| `root@192.168.1.173` | The **destination machine and user**: log in as `root` on the board at that IP |
| `:` | The colon separates the machine from the path. **The path after `:` is empty**, which means "the root user's home directory on the board" (i.e. `/root`) |

So this command means: "Securely copy my whole `deploy` folder onto the board, logging in as root, and drop it in root's home folder." It will ask for the board's root password (unless you set up keys), then show progress as each file transfers.

To copy the *other* direction (from board to laptop), you swap source and destination:

```bash
scp root@192.168.1.173:/root/results.log ~/Downloads/
```

That pulls `results.log` off the board into your laptop's Downloads.

### 10.6 Enabling SSH on your laptop (installing an SSH server)

There's an important distinction:

- The **SSH client** (the `ssh` command) lets you *connect out* to other machines. It's usually already installed.
- The **SSH server** (a background service, `sshd`) lets other machines *connect in* to yours. This is **not** installed by default on a desktop Ubuntu.

If you want to be able to `ssh` **into** your laptop (or you're setting up a machine to receive connections), install and enable the server:

```bash
sudo apt install openssh-server
```

Then turn the service on and make it start automatically every boot:

```bash
sudo systemctl enable --now ssh
```

Let's unpack that last one. **`systemctl`** is the tool for controlling background services (called "services" or "daemons") on Ubuntu. Here:

- `enable` = make the service start automatically at every boot.
- `--now` = *also* start it right now, without waiting for a reboot.
- `ssh` = the name of the service.

So `enable --now` means "start it now **and** every time the computer boots." Related handy commands:

```bash
sudo systemctl status ssh    # is it running? shows active/inactive
sudo systemctl stop ssh      # stop it now
sudo systemctl start ssh     # start it now
sudo systemctl disable ssh   # don't start it at boot anymore
```

The board itself typically already runs an SSH server, which is why you could `ssh root@192.168.1.173` *into* it. Installing the server on your laptop is only needed if you want connections coming *the other way*.

---

## Chapter 11 — Serial Consoles and minicom

Networking (SSH/SCP) is great once the board is on the network. But when a board first boots — before networking is even configured, or when something has gone wrong — you often can't reach it over the network at all. For those situations there's an older, simpler, rock-solid method: the **serial console**. This is exactly what you used to reach the i.MX 93 board with `minicom`.

### 11.1 What is a serial console?

A **serial console** is a direct text link between your laptop and the board over a simple cable, where data travels one bit at a time (that's what "serial" means — in a series, one after another). It doesn't need a network, an IP address, or a screen on the board. The board sends its text output (boot messages, a login prompt, a shell) down the wire, and your keystrokes go back up the wire to the board.

Think of it as a **direct private phone line** between the two machines. Even if everything else about the board is broken or unconfigured, this low-level line usually still works. That reliability is why embedded engineers rely on it constantly — it's the way to watch a board boot and to rescue it when networking fails.

Physically, you connect a **USB-to-serial cable/adapter** from your laptop's USB port to the board's serial (often labeled "debug" or "console" or "UART") header.

### 11.2 What /dev/ttyUSB2 is (again, in context)

When you plug that USB-to-serial adapter into your laptop, Linux recognizes it and creates a device file for it in `/dev` (remember Chapter 4 — hardware appears as files). Serial adapters over USB are named `ttyUSB` followed by a number: `/dev/ttyUSB0`, `/dev/ttyUSB1`, `/dev/ttyUSB2`, and so on. For your setup the board's console appeared as **`/dev/ttyUSB2`**.

- `tty` is a historical abbreviation for "teletypewriter" — it just means a terminal/serial line.
- `USB` means it came from a USB adapter.
- `2` is which one — if you had several serial adapters, they'd be `0`, `1`, `2`.

How do you know which number is yours? A neat trick: run this, then plug the cable in, and see which new `ttyUSB` name appears:

```bash
ls /dev/ttyUSB*
```

```text
/dev/ttyUSB0  /dev/ttyUSB1  /dev/ttyUSB2
```

Or watch the system messages as you plug it in with `dmesg` (kernel log): `sudo dmesg | tail`.

Remember from Chapter 7: to use a serial port without `sudo`, your user must be in the **`dialout`** group.

### 11.3 What is minicom?

**minicom** is a terminal program specifically for talking over serial lines. You point it at the serial device (like `/dev/ttyUSB2`), it opens that connection, and then whatever you type is sent to the board and whatever the board sends back is shown on your screen. It's the classic tool for this job.

Install it if needed:

```bash
sudo apt install minicom
```

### 11.4 Baud rate — 115200

Serial links need both sides to agree on the **speed**, measured in **baud** (roughly, bits per second). If the two ends don't use the same speed, you'll see garbage characters instead of readable text. The i.MX 93 board's console runs at **115200 baud**, a very common speed for embedded consoles. You must tell minicom to use 115200 as well. Other common (older/slower) rates are 9600, 19200, and 38400 — but for this board it's **115200**.

The other settings are almost always "8N1" (8 data bits, No parity, 1 stop bit) with no hardware flow control — these are the standard defaults and minicom uses them unless told otherwise.

### 11.5 Running minicom

Because a serial port needs elevated access (unless you set up the `dialout` group), you typically launch it with `sudo`. A direct way that specifies the device and speed on the command line:

```bash
sudo minicom -D /dev/ttyUSB2 -b 115200
```

Here `-D` names the **d**evice and `-b` sets the **b**aud rate. Alternatively, you can just run `sudo minicom` and configure the device/speed from its menus (see below), or set up a saved profile.

Once connected, if the board is powered on, you'll see its output — boot messages scrolling by, or a login prompt. You can log in and use the board's shell right there, over the cable, no network required.

### 11.6 minicom keyboard commands — the essentials

minicom is controlled with **Ctrl-A** followed by another key. That is: hold **Ctrl** and press **A**, *release both*, then press the next key. The `Ctrl-A` is the "attention" combo that says "the next key is a minicom command, not text for the board."

| Keys | What it does |
|---|---|
| **Ctrl-A** then **Z** | Open the main **help/command menu** — shows all commands. Start here if lost |
| **Ctrl-A** then **X** | **Exit** minicom (it asks you to confirm) |
| **Ctrl-A** then **Q** | Quit **without** reset |
| **Ctrl-A** then **O** | **Options** — configure settings like the serial port and baud rate |
| **Ctrl-A** then **C** | Clear the screen |
| **Ctrl-A** then **W** | Toggle line wrapping |
| **Ctrl-A** then **E** | Toggle **local echo** (see what you type) — handy if your keystrokes seem invisible |

The two you must memorize:

- **Ctrl-A Z** — brings up the help menu, your map to everything else.
- **Ctrl-A X** — exits minicom cleanly when you're done.

To set the baud rate from inside minicom: press **Ctrl-A** then **O** (Options), choose "Serial port setup", and there you can set the device (`/dev/ttyUSB2`) and press the key to cycle the speed to `115200`.

### 11.7 A typical minicom session, start to finish

```bash
# 1. Plug in the USB-serial cable, confirm the device name
ls /dev/ttyUSB*
# -> /dev/ttyUSB2

# 2. Launch minicom pointed at it, at 115200 baud
sudo minicom -D /dev/ttyUSB2 -b 115200

# 3. Power on (or reset) the board. Watch boot messages scroll.
#    When you see a login prompt, log in with the board's credentials.

# 4. Do your work on the board's shell over serial...

# 5. When finished, press Ctrl-A then X, and confirm, to exit minicom.
```

That is the complete loop you used to reach and work on the i.MX 93 board over the console. With SSH (Chapter 10) for network access and minicom for the serial console, you have both ways in.

---

## Chapter 12 — The Bash Shell Essentials

You've been using Bash all along. This chapter reveals the features that turn the shell from "a place to type commands" into a genuinely powerful tool. These are the ideas that make experienced users fast.

### 12.1 Variables vs. commands — the key distinction

A **command** is an *action* — a program you run (`ls`, `pwd`, `cp`). A **variable** is *stored data* — a named container holding a value (some text or a number). They are different things:

- When you type `ls` and press Enter, Bash **runs the program** `ls`.
- A variable just **holds a value** until you ask for it. It doesn't "do" anything by itself.

You set a variable with `NAME=value` (note: **no spaces** around the `=`):

```bash
myname=Hemanth
```

You **read** a variable's value by putting a `$` in front of its name. This is called *expansion* — Bash replaces `$myname` with its stored value before running the line:

```bash
echo $myname
```

```text
Hemanth
```

Without the `$`, Bash would think `myname` is a command to run and complain `command not found`. The `$` is what says "give me the value stored here," not "run this."

### 12.2 Environment variables — settings the whole system sees

Some variables are special **environment variables**: they're shared with the programs you launch, so they act like system-wide settings. By convention their names are UPPERCASE. Important ones:

| Variable | Holds |
|---|---|
| `$HOME` | Path to your home directory (`/home/hemanth`) |
| `$USER` | Your username (`hemanth`) |
| `$PATH` | The list of folders Bash searches to find commands |
| `$PWD` | Your current working directory |
| `$SHELL` | Which shell you're using (`/bin/bash`) |

Look at your `PATH` — this one is important to understand:

```bash
echo $PATH
```

```text
/usr/local/bin:/usr/bin:/bin:/usr/local/games:/usr/games
```

`PATH` is a list of directories separated by colons `:`. When you type a command like `ls`, Bash looks through each of these folders **in order** until it finds a program named `ls`. That's why you can type `ls` instead of `/usr/bin/ls` — Bash found it on the `PATH`. If a command "isn't found", often it means its folder isn't on your `PATH`.

#### export — make a variable available to programs

A plain variable (`myname=Hemanth`) exists only in your current shell. To pass it down to programs you launch from the shell, you **export** it, which promotes it to an environment variable:

```bash
export MY_SETTING=hello
```

Or export an existing variable:

```bash
greeting=hi
export greeting
```

You can also add a folder to your `PATH` this way (appending to the existing value):

```bash
export PATH=$PATH:/home/hemanth/mytools
```

That says "set PATH to its current value, plus `:` plus my new folder." Now programs in `/home/hemanth/mytools` can be run by name.

### 12.3 source (and its shorthand `.`) — run a script *in your current shell*

Normally, when you run a script, it runs in its **own** separate shell, and any variables it sets vanish when it finishes — they don't affect your current shell. Sometimes that's not what you want: you want the script to set up variables *in your current session*.

`source` runs a script's lines *inside your current shell*, so any variables or settings it defines **stay** afterward. The command `.` (just a dot) is an exact shorthand for `source`:

```bash
source setup.sh
# identical to:
. setup.sh
```

**This is exactly how the Yocto SDK environment gets loaded** for cross-compiling to the board. The SDK ships a setup script that sets a batch of environment variables (which compiler to use, where the board's libraries are, etc.). You must `source` it so those variables land in *your* shell and stay there for the build:

```bash
source /opt/poky/4.0/environment-setup-cortexa55-poky-linux
```

(The exact filename depends on the SDK version and board.) After sourcing it, your shell "knows" how to build software for the i.MX 93's ARM processor instead of for your laptop. If you *ran* it instead of *sourcing* it, the variables would disappear immediately and the build wouldn't work — which is why `source` (or `.`) is essential here. Notice it lives under `/opt`, the "optional add-on software" folder from Chapter 4.

### 12.4 Wildcards (globs) — matching many names at once

**Wildcards** (also called **globs**) are special characters that let one pattern match many filenames. Bash expands them *before* running the command.

| Wildcard | Matches |
|---|---|
| `*` | Any number of any characters (including none) |
| `?` | Exactly one of any character |
| `[abc]` | Any one character from the set a, b, or c |

Examples:

```bash
ls *.cpp          # every file ending in .cpp
ls image?.png     # image1.png, image2.png, but NOT image10.png (? is one char)
ls data*          # every name starting with "data"
rm *.tmp          # delete all files ending in .tmp  (careful!)
```

For instance, `*.cpp` might expand to `main.cpp camera.cpp` and then `ls` lists exactly those. Wildcards are enormously handy — but combined with `rm` they're dangerous, so **always** run the pattern with `ls` first to see what it matches before you delete.

### 12.5 Pipes — connect commands together

A **pipe**, written `|`, takes the **output** of one command and feeds it as the **input** of the next. This lets you build powerful chains from simple tools — a core Linux idea: small tools that snap together.

```bash
ls -l | grep ".cpp"
```

That runs `ls -l`, then pipes its listing into `grep`, which keeps only lines containing `.cpp`. Result: a long listing of just your C++ files.

More examples:

```bash
ps aux | grep dms          # of all processes, show only ones mentioning "dms"
cat notes.txt | wc -l      # count the lines in notes.txt
ls /usr/bin | wc -l        # how many programs are in /usr/bin?
history | grep ssh         # find past commands that used ssh
```

You can chain several pipes: `command1 | command2 | command3`. Read them left to right, like a factory assembly line where each station does one job.

### 12.6 Redirection — sending output to files

By default a command's output goes to your screen. **Redirection** sends it somewhere else, usually a file.

| Operator | Effect |
|---|---|
| `>` | Send output to a file, **overwriting** the file's contents |
| `>>` | Send output to a file, **appending** (adding to the end) |
| `2>` | Redirect **errors** specifically (channel 2) |
| `2>&1` | Send errors to the **same place** as normal output |
| `<` | Take input **from** a file |

Examples:

```bash
echo "first line" > log.txt     # creates/overwrites log.txt with "first line"
echo "second line" >> log.txt   # adds a line without erasing the first
ls /nonexistent 2> errors.txt   # capture the error message into errors.txt
```

A word about `2>&1`. Linux programs actually produce **two** output streams: normal output (called **stdout**, channel 1) and error messages (called **stderr**, channel 2). Sometimes you want *both* saved together, for example when saving a full build log:

```bash
make > build.log 2>&1
```

Read it as: send normal output to `build.log` (`> build.log`), and then send channel 2 (errors) to wherever channel 1 is now going (`2>&1`) — i.e. into the same file. Now `build.log` contains everything, both normal messages and errors, in order. This is a common pattern when compiling and you want a complete record.

### 12.7 Command chaining — running several commands in a row

You can put multiple commands on one line with these separators:

| Separator | Meaning |
|---|---|
| `;` | Run the next command **regardless** of whether the previous one succeeded |
| `&&` | Run the next command **only if** the previous one **succeeded** |
| `\|\|` | Run the next command **only if** the previous one **failed** |

Examples:

```bash
cd ~/DMSC- ; ls              # go there, then list — always both
mkdir build && cd build      # only cd if mkdir worked
ping -c1 192.168.1.173 || echo "Board not reachable"   # message only on failure
```

`&&` is especially valuable for builds, where each step should only happen if the previous succeeded:

```bash
mkdir -p build && cd build && cmake .. && make
```

This means: make the build folder, and *if that worked* enter it, and *if that worked* run cmake, and *if that worked* compile. If any step fails, the chain stops right there — so you don't try to compile with a broken setup. This is a very common real-world build line.

### 12.8 Tab-completion (again — it matters)

Press **Tab** to auto-complete commands and file/folder names. It's not just convenience; it prevents typos and confirms things exist. If you type `cd Dow` then Tab, Bash completes it to `cd Downloads/`. If nothing happens, either there's no match or several match — press Tab twice to see all candidates. Use Tab constantly; experienced users press it dozens of times a minute.

### 12.9 Up-arrow history (again — it matters)

Press the **Up arrow** to recall previous commands, one per press; **Down arrow** goes forward. This saves retyping long commands like that `scp -r ...` line. Combined with editing the recalled line, it's a huge time-saver. And **Ctrl + R** searches history: press it, start typing part of an old command, and Bash finds the most recent match — press Enter to run it, or the arrow keys to edit it.

---

## Chapter 13 — Keyboard Shortcuts Cheat Sheet

Shortcuts are what make you *fast*. Here they are grouped by where you use them. Skim now; return often. In these tables, `Ctrl` means the Control key, `Alt` the Alt key, and `Super` the Windows-logo key.

### 13.1 Terminal (Bash) shortcuts

These work at the shell prompt while typing commands.

| Shortcut | What it does |
|---|---|
| **Ctrl + C** | Cancel/stop the currently running command |
| **Ctrl + D** | Signal "end of input"; on an empty prompt, logs out / closes the shell |
| **Ctrl + Z** | Pause (suspend) the current foreground program |
| **Ctrl + L** | Clear the screen (same as the `clear` command) |
| **Ctrl + A** | Jump the cursor to the **start** of the line |
| **Ctrl + E** | Jump the cursor to the **end** of the line |
| **Ctrl + U** | Delete everything from the cursor back to the start of the line |
| **Ctrl + K** | Delete everything from the cursor to the end of the line |
| **Ctrl + W** | Delete the word before the cursor |
| **Ctrl + R** | Search backward through command history (type to find) |
| **Tab** | Auto-complete a command or filename |
| **Tab Tab** | Show all possible completions |
| **Up / Down arrow** | Previous / next command in history |
| **Left / Right arrow** | Move the cursor along the current line |
| **Ctrl + Left / Right** | Move the cursor one word at a time |

Note: **Ctrl + C** in the terminal means "cancel", **not** "copy". To copy/paste in the terminal, use **Ctrl + Shift + C** (copy) and **Ctrl + Shift + V** (paste) — the extra **Shift** distinguishes them from the special Ctrl combos above.

### 13.2 GNOME desktop shortcuts

These control the Ubuntu desktop itself.

| Shortcut | What it does |
|---|---|
| **Ctrl + Alt + T** | Open a new **Terminal** (memorize this one!) |
| **Super** (Windows key) | Open the Activities overview / search |
| **Alt + Tab** | Switch between open applications |
| **Alt + `** (backtick) | Switch between windows of the *same* application |
| **Super + D** | Show the desktop (minimize everything) |
| **Super + A** | Show all applications |
| **Super + Left / Right** | Snap the current window to the left / right half |
| **Super + Up** | Maximize the current window |
| **Super + PageUp / PageDown** | Switch to the workspace above / below |
| **Ctrl + Alt + Left / Right** | Switch workspaces (another way) |
| **Ctrl + Q** or **Alt + F4** | Close the current window |
| **PrtSc** (Print Screen) | Take a screenshot |

A **workspace** is like an extra virtual desktop — a clean separate screen you can switch to, so you can keep, say, your editor on one workspace and the terminal on another.

### 13.3 nano shortcuts

Inside the nano editor (`^` means Ctrl).

| Shortcut | What it does |
|---|---|
| **Ctrl + O** | Save the file (then press Enter to confirm the name) |
| **Ctrl + X** | Exit nano |
| **Ctrl + K** | Cut the current line |
| **Ctrl + U** | Paste (uncut) the line |
| **Ctrl + W** | Search for text |
| **Ctrl + \\** | Search and replace |
| **Ctrl + G** | Open the help screen |
| **Ctrl + C** | Show current line/column position |
| **Alt + U** | Undo |
| **Ctrl + A / Ctrl + E** | Jump to start / end of the line |

### 13.4 vim escape hatch (the one thing to remember)

| Sequence | What it does |
|---|---|
| **Esc** then `:q!` then **Enter** | Quit vim, discarding changes (your escape hatch) |
| **Esc** then `:wq` then **Enter** | Save and quit vim |
| **i** | Enter Insert mode (start typing text) |
| **Esc** | Leave Insert mode (back to command mode) |

### 13.5 minicom shortcuts

Serial console control — press **Ctrl-A**, release, then the next key.

| Sequence | What it does |
|---|---|
| **Ctrl-A** then **Z** | Show the help / command menu |
| **Ctrl-A** then **X** | Exit minicom (confirm) |
| **Ctrl-A** then **Q** | Quit without reset |
| **Ctrl-A** then **O** | Options / configure (serial port, baud rate) |
| **Ctrl-A** then **C** | Clear the screen |
| **Ctrl-A** then **E** | Toggle local echo (see what you type) |
| **Ctrl-A** then **W** | Toggle line wrap |

---

## Chapter 14 — Applications You'll Use

Beyond the terminal, a few graphical (windowed) applications will be part of your daily work on the DMS project. Here's a quick orientation to each.

### 14.1 Terminal

Your primary tool, covered throughout this guide. Open it with **Ctrl + Alt + T**. Everything — building, deploying, connecting to the board — happens here. You can open several terminal windows or tabs (in the Terminal, **Ctrl + Shift + T** opens a new tab) to run multiple things at once.

### 14.2 Files (Nautilus)

**Files** (its internal name is **Nautilus**) is Ubuntu's graphical file browser — the equivalent of File Explorer on Windows. It lets you click through folders, drag files, and open them with a double-click. It shows the same file system you navigate with `cd` and `ls`; it's just a visual view of it. Handy for quick browsing, previewing images, or plugging in a USB drive and copying files by dragging. By default it hides those dot-files; press **Ctrl + H** to show/hide hidden files (the graphical version of `ls -a`).

### 14.3 Text Editor / VS Code

For quick edits with a mouse, Ubuntu includes a simple graphical **Text Editor** (gedit or GNOME Text Editor). For real coding, most developers install **Visual Studio Code** ("VS Code"), a powerful, free code editor from Microsoft that runs beautifully on Ubuntu. It has syntax highlighting (color-coded code), file browsing, search across a project, and a built-in terminal panel. For editing the C++ DMS source, VS Code is the comfortable choice. You'd install it from Ubuntu's App Center or download it from Microsoft's site. In the terminal, `code .` opens the current folder in VS Code.

### 14.4 A web browser (Firefox)

Ubuntu ships with **Firefox**. You'll use a browser constantly for documentation, looking up error messages, and reading library references (like OpenCV docs). Nothing special here — it works like any browser you've used.

### 14.5 minicom

Covered in Chapter 11. Not a windowed app — it runs *inside* a terminal — but it's a distinct tool you'll open specifically to talk to the board over the serial console.

### 14.6 System Monitor (gnome-system-monitor)

**System Monitor** is the graphical equivalent of `top`/`htop` — Ubuntu's version of Windows Task Manager. It shows running processes, CPU and memory graphs, and disk usage in a friendly window, and lets you end a stuck process by clicking it and choosing "End Process." Open it by searching "System Monitor" in Activities, or from the terminal:

```bash
gnome-system-monitor
```

Use whichever you prefer: the terminal tools (`top`, `free -h`, `df -h`) when you're already in a terminal, or System Monitor when you want a visual overview.

---

## Chapter 15 — Command Cheat Sheet, Common Mistakes, and Practice

You've covered a lot. This final chapter is your quick-reference and your test. Keep the cheat sheet handy, learn from the common mistakes, and do the exercises to lock it all in.

### 15.1 One-page command cheat sheet

| Command | Example | What it does |
|---|---|---|
| `pwd` | `pwd` | Show current directory (where am I?) |
| `ls` | `ls -lah` | List files (long, all, human sizes) |
| `cd` | `cd ~/DMSC-` | Change directory; `cd ..` up, `cd` home |
| `mkdir` | `mkdir -p a/b/c` | Make directory (and parents with `-p`) |
| `rmdir` | `rmdir empty` | Remove an **empty** directory |
| `touch` | `touch new.txt` | Create an empty file / update its time |
| `cp` | `cp -r src dst` | Copy files (`-r` for folders) |
| `mv` | `mv old.txt new.txt` | Move or rename |
| `rm` | `rm file` / `rm -r dir` | Delete (careful! no undo) |
| `cat` | `cat notes.txt` | Print a whole file |
| `less` | `less big.log` | Scroll a long file (`q` to quit) |
| `head` / `tail` | `tail -f log.txt` | First / last lines (`-f` follows) |
| `find` | `find . -name "*.cpp"` | Find files by name |
| `grep` | `grep -rn "video0" .` | Search text inside files |
| `wc` | `wc -l notes.txt` | Count lines/words/bytes |
| `echo` | `echo $PATH` | Print text or a variable |
| `man` / `--help` | `man ls` / `ls --help` | Read documentation |
| `history` | `history` | Show past commands |
| `clear` | `clear` | Clear the screen (or Ctrl+L) |
| `tree` | `tree -L 1` | Show folder structure |
| `nano` | `nano file.txt` | Edit a file (friendly) |
| `chmod` | `chmod 755 run.sh` | Change permissions |
| `chown` | `sudo chown me file` | Change ownership |
| `sudo` | `sudo apt update` | Run one command as root |
| `apt` | `sudo apt install cmake` | Install/manage software |
| `ps` | `ps aux` | List processes |
| `top` / `htop` | `top` | Live process monitor (`q` to quit) |
| `kill` | `kill 2891` | Stop a process by PID |
| `df` / `free` | `df -h` / `free -h` | Disk / memory usage |
| `uname` / `lscpu` | `uname -a` | System / CPU info |
| `ip addr` / `hostname -I` | `hostname -I` | Find your IP address |
| `ping` | `ping 192.168.1.173` | Test if a host is reachable |
| `ssh` | `ssh root@192.168.1.173` | Log into another machine |
| `scp` | `scp -r ~/DMSC-/deploy root@192.168.1.173:` | Copy files over the network |
| `systemctl` | `sudo systemctl enable --now ssh` | Control background services |
| `minicom` | `sudo minicom -D /dev/ttyUSB2 -b 115200` | Serial console to the board |
| `source` / `.` | `source env-setup` | Load a script into the current shell |
| `export` | `export VAR=value` | Create an environment variable |

### 15.2 Common beginner mistakes and how to avoid them

| Mistake | What happens | How to avoid it |
|---|---|---|
| Forgetting Linux is case-sensitive | `cd downloads` fails when the folder is `Downloads` | Match capitalization exactly; use **Tab** to auto-complete and avoid typos |
| Running `rm -rf` carelessly | Permanent, unrecoverable deletion | Run `pwd` and `ls` first; avoid `-f`; never trust a stray space in the path |
| Putting spaces around `=` in a variable | `x = 5` fails; must be `x=5` | No spaces: `name=value` |
| Forgetting `$` when reading a variable | `echo PATH` prints the word, not the value | Use `$`: `echo $PATH` |
| Using `sudo` for everything | Small mistakes become system-breaking; also messes up file ownership in your home folder | Only use `sudo` when a command truly needs it |
| Getting stuck in vim | Can't type or can't leave | **Esc**, then `:q!`, then **Enter** |
| Getting stuck in `less` or `man` | Screen won't go back to prompt | Press **`q`** to quit |
| Expecting a Recycle Bin | `rm` deletes permanently; there's no trash | Double-check before deleting; keep backups of important work |
| Wrong baud rate in minicom | Garbage characters on screen | Set **115200** to match the board |
| Not in the `video`/`dialout` group | "Permission denied" opening camera or serial port | Add yourself with `usermod -aG`, then log out and back in |
| Forgetting `-a` in `usermod -aG` | Can accidentally remove you from other groups | Always include `-a` (append) |
| Confusing `>` and `>>` | `>` overwrites and destroys the old file contents | Use `>>` to append; use `>` only when you mean to replace |
| Running an env script instead of sourcing it | SDK variables vanish; build fails | Use `source script` or `. script`, not `./script` |
| Typing the prompt (`$`) into commands | "command not found: $" | Type only what comes *after* the `$` |

### 15.3 Practice exercises

Work through these on a real Ubuntu machine. They cover everything in this guide. Don't peek at the chapters unless you're stuck — struggling a little is how it sticks.

**Set 1 — Navigation and files**
1. Open a terminal. Print your current directory. What is it?
2. List all files in your home directory, including hidden ones, in long human-readable form.
3. Create a folder called `linux_practice` and move into it.
4. Inside it, create three empty files: `a.txt`, `b.txt`, `c.txt`, using a single `touch` command.
5. Create a nested folder structure `project/src/tests` with one command.
6. Copy `a.txt` to `a_backup.txt`. Then rename `b.txt` to `notes.txt`.
7. Use `tree` to display your `linux_practice` folder's structure.

**Set 2 — Viewing and editing**
8. Use `nano` to open `notes.txt`, type three lines about this project, save, and exit.
9. Print the file with `cat`. Then show just its first line with `head`.
10. Count how many lines `notes.txt` has.
11. Use `grep` to find which of your `.txt` files contain the word "board".

**Set 3 — Permissions and packages**
12. Show the permissions of `notes.txt` with `ls -l`. Read them out loud.
13. Make `notes.txt` readable and writable by you only (`chmod 600`). Verify with `ls -l`.
14. Check whether `tree` and `htop` are installed; install whichever is missing.
15. List which groups your user belongs to. Is `video` there? Is `dialout`?

**Set 4 — Processes and system**
16. Show your system's disk usage and free memory in human-readable form.
17. Print your kernel and architecture with one command.
18. Start `top`, watch it for a moment, then quit it properly.
19. Run `sleep 60 &` to start a background job, list your jobs, then bring it to the foreground and cancel it with Ctrl-C.

**Set 5 — Networking and the board**
20. Find your laptop's IP address two different ways.
21. Ping the board at `192.168.1.173` five times (hint: `ping -c 5 ...`), then stop.
22. Write out (don't necessarily run) the exact `scp` command to copy your `~/DMSC-/deploy` folder to the board's root home.
23. Write out the command to open a serial console to the board on `/dev/ttyUSB2` at 115200 baud, and state the two key sequences to (a) open minicom's menu and (b) exit minicom.

**Set 6 — Shell power**
24. Print your `$PATH` and explain in one sentence what it's for.
25. Use a pipe to count how many programs live in `/usr/bin`.
26. Redirect the output of `ls -l` into a file called `listing.txt`, then append the current date (`date`) to the same file.
27. Write a single chained command that makes a `build` folder and only enters it if creation succeeded.

**Cleanup**
28. Delete the entire `linux_practice` folder safely (think about which command, and check where you are first!).

If you can do all 28 without help, you have moved from zero to genuinely confident with Linux and Ubuntu — and you're ready to work on the DMS project. Well done, Hemanth.

---

*End of guide. Keep this document nearby, and remember: the fastest way to learn is to keep a terminal open and try things. You cannot easily break anything as a normal user without `sudo` — so explore freely and have fun.*
