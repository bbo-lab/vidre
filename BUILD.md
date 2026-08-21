# Build instructions

## Windows

### Prerequisites

Check building on Windows
  see https://github.com/conan-io/conan, 
  https://github.com/cpp-pm/hunter?tab=readme-ov-file, 
  https://github.com/microsoft/vcpkg?tab=readme-ov-file

Required packages:
- git
- cmake
- LLVM/VS or LLVM/MinGW
- FFmpeg
- Qt

Further packages used in the build are downloaded and build automatically 
with CMake.

#### GIT

Install git:

    winget install Git.Git

Developers also need to setup SSH. Since Windows 10, OpenSSH is natively included.
To use it with git, run

    git config --global core.sshCommand "C:/Windows/System32/OpenSSH/ssh.exe"

To setup OpenSSH you may follow these steps (note that the author uses different public keys
for different services):

**OpenSSH Agent**

To use the ssh-agent on Windows, execute

    Get-Service ssh-agent | Set-Service -StartupType Automatic -PassThru | Start-Service 

with admin priviledges.

**Generate a public key**

    mkdir ~/.ssh
    cd ~/.ssh
    ssh-keygen -t ed25519 -C '<local-user>@<hostname>' -f .ssh/<github-user>.github.com

**Upload key to your github account**

**Create a SSH config**:

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

#### Compiler

    winget install Ninja-build.Ninja Kitware.CMake

--------------------------------------------------------------------------------

FIXME
FIXME
FIXME
FIXME
FIXME

see https://dev.to/marcosplusplus/how-to-install-the-new-version-of-clangllvm-on-windows-2flb

https://llvm.org/docs/GettingStartedVS.html

etc.


    winget install LLVM.LLVM 
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

FFmpeg FIXME

### Clone repository

The repository can be downloaded with

    git clone https://github.com/bbo-lab/vidre.git

### Building



