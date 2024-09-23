# Developer info

At the moment of writing, sources are split into 

    https://github.com/bbo-lab/vidre

containing fundamental framework libraries as well as the recording 
daemon itself, and,

    https://github.com/bbo-lab/vidre-gui

the user interface development repository.

## Errors and exception

VidRe uses both, error codes as well as exceptions. As a rule of thumb, error
codes should be used when fast, mostly hardware related, data exchange occurs,
while exceptions are better suited (despite nowadays not being slow!) in 
rather complex situations that involve clean-up (e.g. through destruction of 
objects).

TOOD boost exceptions with additional info

## Documentation


### Public documentation

Developers updating bbo-lab.github.io need to clone 

    https://github.com/jothepro/doxygen-awesome-css

in the ''docs/'' directory. At the moment of writing, the 
doxygen generated output is deposited in

    bbo-lab.github.io/future/vidre

