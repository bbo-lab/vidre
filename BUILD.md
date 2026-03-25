# Build instructions

## Windows

Required packages:
- git
- clang
- cmake
- VS Code
  (Note: you need to also install "CMake Tools", "C/C++", "Lua", and, "Python" extensions 
  from within VS Code)

These can be installed using winget (PowerShell):

    winget install Git.Git 
    winget LLVM.LLVM MartinStorsjo.LLVM-MinGW.UCRT Ninja-build.Ninja Kitware.CMake
    winget install Microsoft.VisualStudioCode

Additionally the WindowsSDK is required, which at the moment of writing is

    winget install Microsoft.WindowsSDK.10.0.26100

(check with "winget search WindowsSDK").

Unclear if needed:

    Microsoft.WindowsSDK.10.0.26100
    Microsoft.VCRedist.2015+.x64
    Microsoft.VCRedist.2015+.x86

    Microsoft.VisualStudio.BuildTools 
    MartinStorsjo.LLVM-MinGW.MSVCRT

For Qt, choose your poison:

    https://doc.qt.io/qt-6/get-and-install-qt.html

**Note**: The author does not understand why git can fail with 

    fatal: detected dubious ownership in repository

but, especially when compiling QT is required, one might consider trusting everything:

    git config --global --add safe.directory "*"

YMMV.

FIXME BtbN.FFmpeg.GPL.Shared.6.1

### Clone repository

The repository can be downloaded with

    git clone https://github.com/bbo-lab/vidre.git

Developers requiring access through ssh should note, that OpenSSH is included natively with 
Windows 10 and Windows 11. To use it with git, run

    git config --global core.sshCommand "C:/Windows/System32/OpenSSH/ssh.exe"

To setup OpenSSH you may follow these steps (note that the author uses different public keys
for different services):MartinStorsjo.LLVM-MinGW.

1. Generate a public key

    mkdir ~/.ssh
    cd ~/.ssh
    ssh-keygen -t ed25519 -C '<local-user>@<hostname>' -f .ssh/<github-user>.github.com

2. Upload key to your github account
3. Create a SSH config:

    mkdir known_hosts.d
    edit config

with the following content:

    AddKeysToAgent yes
    UserKnownHostsFile ~/.ssh/known_hosts.d/%k
    IdentitiesOnly yes

    Host <github-user>.github.com
            HostName github.com
            User <github-user>
            IdentityFile ~/.ssh/<github-user>.github.com

With this, the repository can now be cloned using

    git clone git@<github-user>.github.com:bbo-lab/vidre.git

### Building



