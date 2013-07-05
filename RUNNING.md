# Running qml-tools

    qml-tools
    
    Instruments qml files for collecting coverage data
    
    Allowed options:
      --help                shows this help message
      -i [ --input ] arg    input file or folder
      -o [ --output ] arg   output file or folder


    # Takes Test.qml and outputs instrumented file to Rewritten.qml
    qml-tools -i Test.qml -o Rewritten.qml
    
    # Reads all qml and js files from some/folder and replaces them with instrumented version
    qml-tools -i some/folder/
