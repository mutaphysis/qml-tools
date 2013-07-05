# Building qml tools

## Prerequisites

 * boost-progam-options
   * eg. `apt-get install boost-progam-options-dev` or `brew install boost`
 * private qt headers
 * before running instrumented code, the QtCov qml plugin needs to be installed

## Building

    qmake -r qml-tools.pro
    make
    make install
    
 
